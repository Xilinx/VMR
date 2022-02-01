/******************************************************************************
 * Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
 * SPDX-License-Identifier: MIT
 *******************************************************************************/
#include "xil_types.h"
#include "sensors/inc/qsfp.h"

#define BOARD_TEMPERATURE_SENSOR_NUM 2

//Slave Addresses
#define SLAVE_ADDRESS_SE98A_0 0x18
#define SLAVE_ADDRESS_SE98A_1 0x19
#define SLAVE_ADDRESS_MAX6639 0x2E

/*Power threshold limits for vck5000 */
#define POWER_12VPEX_CRITICAL_THRESHOLD 69.000
#define POWER_12VAUX_2X3_CRITICAL_THRESHOLD 78.000
#define POWER_12VAUX_2X4_CRITICAL_THRESHOLD 153.000

#define POWER_MODE_300W 3

typedef struct
{
	s32 board_temp[BOARD_TEMPERATURE_SENSOR_NUM];
	float local_temp;
	float remote_temp;
	float sysmon_max_temp;
	float qsfp_temp[QSFP_TEMPERATURE_SENSOR_NUM];
	u16 fanRpm;

} Versal_sensor_readings;


void se98a_monitor(void);
void max6639_monitor(void);
void qsfp_monitor(void);
