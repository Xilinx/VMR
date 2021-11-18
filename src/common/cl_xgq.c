/******************************************************************************
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/
/* FreeRTOS includes */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"

#include "cl_log.h"
#include "cl_msg.h"
#include "cl_main.h"
#include "xgq_cmd_vmr.h"
#include "cl_xgq_plat.h"

#define MSG_LOG(fmt, arg...) \
	CL_LOG(APP_MAIN, fmt, ##arg)
#define MSG_DBG(fmt, arg...) \
	CL_DBG(APP_MAIN, fmt, ##arg)

#define XGQ_SQ_TAIL_POINTER     0x0
#define XGQ_SQ_INTR_REG         0x4
#define XGQ_SQ_INTR_CTRL        0xC
#define XGQ_CQ_TAIL_POINTER     0x100
#define XGQ_CQ_INTR_REG         0x104
#define XGQ_CQ_INTR_CTRL        0x10C

/* Note: eventually we should be driven by xparameter.h */
#define RPU_RING_BASE (0x38000000)
#define RPU_SQ_BASE (XPAR_BLP_BLP_LOGIC_XGQ_M2R_BASEADDR + XGQ_SQ_TAIL_POINTER)
#define RPU_CQ_BASE (XPAR_BLP_BLP_LOGIC_XGQ_M2R_BASEADDR + XGQ_CQ_TAIL_POINTER)

#define RPU_RING_LEN 0x1000
#define RPU_SLOT_SIZE 512
#define RPU_XGQ_DEV_STATE_OFFSET (RPU_RING_BASE + RPU_RING_LEN)

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

enum task_level {
	TASK_SLOW = 0,
	TASK_QUICK,
};

static msg_handle_t handles[] = {
	{ .type = CL_MSG_PDI },
	{ .type = CL_MSG_XCLBIN },
	{ .type = CL_MSG_AF },
	{ .type = CL_MSG_CLOCK },
	{ .type = CL_MSG_SENSOR },
	{ .type = CL_MSG_APUBIN },
	{ .type = CL_MSG_MULTIBOOT },
};

static const char *handle_name[] = {
	"PDI",
	"XCLBIN",
	"FIREWALL",
	"CLOCK",
	"SENSOR",
	"APUBIN",
	"MULTI-BOOT",
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
				MSG_LOG("FATAL: dup init with same type %d", type);
				return -1;
			}
			handles[i].msg_cb = cb;
			handles[i].arg = arg;
			*hdl = &handles[i];
			MSG_LOG("init handle %s type %d", handle_name[i], type);
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

	if (msg->hdr.type == CL_MSG_CLOCK)
		/* only 1 place to hold ocl_freq, alway get from index 0 */
		cmd_cq->clock_payload.ocl_freq = msg->clock_payload.ocl_req_freq[0];

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
			MSG_LOG("FATAL: failed to send msg");
			return -1;
		};
		xTaskNotifyGive( slowTaskHandle );
		break;
	case TASK_QUICK:
		if (xQueueSend(quickTaskQueue, msg, (TickType_t) 0) != pdPASS) {
			MSG_LOG("FATAL: failed to send msg");
			return -1;
		};
		xTaskNotifyGive( quickTaskHandle );
		break;
	default:
		MSG_LOG("FATAL: unhandled task_level %d", task_level);
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

static cl_sensor_type_t convert_pid(enum xgq_cmd_sensor_page_id xgq_id)
{
	cl_sensor_type_t sid = CL_SENSOR_ALL;

	switch (xgq_id) {
	case XGQ_CMD_SENSOR_PID_BDINFO:
		sid = CL_SENSOR_BDINFO;
		break;	
	case XGQ_CMD_SENSOR_PID_TEMP:
		sid = CL_SENSOR_TEMP;
		break;	
	case XGQ_CMD_SENSOR_PID_VOLTAGE:
		sid = CL_SENSOR_VOLTAGE;
		break;	
	case XGQ_CMD_SENSOR_PID_POWER:
		sid = CL_SENSOR_POWER;
		break;	
	default:
		sid = CL_SENSOR_ALL;
		break;
	}

	return sid;
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
		msg.hdr.type = CL_MSG_AF;
		break;
	case XGQ_CMD_OP_CLOCK:
		msg.hdr.type = CL_MSG_CLOCK;
		break;
	case XGQ_CMD_OP_SENSOR:
		msg.hdr.type = CL_MSG_SENSOR;
		break;
	case XGQ_CMD_OP_MULTIPLE_BOOT:
		msg.hdr.type = CL_MSG_MULTIBOOT;
		break;
	default:
		MSG_LOG("Unhandled opcode: 0x%x", cmd->hdr.opcode);
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

		ret = dispatch_to_queue(&msg, TASK_SLOW);
		break;
	case CL_MSG_AF:
	case CL_MSG_MULTIBOOT:
		ret = dispatch_to_queue(&msg, TASK_QUICK);
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
			MSG_LOG("num_clock out of range", num_clock);
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
	case CL_MSG_SENSOR:
		msg.log_payload.address = (u32)sq->sensor_payload.address;
		msg.log_payload.size = (u32)sq->sensor_payload.size;
		msg.log_payload.pid = convert_pid(sq->sensor_payload.pid);

		ret = dispatch_to_queue(&msg, TASK_SLOW);
		break;
	default:
		MSG_LOG("Unknown msg type:%d", msg.hdr.type);
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
				MSG_LOG("no handle for msg type %d", msg->hdr.type);
				return;
			}
			hdl->msg_cb(msg, hdl->arg);
			return;
		}
	}

	/* complete unhandled msg too */
	MSG_LOG("unhandled msg type %d", msg->hdr.type);
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
			MSG_LOG("value %d", ulNotifiedValue);
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
			MSG_LOG("value %d", ulNotifiedValue);
		}
	}
}

static inline int service_can_start()
{
	for (int i = 0; i < ARRAY_SIZE(handles); i++) {
		if (handles[i].msg_cb == NULL)
			return 0;
	}

	return 1;
}

static void receiveTask(void *pvParameters)
{
	const TickType_t xBlockTime = pdMS_TO_TICKS(500);
	u32 cnt = 0;

	for( ;; ) {
		uint64_t sq_slot_addr;
		
		vTaskDelay(xBlockTime);

		cnt++;
		//xil_printf("%d receiveTask\r", cnt);

		if (!service_can_start())
			continue;

		/* when cnt is not 0, xgq is healthy */
		IO_SYNC_WRITE32(cnt, RPU_XGQ_DEV_STATE_OFFSET);	

		if (xgq_consume(&rpu_xgq, &sq_slot_addr))
			continue;

		process_msg(sq_slot_addr);

		xgq_notify_peer_consumed(&rpu_xgq);
	}
}

static int init_xgq()
{
	int ret = 0;
	size_t ring_len = RPU_RING_LEN;
	uint64_t flags = 0;

	/* re-init device stat to 0 */
	IO_SYNC_WRITE32(0x0, RPU_XGQ_DEV_STATE_OFFSET);	

        ret = xgq_alloc(&rpu_xgq, flags, xgq_io_hdl, RPU_RING_BASE, &ring_len,
		RPU_SLOT_SIZE, RPU_SQ_BASE, RPU_CQ_BASE);
	if (ret) {
		MSG_LOG("xgq_alloc failed: %d", ret);
		return ret;
	}

        MSG_LOG("================================================");
        MSG_LOG("sq_slot_size 0x%lx", (u32)rpu_xgq.xq_sq.xr_slot_sz);
        MSG_LOG("cq_slot_size 0x%lx", (u32)rpu_xgq.xq_cq.xr_slot_sz);
        MSG_LOG("sq_num_slots %d", (u32)rpu_xgq.xq_sq.xr_slot_num);
        MSG_LOG("cq_num_slots %d", (u32)rpu_xgq.xq_cq.xr_slot_num);
        MSG_LOG("SQ slot addr offset 0x%lx", (u32)rpu_xgq.xq_sq.xr_slot_addr);
        MSG_LOG("CQ slot addr offset 0x%lx", (u32)rpu_xgq.xq_cq.xr_slot_addr);
        MSG_LOG("SQ xr_produced_addr 0x%lx", (u32)rpu_xgq.xq_sq.xr_produced_addr);
        MSG_LOG("SQ xr_consumed_addr 0x%lx", (u32)rpu_xgq.xq_sq.xr_consumed_addr);
        MSG_LOG("CQ xr_produced_addr 0x%lx", (u32)rpu_xgq.xq_cq.xr_produced_addr);
        MSG_LOG("CQ xr_consumed_addr 0x%lx", (u32)rpu_xgq.xq_cq.xr_consumed_addr);
        MSG_LOG("================================================");

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
		1024,
		NULL,
		tskIDLE_PRIORITY + 1,
		&receiveTaskHandle) != pdPASS) {

		MSG_LOG("FATAL: receiveTask creation failed");
		return -1;
	}

	if (xTaskCreate( quickTask,
		( const char *) "quick Task",
		1024,
		NULL,
		tskIDLE_PRIORITY + 1,
		&quickTaskHandle) != pdPASS) {

		MSG_LOG("FATAL: quickTask creation failed");
		fini_task();
		return -1;
	}

	if (xTaskCreate( slowTask,
		( const char *) "slow Task",
		1024,
		NULL,
		tskIDLE_PRIORITY + 1,
		&slowTaskHandle) != pdPASS) {

		MSG_LOG("FATAL: slowTask creation failed");
		fini_task();
		return -1;
	}

	MSG_LOG("INFO: creation succeeded.");
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
	quickTaskQueue = xQueueCreate(32, sizeof (cl_msg_t));
	if (quickTaskQueue == NULL) {
		MSG_LOG("FATAL: quickTaskQueue creation failed");
		return -1;
	}

	slowTaskQueue = xQueueCreate(32, sizeof (cl_msg_t));
	if (slowTaskQueue == NULL) {
		MSG_LOG("FATAL: slowTaskQueue creation failed");
		fini_queue();
		return -1;
	}
	MSG_LOG("INFO: quick|slow TaskQueue creation succeeded");
	return 0;
}

static int cl_msg_service_start(void)
{

	MSG_LOG("start");

	if (init_xgq())
		return -1;

	if (init_task())
		return -1;

	if (init_queue()) {
		fini_task();
	}

	return 0;
}

int cl_msg_service_launch(void)
{
	if (cl_msg_service_start() != 0) {
		MSG_LOG("failed");
		return -1;
	}

	MSG_LOG("done");

	return 0;
}

/*TODO: do we need a stop service ? */
