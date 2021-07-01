/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#ifndef COMMON_MAIN_H
#define COMMON_MAIN_H

/**
 * Application type id for logging, mem signature etc.
 */
typedef enum app_type {
	APP_MAIN = 0,
	APP_RMGMT,
	APP_VMC,
} app_type_t;

/**
 * 1) Applications should add callback function definition
 * below and into handler[] array;
 * 
 * 2) Applications then implements their own tasks creation
 * inside callback function, and do any necessary initialization.
 *
 * FreeRTOS main will call all callback within handler[]
 * array, then call vTaskStartScheduler() to start all
 * tasks;
 */
typedef int (*tasks_register_t)(void); 

int CMC_Launch(void);
int RMGMT_Launch(void);

static tasks_register_t handler[] = {
	CMC_Launch,	
	RMGMT_Launch,
};

#define ARRAY_SIZE(x) (sizeof(x) / sizeof (*(x)))

#endif
