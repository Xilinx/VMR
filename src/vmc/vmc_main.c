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

#include "sysmon.h"
#include "vmc_sc_comms.h"

static TaskHandle_t xVMCTask;
static TaskHandle_t xSensorMonTask;
static TaskHandle_t xVMCSCTask;
static TaskHandle_t xVMCUartpoll;

extern uart_rtos_handle_t uart_vmcsc_log;
extern SC_VMC_Data sc_vmc_data;
extern SemaphoreHandle_t vmc_sc_lock;



u8 g_scData[MAX_VMC_SC_UART_BUF_SIZE] = {0x00};
u16 g_scDataCount = 0;
bool isinterruptflag ;
bool ispacketreceived ;

/*Xgq Msg Handle */
static u8 xgq_sensor_flag = 0;
msg_handle_t *sensor_hdl;
#define EP_RING_BUFFER_BASE 	0x38000000

int Enable_DemoMenu(void);
extern s8 Asdm_Process_Sensor_Request(u8 *req, u8 *resp, u16 *respSize);

static int xgq_sensor_cb(cl_msg_t *msg, void *arg)
{
	u32 address = EP_RING_BUFFER_BASE + (u32)msg->data_payload.address;
	u32 size = msg->data_payload.size;
	u8 reqBuffer[2] = {0};
	u8 respBuffer[512] = {0};
	u16 respSize = 0;
	s32 ret = 0;

	reqBuffer[0] = msg->log_payload.pid;
	if(Asdm_Process_Sensor_Request(&reqBuffer[0], &respBuffer[0], &respSize))
	{
		VMC_LOG("ERROR: Failed to Process Sensor Request %d", msg->log_payload.pid);
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
static void Vmc_uartpoll( void *pvParameters )
{
	u32 retval;
	u8 Data[MAX_VMC_SC_UART_BUF_SIZE] = {0x00};
	u32 receivedcount = 0 ;
	for(;;)
	{
		if(isinterruptflag == true)
		{
			vPortEnableInterrupt(XPAR_XUARTPS_0_INTR);
			retval = UART_RTOS_Receive(&uart_vmcsc_log, Data, MAX_VMC_SC_UART_COUNT_WITH_INTR ,&receivedcount);
			if(retval)
			{
				memcpy(&g_scData[g_scDataCount],Data,retval);
				g_scDataCount += retval;
				retval = 0;
				if ((g_scData[g_scDataCount-1] == ETX) && (g_scData[g_scDataCount-2] == ESCAPE_CHAR))
				{
					ispacketreceived = true;
				}
			}
			if(receivedcount)
			{
				memcpy(&g_scData[g_scDataCount],Data,receivedcount);
				g_scDataCount += receivedcount;
				receivedcount = 0;
				if ((g_scData[g_scDataCount-1] == ETX) && (g_scData[g_scDataCount-2] == ESCAPE_CHAR))
				{
					ispacketreceived = true;
				}
			}
		}
		else if(isinterruptflag == false)
		{
			vPortDisableInterrupt(XPAR_XUARTPS_0_INTR);
			retval = UART_RTOS_Receive(&uart_vmcsc_log, Data, MAX_VMC_SC_UART_COUNT_WITHOUT_INTR ,&receivedcount);
			if(retval)
			{
				memcpy(&g_scData[g_scDataCount],Data,retval);
				g_scDataCount += retval;
				retval = 0;
				if ((g_scData[g_scDataCount-1] == ETX) && (g_scData[g_scDataCount-2] == ESCAPE_CHAR))
				{
					ispacketreceived = true;
				}
			}
		}
		vTaskDelay(20);
	}
	vTaskSuspend(NULL);
}

static void VMCSCMonitorTask(void *params)
{

	VMC_LOG("\n\r CMC Task Created !!!\n\r");



	xTaskCreate( Vmc_uartpoll,
						 ( const char * ) "UART_POLL",
						 2048,
						 NULL,
						 tskIDLE_PRIORITY + 1,
						 &xVMCUartpoll );

    /* vmc_sc_lock */
	vmc_sc_lock = xSemaphoreCreateMutex();
    if(vmc_sc_lock == NULL){
	VMC_ERR("vmc_sc_lock creation failed \n\r");
    }

	for(;;)
	{
		vmc_sc_monitor();
		vTaskDelay(50);
	}

	vTaskSuspend(NULL);
}

static void SensorMonitorTask(void *params)
{

	VMC_LOG(" Sensor Monitor Task Created !!!\n\r");

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
		sysmon_monitor();
		vTaskDelay(200);
	}



	vTaskSuspend(NULL);
}

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

    if (xTaskCreate( VMCSCMonitorTask,
		( const char * ) "VMCSC_Monitor",
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

