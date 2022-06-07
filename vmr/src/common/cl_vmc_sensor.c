/******************************************************************************
* Copyright (C) 2022 Xilinx, Inc.  All rights reserved.
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
 * Note: the 2022.1_web flow is the legacy code flow that is revisited and
 *     refactored by newer 2022.2 Resilient VMR new flow. We document legacy
 *     flow for referrence.
 * 
 * 2022.1_web flow:
 * 1) Wait for sensor service ready:
 *   When ever there is a sensor read request, the first step should check if
 *   underneath service is ready or not. The cl_vmc_sensor will return -ENODEV
 *   if checking returns falures.
 *   1.1) Service ready flow:
 *        The sensor services will init all necessary services. Then,
 *        The sensor services will communicate SC and look for SENSOR_GOOD status.
 *        Only when SENSOR_GOOD is received, VMC_SC service is set to ready.
 *        (cl_vmc_sc_services_ready) 
 *   1.2) When VMC_SC service is ready, vmc_sensor request can request data
 *        from VMC_SC.
 * 2) Read vmc_sc data into Adsm sensor data.
 *   2.1) For v5k platform, the data is read by vmc_sc_comms first.
 *        Thus a lock is needed to handle concurrency between Adsm query task
 *        (this task) and vmc_sc update task. vmc_sc_monitoring_lock.
 *   2.2) vmc_sc fetch Data into local SC_VMC_Data (vmc_sc_monitoring_lock)
 *   2.3) Data is collected from extern SC_VMC_Data (vmc_sc_monitoring_lock)
 *        Inside Asdm it copies SC_VMC_Data into its own sensorRecord (sdr_lock)
 *        Asdm_Get_Sensor_Value
 *        Update_Sensor_Valued
 *        Asdm_Get_Sensor_Request(Repository|Value)
 *        sdr_lock is a internal lock among Asdm sensor APIs
 *
 * 2022.2 Resilient VMR new flow:
 * 1) check every 100ms for any critical hardware issues
 *     1.1) V5K: collect vmc_sc_comms data every 1m (1 out of 10 loops)
 *     1.2) collect data withouot vmc_sc for future platforms (e.g. V70)
 *
 * 2) locking
 *     2.1) [DELETE] vmc_sc_comms_lock is lock between VMC_SC_Comms and
 *          VMC_SC_Update(PROGRAM SCFW). Since the PROGRAM_SCFW will suspend
 *          cl_hardware_monitor task, this lock is not needed anymore.
 *     2.2) [DELETE] vmc_sensor_monitoring_lock is lock between VMC_SC_Comms and
 *          VMC_Sensor. Since these 2 tasks are merged in one call, this lock
 *          is not needed any more.
 *     2.3) [KEEP] vmc_sc_lock is lock between Asdm_Get_Senor_Value and
 *          vmc_sensors, keep it.
 *     2.4) [KEEP] sdr_lock is lock between Asdm_Get_Senor_Value and
 *          vmc_sensors, keep it.
 *
 *     Question: Why we need 2 locks for same type of operation?
 *
 * 3) hardware monitor flow
 *   cl_main ---> cl_vmc_init --+
 *                              |
 *                              +---> cl_I2CInit
 *                              +---> Read EEPROM
 *                              +---> max6639 for versal fan
 *                              +---> Init_Asdm
 *                              +---> Init sdr_lock
 *                              +---> Init vmc_sc_lock
 *                              +---> set vmc_is_ready = true
 *
 *   every 1000ms ---> cl_vmc_sc_update --+<---  go back and recheck <--------+
 *                                        |                                   |
 *                                        +---> check SENSOR_GOOD (if not) ---+
 *                                        | (if yes)
 *                                        |
 *                                        +---> Fetch POWERMODE
 *                                        +---> Fetch VMC_RESP
 *                                        +---> Fetch VOLT, POWER, TEMP, I2C
 *
 *   every 100ms  ---> cl_vmc_sensors --+<--- go back and recheck <-----+
 *                                      |                               |
 *                                      +---> vmc_sc_is_ready (not) --->+
 *                                      | (if yes == SENSOR_GOOD )
 *                                      |
 *                                      +---> Monitor_Thresholds ---+
 *                                      |                           +---> temp critical
 *                                      |                           +---> ucs clock shutdown
 *                                      +---> Monitor_Sensors ---+
 *                                                               |
 *                                                               +---> hold sdr_lock then copy
 *                                                                     from sc_vmc_data to
 *                                                                     sdrInfo.
 */

/*
 * Init all necessary services here prior to start creating task func.
 * When task_func started, everything should be good to access.
 */
int cl_vmc_sensor_init(void)
{
	return 0;
}

/*
 * When task func started, all init works should be done already.
 */
void cl_vmc_sensor_func(void *task_args)
{
	while (1) {
		/* vmc_sensors task */
		cl_vmc_monitor_sensors(); //20 ms

		/* every 100ms we should check hardware status */
		vTaskDelay(pdMS_TO_TICKS(100));
	}
}
