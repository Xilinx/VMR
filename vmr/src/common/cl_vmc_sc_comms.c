/******************************************************************************
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/
/* FreeRTOS includes */
#include "FreeRTOS.h"
#include "task.h"

#include "cl_log.h"
#include "cl_msg.h"
#include "cl_main.h"
#include "cl_rmgmt.h"
#include "cl_vmc.h"
#include "vmr_common.h"

/*
 *   every 1000ms ---> cl_vmc_sc_update --+<---  go back and recheck <--------+
 *                                        |                                   |
 *                                        +---> check SENSOR_GOOD (if not) ---+
 *                                        | (if yes)
 *                                        |
 *                                        +---> Fetch POWERMODE
 *                                        +---> Fetch VMC_RESP
 *                                        +---> Fetch VOLT, POWER, TEMP, I2C
 */

static void process_scfw_msg(cl_msg_t *msg)
{
	/* this is a blocking call */
	(void) cl_vmc_scfw_program(msg);
}

/*
 * Init all necessary services here prior to start creating task func.
 * When task_func started, everything should be good to access.
 */
int cl_vmc_sc_comms_init(void)
{
	cl_vmc_scfw_init();

	return 0;
}

/*
 * When task func started, all init works should be done already.
 */
void cl_vmc_sc_comms_func(void *task_args)
{
	cl_msg_t msg;

	while (1) {
		if (cl_recv_from_queue_nowait(&msg, CL_QUEUE_SC) == 0) {
			process_scfw_msg(&msg);
		}

		/* every 1000ms we update sensor from vmc_sc_comms */
		cl_vmc_sc_update();

		/* every 1000ms we should check hardware status */
		vTaskDelay(pdMS_TO_TICKS(1000));
	}
}
