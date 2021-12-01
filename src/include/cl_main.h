/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#ifndef COMMON_MAIN_H
#define COMMON_MAIN_H

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

#define ARRAY_SIZE(x) (sizeof(x) / sizeof (*(x)))

void cl_system_pre_init(void);

#endif
