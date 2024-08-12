/******************************************************************************
 * Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
 * SPDX-License-Identifier: MIT
 *******************************************************************************/

#ifndef INC_VMC_SENSORS_H_
#define INC_VMC_SENSORS_H_

#include "cl_config.h"
#include "xil_types.h"
#include "qsfp.h"

#ifdef BUILD_FOR_RMI
#include "RMI/rmi_sensors.h"
#endif

#define BOARD_TEMPERATURE_SENSOR_NUM 2
#define MAX_COUNT_TO_WAIT_1SEC 10

#define POWER_MODE_300W 3
#define LPD_I2C_0	0x1
#define REDUCE_GAPPING_DEMAND_RATE_TO_FIVE_PERCENTAGE 0x07

typedef enum {
	e12V_PEX = 0,
	e3V3_PEX,
	e3V3_AUX,
	eVCCINT,
	e12V_AUX0,
	e12V_AUX1,
	e1V5_VCC_AUX,
	e3V3_QSFP,
	eVCC_HBM,
	e1V2_VCCO_DIMM,
	e1V2_GTXAVTT,
	eElectrical_Sensor_Max,
}eElectrical_sensors_t;

typedef struct
{
	s16 board_temp[BOARD_TEMPERATURE_SENSOR_NUM];
	float local_temp;
	float remote_temp;
	float sysmon_max_temp;
	float vccint_temp;
	float fVccHbmTemp;
	float f1V2VccoDimmTemp;
	float f1V2GtxaVttTemp;
	float qsfp_temp[QSFP_TEMPERATURE_SENSOR_NUM];
	u16 fanRpm;
	float voltage[eElectrical_Sensor_Max];
	float current[eElectrical_Sensor_Max];
	float total_power;

} Versal_sensor_readings;

void ucs_clock_shutdown();

#ifdef BUILD_FOR_RMI
sensors_ds_t* Vmc_Init_RMI_Sensor_Buffer(u32 counts);
#endif

#endif
