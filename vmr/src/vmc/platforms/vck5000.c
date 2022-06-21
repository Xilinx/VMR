/******************************************************************************
* Copyright (C) 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/
#include "cl_vmc.h"
#include "cl_mem.h"
#include "cl_i2c.h"
#include "../vmc_api.h"
#include "../sensors/inc/se98a.h"
#include "../sensors/inc/max6639.h"
#include "../sensors/inc/qsfp.h"
#include "../vmc_sensors.h"
#include "../vmc_main.h"
#include "vck5000.h"

extern Vmc_Global_Variables vmc_g_var;

static u8 i2c_num = LPD_I2C_0;

s8 Vck5000_Temperature_Read_Inlet(snsrRead_t *snsrData)
{
	s8 status = XST_FAILURE;
	s16 tempValue = 0;

	status = SE98A_ReadTemperature(i2c_num, SLAVE_ADDRESS_SE98A_0, &tempValue);
	if (status == XST_SUCCESS)
	{
		Cl_SecureMemcpy(snsrData->snsrValue,sizeof(tempValue),&tempValue,sizeof(tempValue));
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

s8 Vck5000_Temperature_Read_Outlet(snsrRead_t *snsrData)
{
	s8 status = XST_FAILURE;
	s16 tempValue = 0;

	status = SE98A_ReadTemperature(i2c_num, SLAVE_ADDRESS_SE98A_1, &tempValue);
	if (status == XST_SUCCESS)
	{
		Cl_SecureMemcpy(snsrData->snsrValue,sizeof(tempValue),&tempValue,sizeof(tempValue));
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

s8 Vck5000_Temperature_Read_Board(snsrRead_t *snsrData)
{
	s8 status = XST_FAILURE;
	float TempReading = 0;

	status = max6639_ReadDDRTemperature(i2c_num, SLAVE_ADDRESS_MAX6639, &TempReading);
	if (status == XST_SUCCESS)
	{
		u16 roundedOffVal = (TempReading > 0) ? TempReading : 0;
		Cl_SecureMemcpy(&snsrData->snsrValue[0],sizeof(roundedOffVal),&roundedOffVal,sizeof(roundedOffVal));
		snsrData->sensorValueSize = sizeof(roundedOffVal);
		snsrData->snsrSatus = Vmc_Snsr_State_Normal;
	}
	else
	{
		snsrData->snsrSatus = Vmc_Snsr_State_Comms_failure;
		VMC_DBG("Failed to read slave : %d \n\r",SLAVE_ADDRESS_MAX6639);
	}

	return status;
}

s8 Vck5000_Temperature_Read_QSFP(snsrRead_t *snsrData)
{
	u8 status = XST_FAILURE;
	float TempReading = 0.0;

	static bool is_qsfp_critical_threshold_reached = false;
	status = QSFP_ReadTemperature(&TempReading, snsrData->sensorInstance);

	if (status == XST_SUCCESS)
	{
		u16 roundedOffVal = (TempReading > 0) ? TempReading : 0;
		Cl_SecureMemcpy(&snsrData->snsrValue[0],sizeof(roundedOffVal),&roundedOffVal,sizeof(roundedOffVal));
		snsrData->sensorValueSize = sizeof(roundedOffVal);
		snsrData->snsrSatus = Vmc_Snsr_State_Normal;
	}
	else if (status == XST_FAILURE)
	{
		snsrData->snsrSatus = Vmc_Snsr_State_Comms_failure;
		VMC_ERR("Failed to read slave : %d \n\r",QSFP_SLAVE_ADDRESS);
	}
	else
	{
		snsrData->snsrSatus = Vmc_Snsr_State_Unavailable;
		VMC_DBG("QSFP_%d module not present \n\r",(snsrData->sensorInstance));
	}

	if (TempReading >= TEMP_QSFP_CRITICAL_THRESHOLD)
	{
		ucs_clock_shutdown();
		is_qsfp_critical_threshold_reached = true;

	}
	if((is_qsfp_critical_threshold_reached == true) && 
		(TempReading <  TEMP_QSFP_CRITICAL_THRESHOLD ))
	{
		clear_clock_shutdown_status();
		is_qsfp_critical_threshold_reached = false;
	}
	return status;
}

s8 Vck5000_Fan_RPM_Read(snsrRead_t *snsrData)
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
		Cl_SecureMemcpy(snsrData->snsrValue,sizeof(avgFanRPM),&avgFanRPM,sizeof(avgFanRPM));
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

void se98a_monitor(void)
{
	u8 i = 0;
	u8 status = XST_FAILURE;
	for (i = 0 ; i < BOARD_TEMPERATURE_SENSOR_NUM ; i++)
	{
		status = SE98A_ReadTemperature(i2c_num, SLAVE_ADDRESS_SE98A_0 + i, &vmc_g_var.sensor_readings.board_temp[i]);
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
	vmc_g_var.sensor_readings.remote_temp = TempReading;

	status = max6639_ReadDDRTemperature(i2c_num, SLAVE_ADDRESS_MAX6639, &TempReading);
	if (status == XST_FAILURE)
	{
			VMC_DBG( "Failed to read MAX6639 \n\r");
			return;
	}
	//CL_LOG (APP_VMC,"local temp %f",TempReading);
	vmc_g_var.sensor_readings.local_temp = TempReading;

	status = max6639_ReadFanTach(i2c_num, SLAVE_ADDRESS_MAX6639, 1, &fanSpeed);
	fanRpm1 = MAX6639_FAN_TACHO_TO_RPM(fanSpeed);

	fanSpeed = 0;
	status = max6639_ReadFanTach(i2c_num, SLAVE_ADDRESS_MAX6639, 2, &fanSpeed);
   	fanRpm2 = MAX6639_FAN_TACHO_TO_RPM(fanSpeed);

	vmc_g_var.sensor_readings.fanRpm = (fanRpm1 + fanRpm2)/2;

	//CL_LOG (APP_VMC,"Fan RPM %d",fanRpm);

	return;

}

void qsfp_monitor(void)
{
	u8 snsrIndex = 0;
	float TemperatureValue = 0;
	u8 status = XST_FAILURE;

	for (snsrIndex = 0; snsrIndex < QSFP_TEMPERATURE_SENSOR_NUM; snsrIndex++)
	{
		status = QSFP_ReadTemperature(&TemperatureValue,snsrIndex);

		if (status == XST_SUCCESS)
		{
			vmc_g_var.sensor_readings.qsfp_temp[snsrIndex] = TemperatureValue;
		}
		if (status == XST_FAILURE)
		{
			VMC_PRNT("\n\r Failed to read QSFP_%d temp \n\r", snsrIndex);
		}
		if (status == XST_DEVICE_NOT_FOUND)
		{
			//VMC_PRNT("QSFP_%d module not present", snsrIndex);
		}

	}
	return;
}
