/******************************************************************************
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/
/* FreeRTOS includes */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"

#include <stdbool.h>

#include "cl_log.h"
#include "cl_msg.h"
#include "cl_main.h"
#include "cl_flash.h"
#include "cl_rmgmt.h"
#include "xgq_cmd_vmr.h"
#include "cl_xgq_server_plat.h"
#include "vmr_common.h"

#define MSG_ERR(fmt, arg...) \
	CL_ERR(APP_XGQ, fmt, ##arg)
#define MSG_WARN(fmt, arg...) \
	CL_ERR(APP_XGQ, fmt, ##arg)
#define MSG_LOG(fmt, arg...) \
	CL_LOG(APP_XGQ, fmt, ##arg)
#define MSG_DBG(fmt, arg...) \
	CL_DBG(APP_XGQ, fmt, ##arg)

#define XGQ_XQUEUE_LENGTH	8
#define XGQ_SEND_QUEUE_WAIT_MS	10

static void receiveTask( void *pvParameters );
static TaskHandle_t receiveTaskHandle = NULL;

static void quickTask( void *pvParameters );
static void slowTask( void *pvParameters );
static TaskHandle_t quickTaskHandle = NULL;
static TaskHandle_t slowTaskHandle = NULL;
static QueueHandle_t quickTaskQueue = NULL;
static QueueHandle_t slowTaskQueue = NULL;

static struct xgq rpu_xgq;
static int xgq_io_hdl = 0;

static bool service_is_started = false;

enum task_level {
	TASK_SLOW = 0,
	TASK_QUICK,
};

static msg_handle_t handles[] = {
	{ .type = CL_MSG_PDI, .name = "PDI", },
	{ .type = CL_MSG_XCLBIN, .name = "XCLBIN", },
	{ .type = CL_MSG_LOG_PAGE, .name = "LOG_PAGE", },
	{ .type = CL_MSG_CLOCK, .name = "CLOCK", },
	{ .type = CL_MSG_APUBIN, .name = "API_BIN", },
	{ .type = CL_MSG_VMR_CONTROL, .name = "VMR_CONTROL", },
#ifndef VMR_BUILD_XRT_ONLY
	{ .type = CL_MSG_SENSOR, .name = "SENSOR", },
#endif
};

int cl_msg_handle_init(msg_handle_t **hdl, cl_msg_type_t type,
	process_msg_cb cb, void *arg)
{
	if (cb == NULL) {
		MSG_LOG("process_msg_cb cannot be NULL");
		return -1;
	}

	for (int i = 0; i < ARRAY_SIZE(handles); i++) {
		if (handles[i].type == type) {
			if (handles[i].msg_cb != NULL) {
				MSG_ERR("FATAL: dup init with same type %d", type);
				return -1;
			}
			handles[i].msg_cb = cb;
			handles[i].arg = arg;
			*hdl = &handles[i];
			MSG_LOG("init handle %s type %d", handles[i].name, type);
			return 0;
		}
	}

	MSG_LOG("FATAL: unsupported type %d", type);
	return -1;
}

void cl_msg_handle_fini(msg_handle_t *hdl)
{
	hdl->msg_cb = NULL;
	hdl->arg = NULL;
	MSG_LOG("done");
}

int cl_msg_handle_complete(cl_msg_t *msg)
{
	struct xgq_com_queue_entry cq_cmd = {
		.hdr.cid = msg->hdr.cid,
		.hdr.state = XGQ_CMD_STATE_COMPLETED,
		.rcode = msg->hdr.rcode,
	};
	struct xgq_cmd_cq *cmd_cq = (struct xgq_cmd_cq *)&cq_cmd;
	u64 cq_slot_addr;

	/*TODO:
	 * cleanup this code, should not mix request and result within same payload.
	 * separate request and results
	 */
	if (msg->hdr.type == CL_MSG_CLOCK) {
		/* only 1 place to hold ocl_freq, alway get from index 0 */
		cmd_cq->cq_clock_payload.ocl_freq = msg->clock_payload.ocl_req_freq[0];
	} else if (msg->hdr.type == CL_MSG_VMR_CONTROL) {
		/* explicitly copy back fpt status from cl_msg to xgq completion cmd */
		cmd_cq->cq_vmr_payload.has_fpt =
			msg->multiboot_payload.has_fpt;
		cmd_cq->cq_vmr_payload.has_fpt_recovery =
			msg->multiboot_payload.has_fpt_recovery;
		cmd_cq->cq_vmr_payload.boot_on_default =
			msg->multiboot_payload.boot_on_default;
		cmd_cq->cq_vmr_payload.boot_on_backup =
			msg->multiboot_payload.boot_on_backup;
		cmd_cq->cq_vmr_payload.boot_on_recovery =
			msg->multiboot_payload.boot_on_recovery;
		cmd_cq->cq_vmr_payload.current_multi_boot_offset =
			msg->multiboot_payload.current_multi_boot_offset;
		cmd_cq->cq_vmr_payload.boot_on_offset =
			msg->multiboot_payload.boot_on_offset;

		cmd_cq->cq_vmr_payload.has_extfpt =
			msg->multiboot_payload.has_extfpt;
		cmd_cq->cq_vmr_payload.has_ext_scfw =
			msg->multiboot_payload.has_ext_scfw;
		cmd_cq->cq_vmr_payload.has_ext_xsabin =
			msg->multiboot_payload.has_ext_xsabin;
		cmd_cq->cq_vmr_payload.has_ext_sysdtb =
			msg->multiboot_payload.has_ext_sysdtb;

		/* pass back apu status */
		cmd_cq->cq_vmr_payload.ps_is_ready = cl_xgq_apu_is_ready();
		cmd_cq->cq_vmr_payload.pl_is_ready = cl_xgq_pl_is_ready();

		/* pass back log level and flash progress */
		cmd_cq->cq_vmr_payload.debug_level = cl_loglevel_get();
		cmd_cq->cq_vmr_payload.program_progress = flash_progress();
	} else if (msg->hdr.type == CL_MSG_LOG_PAGE) {
		cmd_cq->cq_log_payload.count = msg->log_payload.size;
	}

	xgq_produce(&rpu_xgq, &cq_slot_addr);
	cl_memcpy_toio32(cq_slot_addr, &cq_cmd, sizeof(struct xgq_com_queue_entry));
	xgq_notify_peer_produced(&rpu_xgq);

	return 0;
}

static int dispatch_to_queue(cl_msg_t *msg, int task_level)
{
	/* send will do deep copy of msg, so that we preserved data */
	switch (task_level) {
	case TASK_SLOW:
		if (xQueueSend(slowTaskQueue, msg, pdMS_TO_TICKS(XGQ_SEND_QUEUE_WAIT_MS)) != pdPASS) {
			MSG_ERR("failed to send msg to slowTask cid: %d", msg->hdr.cid);
			return -EIO;
		}
		break;
	case TASK_QUICK:
		if (xQueueSend(quickTaskQueue, msg, pdMS_TO_TICKS(XGQ_SEND_QUEUE_WAIT_MS)) != pdPASS) {
			MSG_ERR("failed to send msg to quickTask cid: %d", msg->hdr.cid);
			return -EIO;
		}
		break;
	default:
		MSG_ERR("FATAL: unhandled task_level %d", task_level);
		return -EINVAL;
	}

	return 0;
}

/* mapping from xgq_cmd to vmr internal common layer msg type */
struct xgq_cmd_cl_map {
	u32	xgq_request_type;
	u32	common_request_type;
};

static struct xgq_cmd_cl_map xgq_cmd_clock_map[] = {
	{XGQ_CMD_CLOCK_WIZARD, CL_CLOCK_WIZARD},
	{XGQ_CMD_CLOCK_COUNTER, CL_CLOCK_COUNTER},
	{XGQ_CMD_CLOCK_SCALE, CL_CLOCK_SCALE},
};

static struct xgq_cmd_cl_map xgq_cmd_vmr_control_map[] = {
	{XGQ_CMD_BOOT_DEFAULT, CL_MULTIBOOT_DEFAULT},
	{XGQ_CMD_BOOT_BACKUP, CL_MULTIBOOT_BACKUP},
	{XGQ_CMD_PROGRAM_SC, CL_PROGRAM_SC},
	{XGQ_CMD_VMR_DEBUG, CL_VMR_DEBUG},
	{XGQ_CMD_VMR_QUERY, CL_VMR_QUERY},
};

static struct xgq_cmd_cl_map xgq_cmd_vmr_debug_map[] = {
	{XGQ_CMD_DBG_RMGMT, CL_DBG_DISABLE_RMGMT},
	{XGQ_CMD_DBG_VMC, CL_DBG_DISABLE_VMC},
	{XGQ_CMD_DBG_CLEAR, CL_DBG_CLEAR},
};

static struct xgq_cmd_cl_map xgq_cmd_log_page_map[] = {
	{XGQ_CMD_LOG_AF_CHECK, CL_LOG_AF_CHECK},
	{XGQ_CMD_LOG_AF_CLEAR, CL_LOG_AF_CLEAR},
	{XGQ_CMD_LOG_FW, CL_LOG_FW},
	{XGQ_CMD_LOG_INFO, CL_LOG_INFO},
	{XGQ_CMD_LOG_ENDPOINT, CL_LOG_ENDPOINT},
};

static struct xgq_cmd_cl_map xgq_cmd_sensor_map[] = {
	{XGQ_CMD_SENSOR_SID_GET_SIZE, CL_SENSOR_GET_SIZE},
	{XGQ_CMD_SENSOR_SID_BDINFO, CL_SENSOR_BDINFO},
	{XGQ_CMD_SENSOR_SID_TEMP, CL_SENSOR_TEMP},
	{XGQ_CMD_SENSOR_SID_VOLTAGE, CL_SENSOR_VOLTAGE},
	{XGQ_CMD_SENSOR_SID_CURRENT, CL_SENSOR_CURRENT},
	{XGQ_CMD_SENSOR_SID_POWER, CL_SENSOR_POWER},
	{XGQ_CMD_SENSOR_SID_QSFP, CL_SENSOR_QSFP},
	{XGQ_CMD_SENSOR_SID_ALL, CL_SENSOR_ALL},
};

static inline int convert_cl_type(u32 xgq_type, u32 *cl_type,
	struct xgq_cmd_cl_map *map, int map_size)
{
	for (int i = 0; i < map_size; i++) {
		if (map[i].xgq_request_type == xgq_type) {
			*cl_type = map[i].common_request_type;
			return 0;
		}
	}

	MSG_ERR("unknown type %d", xgq_type);
	return -EINVAL;
}


int xclbin_handle(cl_msg_t *msg, struct xgq_cmd_sq *sq)
{
	msg->data_payload.address = (u32)sq->xclbin_payload.address;
	msg->data_payload.size = (u32)sq->xclbin_payload.size;
	return 0;
}

int pdi_handle(cl_msg_t *msg, struct xgq_cmd_sq *sq)
{
	msg->data_payload.address = (u32)sq->pdi_payload.address;
	msg->data_payload.size = (u32)sq->pdi_payload.size;

	switch (sq->pdi_payload.flash_type) {
	case XGQ_CMD_FLASH_NO_BACKUP:
		msg->data_payload.flash_no_backup = 1;
		break;
	case XGQ_CMD_FLASH_TO_LEGACY:
		msg->data_payload.flash_to_legacy = 1;
		break;
	case XGQ_CMD_FLASH_DEFAULT:
	default:
		break;
	}

	return 0;
}

int vmr_control_handle(cl_msg_t *msg, struct xgq_cmd_sq *sq)
{
	int ret = 0;
	u32 vmr_debug_type = CL_DBG_CLEAR;

	/* we always set log level by this request */
	cl_loglevel_set(sq->vmr_control_payload.debug_level);

	ret = convert_cl_type(sq->vmr_control_payload.req_type, &msg->multiboot_payload.req_type,
		xgq_cmd_vmr_control_map, ARRAY_SIZE(xgq_cmd_vmr_control_map));
	if (ret)
		return ret;

	ret = convert_cl_type(sq->vmr_control_payload.debug_type, &vmr_debug_type,
		xgq_cmd_vmr_debug_map, ARRAY_SIZE(xgq_cmd_vmr_debug_map));
	if (ret)
		return ret;

	msg->multiboot_payload.vmr_debug_type = vmr_debug_type;

	return 0;
}

int clock_handle(cl_msg_t *msg, struct xgq_cmd_sq *sq)
{
	int num_clock = 0;
	int ret = 0;

	ret = convert_cl_type(sq->clock_payload.ocl_req_type, &msg->clock_payload.ocl_req_type,
		xgq_cmd_clock_map, ARRAY_SIZE(xgq_cmd_clock_map));
	if (ret)
		return ret;

	msg->clock_payload.ocl_region = sq->clock_payload.ocl_region;
	msg->clock_payload.ocl_req_id = sq->clock_payload.ocl_req_id;
	msg->clock_payload.ocl_req_num = sq->clock_payload.ocl_req_num;
	num_clock = msg->clock_payload.ocl_req_num;
	MSG_DBG("req_type %d, req_id %d, req_num %d",
		sq->clock_payload.ocl_req_type,
		sq->clock_payload.ocl_req_id,
		sq->clock_payload.ocl_req_num);

	if (num_clock > ARRAY_SIZE(msg->clock_payload.ocl_req_freq)) {
		MSG_ERR("num_clock %d out of range", num_clock);
		return -EINVAL;
	}

	/* deep copy freq requests */
	for (int i = 0; i < num_clock; i++) {
		msg->clock_payload.ocl_req_freq[i] =
			sq->clock_payload.ocl_req_freq[i];
	}

	return 0;
}

int log_page_handle(cl_msg_t *msg, struct xgq_cmd_sq *sq)
{
	int ret = 0;
	u32 pid = CL_LOG_UNKNOWN;

	ret = convert_cl_type(sq->log_payload.pid, &pid,
		xgq_cmd_log_page_map, ARRAY_SIZE(xgq_cmd_log_page_map));
	if (ret)
		return ret;

	msg->log_payload.pid = pid;
	msg->log_payload.address = (u32)sq->log_payload.address;
	msg->log_payload.size = (u32)sq->log_payload.size;

	return 0;
}

int sensor_handle(cl_msg_t *msg, struct xgq_cmd_sq *sq)
{
	int ret = 0;
	u32 sid = CL_SENSOR_ALL;

	ret = convert_cl_type(sq->sensor_payload.sid, &sid,
		xgq_cmd_sensor_map, ARRAY_SIZE(xgq_cmd_sensor_map));
	if (ret)
		return ret;

	msg->sensor_payload.sid = sid;
	msg->sensor_payload.address = (u32)sq->sensor_payload.address;
	msg->sensor_payload.size = (u32)sq->sensor_payload.size;
	msg->sensor_payload.aid = (u8) sq->sensor_payload.aid;

	return 0;
}


struct xgq_cmd_handler {
	uint16_t	opcode;			/* xgq cmd opcode */
	uint16_t	msg_type;		/* common layer internal msg type */
	char		*name;			/* handler name */
	int (*msg_handle)(cl_msg_t *msg, struct xgq_cmd_sq *sq); /* handler callback */
	enum task_level task_level_type;	/* which task to handle the request */
};

static struct xgq_cmd_handler xgq_cmd_handlers[] = {
	{XGQ_CMD_OP_LOAD_XCLBIN, CL_MSG_XCLBIN, "LOAD XCLBIN", xclbin_handle, TASK_SLOW},
	{XGQ_CMD_OP_DOWNLOAD_PDI, CL_MSG_PDI, "LOAD BASE PDI", pdi_handle, TASK_SLOW},
	{XGQ_CMD_OP_LOAD_APUBIN, CL_MSG_APUBIN, "LOAD APU PDI", pdi_handle, TASK_SLOW},
	{XGQ_CMD_OP_GET_LOG_PAGE, CL_MSG_LOG_PAGE, "LOG PAGE", log_page_handle, TASK_QUICK},
	{XGQ_CMD_OP_CLOCK, CL_MSG_CLOCK, "CLOCK", clock_handle, TASK_QUICK},
	{XGQ_CMD_OP_VMR_CONTROL, CL_MSG_VMR_CONTROL, "VMR_CONTROL", vmr_control_handle, TASK_QUICK},
	{XGQ_CMD_OP_SENSOR, CL_MSG_SENSOR, "SENSOR", sensor_handle, TASK_QUICK},
};

/*
 * 1. parse the xgq com to internal cl_msg_t.
 * 2. send cl_msg_t to either quick or slow TASK.
 * 3. wait xgqHandleTask complete the command.
 * Future: xgq cmd complete will be handled by xgq_receive task.
 */
static int submit_to_queue(u32 sq_addr)
{	
	struct xgq_sub_queue_entry *cmd = (struct xgq_sub_queue_entry *)sq_addr;
	struct xgq_cmd_sq *sq = (struct xgq_cmd_sq *)sq_addr;
	int ret = 0;
	cl_msg_t msg = { 0 };

	/* Sync data from cache to memory */
	Xil_DCacheFlush();

	msg.hdr.cid = cmd->hdr.cid;
	MSG_DBG("get cid %d opcode 0x%x", cmd->hdr.cid, cmd->hdr.opcode);

	for (int i = 0; i < ARRAY_SIZE(xgq_cmd_handlers); i++) {
		if (xgq_cmd_handlers[i].opcode == cmd->hdr.opcode) {

			msg.hdr.type = xgq_cmd_handlers[i].msg_type;
			ret = xgq_cmd_handlers[i].msg_handle(&msg, sq);
			if (ret)
				return ret;

			MSG_DBG("dispatch msg %s", xgq_cmd_handlers[i].name);
			ret = dispatch_to_queue(&msg, xgq_cmd_handlers[i].task_level_type);
			
			return ret;
		}
	}

	MSG_ERR("Unhandled opcode: 0x%x", cmd->hdr.opcode);
	return -EINVAL;
}

static void abort_msg(u32 sq_addr)
{
	struct xgq_cmd_sq_hdr *cmd = (struct xgq_cmd_sq_hdr *)sq_addr;
	cl_msg_t msg = { 0 };

	msg.hdr.cid = cmd->cid;
	msg.hdr.rcode = -EINVAL;

	MSG_LOG("cid %d", cmd->cid);
	cl_msg_handle_complete(&msg);
}

static void inline process_msg(u32 sq_slot_addr)
{
	
	if (submit_to_queue(sq_slot_addr)) {
		abort_msg(sq_slot_addr);
	}

	return;
}

/*
 * Note: we don't need lock here yet.
 *     because different msg_type will be handled
 *     separately with different copy of msg, hdl.
 */
static void process_from_queue(cl_msg_t *msg)
{
	msg_handle_t *hdl;

	for (int i = 0; i < ARRAY_SIZE(handles); i++) {
		hdl = &handles[i];
		if (hdl->type == msg->hdr.type) {
			if (hdl->msg_cb == NULL) {
				MSG_ERR("no handle for msg type %d", msg->hdr.type);
				break;
			}
			hdl->msg_cb(msg, hdl->arg);
			return;
		}
	}

	/* complete unhandled msg too */
	MSG_ERR("unhandled msg type %d", msg->hdr.type);
	msg->hdr.rcode = -EINVAL;
	cl_msg_handle_complete(msg);
}

static void quickTask (void *pvParameters )
{
	cl_msg_t msg;

	for( ;; ) {
		MSG_DBG("Block to wait for new message. ");
		xQueueReceive( quickTaskQueue, &msg, portMAX_DELAY);
		process_from_queue(&msg);
	}
}

static void slowTask (void *pvParameters )
{
	cl_msg_t msg;

	for( ;; ) {
		MSG_DBG("Block to wait for new message. ");
		xQueueReceive( slowTaskQueue, &msg, portMAX_DELAY);
		process_from_queue(&msg);
	}
}

static inline bool read_vmr_shared_mem(struct vmr_shared_mem *mem)
{
	int ret = 0;

	ret = cl_memcpy_fromio32(VMR_EP_RPU_SHARED_MEMORY_START, mem, sizeof(*mem));
	if (ret == -1 || mem->vmr_magic_no != VMR_MAGIC_NO) {
		MSG_ERR("read shared memory partition table failed");
		return false;
	}

	return true;
}

static void vmr_status_service_start()
{
	struct vmr_shared_mem mem = { 0 };

	if (!read_vmr_shared_mem(&mem))
		return;

	IO_SYNC_WRITE32(1, RPU_SHARED_MEMORY_ADDR(mem.vmr_status_off));

	MSG_LOG("magic_no %x, ring %x, status off %x, value %x",
		mem.vmr_magic_no,
		mem.ring_buffer_off,
		mem.vmr_status_off,
		IO_SYNC_READ32(RPU_SHARED_MEMORY_ADDR(mem.vmr_status_off)));
	MSG_LOG("log_idx %d, log off %x, data start %x, data end %x",
		mem.log_msg_index,
		mem.log_msg_buf_off,
		mem.vmr_data_start,
		mem.vmr_data_end);
}

/*
 * Once minimum services started, we can start providing services. 
 * TODO: move critical functions into cl_xgq_server. OSPI, SCFW program.
 */
static inline int service_can_start()
{
	bool found_vmr_op = false;

	for (int i = 0; i < ARRAY_SIZE(handles); i++) {
		if (handles[i].msg_cb != NULL && handles[i].type == CL_MSG_VMR_CONTROL) {
			found_vmr_op = true;
			break;
		}
	}

	if (!found_vmr_op)
		return 0;

	if (!service_is_started) {
		service_is_started = true;
		vmr_status_service_start();
	}

	return 1;
}

static void receiveTask(void *pvParameters)
{
	const TickType_t xBlockTime = pdMS_TO_TICKS(500);

	for( ;; ) {
		uint64_t sq_slot_addr;
		
		vTaskDelay(xBlockTime);

		if (!service_can_start())
			continue;

		if (xgq_consume(&rpu_xgq, &sq_slot_addr))
			continue;

		process_msg(sq_slot_addr);

		xgq_notify_peer_consumed(&rpu_xgq);
	}
}

/*
 * Init shared memory partion table
 * === the partition metadata layout ===
 * u32: magic no. 			(offset@ 0)
 * u32: xgq ring buffer offset start 	(offset@ right after metadata buffer)
 * u32: xgq ring buffer size
 * u32: vmr_status_off			(offset@ right after xgq_ring_buffer)
 * u32: vmr_status_size
 * u32: log_msg_index
 * u32: log_msg_buf_off			(offset@ right after vmr_status buffer)
 * u32: log_msg_buf_len
 * u32: vmr_data_start			(offset@ right after log_msg buffer)
 * u32: vmr_data_end			(offset@ end of shared memory)
 */
static void init_vmr_status(uint32_t ring_len)
{
	struct vmr_shared_mem mem = {
		.vmr_magic_no = VMR_MAGIC_NO,
		.ring_buffer_off = VMR_PARTITION_TABLE_SIZE,
		.ring_buffer_len = ring_len,
	};

	mem.vmr_status_off = mem.ring_buffer_off + mem.ring_buffer_len;
	mem.vmr_status_len = sizeof (uint32_t);
	mem.log_msg_index = 0;
	mem.log_msg_buf_off = mem.vmr_status_off + mem.vmr_status_len;
	mem.log_msg_buf_len = LOG_BUF_LEN;
	mem.vmr_data_start = mem.log_msg_buf_off + mem.log_msg_buf_len;
	mem.vmr_data_end = VMR_EP_RPU_SHARED_MEMORY_END - VMR_EP_RPU_SHARED_MEMORY_START;

	cl_memcpy_toio32(VMR_EP_RPU_SHARED_MEMORY_START, &mem, sizeof(mem));

	/* re-init device stat to 0 */
	IO_SYNC_WRITE32(0x0, RPU_SHARED_MEMORY_ADDR(mem.vmr_status_off));	
}

typedef int (*vmr_op_handler)(cl_msg_t *msg);
struct xgq_vmr_op {
	int op_req_type;
	char *op_name;
	vmr_op_handler handle;
} xgq_vmr_op_table[] = {
	{ CL_MULTIBOOT_DEFAULT, "MULTIBOOT_DEFAULT", cl_rmgmt_enable_boot_default },
	{ CL_MULTIBOOT_BACKUP, "MULTIBOOT_BACKUP", cl_rmgmt_enable_boot_backup },
	{ CL_VMR_QUERY, "VMR_QUERY", cl_rmgmt_fpt_query },
	{ CL_PROGRAM_SC, "PROGRAM_SC", cl_rmgmt_program_sc },
	{ CL_VMR_DEBUG, "VMR_DEBUG", cl_rmgmt_fpt_debug },
};

static int xgq_vmr_op_cb(cl_msg_t *msg, void *arg)
{
	int ret = 0;

	for (int i = 0; i < ARRAY_SIZE(xgq_vmr_op_table); i++) {
		if (xgq_vmr_op_table[i].op_req_type == msg->multiboot_payload.req_type) {
			ret = xgq_vmr_op_table[i].handle(msg);
			goto done;
		}
	}

	MSG_ERR("unknown type %d", msg->multiboot_payload.req_type);
	ret = -EINVAL;
done:
	msg->hdr.rcode = ret;
	cl_msg_handle_complete(msg);
	MSG_DBG("complete msg id %d, ret %d", msg->hdr.cid, ret);
	return 0;
}

static int init_xgq()
{
	int ret = 0;
	size_t ring_len = RPU_RING_BUFFER_LEN;
	uint64_t flags = 0;
	msg_handle_t *vmr_hdl = NULL;

	/* Reset ring buffer */
	cl_memset_io8(VMR_EP_RPU_RING_BUFFER_BASE, 0, ring_len);

        ret = xgq_alloc(&rpu_xgq, flags, xgq_io_hdl, VMR_EP_RPU_RING_BUFFER_BASE, &ring_len,
		RPU_XGQ_SLOT_SIZE, VMR_EP_RPU_SQ_BASE, VMR_EP_RPU_CQ_BASE);
	if (ret) {
		MSG_ERR("xgq_alloc failed: %d", ret);
		return ret;
	}

        MSG_DBG("================================================");
        MSG_DBG("sq_slot_size 0x%lx", (u32)rpu_xgq.xq_sq.xr_slot_sz);
        MSG_DBG("cq_slot_size 0x%lx", (u32)rpu_xgq.xq_cq.xr_slot_sz);
        MSG_DBG("sq_num_slots %d", (u32)rpu_xgq.xq_sq.xr_slot_num);
        MSG_DBG("cq_num_slots %d", (u32)rpu_xgq.xq_cq.xr_slot_num);
        MSG_DBG("SQ slot addr offset 0x%lx", (u32)rpu_xgq.xq_sq.xr_slot_addr);
        MSG_DBG("CQ slot addr offset 0x%lx", (u32)rpu_xgq.xq_cq.xr_slot_addr);
        MSG_DBG("SQ xr_produced_addr 0x%lx", (u32)rpu_xgq.xq_sq.xr_produced_addr);
        MSG_DBG("SQ xr_consumed_addr 0x%lx", (u32)rpu_xgq.xq_sq.xr_consumed_addr);
        MSG_DBG("CQ xr_produced_addr 0x%lx", (u32)rpu_xgq.xq_cq.xr_produced_addr);
        MSG_DBG("CQ xr_consumed_addr 0x%lx", (u32)rpu_xgq.xq_cq.xr_consumed_addr);
        MSG_DBG("================================================");

	init_vmr_status(ring_len);
	
	/* start basic vmr services */
	ret = cl_msg_handle_init(&vmr_hdl, CL_MSG_VMR_CONTROL, xgq_vmr_op_cb, NULL);
	if (ret)
		return ret;
	
	MSG_LOG("done.");
	return 0;
}

static void fini_task()
{
	if (receiveTaskHandle != NULL)
		vTaskDelete(receiveTaskHandle);

	if (quickTaskHandle != NULL)
		vTaskDelete(quickTaskHandle);

	if (slowTaskHandle != NULL)
		vTaskDelete(slowTaskHandle);
}

static int init_task()
{
	if (xTaskCreate( receiveTask,
		( const char *) "XGQ Receive Task",
		TASK_STACK_DEPTH,
		NULL,
		tskIDLE_PRIORITY + 1,
		&receiveTaskHandle) != pdPASS) {

		MSG_ERR("FATAL: receiveTask creation failed");
		return -1;
	}

	if (xTaskCreate( quickTask,
		( const char *) "XGQ quick Task",
		TASK_STACK_DEPTH,
		NULL,
		tskIDLE_PRIORITY + 1,
		&quickTaskHandle) != pdPASS) {

		MSG_ERR("FATAL: quickTask creation failed");
		fini_task();
		return -1;
	}

	if (xTaskCreate( slowTask,
		( const char *) "XGQ slow Task",
		TASK_STACK_DEPTH,
		NULL,
		tskIDLE_PRIORITY + 1,
		&slowTaskHandle) != pdPASS) {

		MSG_ERR("FATAL: slowTask creation failed");
		fini_task();
		return -1;
	}

	MSG_LOG("done.");
	return 0;
}

static void fini_queue()
{
	if (quickTaskQueue != NULL)
		vQueueDelete(quickTaskQueue);

	if (slowTaskQueue != NULL)
		vQueueDelete(slowTaskQueue);
}

static int init_queue()
{
	quickTaskQueue = xQueueCreate(XGQ_XQUEUE_LENGTH, sizeof (cl_msg_t));
	if (quickTaskQueue == NULL) {
		MSG_ERR("FATAL: quickTaskQueue creation failed");
		return -1;
	}

	slowTaskQueue = xQueueCreate(XGQ_XQUEUE_LENGTH, sizeof (cl_msg_t));
	if (slowTaskQueue == NULL) {
		MSG_ERR("FATAL: slowTaskQueue creation failed");
		fini_queue();
		return -1;
	}

	MSG_LOG("done.");
	return 0;
}

static int cl_msg_service_start(void)
{
	if (init_xgq())
		return -1;

	if (init_task())
		return -1;

	if (init_queue()) {
		fini_task();
	}

	MSG_LOG("done.");
	return 0;
}

int CL_MSG_Launch(void)
{
	if (cl_msg_service_start() != 0) {
		MSG_ERR("failed");
		return -1;
	}

	MSG_LOG("done");

	return 0;
}

/*TODO: do we need a stop service ? */
