/******************************************************************************
* Copyright (C) 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#include "cl_vmc.h"
#include "cl_mem.h"
#include "cl_i2c.h"
#include "../vmc_api.h"
#include "v70.h"
#include "cl_i2c.h"
#include "../sensors/inc/lm75.h"
#include "../vmc_main.h"
#include "vmr_common.h"

extern Vmc_Global_Variables vmc_g_var;
static u8 i2c_main = LPD_I2C_0;

u8 V70_Init(void)
{
	//s8 status = XST_FAILURE;

	return XST_SUCCESS;
}


s8 V70_Temperature_Read_Inlet(snsrRead_t *snsrData)
{
	s8 status = XST_FAILURE;
	s16 tempValue = 0;

	status = LM75_ReadTemperature(i2c_main, SLAVE_ADDRESS_LM75_0_V70, &tempValue);
	if (status == XST_SUCCESS)
	{
		Cl_SecureMemcpy(snsrData->snsrValue,sizeof(tempValue),&tempValue,sizeof(tempValue));
		snsrData->sensorValueSize = sizeof(tempValue);
		snsrData->snsrSatus = Vmc_Snsr_State_Normal;
	}
	else
	{
		snsrData->snsrSatus = Vmc_Snsr_State_Comms_failure;
		VMC_DBG("Failed to read slave : 0x%0.2x \n\r", SLAVE_ADDRESS_LM75_0_V70);
	}

	return status;
}

s8 V70_Temperature_Read_Outlet(snsrRead_t *snsrData)
{
	s8 status = XST_FAILURE;
	s16 tempValue = 0;

	status = LM75_ReadTemperature(i2c_main, SLAVE_ADDRESS_LM75_1_V70, &tempValue);
	if (status == XST_SUCCESS)
	{
		Cl_SecureMemcpy(snsrData->snsrValue,sizeof(tempValue),&tempValue,sizeof(tempValue));
		snsrData->sensorValueSize = sizeof(tempValue);
		snsrData->snsrSatus = Vmc_Snsr_State_Normal;
	}
	else
	{
		snsrData->snsrSatus = Vmc_Snsr_State_Comms_failure;
		VMC_DBG("Failed to read slave : 0x%.2x \n\r", SLAVE_ADDRESS_LM75_1_V70);
	}

	return status;
}

s8 V70_Temperature_Read_Board(snsrRead_t *snsrData)
{
	s8 status = XST_SUCCESS;
	s16 TempReading = 0;

	TempReading = (vmc_g_var.sensor_readings.board_temp[0] + vmc_g_var.sensor_readings.board_temp[1])/2;

	Cl_SecureMemcpy(&snsrData->snsrValue[0],sizeof(TempReading),&TempReading,sizeof(TempReading));
	snsrData->sensorValueSize = sizeof(TempReading);
	snsrData->snsrSatus = Vmc_Snsr_State_Normal;

	return status;
}

void LM75_monitor(void)
{
	u8 status = XST_FAILURE;
	status = LM75_ReadTemperature(i2c_main, SLAVE_ADDRESS_LM75_0_V70, &vmc_g_var.sensor_readings.board_temp[0]);
	if (status == XST_FAILURE)
	{
		VMC_DBG("Failed to read LM75_0 \n\r");
	}

	status = LM75_ReadTemperature(i2c_main, SLAVE_ADDRESS_LM75_1_V70, &vmc_g_var.sensor_readings.board_temp[1]);
	if (status == XST_FAILURE)
	{
		VMC_DBG("Failed to read LM75_1 \n\r");
	}

	return;


}
