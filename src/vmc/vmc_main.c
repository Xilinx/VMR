/******************************************************************************
 * Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
 * SPDX-License-Identifier: MIT
 *******************************************************************************/

#include "FreeRTOS.h"
#include "task.h"

#include "cl_uart_rtos.h"
#include "cl_log.h"
#include "cl_i2c.h"
#include "vmc_api.h"

static TaskHandle_t xVMCTask;
static TaskHandle_t xSensorMonTask;

int Enable_DemoMenu(void);

static void SensorMonitorTask(void *params)
{
	VMC_LOG("\n\rSensor Monitor Task Created !!!\n\r");
}

static void pVMCTask(void *params)
{
    /* Platform Init will Initialise I2C, GPIO, SPI, etc
     */

	VMC_LOG("\n\r VMC launched \n\r");

    /* Demo menu log is enabled */
	Enable_DemoMenu();

    I2CInit();

    /* Read the EEPROM */
    
    /* Start Sensor Monitor task */

    if (xTaskCreate( SensorMonitorTask,
		( const char * ) "Sensor_Monitor",
		2048,
		NULL,
		tskIDLE_PRIORITY + 1,
		&xSensorMonTask
		) != pdPASS) {
	CL_LOG(APP_VMC,"Failed to Create Sensor Monitor Task \n\r");
	return -1;
    }
}

int VMC_Launch( void )
{
    if (xTaskCreate( pVMCTask,
		( const char * ) "VMC",
		2048,
		NULL,
		tskIDLE_PRIORITY + 1,
		&xVMCTask
		) != pdPASS) {
	CL_LOG(APP_VMC,"Failed to Create VMC Task \n\r");
	return -1;
    }

    return 0;
}

