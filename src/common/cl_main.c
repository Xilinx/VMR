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

#include "sysmon.h"

int ospi_flash_init();
int VMC_Launch(void);
int RMGMT_Launch(void);
int CL_MSG_launch(void);

uart_rtos_handle_t uart_log;
XSysMonPsv InstancePtr;
XScuGic IntcInst;
SemaphoreHandle_t cl_logbuf_lock;

static tasks_register_t handler[] = {
	CL_MSG_launch, /* make sure CL MSG launched first */
	RMGMT_Launch,
#ifndef VMR_BUILD_XRT_ONLY
	VMC_Launch,	
#endif
};

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

void cl_log_system_info()
{
#ifdef VMR_BUILD_XRT_ONLY
	CL_LOG(APP_MAIN, "XRT only build");
#else
	CL_LOG(APP_MAIN, "VMR full build");
#endif
	CL_LOG(APP_MAIN,
		"\r\ngit hash: %s.\r\ngit branch: %s.\r\nbuild date: %s",
		VMR_GIT_HASH, VMR_GIT_BRANCH, VMR_GIT_BUILD_DATE);
}

int main( void )
{
	cl_logbuf_lock = xSemaphoreCreateMutex();
	configASSERT(cl_logbuf_lock != NULL);

	CL_LOG(APP_MAIN, "\r\n=== VMR Starts  ===");

	cl_log_system_info();

	cl_system_pre_init();

	for (int i = 0; i < ARRAY_SIZE(handler); i++) {
		configASSERT(handler[i]() == 0);
	}

	vTaskStartScheduler();

	/* Should never go here unless there is not enough memory */
	CL_LOG(APP_MAIN, "not enough memory, exit.");
}
