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

int ospi_flash_init();

int CMC_Launch(void);
int RMGMT_Launch(void);
int cl_msg_service_launch(void);

static tasks_register_t handler[] = {
	//CMC_Launch,	
	RMGMT_Launch,
	cl_msg_service_launch,
};

int main( void )
{
	CL_DBG(APP_MAIN, "->");

	/* Init flash service */
	ospi_flash_init();

	/* Init application tasks */
	for (int i = 0; i < ARRAY_SIZE(handler); i++) {
		configASSERT(handler[i]() == 0);
	}

	CL_LOG(APP_MAIN, "start scheduler");
	vTaskStartScheduler();

	/* Should never go here unless there is not enough memory */
	CL_LOG(APP_MAIN, "not enough memory, exit.");
}
