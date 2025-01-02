/******************************************************************************
 * Copyright (C) 2024 Advanced Micro Devices, Inc.    All rights reserved.
 * SPDX-License-Identifier: MIT
 *******************************************************************************/

#ifndef INC_VMC_MAIN_H_
#define INC_VMC_MAIN_H_

#include "vmc_sensors.h"

/*
 * To support MPU, each task should keep their own variables within
 * a certain memory address space, so that it won't be accessible
 * by other tasks.
 * There are many global variables in vmc tasks, thus
 * we define one global struct for each task.
 */

/* Global variables for cl_vmc_monitor_sensors task */
typedef struct Vmc_Sensors_Global_Variables {
	Versal_sensor_readings 	sensor_readings;
	u8    			logging_level;
	u8			clk_throttling_enabled;
} Vmc_Sensors_Gl_t;

ePlatformType Vmc_Get_PlatformType(void);

#endif
