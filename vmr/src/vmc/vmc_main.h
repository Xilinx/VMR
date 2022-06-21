/******************************************************************************
 * Copyright (C) 2020-2022 Xilinx, Inc.  All rights reserved.
 * SPDX-License-Identifier: MIT
 *******************************************************************************/

#ifndef INC_VMC_MAIN_H_
#define INC_VMC_MAIN_H_

#include "vmc_sensors.h"

typedef struct Vmc_Global_Variable {
	Versal_sensor_readings 	sensor_readings;
	u8    			logging_level;
} Vmc_Global_Variables;

#endif
