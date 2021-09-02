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

int CMC_Launch(void);
int RMGMT_Launch(void);

static tasks_register_t handler[] = {
	//CMC_Launch,	
	RMGMT_Launch,
};

uart_rtos_handle_t uart_log;

int main( void )
{
	/* Init flash device */
	ospi_flash_init();

	/* Enable FreeRTOS Debug UART */
	UART_RTOS_Debug_Enable(&uart_log);

	for (int i = 0; i < ARRAY_SIZE(handler); i++) {
		configASSERT(handler[i]() == 0);
	}

	vTaskStartScheduler();

	/* Should never go here unless there is not enough memory */
	CL_LOG(APP_MAIN, "not enough memory, exit.");
}
