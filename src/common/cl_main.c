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
#include "../vmc/vmc_api.h"
#include "sysmon.h"

int ospi_flash_init();
int VMC_Launch(void);
int RMGMT_Launch(void);
int CL_MSG_launch(void);

uart_rtos_handle_t uart_log;
uart_rtos_handle_t uart_vmcsc_log;

XSysMonPsv InstancePtr;
XScuGic IntcInst;
SemaphoreHandle_t cl_logbuf_lock;

static tasks_register_t handler[] = {
#ifndef VMR_BUILD_VMC_ONLY
	CL_MSG_launch, /* make sure CL MSG launched first */
	RMGMT_Launch,
#endif
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

	UART_VMC_SC_Enable(&uart_vmcsc_log);

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

#ifdef _VMR_VERSION_
	CL_LOG(APP_MAIN,
		"\r\ngit hash: %s.\r\ngit branch: %s.\r\nbuild date: %s",
		VMR_GIT_HASH, VMR_GIT_BRANCH, VMR_GIT_BUILD_DATE);
#endif

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
