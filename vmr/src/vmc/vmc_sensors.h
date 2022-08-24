/******************************************************************************
 * Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
 * SPDX-License-Identifier: MIT
 *******************************************************************************/

#ifndef INC_VMC_SENSORS_H_
#define INC_VMC_SENSORS_H_

#include "xil_types.h"
#include "sensors/inc/qsfp.h"

#define BOARD_TEMPERATURE_SENSOR_NUM 2
#define MAX_COUNT_TO_WAIT_1SEC 10

#define POWER_MODE_300W 3
#define LPD_I2C_0	0x1

typedef enum {
	e12V_PEX = 0,
	e3V3_PEX,
	e3V3_AUX,
	eVCCINT,
	eElectrical_Sensor_Max,
}eElectrical_sensors_t;

typedef struct
{
	s16 board_temp[BOARD_TEMPERATURE_SENSOR_NUM];
	float local_temp;
	float remote_temp;
	float sysmon_max_temp;
	float vccint_temp;
	float qsfp_temp[QSFP_TEMPERATURE_SENSOR_NUM];
	u16 fanRpm;
	float voltage[eElectrical_Sensor_Max];
	float current[eElectrical_Sensor_Max];
	float total_power;

} Versal_sensor_readings;

void ucs_clock_shutdown();

#endif
