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

/*
 * Run SCFW program till finish and return result by xgq complete.
 *
 * xgq request  --> SCFW_REQ queue  --> cl_vmc_sc_comms --> SCFW_program --+
 *                                                                         |
 * xgq complete <-- SCFW_RESP queue <---------------------- return val   --+
 *                                                                         |
 *                                         cl_vmc_sc_comms task continues  V
 */
static int program_scfw(cl_msg_t *msg)
{
	int ret = 0;

	ret = cl_send_to_queue(msg, CL_QUEUE_SCFW_REQ);
	if (ret)
		return ret;

	ret = cl_recv_from_queue(msg, CL_QUEUE_SCFW_RESP);
	if (ret)
		return ret;

	return cl_msg_get_rcode(msg);
}

struct program_handle {
	const char 	*msg_type_name;
	cl_msg_type_t	msg_type;
	int(*msg_cb)(cl_msg_t *arg);
} program_handles[] = {
	{"DOWNLOAD BASE PDI", CL_MSG_PDI, cl_rmgmt_program_base_pdi},
	{"DOWNLOAD APU PDI", CL_MSG_APUBIN, cl_rmgmt_program_apu_pdi},
	{"DOWNLOAD XCLBIN", CL_MSG_XCLBIN, cl_rmgmt_program_xclbin},
	{"PROGRAM SCFW", CL_MSG_PROGRAM_SCFW, program_scfw},
	{"PROGRAM VMR", CL_MSG_PROGRAM_VMR, cl_rmgmt_program_vmr},
};

static void process_program_msg(cl_msg_t *msg)
{
	for (int i = 0; i < ARRAY_SIZE(program_handles); i++) {
		if (msg->hdr.type == program_handles[i].msg_type) {
			/* The program_vmr will reset subsystem, thus we complete cmd first */
			if (msg->hdr.type == CL_MSG_PROGRAM_VMR) {
				msg->hdr.rcode = 0;
				cl_msg_handle_complete(msg);
				program_handles[i].msg_cb(msg);
				return;
			}

			msg->hdr.rcode = program_handles[i].msg_cb(msg);
			cl_msg_handle_complete(msg);
			return;
		}
	}

	VMR_ERR("unhandled msg type %d", msg->hdr.type);
	msg->hdr.rcode = -EINVAL;
	cl_msg_handle_complete(msg);
}

int cl_xgq_program_init(void)
{
	return 0;
}

void cl_xgq_program_func(void *task_args)
{
	cl_msg_t msg;

	for ( ;; ) {
		VMR_DBG("Wait for new messages ...");
		(void) cl_recv_from_queue(&msg, CL_QUEUE_PROGRAM);
		process_program_msg(&msg);
	}
}
