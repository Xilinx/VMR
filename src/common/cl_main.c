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

static tasks_register_t handler[] = {
	//CMC_Launch,	
	RMGMT_Launch,
};

int main( void )
{
	/* Init flash device */
	ospi_flash_init();

	for (int i = 0; i < ARRAY_SIZE(handler); i++) {
		configASSERT(handler[i]() == 0);
	}

	vTaskStartScheduler();

	/* Should never go here unless there is not enough memory */
	CL_LOG(APP_MAIN, "not enough memory, exit.");
}
