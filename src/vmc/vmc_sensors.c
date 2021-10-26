/******************************************************************************
 * Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
 * SPDX-License-Identifier: MIT
 *******************************************************************************/

#include "FreeRTOS.h"
#include "task.h"

#include "cl_uart_rtos.h"
#include "cl_log.h"
#include "cl_i2c.h"
#include "sensors/inc/se98a.h"
#include "sensors/inc/max6639.h"
#include "vmc_sensors.h"
#include "vmc_asdm.h"
#include "sysmon.h"

extern XSysMonPsv InstancePtr;
extern XScuGic IntcInst;

#define MAX6639_FAN_TACHO_TO_RPM(x) (8000*60)/(x*2)

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
		VMC_ERR("Failed to read slave : %d \n\r",SLAVE_ADDRESS_SE98A_0);
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
		VMC_ERR("Failed to read slave : %d \n\r",SLAVE_ADDRESS_SE98A_1);
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
		memcpy(snsrData->snsrValue,&TempReading,sizeof(TempReading));
		snsrData->sensorValueSize = sizeof(TempReading);
		snsrData->snsrSatus = Vmc_Snsr_State_Normal;
	}
	else
	{
		snsrData->snsrSatus = Vmc_Snsr_State_Comms_failure;
		VMC_ERR("Failed to read slave : %d \n\r",SLAVE_ADDRESS_MAX6639);
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
		VMC_ERR("Failed to  read  Fan Speed from slave : 0x%x \n\r",SLAVE_ADDRESS_MAX6639);
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
			VMC_ERR("Failed to read SE98A_%d \n\r",i);
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
                VMC_ERR( "Failed to read MAX6639 \n\r");
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
