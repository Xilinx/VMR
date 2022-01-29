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

int ospi_flash_init(void);
int VMC_Launch(void);
int RMGMT_Launch(void);
int CL_MSG_launch(void);
void cl_system_pre_init(void);

struct cl_msg;

void cl_rpu_status_query(struct cl_msg *msg);
void cl_apu_status_query(struct cl_msg *msg);
int cl_xgq_client_probe(void);
int cl_xgq_apu_is_ready(void);
int cl_xgq_apu_identify(struct cl_msg *msg);
int cl_xgq_apu_download_xclbin(char *data, u32 size);

#endif
