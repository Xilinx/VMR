/******************************************************************************
* Copyright (C) 2020-2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/
/* FreeRTOS includes */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"

#include <stdbool.h>

#include "cl_main.h"
#include "cl_log.h"
#include "cl_uart_rtos.h"
#include "cl_flash.h"
#include "cl_config.h"
#include "cl_io.h"
#include "cl_msg.h"
#include "cl_rmgmt.h"
#include "cl_vmc.h"
#include "vmr_common.h"

#ifdef BUILD_FOR_RMI
#include "cl_rmi.h"
#endif

#define XGQ_XQUEUE_LENGTH	8
#define XGQ_XQUEUE_WAIT_MS	10
/* 
 * use check_usage.sh to check peak usage in theory.
 * use vmr_mem_status to check peak usage in certain load.
 * the current peak usage is < 2k, seting thread to 16k heap size.
 *  The thread Depth "number of words" of the stack.
 *  depth (12k) * sizeof(word) = total size (48k)
 * If we have 5 tasks, it takes up to 240k.
 */
#define TASK_STACK_DEPTH 	0x3000

/*
 * VMR (Versal Management Runtime) design diagram.
 *
 * 1) VMR start diagram
 * cl_main_task -+
 *               |
 *               +---> cl_platform_init
 *               +---> cl_rmgmt_init
 *               +---> cl_vmc_init
 *               |
 *               +---> cl_queue_create(xgq_programe)
 *               +---> cl_queue_create(xgq_opcode)
 *               +---> cl_queue_create(xgq_scfw)
 *               +---> cl_queue_create(xgq_sensor)
 *               |
 *               +---> cl_task_create(xgq_receive)
 *               +---> cl_task_create(xgq_program)
 *               +---> cl_task_create(xgq_opcode)
 *               +---> cl_task_create(vmc_sensors_monitor) //this one cannot be blocked, 100ms (producer)
 *               +---> cl_task_create(vmc_sc_comms) //this one can block forever, platform dependet
 *               |
 *               +---> check_apu_is_ready
 *               +---> fully started.
 *
 * 2) VMR communication diagram
 * Host Driver ----> cl_xgq_receive task --+
 *                                         |
 *                   +----  xgq opcode   <-+
 *                   |
 *                   +----> cl_xgq_program queue -> cl_xgq_program task
 *                   |      each request will be put into FIFO queue. Host Driver
 *                   |      will use semaphore (1 resource) to exclusively send
 *                   |      long standing operations in parallel. Including,
 *                   |      PDI flash, xclbin download, APUPDI download, SCFW program.
 *                   |
 *                   +----> cl_xgq_opcode queue -> cl_xgq_opcode task
 *                          request might be sent to FIFO queue at the same time.
 *                          If requests need shared memory, it is protected by a
 *                          semaphore. All requests should be done very quick,
 *                          including clock config, firewall check, vmr_status_query etc.
 *
 * 3) VMR xgq alloc server init diagram
 * cl_xgq_receive ---> vmr_xgq_init ---+
 *                                     +---> xgq_alloc()
 *                                     +---> init_vmr_status()
 *                                     |     iowrite32 shared memory metadata
 *                                     |
 *                                     +---> vmr_status_service_start()
 *                                           iowrite32 shared memory vmr is ready bit
 *                                           Host Driver will wait till vmr is ready
 *
 * 4) Cocurrency handling
 *    4.1) xgq receive is a single task, no race condition;
 *    4.2) xgq program handles long standing request and exclusively keep shared memory
 *         for data till one request is finished;
 *    4.3) xgq opcode handles other quick requests. Those request may not or may reserve
 *         the shared memory for log.
 *    4.4) hardware monitor periodically collect sensor data from SC and cache it into
 *         local cache. There is a lock (sdr_lock) to protect local cache update and
 *         sensor data request.
 */

static TaskHandle_t cl_main_task_handle = NULL;

static TaskHandle_t cl_xgq_receive_handle = NULL;
static TaskHandle_t cl_xgq_program_handle = NULL;
static TaskHandle_t cl_xgq_opcode_handle = NULL;
static TaskHandle_t cl_vmc_sensor_handle = NULL;
static TaskHandle_t cl_vmc_sc_comms_handle = NULL;
#ifdef BUILD_FOR_RMI
static TaskHandle_t cl_rmi_handle = NULL;
#endif

#ifdef VMC_DEBUG
static TaskHandle_t cl_uart_demo_handle = NULL;
#endif

static QueueHandle_t cl_xgq_program_queue = NULL;
static QueueHandle_t cl_xgq_opcode_queue = NULL;
static QueueHandle_t cl_vmc_sc_req_queue = NULL;
static QueueHandle_t cl_vmc_sc_resp_queue = NULL;
static QueueHandle_t cl_vmc_sensor_req_queue = NULL;
static QueueHandle_t cl_vmc_sensor_resp_queue = NULL;

extern void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
	VMR_ERR("stack overflow %x %s\r\n", &xTask, (portCHAR *)pcTaskName);
	/* If the parameters have been corrupted then inspect pxCurrentTCB to
	 * identify which task has overflowed its stack.
	 */
	for (;;) { }
}

struct cl_task_handle {
	app_type_t	cl_task_type;
	int (*cl_task_init)(void);
	void (*cl_task_func)(void *task_args);
	const char 	*cl_task_name;
	void 		*cl_task_arg;
	TaskHandle_t	*cl_task_handle;
} task_handles[] = {
	{APP_VMR, cl_xgq_receive_init, cl_xgq_receive_func,
		"XGQ Receive", NULL, &cl_xgq_receive_handle},
	{APP_VMR,  cl_xgq_program_init, cl_xgq_program_func,
		"XGQ Program", NULL, &cl_xgq_program_handle},
	{APP_VMR, cl_xgq_opcode_init, cl_xgq_opcode_func,
		"XGQ Operation", NULL, &cl_xgq_opcode_handle},
	{APP_VMC, cl_vmc_sensor_init, cl_vmc_sensor_func,
		"Sensor Monitor", NULL, &cl_vmc_sensor_handle},
	{APP_VMC, cl_vmc_sc_comms_init, cl_vmc_sc_comms_func,
		"SC Common Handler", NULL, &cl_vmc_sc_comms_handle},
#ifdef BUILD_FOR_RMI
	{APP_VMC, cl_rmi_init, cl_rmi_func,
		"RMI Task Handler", NULL, &cl_rmi_handle},
#endif
		
#ifdef VMC_DEBUG
	{APP_VMC, cl_uart_demo_init, cl_uart_demo_func,
		"Uart Demo", NULL, &cl_uart_demo_handle},
#endif
};

struct cl_queue_handle {
	enum cl_queue_id cl_queue_qid;
	const char	*cl_queue_name;
	QueueHandle_t	*cl_queue_handle;
} queue_handles[] = {
	{CL_QUEUE_PROGRAM, "XGQ Program", &cl_xgq_program_queue},
	{CL_QUEUE_OPCODE, "XGQ Opcode", &cl_xgq_opcode_queue},
	{CL_QUEUE_SCFW_REQ, "SCFW Request", &cl_vmc_sc_req_queue},
	{CL_QUEUE_SCFW_RESP, "SCFW Respond", &cl_vmc_sc_resp_queue},
	{CL_QUEUE_SENSOR_REQ, "Sensor Request", &cl_vmc_sensor_req_queue},
	{CL_QUEUE_SENSOR_RESP, "Sensor Respond", &cl_vmc_sensor_resp_queue},
};

/*
 * all init work should be done in task_handle->cl_task_init()
 * prior to create task and running.
 */
static int cl_task_create(struct cl_task_handle *task_handle)
{
	int ret = 0;

	if (task_handle && task_handle->cl_task_init) {
		ret = task_handle->cl_task_init();
		if (ret)
			return ret;
	}

	ret = xTaskCreate( task_handle->cl_task_func,
			task_handle->cl_task_name,
			TASK_STACK_DEPTH,
			task_handle->cl_task_arg,
			tskIDLE_PRIORITY + 1,
			task_handle->cl_task_handle);
	if (ret != pdPASS) {
		VMR_ERR("Failed to create %s Task.", task_handle->cl_task_name);
		return -EINVAL;
	}

	VMR_LOG("Successfully create %s Task.", task_handle->cl_task_name);
	return 0;
}

static int cl_queue_create(struct cl_queue_handle *queue_handle)
{
	QueueHandle_t queueHandle = NULL;
	
	configASSERT(queue_handle->cl_queue_handle != NULL);
	
	queueHandle = xQueueCreate(XGQ_XQUEUE_LENGTH, sizeof(cl_msg_t)); 
	if (queueHandle == NULL) {
		VMR_ERR("Failed to create %s Queue", queue_handle->cl_queue_name);
		return -EINVAL;
	}
	*(queue_handle->cl_queue_handle) = queueHandle;

	VMR_LOG("Successfully create %s Queue", queue_handle->cl_queue_name);
	return 0;
}

int cl_send_to_queue(struct cl_msg *msg, enum cl_queue_id qid)
{
	for (int i = 0; i < ARRAY_SIZE(queue_handles); i++) {
		if (queue_handles[i].cl_queue_qid == qid) {
			if (xQueueSend(*(queue_handles[i].cl_queue_handle), msg,
				pdMS_TO_TICKS(XGQ_XQUEUE_WAIT_MS)) == pdPASS) {
				return 0;
			}

			VMR_ERR("failed to send msg %d to qid %d", msg->hdr.cid, qid);
			return -EIO;
		}
	}

	VMR_ERR("unhandled qid %d", qid);
	return -EINVAL;
}

int cl_recv_from_queue(struct cl_msg *msg, enum cl_queue_id qid)
{
	for (int i = 0; i < ARRAY_SIZE(queue_handles); i++) {
		if (queue_handles[i].cl_queue_qid == qid) {
			xQueueReceive(*(queue_handles[i].cl_queue_handle), msg, portMAX_DELAY);
			return 0;
		}
	}

	VMR_ERR("unhandled qid %d", qid);
	return -EINVAL;
}

int cl_recv_from_queue_nowait(struct cl_msg *msg, enum cl_queue_id qid)
{
	for (int i = 0; i < ARRAY_SIZE(queue_handles); i++) {
		if (queue_handles[i].cl_queue_qid == qid) {
			if (xQueueReceive(*(queue_handles[i].cl_queue_handle), msg,
				(TickType_t)0) == pdPASS) {
				return 0;
			}

			return -ENOENT;
		}
	}

	VMR_ERR("unhandled qid %d", qid);
	return -EINVAL;
}

static int cl_main_task_init(void)
{
	cl_log_init();
	VMR_WARN("=== VMR Service Starting ... ===");
#ifdef _VMR_VERSION_
	VMR_WARN("=== VMR Version: %s", VMR_GIT_HASH);
#endif

	/*
	 * workaround: sysmon cannot be inited after vTaskSchedulerStart for 2022.1 release
	 * AI: find the root cause of this issue and add comments.
	 */
	if (cl_vmc_sysmon_init() == 0)
	{
		VMR_WARN("Failed to init sysmon \n\r");
	}
	return 0;
}

static int cl_platform_init()
{
	return 0;
}

static void cl_main_task_func(void *task_args)
{
	/*
	 * TODO for multi-platform support.
	 */
	(void) cl_platform_init();

	/*
	 * If rmgmt is not ready, cl_rmgmt_is_ready will return false.
	 * We still continue to start xgq receiving tasks so that host driver
	 * can query debugging logs.
	 */
	(void) cl_rmgmt_init();

	/*
	 * If vmc is not ready, cl_vmc_is_ready will return false.
	 * We stop creating vmc task and report error via xgq receiving task.
	 * TODO: In the future, we create and pause vmc tasks, then retry till vmc becomes ready.
	 */
	(void) cl_vmc_init();

	/*
	 * Make sure queue is created prior to being used.
         */
	for (int i = 0; i < ARRAY_SIZE(queue_handles); i++) {
		configASSERT(cl_queue_create(&queue_handles[i]) == 0);
	}

	for (int i = 0; i < ARRAY_SIZE(task_handles); i++) {
		if (task_handles[i].cl_task_type == APP_VMC &&
			cl_vmc_is_ready() == false) {
			VMR_ERR("vmc init failed, skip create vmc tasks");
			continue;
		}
		configASSERT(cl_task_create(&task_handles[i]) == 0);
	}


	while (1) {
		/* check APU status every second */
		vTaskDelay(pdMS_TO_TICKS(1000));
		int rVal = cl_rmgmt_apu_channel_probe();
		if (rVal != -ENODEV)
			break;
	}

	VMR_WARN("\r\n=== VMR Services fully started. ===");
	vTaskDelay(pdMS_TO_TICKS(1000));
	vTaskDelete(NULL);
}

int main( void )
{
	struct cl_task_handle mainTask = {
		.cl_task_init = cl_main_task_init,
		.cl_task_func = cl_main_task_func,
		.cl_task_name = "Main Task",
		.cl_task_arg = NULL,
		.cl_task_handle = &cl_main_task_handle,
	};

	if (cl_task_create(&mainTask))
		return -1;

	vTaskStartScheduler();

	/* Should never go here unless there is not enough memory */
	VMR_LOG("not enough memory, exit.");
}

