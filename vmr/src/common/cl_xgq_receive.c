/******************************************************************************
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/
/* FreeRTOS includes */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "semphr.h"

#include <stdbool.h>

#include "cl_log.h"
#include "cl_msg.h"
#include "cl_main.h"
#include "cl_flash.h"
#include "cl_rmgmt.h"
#include "cl_vmc.h"
#include "xgq_cmd_vmr.h"
#include "cl_xgq_receive_plat.h"
#include "vmr_common.h"

#define XGQ_XQUEUE_LENGTH	8
#define XGQ_SEND_QUEUE_WAIT_MS	10

#define FLASH_PROGRESS \
	cl_vmc_scfw_program_progress() ? cl_vmc_scfw_program_progress() : ospi_flash_progress()

static struct xgq rpu_xgq;
static int xgq_io_hdl = 0;
static SemaphoreHandle_t msg_complete_lock = NULL;

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
	{XGQ_CMD_LOG_TASK_STATS, CL_LOG_TASK_STATS},
	{XGQ_CMD_LOG_MEM_STATS, CL_LOG_MEM_STATS},
	{XGQ_CMD_LOG_SYSTEM_DTB, CL_LOG_SYSTEM_DTB},
	{XGQ_CMD_LOG_PLM_LOG, CL_LOG_PLM_LOG},
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

static struct xgq_cmd_cl_map xgq_cmd_clk_scaling_map[] = {
	{XGQ_CMD_CLK_SCALING_GET_STATUS,   CL_CLK_SCALING_READ},
	{XGQ_CMD_CLK_SCALING_SET_OVERRIDE, CL_CLK_SCALING_SET},
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

	VMR_ERR("unknown type %d", xgq_type);
	return -EINVAL;
}

static int xclbin_handle(cl_msg_t *msg, struct xgq_cmd_sq *sq)
{
	msg->data_payload.address = (u32)sq->xclbin_payload.address;
	msg->data_payload.size = (u32)sq->xclbin_payload.size;
	msg->data_payload.priv = sq->xclbin_payload.priv;

	return 0;
}

static int pdi_handle(cl_msg_t *msg, struct xgq_cmd_sq *sq)
{
	msg->data_payload.address = (u32)sq->pdi_payload.address;
	msg->data_payload.size = (u32)sq->pdi_payload.size;
	msg->data_payload.priv = sq->xclbin_payload.priv;

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

static int vmr_control_handle(cl_msg_t *msg, struct xgq_cmd_sq *sq)
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

static int clock_handle(cl_msg_t *msg, struct xgq_cmd_sq *sq)
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
	VMR_DBG("req_type %d, req_id %d, req_num %d",
		sq->clock_payload.ocl_req_type,
		sq->clock_payload.ocl_req_id,
		sq->clock_payload.ocl_req_num);

	if (num_clock > ARRAY_SIZE(msg->clock_payload.ocl_req_freq)) {
		VMR_ERR("num_clock %d out of range", num_clock);
		return -EINVAL;
	}

	/* deep copy freq requests */
	for (int i = 0; i < num_clock; i++) {
		msg->clock_payload.ocl_req_freq[i] =
			sq->clock_payload.ocl_req_freq[i];
	}

	return 0;
}

static int log_page_handle(cl_msg_t *msg, struct xgq_cmd_sq *sq)
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

static int sensor_handle(cl_msg_t *msg, struct xgq_cmd_sq *sq)
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
	msg->sensor_payload.sensor_id = (u8) sq->sensor_payload.sensor_id;

	return 0;
}

static int clk_throttling_handle(cl_msg_t *msg, struct xgq_cmd_sq *sq)
{
	int ret = 0;
	u32 aid = 0x1;

	ret = convert_cl_type(sq->clk_scaling_payload.aid, &aid,
		xgq_cmd_clk_scaling_map, ARRAY_SIZE(xgq_cmd_clk_scaling_map));
	if (ret)
		return ret;

	msg->clk_scaling_payload.aid = aid;
	msg->clk_scaling_payload.scaling_en =
			(u32)sq->clk_scaling_payload.scaling_en;
	msg->clk_scaling_payload.pwr_scaling_ovrd_limit =
			(u32)sq->clk_scaling_payload.pwr_scaling_ovrd_limit;
	msg->clk_scaling_payload.temp_scaling_ovrd_limit =
			(u8) sq->clk_scaling_payload.temp_scaling_ovrd_limit;

	return 0;
}

static void clock_complete(cl_msg_t *msg, struct xgq_cmd_cq *cmd_cq)
{	
	/*TODO:
	 * cleanup this code, should not mix request and result within same payload.
	 * separate request and results
	 */

	/* only 1 place to hold ocl_freq, always get from index 0 */
	cmd_cq->cq_clock_payload.ocl_freq = msg->clock_payload.ocl_req_freq[0];
}

static void vmr_control_complete(cl_msg_t *msg, struct xgq_cmd_cq *cmd_cq)
{
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
	cmd_cq->cq_vmr_payload.ps_is_ready = cl_rmgmt_apu_is_ready();
	cmd_cq->cq_vmr_payload.pl_is_ready = cl_rmgmt_pl_is_ready();
	
	/* pass back status, SC is ready and VMC has SC version */
	cmd_cq->cq_vmr_payload.sc_is_ready = cl_vmc_has_sc_version();

	/* pass back log level and flash progress */
	cmd_cq->cq_vmr_payload.debug_level = cl_loglevel_get();
	cmd_cq->cq_vmr_payload.program_progress = FLASH_PROGRESS;
}

static void log_page_complete(cl_msg_t *msg, struct xgq_cmd_cq *cmd_cq)
{
	cmd_cq->cq_log_payload.count = msg->log_payload.size;
}

static void clk_throttling_complete(cl_msg_t *msg, struct xgq_cmd_cq *cmd_cq)
{
	clk_throttling_params_t scaling_params = { 0 };

	cl_vmc_get_clk_throttling_params(&scaling_params);

	cmd_cq->cq_clk_scaling_payload.has_clk_scaling = 
				scaling_params.is_clk_scaling_supported;
	cmd_cq->cq_clk_scaling_payload.clk_scaling_mode =
				scaling_params.clk_scaling_mode;
	cmd_cq->cq_clk_scaling_payload.clk_scaling_en = 
				scaling_params.clk_scaling_enable;
	cmd_cq->cq_clk_scaling_payload.temp_shutdown_limit = 
				scaling_params.limits.shutdown_limit_temp;
	cmd_cq->cq_clk_scaling_payload.temp_scaling_limit = 
				scaling_params.limits.throttle_limit_temp;
	cmd_cq->cq_clk_scaling_payload.pwr_shutdown_limit = 
				scaling_params.limits.shutdown_limit_pwr;
	cmd_cq->cq_clk_scaling_payload.pwr_scaling_limit = 
				scaling_params.limits.throttle_limit_pwr;
}

static void vmr_identify_complete(cl_msg_t *msg, struct xgq_cmd_cq *cmd_cq)
{
	cmd_cq->cq_vmr_identify_payload.ver_major = msg->hdr.version_major;
	cmd_cq->cq_vmr_identify_payload.ver_minor = msg->hdr.version_minor;
}

struct xgq_cmd_handler {
	uint16_t	opcode;			/* xgq cmd opcode */
	uint16_t	msg_type;		/* common layer internal msg type */
	char		*name;			/* handler name */
	enum cl_queue_id qid;	/* which task to handle the request */
	int (*msg_handle)(cl_msg_t *msg, struct xgq_cmd_sq *sq); /* submit callback */
	void(*msg_complete)(cl_msg_t *msg, struct xgq_cmd_cq *cq); /* complete callback */
};

static struct xgq_cmd_handler xgq_cmd_handlers[] = {
	{XGQ_CMD_OP_LOAD_XCLBIN, CL_MSG_XCLBIN, "LOAD XCLBIN", CL_QUEUE_PROGRAM,
		xclbin_handle, NULL},
	{XGQ_CMD_OP_DOWNLOAD_PDI, CL_MSG_PDI, "LOAD BASE PDI", CL_QUEUE_PROGRAM,
		pdi_handle, NULL},
	{XGQ_CMD_OP_LOAD_APUBIN, CL_MSG_APUBIN, "LOAD APU PDI", CL_QUEUE_PROGRAM,
		pdi_handle, NULL},
	{XGQ_CMD_OP_PROGRAM_VMR, CL_MSG_PROGRAM_VMR, "PROGRAM VMR", CL_QUEUE_PROGRAM,
		xclbin_handle, NULL},
	{XGQ_CMD_OP_GET_LOG_PAGE, CL_MSG_LOG_PAGE, "LOG PAGE", CL_QUEUE_OPCODE,
		log_page_handle, log_page_complete},
	{XGQ_CMD_OP_CLOCK, CL_MSG_CLOCK, "CLOCK", CL_QUEUE_OPCODE,
		clock_handle, clock_complete},
	{XGQ_CMD_OP_VMR_CONTROL, CL_MSG_VMR_CONTROL, "VMR_CONTROL", CL_QUEUE_OPCODE,
		vmr_control_handle, vmr_control_complete},
	{XGQ_CMD_OP_SENSOR, CL_MSG_SENSOR, "SENSOR", CL_QUEUE_OPCODE,
		sensor_handle, NULL},
	{XGQ_CMD_OP_PROGRAM_SCFW, CL_MSG_PROGRAM_SCFW, "PROGRAM SCFW", CL_QUEUE_PROGRAM,
		NULL, NULL},
	{XGQ_CMD_OP_CLK_THROTTLING, CL_MSG_CLK_THROTTLING, "CLOCK THROTTLING", CL_QUEUE_OPCODE,
		clk_throttling_handle, clk_throttling_complete},
	{XGQ_CMD_OP_IDENTIFY, CL_MSG_VMR_IDENTIFY, "VMR IDENTIFY", CL_QUEUE_OPCODE,
		NULL, vmr_identify_complete},
};

void cl_msg_handle_complete(cl_msg_t *msg)
{
	struct xgq_com_queue_entry cmd_cq_entry = {
		.hdr.cid = msg->hdr.cid,
		.hdr.state = XGQ_CMD_STATE_COMPLETED,
		.rcode = msg->hdr.rcode,
	};
	struct xgq_cmd_cq *cmd_cq = (struct xgq_cmd_cq *)&cmd_cq_entry;
	u64 cq_slot_addr;

	if (cmd_cq->rcode)
		VMR_WARN("WARN: cid:%d rcode:%d", cmd_cq->hdr.cid, cmd_cq->rcode);

	for (int i = 0; i < ARRAY_SIZE(xgq_cmd_handlers); i++) {
		if (xgq_cmd_handlers[i].msg_type == msg->hdr.type &&
			xgq_cmd_handlers[i].msg_complete != NULL)
			xgq_cmd_handlers[i].msg_complete(msg, cmd_cq);
	}

	if (xSemaphoreTake(msg_complete_lock, portMAX_DELAY)) {
		xgq_produce(&rpu_xgq, &cq_slot_addr);
		cl_memcpy_toio(cq_slot_addr, &cmd_cq_entry, sizeof(struct xgq_com_queue_entry));
		xgq_notify_peer_produced(&rpu_xgq);
		xSemaphoreGive(msg_complete_lock);
	} else {
		/* Should never been here if use portMAX_DELAY */
		VMR_ERR("msg_complete_lock lock failed");
	}
	
	return;
}


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
	VMR_DBG("get cid %d opcode 0x%x", cmd->hdr.cid, cmd->hdr.opcode);

	for (int i = 0; i < ARRAY_SIZE(xgq_cmd_handlers); i++) {
		if (xgq_cmd_handlers[i].opcode == cmd->hdr.opcode) {

			msg.hdr.type = xgq_cmd_handlers[i].msg_type;
			if (xgq_cmd_handlers[i].msg_handle != NULL) {
				ret = xgq_cmd_handlers[i].msg_handle(&msg, sq);
				if (ret)
					return ret;
			}

			VMR_DBG("send msg %s to handle task via message queue.", xgq_cmd_handlers[i].name);
			ret = cl_send_to_queue(&msg, xgq_cmd_handlers[i].qid);
			
			return ret;
		}
	}

	VMR_ERR("Unhandled opcode: 0x%x", cmd->hdr.opcode);
	return -EINVAL;
}

static void abort_msg(u32 sq_addr)
{
	struct xgq_cmd_sq_hdr *cmd = (struct xgq_cmd_sq_hdr *)sq_addr;
	cl_msg_t msg = { 0 };

	msg.hdr.cid = cmd->cid;
	msg.hdr.rcode = -EINVAL;

	VMR_LOG("cid %d", cmd->cid);
	cl_msg_handle_complete(&msg);
}

static void inline process_msg(u32 sq_slot_addr)
{
	
	if (submit_to_queue(sq_slot_addr)) {
		abort_msg(sq_slot_addr);
	}

	return;
}

static inline bool read_vmr_shared_mem(struct vmr_shared_mem *mem)
{
	int ret = 0;

	ret = cl_memcpy_fromio(VMR_EP_RPU_SHARED_MEMORY_START, mem, sizeof(*mem));
	if (ret == -1 || mem->vmr_magic_no != VMR_MAGIC_NO) {
		VMR_ERR("read shared memory partition table failed");
		return false;
	}

	return true;
}

static int vmr_status_service_stop()
{
	struct vmr_shared_mem mem = { 0 };

	if (!read_vmr_shared_mem(&mem))
		return -EINVAL;

	IO_SYNC_WRITE32(0, RPU_SHARED_MEMORY_ADDR(mem.vmr_status_off));

	return 0;
}

static int vmr_status_service_start()
{
	struct vmr_shared_mem mem = { 0 };

	if (!read_vmr_shared_mem(&mem))
		return -1;

	IO_SYNC_WRITE32(1, RPU_SHARED_MEMORY_ADDR(mem.vmr_status_off));

	VMR_LOG("magic_no %x, ring %x, status off %x, value %x",
		mem.vmr_magic_no,
		mem.ring_buffer_off,
		mem.vmr_status_off,
		IO_SYNC_READ32(RPU_SHARED_MEMORY_ADDR(mem.vmr_status_off)));
	VMR_LOG("log_idx %d, log off %x, data start %x, data end %x",
		mem.log_msg_index,
		mem.log_msg_buf_off,
		mem.vmr_data_start,
		mem.vmr_data_end);

	return 0;
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

	cl_memcpy_toio(VMR_EP_RPU_SHARED_MEMORY_START, &mem, sizeof(mem));

	/* re-init device stat to 0 */
	IO_SYNC_WRITE32(0x0, RPU_SHARED_MEMORY_ADDR(mem.vmr_status_off));	
}

static int vmr_xgq_init()
{
	int ret = 0;
	size_t ring_len = RPU_RING_BUFFER_LEN;
	uint64_t flags = 0;

	/* Reset ring buffer */
	cl_memset_io8(VMR_EP_RPU_RING_BUFFER_BASE, 0, ring_len);

        ret = xgq_alloc(&rpu_xgq, flags, xgq_io_hdl, VMR_EP_RPU_RING_BUFFER_BASE, &ring_len,
		RPU_XGQ_SLOT_SIZE, VMR_EP_RPU_SQ_BASE, VMR_EP_RPU_CQ_BASE);
	if (ret) {
		VMR_ERR("xgq_alloc failed: %d", ret);
		return ret;
	}

        VMR_DBG("================================================");
        VMR_DBG("sq_slot_size 0x%lx", (u32)rpu_xgq.xq_sq.xr_slot_sz);
        VMR_DBG("cq_slot_size 0x%lx", (u32)rpu_xgq.xq_cq.xr_slot_sz);
        VMR_DBG("sq_num_slots %d", (u32)rpu_xgq.xq_sq.xr_slot_num);
        VMR_DBG("cq_num_slots %d", (u32)rpu_xgq.xq_cq.xr_slot_num);
        VMR_DBG("SQ slot addr offset 0x%lx", (u32)rpu_xgq.xq_sq.xr_slot_addr);
        VMR_DBG("CQ slot addr offset 0x%lx", (u32)rpu_xgq.xq_cq.xr_slot_addr);
        VMR_DBG("SQ xr_produced_addr 0x%lx", (u32)rpu_xgq.xq_sq.xr_produced_addr);
        VMR_DBG("SQ xr_consumed_addr 0x%lx", (u32)rpu_xgq.xq_sq.xr_consumed_addr);
        VMR_DBG("CQ xr_produced_addr 0x%lx", (u32)rpu_xgq.xq_cq.xr_produced_addr);
        VMR_DBG("CQ xr_consumed_addr 0x%lx", (u32)rpu_xgq.xq_cq.xr_consumed_addr);
        VMR_DBG("================================================");

	init_vmr_status(ring_len);
	
	VMR_LOG("done.");
	return 0;
}

int cl_xgq_receive_init(void)
{
	if (vmr_xgq_init())
		return -1;

	if (vmr_status_service_start())
		return -1;

	/* Init mutex for cl_msg_handle_complete */
	msg_complete_lock = xSemaphoreCreateMutex();

	return 0;
}

/*
 * We don't live unloading the VMR, just restart the RPU and reloading the vmr.elf.
 * But we do need to reset the ready bits so that the host xgq driver will be set
 * to correct status.
 */
int cl_xgq_receive_fini(void)
{
	return vmr_status_service_stop();
}

void cl_xgq_receive_func(void *task_args)
{
	const TickType_t xBlockTime = pdMS_TO_TICKS(100);

	for( ;; ) {
		uint64_t sq_slot_addr;
		vTaskDelay(xBlockTime);

		if (xgq_consume(&rpu_xgq, &sq_slot_addr))
			continue;

		process_msg(sq_slot_addr);

		xgq_notify_peer_consumed(&rpu_xgq);
	}
}
