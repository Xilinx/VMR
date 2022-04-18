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
	switch (task_level) {
	case TASK_SLOW:
		/* send will do deep copy of msg, so that we preserved data */
		if (xQueueSend(slowTaskQueue, msg, (TickType_t) 0) != pdPASS) {
			MSG_ERR("FATAL: failed to send msg");
			return -1;
		};
		xTaskNotifyGive( slowTaskHandle );
		break;
	case TASK_QUICK:
		if (xQueueSend(quickTaskQueue, msg, (TickType_t) 0) != pdPASS) {
			MSG_ERR("FATAL: failed to send msg");
			return -1;
		};
		xTaskNotifyGive( quickTaskHandle );
		break;
	default:
		MSG_ERR("FATAL: unhandled task_level %d", task_level);
		return -1;
	}

	return 0;
}

static cl_clock_type_t convert_ctype(enum xgq_cmd_clock_req_type req_type)
{
	cl_clock_type_t cid = CL_CLOCK_UNKNOWN;

	switch (req_type) {
	case XGQ_CMD_CLOCK_WIZARD:
		cid = CL_CLOCK_WIZARD;
		break;	
	case XGQ_CMD_CLOCK_COUNTER:
		cid = CL_CLOCK_COUNTER;
		break;	
	case XGQ_CMD_CLOCK_SCALE:
		cid = CL_CLOCK_SCALE;
		break;	
	default:
		cid = CL_CLOCK_UNKNOWN;
		break;
	}

	return cid;
}

static cl_log_type_t convert_log_pid(enum xgq_cmd_log_page_type type)
{
	cl_log_type_t ltype = CL_LOG_UNKNOWN;

	switch (type) {
	case XGQ_CMD_LOG_AF_CHECK:
		ltype = CL_LOG_AF_CHECK;
		break;
	case XGQ_CMD_LOG_AF_CLEAR:
		ltype = CL_LOG_AF_CLEAR;
		break;
	case XGQ_CMD_LOG_FW:
		ltype = CL_LOG_FW;
		break;
	case XGQ_CMD_LOG_INFO:
		ltype = CL_LOG_INFO;
		break;
	case XGQ_CMD_LOG_ENDPOINT:
		ltype = CL_LOG_ENDPOINT;
		break;
	default:
		ltype = CL_LOG_UNKNOWN;
		break;
	}

	return ltype;
}

static cl_sensor_type_t convert_sensor_sid(enum xgq_cmd_sensor_page_id xgq_id)
{
	cl_sensor_type_t sid = CL_SENSOR_ALL;

	switch (xgq_id) {
	case XGQ_CMD_SENSOR_SID_GET_SIZE:
		sid = CL_SENSOR_GET_SIZE;
		break;
	case XGQ_CMD_SENSOR_SID_BDINFO:
		sid = CL_SENSOR_BDINFO;
		break;	
	case XGQ_CMD_SENSOR_SID_TEMP:
		sid = CL_SENSOR_TEMP;
		break;	
	case XGQ_CMD_SENSOR_SID_VOLTAGE:
		sid = CL_SENSOR_VOLTAGE;
		break;	
	case XGQ_CMD_SENSOR_SID_CURRENT:
		sid = CL_SENSOR_CURRENT;
		break;	
	case XGQ_CMD_SENSOR_SID_POWER:
		sid = CL_SENSOR_POWER;
		break;	
	case XGQ_CMD_SENSOR_SID_QSFP:
		sid = CL_SENSOR_QSFP;
		break;	
	default:
		sid = CL_SENSOR_ALL;
		break;
	}

	return sid;
}

static cl_vmr_control_type_t convert_control_type(enum xgq_cmd_vmr_control_type req_type)
{
	cl_vmr_control_type_t type = CL_VMR_QUERY;

	switch (req_type) {
	case XGQ_CMD_BOOT_DEFAULT:
		type = CL_MULTIBOOT_DEFAULT;
		break;
	case XGQ_CMD_BOOT_BACKUP:
		type = CL_MULTIBOOT_BACKUP;
		break;
	case XGQ_CMD_PROGRAM_SC:
		type = CL_PROGRAM_SC;
		break;
	case XGQ_CMD_VMR_DEBUG:
		type = CL_VMR_DEBUG;
		break;
	case XGQ_CMD_VMR_QUERY:
	default:
		type = CL_VMR_QUERY;
		break;
	}


	return type;
}

static cl_vmr_debug_type_t convert_debug_type(enum xgq_cmd_debug_type debug_type)
{
	cl_vmr_debug_type_t type = CL_DBG_CLEAR;

	switch (debug_type) {
	case XGQ_CMD_DBG_RMGMT:
		type = CL_DBG_DISABLE_RMGMT;
		break;
	case XGQ_CMD_DBG_VMC:
		type = CL_DBG_DISABLE_VMC;
		break;
	case XGQ_CMD_DBG_CLEAR:
	default:
		type = CL_DBG_CLEAR;
		break;
	}

	return type;
}

/*
 * submit dispatch msg to different software queues based on msg_type.
 * The quickTask only handles commands should be complished very fast.
 * The slowTask should handle all other messages which can run longer.
 * quick: < 10s vs. slow: 10s to minutes
 * In the future, we might use per task per message type if necessary.
 */
static int submit_to_queue(u32 sq_addr)
{
	struct xgq_sub_queue_entry *cmd = (struct xgq_sub_queue_entry *)sq_addr;
	struct xgq_cmd_sq *sq = (struct xgq_cmd_sq *)sq_addr;
	int ret = 0;
	int num_clock = 0;

	/* cast data to RPU common message type */
	cl_msg_t msg = { 0 };

	/* Sync data from cache to memory */
	Xil_DCacheFlush();

	msg.hdr.cid = cmd->hdr.cid;
	MSG_DBG("get cid %d opcode %d", cmd->hdr.cid, cmd->hdr.opcode);

	/* Convert xgq opcode to cl_common msg type */
	switch (cmd->hdr.opcode) {
	case XGQ_CMD_OP_LOAD_XCLBIN:
		msg.hdr.type = CL_MSG_XCLBIN;
		break;
	case XGQ_CMD_OP_DOWNLOAD_PDI:
		msg.hdr.type = CL_MSG_PDI;
		break;
	case XGQ_CMD_OP_LOAD_APUBIN:
		msg.hdr.type = CL_MSG_APUBIN;
		break;
	case XGQ_CMD_OP_GET_LOG_PAGE:
		msg.hdr.type = CL_MSG_LOG_PAGE;
		break;
	case XGQ_CMD_OP_CLOCK:
		msg.hdr.type = CL_MSG_CLOCK;
		break;
	case XGQ_CMD_OP_SENSOR:
		msg.hdr.type = CL_MSG_SENSOR;
		break;
	case XGQ_CMD_OP_VMR_CONTROL:
		msg.hdr.type = CL_MSG_VMR_CONTROL;
		break;
	default:
		MSG_ERR("Unhandled opcode: 0x%x", cmd->hdr.opcode);
		return -1;
	}

	/*
	 * set up payload based on msg type
	 * quick task handles time sensitive requests
	 * slow task handles time insensitive requests
	 */
	switch (msg.hdr.type) {
	case CL_MSG_XCLBIN:
		msg.data_payload.address = (u32)sq->xclbin_payload.address;
		msg.data_payload.size = (u32)sq->xclbin_payload.size;

		ret = dispatch_to_queue(&msg, TASK_SLOW);
		break;
	case CL_MSG_PDI:
	case CL_MSG_APUBIN:
		msg.data_payload.address = (u32)sq->pdi_payload.address;
		msg.data_payload.size = (u32)sq->pdi_payload.size;

		switch (sq->pdi_payload.flash_type) {
		case XGQ_CMD_FLASH_NO_BACKUP:
			msg.data_payload.flash_no_backup = 1;
			break;
		case XGQ_CMD_FLASH_TO_LEGACY:
			msg.data_payload.flash_to_legacy = 1;
			break;
		case XGQ_CMD_FLASH_DEFAULT:
		default:
			break;
		}

		ret = dispatch_to_queue(&msg, TASK_SLOW);
		break;
	case CL_MSG_VMR_CONTROL:
		/* we always set log level by this request */
		cl_loglevel_set(sq->vmr_control_payload.debug_level);
		msg.multiboot_payload.req_type =
			convert_control_type(sq->vmr_control_payload.req_type);
		msg.multiboot_payload.vmr_debug_type = 
			convert_debug_type(sq->vmr_control_payload.debug_type);
		ret = dispatch_to_queue(&msg, TASK_SLOW);
		break;
	case CL_MSG_CLOCK:
		msg.clock_payload.ocl_region = sq->clock_payload.ocl_region;
		msg.clock_payload.ocl_req_type =
			convert_ctype(sq->clock_payload.ocl_req_type);
		msg.clock_payload.ocl_req_id = sq->clock_payload.ocl_req_id;
		msg.clock_payload.ocl_req_num = sq->clock_payload.ocl_req_num;
		num_clock = msg.clock_payload.ocl_req_num;
		MSG_DBG("req_type %d, req_id %d, req_num %d",
			sq->clock_payload.ocl_req_type,
			sq->clock_payload.ocl_req_id,
			sq->clock_payload.ocl_req_num);

		if (num_clock > ARRAY_SIZE(msg.clock_payload.ocl_req_freq)) {
			MSG_ERR("num_clock out of range", num_clock);
			ret = -1;
			break;
		}

		/* deep copy freq requests */
		for (int i = 0; i < num_clock; i++) {
			msg.clock_payload.ocl_req_freq[i] =
				sq->clock_payload.ocl_req_freq[i];
		}
		ret = dispatch_to_queue(&msg, TASK_QUICK);
		break;
	case CL_MSG_LOG_PAGE:
		msg.log_payload.address = (u32)sq->log_payload.address;
		msg.log_payload.size = (u32)sq->log_payload.size;
		msg.log_payload.pid = convert_log_pid(sq->log_payload.pid);

		ret = dispatch_to_queue(&msg, TASK_QUICK);
		break;
	case CL_MSG_SENSOR:
		msg.sensor_payload.address = (u32)sq->sensor_payload.address;
		msg.sensor_payload.size = (u32)sq->sensor_payload.size;
		msg.sensor_payload.aid = (u8) sq->sensor_payload.aid;
		msg.sensor_payload.sid = (u8) convert_sensor_sid(sq->sensor_payload.sid);

		ret = dispatch_to_queue(&msg, TASK_QUICK);
		break;
	default:
		MSG_ERR("Unknown msg type:%d", msg.hdr.type);
		ret = -1;
		break;
	}

	return ret;
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
	u32 ulNotifiedValue;
	cl_msg_t msg;

	for( ;; ) {
		MSG_DBG("Block to wait for receiveTask to notify ");

		ulNotifiedValue = ulTaskNotifyTake( pdFALSE, portMAX_DELAY );
		if (ulNotifiedValue > 0 &&
		    xQueueReceive( quickTaskQueue, &msg, portMAX_DELAY) == pdPASS) {
			/* now we can use recvMsg */
			process_from_queue(&msg);
		} else {
			MSG_DBG("value %d", ulNotifiedValue);
		}
	}
}

static void slowTask (void *pvParameters )
{
	u32 ulNotifiedValue;
	cl_msg_t msg;

	for( ;; ) {
		MSG_DBG("Block to wait for receiveTask to notify ");

		ulNotifiedValue = ulTaskNotifyTake( pdFALSE, portMAX_DELAY );
		if (ulNotifiedValue > 0 &&
		    xQueueReceive( slowTaskQueue, &msg, portMAX_DELAY) == pdPASS) {
			/* now we can use recvMsg */
			process_from_queue(&msg);
		} else {
			MSG_DBG("value %d", ulNotifiedValue);
		}
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
