/******************************************************************************
 * Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
 * SPDX-License-Identifier: MIT
 *******************************************************************************/

#include "FreeRTOS.h"
#include "task.h"

#include "cl_uart_rtos.h"
#include "cl_i2c.h"
#include "cl_log.h"
#include "cl_msg.h"
#include "vmc_api.h"
#include "sensors/inc/se98a.h"
#include "sensors/inc/max6639.h"
#include "sensors/inc/qsfp.h"
#include "vmc_sensors.h"
#include "vmc_asdm.h"
#include "sysmon.h"
#include "vmc_sc_comms.h"

extern TaskHandle_t xSensorMonTask;
extern XSysMonPsv InstancePtr;
extern XScuGic IntcInst;

extern SC_VMC_Data sc_vmc_data;

/*Xgq Msg Handle */
static u8 xgq_sensor_flag = 0;
msg_handle_t *sensor_hdl;
#define EP_RING_BUFFER_BASE     0x38000000
extern s8 Asdm_Process_Sensor_Request(u8 *req, u8 *resp, u16 *respSize);

#define MAX6639_FAN_TACHO_TO_RPM(x) (8000*60)/(x*2)

extern uint8_t sc_update_flag;

Versal_sensor_readings sensor_readings;
u8 i2c_num = 1;  // LPD_I2C0
#define LPD_I2C_0	0x1

s8 Temperature_Read_Inlet(snsrRead_t *snsrData)
{
	s8 status = XST_FAILURE;
	s32 tempValue = 0;

	status = SE98A_ReadTemperature(LPD_I2C_0, SLAVE_ADDRESS_SE98A_0, &tempValue);
	if (status == XST_SUCCESS)
	{
		memcpy(snsrData->snsrValue,&tempValue,sizeof(tempValue));
		snsrData->sensorValueSize = sizeof(tempValue);
		snsrData->snsrSatus = Vmc_Snsr_State_Normal;
	}
	else
	{
		snsrData->snsrSatus = Vmc_Snsr_State_Comms_failure;
		VMC_DBG("Failed to read slave : %d \n\r",SLAVE_ADDRESS_SE98A_0);
	}

	return status;
}

s8 Temperature_Read_Outlet(snsrRead_t *snsrData)
{
	s8 status = XST_FAILURE;
	s32 tempValue = 0;

	status = SE98A_ReadTemperature(LPD_I2C_0, SLAVE_ADDRESS_SE98A_1, &tempValue);
	if (status == XST_SUCCESS)
	{
		memcpy(snsrData->snsrValue,&tempValue,sizeof(tempValue));
		snsrData->sensorValueSize = sizeof(tempValue);
		snsrData->snsrSatus = Vmc_Snsr_State_Normal;
	}
	else
	{
		snsrData->snsrSatus = Vmc_Snsr_State_Comms_failure;
		VMC_DBG("Failed to read slave : %d \n\r",SLAVE_ADDRESS_SE98A_1);
	}

	return status;
}

s8 Temperature_Read_Board(snsrRead_t *snsrData)
{
	s8 status = XST_FAILURE;
	float TempReading = 0;

	status = max6639_ReadDDRTemperature(i2c_num, SLAVE_ADDRESS_MAX6639, &TempReading);
	if (status == XST_SUCCESS)
	{
		memcpy(&snsrData->snsrValue[0],&TempReading,sizeof(TempReading));
		snsrData->sensorValueSize = sizeof(TempReading);
		snsrData->snsrSatus = Vmc_Snsr_State_Normal;
	}
	else
	{
		snsrData->snsrSatus = Vmc_Snsr_State_Comms_failure;
		VMC_DBG("Failed to read slave : %d \n\r",SLAVE_ADDRESS_MAX6639);
	}

	return status;
}

s8 Temperature_Read_ACAP_Device_Sysmon(snsrRead_t *snsrData)
{
	s8 status = XST_FAILURE;
	float TempReading = 0.0;

	status = XSysMonPsv_ReadTempProcessed(&InstancePtr, SYSMONPSV_TEMP_MAX, &TempReading);
	if (status == XST_SUCCESS)
	{
		memcpy(&snsrData->snsrValue[0],&TempReading,sizeof(TempReading));
		snsrData->sensorValueSize = sizeof(TempReading);
		snsrData->snsrSatus = Vmc_Snsr_State_Normal;
	}
	else
	{
		snsrData->snsrSatus = Vmc_Snsr_State_Comms_failure;
		VMC_DBG("Failed to read Sysmon : %d \n\r",SYSMONPSV_TEMP_MAX);
	}

	return status;
}

s8 Fan_RPM_Read(snsrRead_t *snsrData)
{
	s8 status = XST_FAILURE;
	u8 fanSpeed = 0;
	u16 fanRPM1 = 0;
	u16 fanRPM2 = 0;
	u16 avgFanRPM = 0;


	status = max6639_ReadFanTach(i2c_num, SLAVE_ADDRESS_MAX6639, 1, &fanSpeed);
	fanRPM1 = MAX6639_FAN_TACHO_TO_RPM(fanSpeed);

	fanSpeed = 0;
	status = max6639_ReadFanTach(i2c_num, SLAVE_ADDRESS_MAX6639, 2, &fanSpeed);
	fanRPM2 = MAX6639_FAN_TACHO_TO_RPM(fanSpeed);

	avgFanRPM = (fanRPM1 + fanRPM2)/2;
	if (status == XST_SUCCESS)
	{
		memcpy(snsrData->snsrValue,&avgFanRPM,sizeof(avgFanRPM));
		snsrData->sensorValueSize = sizeof(avgFanRPM);
		snsrData->snsrSatus = Vmc_Snsr_State_Normal;
	}
	else
	{
		snsrData->snsrSatus = Vmc_Snsr_State_Comms_failure;
		VMC_DBG("Failed to  read  Fan Speed from slave : 0x%x \n\r",SLAVE_ADDRESS_MAX6639);
	}

	return status;
}
s8 Temperature_Read_QSFP(snsrRead_t *snsrData)
{
	u8 status = XST_FAILURE;
	float TempReading = 0.0;

	status = QSFP_ReadTemperature(&TempReading, snsrData->sensorInstance);

	memcpy(&snsrData->snsrValue[0],&TempReading,sizeof(TempReading));
	snsrData->sensorValueSize = sizeof(TempReading);

	if (status == XST_SUCCESS)
	{
		snsrData->snsrSatus = Vmc_Snsr_State_Normal;
	}
	else if (status == XST_FAILURE)
	{
		snsrData->snsrSatus = Vmc_Snsr_State_Comms_failure;
		VMC_PRNT("Failed to read slave : %d \n\r",QSFP_SLAVE_ADDRESS);
	}
	else
	{
		snsrData->snsrSatus = Vmc_Snsr_State_Unavailable;
		VMC_PRNT("QSFP_%d module not present \n\r",(snsrData->sensorInstance-1));
	}
	return status;
}

s8 PMBUS_SC_Sensor_Read(snsrRead_t *snsrData)
{
    s8 status = XST_FAILURE;
    u16 sensorReading = 0;

    sensorReading = sc_vmc_data.sensor_values[snsrData->mspSensorIndex];
    if(sensorReading != 0)
    {
        memcpy(&snsrData->snsrValue[0],&sensorReading,sizeof(sensorReading));
        snsrData->sensorValueSize = sizeof(sensorReading);
        snsrData->snsrSatus = Vmc_Snsr_State_Normal;
    }
    else
    {
        memcpy(&snsrData->snsrValue[0],&sensorReading,sizeof(sensorReading));
        snsrData->snsrSatus = Vmc_Snsr_State_Comms_failure;
        VMC_DBG("MSP Sensor Id : %d Data read failed \n\r",snsrData->mspSensorIndex);
    }
    return status;
}

s8 Power_Monitor(snsrRead_t *snsrData)
{
    s8 status = XST_FAILURE;
    float totalPower = 0;

    totalPower = (sc_vmc_data.sensor_values[PEX_12V] *
                        (sc_vmc_data.sensor_values[V12_IN_AUX0_I] +
                              sc_vmc_data.sensor_values[V12_IN_AUX0_I]));
    if(totalPower != 0)
    {
        memcpy(&snsrData->snsrValue[0],&totalPower,sizeof(totalPower));
        snsrData->sensorValueSize = sizeof(totalPower);
        snsrData->snsrSatus = Vmc_Snsr_State_Normal;
    }
    else
    {
        memcpy(&snsrData->snsrValue[0],&totalPower,sizeof(totalPower));
        snsrData->snsrSatus = Vmc_Snsr_State_Comms_failure;
    }

    return status;
}

void se98a_monitor(void)
{
	u8 i = 0;
	u8 status = XST_FAILURE;
	for (i = 0 ; i < BOARD_TEMPERATURE_SENSOR_NUM ; i++)
	{
		status = SE98A_ReadTemperature(i2c_num, SLAVE_ADDRESS_SE98A_0 + i, &sensor_readings.board_temp[i]);
		if (status == XST_FAILURE)
		{
			VMC_DBG("Failed to read SE98A_%d \n\r",i);
		}
	}
	return;
}
void max6639_monitor(void)
{

	u8 status = XST_FAILURE;
	float TempReading = 0;
	u8 fanSpeed = 0;
    	u16 fanRpm1 = 0;
    	u16 fanRpm2 = 0;

	status = max6639_ReadFPGATemperature(i2c_num, SLAVE_ADDRESS_MAX6639, &TempReading);
        if (status == XST_FAILURE)
	{
		VMC_ERR("Failed to read MAX6639 \n\r");
		return;
	}
	//CL_LOG (APP_VMC,"fpga temp %f",TempReading);
	sensor_readings.remote_temp = TempReading;

        status = max6639_ReadDDRTemperature(i2c_num, SLAVE_ADDRESS_MAX6639, &TempReading);
        if (status == XST_FAILURE)
        {
                VMC_DBG( "Failed to read MAX6639 \n\r");
                return;
        }
        //CL_LOG (APP_VMC,"local temp %f",TempReading);
	sensor_readings.local_temp = TempReading;
	
    	status = max6639_ReadFanTach(i2c_num, SLAVE_ADDRESS_MAX6639, 1, &fanSpeed);
    	fanRpm1 = MAX6639_FAN_TACHO_TO_RPM(fanSpeed);

    	fanSpeed = 0;
    	status = max6639_ReadFanTach(i2c_num, SLAVE_ADDRESS_MAX6639, 2, &fanSpeed);
   	fanRpm2 = MAX6639_FAN_TACHO_TO_RPM(fanSpeed);

    	sensor_readings.fanRpm = (fanRpm1 + fanRpm2)/2;
	
	//CL_LOG (APP_VMC,"Fan RPM %d",fanRpm);
	
	return;

}

void sysmon_monitor(void)
{

	float TempReading = 0.0;
	if (XSysMonPsv_ReadTempProcessed(&InstancePtr, SYSMONPSV_TEMP_MAX, &TempReading))
	{
		CL_LOG(APP_VMC, "Failed to read sysmon temperature \n\r");
		sensor_readings.sysmon_max_temp = -1.0;
		return;
	}

	sensor_readings.sysmon_max_temp = TempReading;
	return;
}

void qsfp_monitor(void)
{
	u8 snsrIndex = 0;
	float TemperatureValue = 0;
	u8 status = XST_FAILURE;

	for (snsrIndex = 1 ; snsrIndex <= QSFP_TEMPERATURE_SENSOR_NUM ; snsrIndex++)
	{
		status = QSFP_ReadTemperature(&TemperatureValue,snsrIndex);

		sensor_readings.qsfp_temp[snsrIndex-1] = TemperatureValue;

		if (status == XST_FAILURE)
		{
			VMC_PRNT("\n\r Failed to read QSFP_%d temp \n\r",(snsrIndex-1));
		}
		if (status == XST_DEVICE_NOT_FOUND)
		{
			//VMC_PRNT("QSFP_%d module not present",(snsrIndex-1));
		}

	}
	return;
}

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

void SensorMonitorTask(void *params)
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
	if(!sc_update_flag)
	{
        	/* Read All Sensors */
        	Monitor_Sensors();

#ifdef VMC_TEST
        	se98a_monitor();
        	max6639_monitor();
        	sysmon_monitor();
        	qsfp_monitor ();
#endif
        	vTaskDelay(200);
	}
	else
	{
		vTaskDelay(2000);
	}
    }

    vTaskSuspend(NULL);
}
