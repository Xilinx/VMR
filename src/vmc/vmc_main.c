/******************************************************************************
 * Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
 * SPDX-License-Identifier: MIT
 *******************************************************************************/

#include "FreeRTOS.h"
#include "task.h"

#include "cl_log.h"
#include "cl_i2c.h"
#include "cl_uart_rtos.h"
#include "vmc_api.h"
#include "vmc_sensors.h"
#include "sensors/inc/m24c128.h"
#include "sensors/inc/max6639.h"
#include "vmc_asdm.h"

#include "sysmon.h"
#include "vmc_sc_comms.h"

/* Task Handles */
static TaskHandle_t xVMCTask;
TaskHandle_t xSensorMonTask;
TaskHandle_t xVMCSCTask;
TaskHandle_t xVMCUartpoll;
TaskHandle_t xVMCTaskMonitor;

SemaphoreHandle_t vmc_sc_lock;

int Enable_DemoMenu(void);

extern void VMC_SC_CommsTask(void *params);
extern void SensorMonitorTask(void *params);

static void pVMCTask(void *params)
{
    /* Platform Init will Initialise I2C, GPIO, SPI, etc */
	
    VMC_LOG(" VMC launched \n\r");

    I2CInit();

#ifdef VMC_DEBUG
    /* Demo menu log is enabled */    
    Enable_DemoMenu();
#endif

    /* Read the EEPROM */
    Versal_EEPROM_ReadBoardIno();

    /* Retry till fan controller is programmed */
    while (max6639_init(1, 0x2E));  // only for vck5000
    
    /* Start Sensor Monitor task */

    if (xTaskCreate( SensorMonitorTask,
		( const char * ) "Sensor_Monitor",
		2048,
		NULL,
		tskIDLE_PRIORITY + 1,
		&xSensorMonTask
		) != pdPASS) {
	CL_LOG(APP_VMC,"Failed to Create Sensor Monitor Task \n\r");
	return ;
    }

    /* vmc_sc_lock */
    vmc_sc_lock = xSemaphoreCreateMutex();
    if(vmc_sc_lock == NULL){
    VMC_ERR("vmc_sc_lock creation failed \n\r");
    }
    xSemaphoreGive(vmc_sc_lock);

    if (xTaskCreate( VMC_SC_CommsTask,
		( const char * ) "VMC_SC_Comms",
		2048,
		NULL,
		tskIDLE_PRIORITY + 1,
		&xVMCSCTask
		) != pdPASS) {
	CL_LOG(APP_VMC,"Failed to Create VMCSC Monitor Task \n\r");
	return ;
    }
    vTaskSuspend(NULL);
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

