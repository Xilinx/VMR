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

static int process_scfw_msg(cl_msg_t *msg)
{
	/* this is a blocking call */
	return (cl_vmc_scfw_program(msg));
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
	int ret = 0;

	while (1) {
		if (cl_recv_from_queue_nowait(&msg, CL_QUEUE_SCFW_REQ) == 0) {
			ret = process_scfw_msg(&msg);
			/* set correct rcode based on scfw program */
			VMR_ERR("SCFW Update return code: %02x", ret);
			cl_msg_set_rcode(&msg, ret);
			/* send msg back via SCFW Response Queue */
			(void) cl_send_to_queue(&msg, CL_QUEUE_SCFW_RESP);
		}

		/* every 1000ms we update sensor from vmc_sc_comms */
		cl_vmc_sc_tx_rx_data();

		/* every 1000ms we should check hardware status */
		vTaskDelay(pdMS_TO_TICKS(1000));
	}
}
