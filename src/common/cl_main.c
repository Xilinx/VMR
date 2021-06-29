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
#include "xil_printf.h"

int main( void )
{
	//CMC_Launch();

	//RMGMT_Launch();
	
	int len = sizeof (handler) / sizeof(*handler);

	for (int i = 0; i < len; i++) {
		configASSERT(handler[i]() == 0);
	}

	vTaskStartScheduler();

	/* Should never go here unless there is not enough memory */
	xil_printf( "not enough memory\n" );
}
