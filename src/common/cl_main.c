/******************************************************************************
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/
/* FreeRTOS includes */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"

#include "cl_main.h"
#include "cl_log.h"

#include "uart_rtos.h"

int ospi_flash_init();

int VMC_Launch(void);
int RMGMT_Launch(void);
int cl_msg_service_launch(void);

static tasks_register_t handler[] = {
	VMC_Launch,	
	RMGMT_Launch,
	cl_msg_service_launch,
};

uart_rtos_handle_t uart_log;

int main( void )
{
	CL_DBG(APP_MAIN, "->");

	/* Init flash service */
	ospi_flash_init();

	/* Enable FreeRTOS Debug UART */
	UART_RTOS_Debug_Enable(&uart_log);

	/* Init application tasks */
	for (int i = 0; i < ARRAY_SIZE(handler); i++) {
		configASSERT(handler[i]() == 0);
	}

	CL_LOG(APP_MAIN, "start scheduler");
	vTaskStartScheduler();

	/* Should never go here unless there is not enough memory */
	CL_LOG(APP_MAIN, "not enough memory, exit.");
}
