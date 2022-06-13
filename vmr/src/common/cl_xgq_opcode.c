/******************************************************************************
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/
/* FreeRTOS includes */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"

#include <stdbool.h>

#include "cl_log.h"
#include "cl_msg.h"
#include "cl_main.h"
#include "cl_flash.h"
#include "cl_rmgmt.h"
#include "cl_vmc.h"
#include "xgq_cmd_vmr.h"
#include "vmr_common.h"

static int opcode_log_page(cl_msg_t *msg)
{
	return cl_rmgmt_log_page(msg);
}

static int opcode_clock(cl_msg_t *msg)
{
	return cl_rmgmt_clock(msg);
}

static int opcode_vmr_control(cl_msg_t *msg)
{
	return cl_rmgmt_vmr_control(msg);
}

static int opcode_vmc_sensor(cl_msg_t *msg)
{
	return cl_vmc_sensor(msg);
}

struct opcode_handle {
	const char 	*msg_type_name;
	cl_msg_type_t	msg_type;
	int(*msg_cb)(cl_msg_t *arg);
} opcode_handles[] = {
	{"LOG PAGE", CL_MSG_LOG_PAGE, opcode_log_page},
	{"CLOCK", CL_MSG_CLOCK, opcode_clock},
	{"VMR CONTROL", CL_MSG_VMR_CONTROL, opcode_vmr_control},
	{"SENSOR", CL_MSG_SENSOR, opcode_vmc_sensor},
};

static void process_program_msg(cl_msg_t *msg)
{
	for (int i = 0; i < ARRAY_SIZE(opcode_handles); i++) {
		if (msg->hdr.type == opcode_handles[i].msg_type) {
			msg->hdr.rcode = opcode_handles[i].msg_cb(msg);
			cl_msg_handle_complete(msg);
			return;
		}
	}

	VMR_ERR("unhandled msg type %d", msg->hdr.type);
	msg->hdr.rcode = -EINVAL;
	cl_msg_handle_complete(msg);
}

int cl_xgq_opcode_init(void)
{
	return 0;
}

void cl_xgq_opcode_func(void *task_args)
{
	cl_msg_t msg = { 0 };

	for ( ;; ) {
		VMR_DBG("Wait for new messages ...");
		(void) cl_recv_from_queue(&msg, CL_QUEUE_OPCODE);
		process_program_msg(&msg);
	}
}
