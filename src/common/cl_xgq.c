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
#include "xgq_cmd.h"
#include "cl_xgq_plat.h"

#define MSG_LOG(fmt, arg...) \
	CL_LOG(APP_MAIN, fmt, ##arg)

/* Note: eventually we should be driven by xparameter.h */
#define RPU_RING_BASE (0x38000000)
#define RPU_SQ_BASE (0x80011000)
#define RPU_CQ_BASE (0x80010000)
//#define RPU_CQ_BASE XPAR_BLP_BLP_LOGIC_XGQ_R2M_S00_AXI_BASEADDR

#define RPU_RING_LEN 0x1000
#define RPU_SLOT_SIZE 512
#define RPU_XGQ_DEV_STATE_OFFSET (RPU_RING_LEN)

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

static msg_handle_t handles[] = {
	{ .type = CL_MSG_PDI },
	{ .type = CL_MSG_XCLBIN },
	{ .type = CL_MSG_AF },
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
				MSG_LOG("FATAL: dup init with same type");
				return -1;
			}
			handles[i].msg_cb = cb;
			handles[i].arg = arg;
			*hdl = &handles[i];
			MSG_LOG("init type %d", type);
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
	struct xrt_com_queue_entry cq_cmd = {
		.rcode = msg->pkt.head.rcode,
		.cid = msg->pkt.head.cid,
		.state = XRT_CMD_STATE_COMPLETED,
	};
	u64 cq_slot_addr;

	xgq_produce(&rpu_xgq, &cq_slot_addr);

	cl_memcpy_toio(cq_slot_addr, &cq_cmd, sizeof(struct xrt_com_queue_entry));
	xgq_notify_peer_produced(&rpu_xgq);

	return 0;
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
	int msg_type;
	struct xrt_sub_queue_entry *cmd = (struct xrt_sub_queue_entry *)sq_addr;
	/* cast data to RPU common message type */
	cl_msg_t msg = { 0 };

	cl_memcpy_fromio((u32)cmd + sizeof(*cmd), &msg, sizeof(cl_msg_t));
	msg.pkt.head.cid = cmd->cid; //remember the cid, cmd will be freed
	msg_type = msg.pkt.head.type;

	switch (msg_type) {
	case CL_MSG_PDI:
	case CL_MSG_XCLBIN:
		/* send will do deep copy of msg, so that we preserved data */
		if (xQueueSend(slowTaskQueue, &msg, (TickType_t) 0) != pdPASS) {
			MSG_LOG("FATAL: failed to send msg");
			return -1;
		};
		xTaskNotifyGive( slowTaskHandle );
		break;
	case CL_MSG_AF:
		if (xQueueSend(quickTaskQueue, &msg, (TickType_t) 0) != pdPASS) {
			MSG_LOG("FATAL: failed to send msg");
			return -1;
		};
		xTaskNotifyGive( quickTaskHandle );
		break;
	default:
		MSG_LOG("Unknown mest type:%d", msg_type);
		return -1;
	}

	return 0;
}

static void abort_msg(u32 sq_addr)
{
	struct xrt_cmd_configure *cmd = (struct xrt_cmd_configure *)sq_addr;
	cl_msg_t *msg = (cl_msg_t *)cmd->data;
	msg->pkt.head.cid = cmd->cid;
	msg->pkt.head.rcode = 1;
	cl_msg_handle_complete(msg);
}

static void inline process_msg(u32 sq_slot_addr)
{
	/*
	 * Cast incoming sq_slot_addr to customized cmd,
	 * then we can decode type of this cmd,
	 * and dispatch it into software queue.
	 */
	struct xrt_sub_queue_entry *cmd = (struct xrt_sub_queue_entry *)sq_slot_addr;

	switch (cmd->opcode) {
	case XRT_CMD_OP_CONFIGURE:
		MSG_LOG("handling opcode 0x%x cid %d", cmd->opcode, cmd->cid);
		if (submit_to_queue(sq_slot_addr) != 0)
			break;
		return;
	default:
		MSG_LOG("unhandled opcode 0x%x", cmd->opcode);
		break;
	}
	/*TODO: complete with error */
	abort_msg(sq_slot_addr);
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
		if (hdl->type == msg->pkt.head.type) {
			if (hdl->msg_cb == NULL) {
				MSG_LOG("no handle for msg type %d", msg->pkt.head.type);
				return;
			}
			//MSG_LOG("handle msg type %d", msg->pkt.head.type);
			hdl->msg_cb(msg, hdl->arg);
			return;
		}
	}

	/* complete unhandled msg too */
	MSG_LOG("unhandled msg type %d", msg->pkt.head.type);
	msg->pkt.head.rcode = -EINVAL;
	cl_msg_handle_complete(msg);
}

static void quickTask (void *pvParameters )
{
	u32 ulNotifiedValue;
	cl_msg_t msg;

	for( ;; ) {
		MSG_LOG("Block to wait for receiveTask to notify ");

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
		MSG_LOG("Block to wait for receiveTask to notify ");

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

static void receiveTask(void *pvParameters)
{
	const TickType_t xBlockTime = pdMS_TO_TICKS(1000*1);
	u32 cnt = 0;

	for( ;; ) {
		uint64_t sq_slot_addr;
		
		vTaskDelay(xBlockTime);

		cnt++;
		/* when cnt is not 0, xgq is healthy */
		IO_SYNC_WRITE32(cnt, RPU_XGQ_DEV_STATE_OFFSET);	

		if (xgq_consume(&rpu_xgq, &sq_slot_addr))
			continue;

		MSG_LOG("xgq_consume slot 0x%x", (u32)sq_slot_addr);

		process_msg(sq_slot_addr);

		MSG_LOG("slot 0x%x consumed", (u32)sq_slot_addr);

		xgq_notify_peer_consumed(&rpu_xgq);
	}
}

static int init_xgq()
{
	int ret = 0;
	size_t ring_len = RPU_RING_LEN;

	MSG_LOG("->");
#if 0
	MSG_LOG("write range 0x%x to 0x%x all zero", RPU_RING_BASE, RPU_RING_BASE + 0xfff);
	{
		u32 i;
		for (i = RPU_RING_BASE; i < RPU_RING_BASE + 0x1000; i += 4) {
			//xil_printf("%x\r", i);
			IO_SYNC_WRITE32(0x0, i);
		}
		//IO_SYNC_WRITE32(0x0, RPU_CQ_BASE);	
	}
#endif	
	IO_SYNC_WRITE32(0x0, RPU_XGQ_DEV_STATE_OFFSET);	
        ret = xgq_alloc(&rpu_xgq, XGQ_SERVER, xgq_io_hdl, RPU_RING_BASE, &ring_len,
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

	MSG_LOG("<-");

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
