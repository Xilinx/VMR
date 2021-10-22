/******************************************************************************
 * Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
 * SPDX-License-Identifier: MIT
 *******************************************************************************/
#include "xil_types.h"

#define BOARD_TEMPERATURE_SENSOR_NUM 2

//Slave Addresses
#define SLAVE_ADDRESS_SE98A_0 0x18
#define SLAVE_ADDRESS_SE98A_1 0x19
#define SLAVE_ADDRESS_MAX6639 0x2E

typedef struct
{
	s32 board_temp[BOARD_TEMPERATURE_SENSOR_NUM];
	float local_temp;
	float remote_temp;
	u16 fanRpm;

} Versal_sensor_readings;


void se98a_monitor(void);
void max6639_monitor(void);
