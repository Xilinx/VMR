/******************************************************************************
* Copyright (C) 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#ifndef INC_PLATFORMS_V70_H_
#define INC_PLATFORMS_V70_H_

#include "../vmc_asdm.h"

#define V70_NUM_POWER_RAILS 	2
#define V70_PEX_12V_I_IN_THROTTLING_LIMIT	5500
#define V70_PEX_3V3_I_IN_THROTTLING_LIMIT	3000

#define V70_IDLE_POWER 			14250000 //9625000
#define V70_TEMP_GAIN_KP_FPGA		3.000e+06
#define V70_TEMP_GAIN_KI		1.000e+04
#define V70_TEMP_GAIN_KP_VCCINT		1.000e+07
#define V70_TEMP_GAIN_KAW 		2.400e-03

#define V70_INTEGRATION_SUM_INITIAL	7.300e+07

/*Power threshold limits for v70 */
#define POWER_12VPEX_CRITICAL_THRESHOLD 69.000
#define POWER_3V3PEX_CRITICAL_THRESHOLD 10.400

#define POWER_12VPEX_THROTTLING_THRESHOLD 66.000
#define POWER_3V3PEX_THROTTLING_THRESHOLD 9.900

#define V70_TEMP_VCCINT_CRITICAL_THRESHOLD  110.000

#define V70_TEMP_FPGA_CRITICAL_THRESHOLD_LIMIT	100.000

#define V70_POWER_CRITICAL_THRESHOLD_LIMIT	(POWER_12VPEX_CRITICAL_THRESHOLD + \
						POWER_3V3PEX_CRITICAL_THRESHOLD)

#define V70_TEMP_THROTTLING_THRESHOLD_LIMIT		97.000

#define V70_POWER_THROTTLING_THRESHOLD_LIMIT	(POWER_12VPEX_THROTTLING_THRESHOLD + \
						POWER_3V3PEX_THROTTLING_THRESHOLD)

#define V70_FPGA_THROTTLING_TEMP_LIMIT    95
#define V70_VCCINT_THROTTLING_TEMP_LIMIT 105

u8 V70_Init(void);
s8 V70_Temperature_Read_Inlet(snsrRead_t *snsrData);
s8 V70_Temperature_Read_Outlet(snsrRead_t *snsrData);
s8 V70_Temperature_Read_Board(snsrRead_t *snsrData);
s32 V70_VMC_Fetch_BoardInfo(u8 *board_snsr_data);
s8 V70_Asdm_Read_Power(snsrRead_t *snsrData);

s8 V70_Get_Voltage_Names(u8 index, char8* snsrName, u8 *sensorId,sensorMonitorFunc *sensor_handler);
s8 V70_Get_Current_Names(u8 index, char8* snsrName, u8 *sensorId,sensorMonitorFunc *sensor_handler);

#endif
