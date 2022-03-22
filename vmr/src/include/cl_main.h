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
#define SHUTDOWN_LATCHED_STATUS 0x01

int ospi_flash_init(void);
int VMC_Launch(void);
int RMGMT_Launch(void);
int CL_MSG_launch(void);
void cl_system_pre_init(void);
u32 cl_check_clock_shutdown_status(void);

struct cl_msg;

u32 cl_rpu_status_query(struct cl_msg *msg, char *buf, u32 size);
u32 cl_apu_status_query(struct cl_msg *msg, char *buf, u32 size);

int cl_xgq_client_probe(void);
int cl_xgq_apu_is_ready(void);
int cl_xgq_pl_is_ready(void);
int cl_xgq_apu_identify(struct cl_msg *msg);
int cl_xgq_apu_download_xclbin(char *data, u32 size);

#define TASK_STACK_DEPTH 0x10000 /* 64k * sizeof(word) = 256k */
int flash_progress(void);
int32_t VMC_SCFW_Program_Progress(void);

#endif
