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
#include "../vmc_sc_comms.h"

extern Vmc_Sensors_Gl_t sensor_glvr;
extern msg_id_ptr msg_id_handler_ptr;
extern Fetch_BoardInfo_Func fetch_boardinfo_ptr;

static u8 i2c_main = LPD_I2C_0;

u8 V70_VMC_SC_Comms_Msg[] = {
		SC_COMMS_TX_I2C_SNSR
};

#define V70_MAX_MSGID_COUNT     (sizeof(V70_VMC_SC_Comms_Msg)/sizeof(V70_VMC_SC_Comms_Msg[0]))

u8 V70_Init(void)
{
	//s8 status = XST_FAILURE;
	msg_id_handler_ptr = V70_VMC_SC_Comms_Msg;
	set_total_req_size(V70_MAX_MSGID_COUNT);
	fetch_boardinfo_ptr = &V70_VMC_Fetch_BoardInfo;

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

	TempReading = (sensor_glvr.sensor_readings.board_temp[0] + sensor_glvr.sensor_readings.board_temp[1])/2;

	Cl_SecureMemcpy(&snsrData->snsrValue[0],sizeof(TempReading),&TempReading,sizeof(TempReading));
	snsrData->sensorValueSize = sizeof(TempReading);
	snsrData->snsrSatus = Vmc_Snsr_State_Normal;

	return status;
}

void LM75_monitor(void)
{
	u8 status = XST_FAILURE;
	status = LM75_ReadTemperature(i2c_main, SLAVE_ADDRESS_LM75_0_V70, &sensor_glvr.sensor_readings.board_temp[0]);
	if (status == XST_FAILURE)
	{
		VMC_DBG("Failed to read LM75_0 \n\r");
	}

	status = LM75_ReadTemperature(i2c_main, SLAVE_ADDRESS_LM75_1_V70, &sensor_glvr.sensor_readings.board_temp[1]);
	if (status == XST_FAILURE)
	{
		VMC_DBG("Failed to read LM75_1 \n\r");
	}

	return;
}

s32 V70_VMC_Fetch_BoardInfo(u8 *board_snsr_data)
{
    Versal_BoardInfo board_info = {0};
    /* byte_count will indicate the length of the response payload being generated */
    u32 byte_count = 0;

    (void)VMC_Get_BoardInfo(&board_info);

    Cl_SecureMemcpy(board_snsr_data, sizeof(Versal_BoardInfo), &board_info, sizeof(Versal_BoardInfo));
    byte_count = sizeof(Versal_BoardInfo);

    /* Check and return -1 if size of response is > 256 */
    return ((byte_count <= MAX_VMC_SC_UART_BUF_SIZE) ? (byte_count) : (-1));
}
