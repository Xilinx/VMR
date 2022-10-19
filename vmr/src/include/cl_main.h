/*****************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#ifndef COMMON_MAIN_H
#define COMMON_MAIN_H

#define ARRAY_SIZE(x) (sizeof(x) / sizeof (*(x)))

int cl_xgq_receive_init(void);
int cl_xgq_program_init(void);
int cl_xgq_opcode_init(void);
int cl_vmc_sensor_init(void);
int cl_vmc_sc_comms_init(void);
int cl_uart_demo_init(void);

void cl_xgq_receive_func(void *task_args);
void cl_xgq_program_func(void *task_args);
void cl_xgq_opcode_func(void *task_args);
void cl_vmc_sensor_func(void *task_args);
void cl_vmc_sc_comms_func(void *task_args);
void cl_uart_demo_func(void *task_args);

struct cl_msg;
enum cl_queue_id {
	CL_QUEUE_PROGRAM = 0,
	CL_QUEUE_OPCODE,
	CL_QUEUE_SCFW_REQ,
	CL_QUEUE_SCFW_RESP,
	CL_QUEUE_SENSOR_REQ,
	CL_QUEUE_SENSOR_RESP,
};
int cl_send_to_queue(struct cl_msg *msg, enum cl_queue_id qid);
int cl_recv_from_queue(struct cl_msg *msg, enum cl_queue_id qid);
int cl_recv_from_queue_nowait(struct cl_msg *msg, enum cl_queue_id qid);

#endif
