/******************************************************************************
 * Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
 * SPDX-License-Identifier: MIT
 *******************************************************************************/

#include "FreeRTOS.h"
#include "task.h"

#include "cl_msg.h"
#include "cl_log.h"
#include "cl_i2c.h"
#include "cl_uart_rtos.h"
#include "vmc_api.h"
#include "vmc_sensors.h"
#include "sensors/inc/m24c128.h"
#include "sensors/inc/max6639.h"
#include "vmc_asdm.h"


static TaskHandle_t xVMCTask;
static TaskHandle_t xSensorMonTask;

/*Xgq Msg Handle */
static u8 xgq_sensor_flag = 0;
msg_handle_t *sensor_hdl;
#define EP_RING_BUFFER_BASE 	0x38000000

int Enable_DemoMenu(void);
extern s8 Asdm_Get_Sensor_Repository(u8 *req, u8 *resp, u16 *respSize);

static int xgq_sensor_cb(cl_msg_t *msg, void *arg)
{
	u32 address = EP_RING_BUFFER_BASE + (u32)msg->data_payload.address;
	u32 size = msg->data_payload.size;
	u8 reqBuffer[2] = {0};
	u8 respBuffer[255] = {0};
	u16 respSize = 0;
	s32 ret = 0;

	reqBuffer[0] = msg->log_payload.pid;
	if(Asdm_Get_Sensor_Repository(&reqBuffer[0], &respBuffer[0], &respSize))
	{
		VMC_LOG("ERROR: unknown pid %d", msg->log_payload.pid);
		ret = -1;
	}
	else
	{
		if(size < respSize)
		{
			VMC_LOG("ERROR: Expected Size %d Actual Size: %d", size, respSize);
			ret = -1;
		}
		else
		{
			cl_memcpy_toio8(address, &respBuffer[0], respSize);
		}
	}


	msg->hdr.rcode = ret;
	VMC_DBG("complete msg id%d, ret %d", msg->hdr.cid, ret);
	cl_msg_handle_complete(msg);
	return 0;
}


static void SensorMonitorTask(void *params)
{
	VMC_LOG("\n\rSensor Monitor Task Created !!!\n\r");

	if (xgq_sensor_flag == 0 &&
		cl_msg_handle_init(&sensor_hdl, CL_MSG_SENSOR, xgq_sensor_cb, NULL) == 0) {
		VMC_LOG("init sensor handle done.");
		xgq_sensor_flag = 1;
	}

	if(Init_Asdm())
	{
		 VMC_ERR(" ASDM Init Failed \n\r");
	}

	for(;;)
	{
		/* Read All Sensors */
		Monitor_Sensors();

		/* Todo: Replace once ASDM is stable */
		se98a_monitor();
		max6639_monitor();

		vTaskDelay(500);
	}



	vTaskSuspend(NULL);
}

static void pVMCTask(void *params)
{
    /* Platform Init will Initialise I2C, GPIO, SPI, etc */
	
    VMC_LOG("\n\r VMC launched \n\r");


    I2CInit();

    /* Demo menu log is enabled */    
    Enable_DemoMenu();

    /* Read the EEPROM */
    Versal_EEPROM_ReadBoardIno();

    /* Retry till fan controller is programmed */
    while (!max6639_init(1, 0x2E));  // only for vck5000
    
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

