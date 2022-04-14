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

#include "cl_main.h"
#include "cl_log.h"
#include "cl_uart_rtos.h"
#include "cl_flash.h"
#include "cl_config.h"
#include "cl_io.h"
#include "cl_msg.h"
#include "cl_rmgmt.h"
#include "vmr_common.h"
#include "sysmon.h"

uart_rtos_handle_t uart_log;

XSysMonPsv InstancePtr;
XScuGic IntcInst;
SemaphoreHandle_t cl_logbuf_lock;

struct task_handler {
	tasks_register_t task_hdl;
	char 		 *task_name;
	int 		 task_index;
};

static struct task_handler task_handlers[] = {
	{CL_MSG_Launch, "Common Layer", 0},
	{RMGMT_Launch, "Rmgmt Service", 1},
	{VMC_Launch, "VMC Service", 2},
};

extern void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
	CL_ERR(APP_MAIN, "stack overflow %x %s\r\n", &xTask, (portCHAR *)pcTaskName);
	/* If the parameters have been corrupted then inspect pxCurrentTCB to
	 * identify which task has overflowed its stack.
	 */
	for (;;) { }
}

u32 cl_check_clock_shutdown_status(void)
{
	u32 shutdown_status = 0 ;

	//offset to read shutdown status
	shutdown_status = IO_SYNC_READ32(VMR_EP_UCS_CONTROL_STATUS_BASEADDR);

	return (shutdown_status & SHUTDOWN_LATCHED_STATUS);
}

void cl_system_pre_init(void)
{
	/* Init flash device */
	ospi_flash_init();

#ifndef VMR_BUILD_XRT_ONLY
#ifdef VMC_DEBUG
	/* Enable FreeRTOS Debug UART */
	UART_RTOS_Debug_Enable(&uart_log);
#endif

	if (XSysMonPsv_Init(&InstancePtr, &IntcInst) != XST_SUCCESS)
	{
		CL_LOG(APP_VMC, "Failed to init sysmon \n\r");
	}
#endif

}

u32 cl_rpu_status_query(struct cl_msg *msg, char *buf, u32 size)
{
	u32 count = 0;

#ifdef VMR_BUILD_XRT_ONLY
	CL_LOG(APP_MAIN, "Build flags: -XRT");
	count = snprintf(buf, size, "Build flags: -XRT\n");
#else
	CL_LOG(APP_MAIN, "Build flags: default full build");
	count = snprintf(buf, size, "Build flags: default full build\n");
#endif

	if (count > size) {
		CL_ERR(APP_MAIN, "msg is truncated");
		return size;
	}
	size = size - count;

#ifdef _VMR_VERSION_
	CL_LOG(APP_MAIN, "vitis version: %s", VMR_TOOL_VERSION);
	CL_LOG(APP_MAIN, "git hash: %s", VMR_GIT_HASH);
	CL_LOG(APP_MAIN, "git branch: %s", VMR_GIT_BRANCH);
	CL_LOG(APP_MAIN, "git hash date: %s", VMR_GIT_HASH_DATE);

	count += snprintf(buf + count, size,
		"vitis version: %s\ngit hash: %s\ngit branch: %s\ngit hash date: %s\n",
		VMR_TOOL_VERSION, VMR_GIT_HASH, VMR_GIT_BRANCH, VMR_GIT_HASH_DATE);
	if (count > size) {
		CL_ERR(APP_MAIN, "msg is truncated");
		return size;
	}
#endif

	return count;
}

u32 cl_apu_status_query(struct cl_msg *msg, char *buf, u32 size)
{
	u32 count = 0;

	//cl_xgq_apu_identify(msg);
	count = snprintf(buf, size, "is PS ready: %d\n", cl_xgq_apu_is_ready());

	return count;
}

int32_t VMC_SCFW_Program_Progress(void);

int flash_progress() {
	return VMC_SCFW_Program_Progress() ?
		VMC_SCFW_Program_Progress() : ospi_flash_progress();
}

static TaskHandle_t vmrMainTask_handle = NULL;
static void vmrMainTask(void *task_params)
{
	cl_msg_t msg = { 0 };
	u8 debug_type = CL_DBG_CLEAR;

	cl_rmgmt_fpt_get_debug_type(&msg, &debug_type);

	for (int i = 0; i < ARRAY_SIZE(task_handlers); i++) {
		if (debug_type != 0 &&
		    task_handlers[i].task_index == debug_type) {
			CL_ERR(APP_MAIN, "task [ %s ] - skipped", task_handlers[i].task_name);
			continue;
		}
		CL_LOG(APP_MAIN, "task [ %s ] - starting", task_handlers[i].task_name);
		configASSERT(task_handlers[i].task_hdl() == 0);
	}


	CL_LOG(APP_MAIN, "main task finished job.");
	vTaskDelete(NULL);
}

int main( void )
{
	cl_logbuf_lock = xSemaphoreCreateMutex();
	configASSERT(cl_logbuf_lock != NULL);

	CL_LOG(APP_MAIN, "\r\n=== VMR Starts ===");

	cl_system_pre_init();

	if (xTaskCreate( vmrMainTask,
			(const char *) "VMR Main Task",
			TASK_STACK_DEPTH,
			NULL,
			tskIDLE_PRIORITY + 1,
			&vmrMainTask_handle) != pdPASS) {
		CL_ERR(APP_MAIN, "Failed to create VMR Main Task");
		return -1;
	}

	vTaskStartScheduler();

	/* Should never go here unless there is not enough memory */
	CL_LOG(APP_MAIN, "not enough memory, exit.");
}

