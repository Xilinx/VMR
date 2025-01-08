/******************************************************************************
* Copyright (C) 2024 Advanced Micro Devices, Inc.    All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <cmocka.h>
#include <stdlib.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

int lock = 0;

/*****************************Internal functions*******************************/
/* Used to assert lock value in testcase*/
int get_sem_lock()
{
	return lock;
}

/* Used to reset lock value in testcase*/
int set_sem_lock()
{
    lock = 0;
}


/*****************************Mock functions *******************************/
BaseType_t __wrap_xQueueSemaphoreTake( QueueHandle_t xQueue, TickType_t xTicksToWait )
{
	lock++;
	/*Returns the value passed to 'will_return()' written in testcase */
	return mock_type(BaseType_t); 
}

BaseType_t __wrap_xQueueGenericSend( QueueHandle_t xQueue, const void * const pvItemToQueue, TickType_t xTicksToWait, const BaseType_t xCopyPosition )
{
	lock--;
	/*Returns the value passed to 'will_return()' written in testcase */
	return mock_type(BaseType_t);
}

void  __wrap_vTaskDelay( const TickType_t xTicksToDelay )
{
    return;
}
