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
#include "../sensors/inc/ina3221.h"
#include "../sensors/inc/isl68221.h"
#include "../vmc_main.h"
#include "vmr_common.h"
#include "../vmc_sc_comms.h"

#define SLAVE_ADDRESS_LM75_0_V70  (0x48)
#define SLAVE_ADDRESS_LM75_1_V70  (0x4A)

#define SLAVE_ADDRESS_INA3221   (0x40)
#define SLAVE_ADDRESS_ISL68221  (0x60)

extern Vmc_Sensors_Gl_t sensor_glvr;
extern msg_id_ptr msg_id_handler_ptr;
extern Fetch_BoardInfo_Func fetch_boardinfo_ptr;

static u8 i2c_main = LPD_I2C_0;

u8 V70_VMC_SC_Comms_Msg[] = {
		SC_COMMS_TX_I2C_SNSR
};

#define V70_MAX_MSGID_COUNT     (sizeof(V70_VMC_SC_Comms_Msg)/sizeof(V70_VMC_SC_Comms_Msg[0]))
#define MAX(a,b)	((a>b) ? a : b)

#define V70_NUM_BOARD_INFO_SENSORS	(11)
#define V70_NUM_TEMPERATURE_SENSORS	(3)
#define V70_NUM_SC_VOLTAGE_SENSORS	(2)
#define V70_NUM_SYSMON_VOLTAGE_SENSORS	(1)
#define V70_NUM_SC_CURRENT_SENSORS	(3)
#define V70_NUM_POWER_SENSORS		(1)

extern platform_sensors_monitor_ptr Monitor_Sensors;
extern supported_sdr_info_ptr get_supported_sdr_info;
extern asdm_update_record_count_ptr asdm_update_record_count;

void V70_Voltage_Monitor_12V_PEX(void);
void V70_Current_Monitor_12V_PEX(void);
void V70_Voltage_Monitor_3v3_PEX(void);
void V70_Current_Monitor_3v3_PEX(void);
void V70_Voltage_Monitor_3v3_AUX(void);

AsdmHeader_info_t v70_asdmHeader_info[] = {
		{BoardInfoSDR,		V70_NUM_BOARD_INFO_SENSORS},
		{TemperatureSDR,	V70_NUM_TEMPERATURE_SENSORS},
		{VoltageSDR,		V70_NUM_SC_VOLTAGE_SENSORS + V70_NUM_SYSMON_VOLTAGE_SENSORS},
		{CurrentSDR,		V70_NUM_SC_CURRENT_SENSORS},
		{PowerSDR,		V70_NUM_POWER_SENSORS},
};
#define MAX_SDR_REPO 	(sizeof(v70_asdmHeader_info)/sizeof(v70_asdmHeader_info[0]))

u32 V70_Supported_Sensors[] = {
	eProduct_Name,
	eSerial_Number,
	ePart_Number,
	eRevision,
	eMfg_Date,
	eUUID,
	eMAC_0,
	eFpga_Fan_1,
	eActive_SC_Ver,
	eTarget_SC_Ver,
	eOEM_Id,

	/* Temperature SDR */
	eTemp_Board,
	eTemp_Sysmon_Fpga,
	eTemp_Vccint_Temp,

	/* Voltage SDR */
	eVoltage_Sysmon_Vccint,
	eVoltage_12v_Pex,
	eVoltage_3v3_Pex,

	/* Current SDR */
	eCurrent_12v_Pex,
	eCurrent_3v3_Pex,
	eCurrent_Vccint,


	/* Power SDR */
	ePower_Total
};

void V70_Get_Supported_Sdr_Info(u32 *platform_Supported_Sensors, u32 *sdr_count)
{
	*sdr_count = (sizeof(V70_Supported_Sensors) / sizeof(V70_Supported_Sensors[0]));
	Cl_SecureMemcpy(platform_Supported_Sensors, sizeof(V70_Supported_Sensors),
			V70_Supported_Sensors,sizeof(V70_Supported_Sensors));

	return;
}

void V70_Asdm_Update_Record_Count(Asdm_Header_t *headerInfo)
{
	u8 i = 0;
	for(i=0; i < MAX_SDR_REPO; i++)
	{
		if(headerInfo[i].repository_type == v70_asdmHeader_info[i].record_type) {
			headerInfo[i].no_of_records = v70_asdmHeader_info[i].record_count;
		}
	}
	return;
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

	TempReading = MAX(sensor_glvr.sensor_readings.board_temp[0], sensor_glvr.sensor_readings.board_temp[1]);

	Cl_SecureMemcpy(&snsrData->snsrValue[0],sizeof(TempReading),&TempReading,sizeof(TempReading));
	snsrData->sensorValueSize = sizeof(TempReading);
	snsrData->snsrSatus = Vmc_Snsr_State_Normal;

	return status;
}

void V70_Temperature_Monitor(void)
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

	status =  ISL68221_ReadVCCINT_Temperature(i2c_main, SLAVE_ADDRESS_ISL68221, &sensor_glvr.sensor_readings.vccint_temp);
	if (status == XST_FAILURE)
	{
		VMC_DBG("Failed to read Vccint_temp from : 0x%x", SLAVE_ADDRESS_ISL68221);
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

void V70_Voltage_Monitor_12V_PEX()
{
	u8 status = XST_FAILURE;
	float volatge = 0.0;
	status = INA3221_ReadVoltage(i2c_main, SLAVE_ADDRESS_INA3221, 0, &volatge);
	if(XST_SUCCESS != status)
	{
		VMC_ERR("Failed to read 12vPex voltage");
	}

	/* Divide by 1000 to convert to milli Volts */
	sensor_glvr.sensor_readings.voltage[e12V_PEX] = (volatge/1000.0);
}

void V70_Current_Monitor_12V_PEX()
{
	u8 status = XST_FAILURE;
	float current = 0.0;
	status = INA3221_ReadCurrent(i2c_main, SLAVE_ADDRESS_INA3221, 0, &current);
	if(XST_SUCCESS != status)
	{
		VMC_ERR("Failed to read 12vPex current");
	}

	/* Divide by 1000 to convert to milli Amps */
	sensor_glvr.sensor_readings.current[e12V_PEX] = (current/1000.0);
}

void V70_Voltage_Monitor_3v3_PEX()
{
	u8 status = XST_FAILURE;
	float volatge = 0.0;
	status = INA3221_ReadVoltage(i2c_main, SLAVE_ADDRESS_INA3221, 1, &volatge);
	if(XST_SUCCESS != status)
	{
		VMC_ERR("Failed to read 3v3 pex voltage");
	}
	/* Divide by 1000 to convert to milli Volts */
	sensor_glvr.sensor_readings.voltage[e3V3_PEX] = (volatge/1000.0);
}

void V70_Current_Monitor_3v3_PEX()
{
	u8 status = XST_FAILURE;
	float current = 0.0;
	status = INA3221_ReadCurrent(i2c_main, SLAVE_ADDRESS_INA3221, 1, &current);
	if(XST_SUCCESS != status)
	{
		VMC_ERR("Failed to read 3v3 Pex current");
	}
	/* Divide by 1000 to convert to milli Amps */
	sensor_glvr.sensor_readings.current[e3V3_PEX] = (current/1000.0);
}

void V70_Voltage_Monitor_3v3_AUX()
{
	u8 status = XST_FAILURE;
	float volatge = 0.0;
	status = INA3221_ReadVoltage(i2c_main, SLAVE_ADDRESS_INA3221, 2, &volatge);
	if(XST_SUCCESS != status)
	{
		VMC_ERR("Failed to read 3v3 Aux voltage");
	}

	/* Divide by 1000 to convert to milli Volts */
	sensor_glvr.sensor_readings.voltage[e3V3_AUX] = (volatge/1000.0);
}

void V70_Power_Monitor()
{
	u8 status = XST_FAILURE;
	float power_12v_pex = 0.0;
	float power_3v3_pex = 0.0;
	status = INA3221_ReadPower(i2c_main, SLAVE_ADDRESS_INA3221, 0, &power_12v_pex);
	if(XST_SUCCESS != status)
	{
		VMC_ERR("Failed to read 12v Pex power");
	}

	status = INA3221_ReadPower(i2c_main, SLAVE_ADDRESS_INA3221, 1, &power_3v3_pex);
	if(XST_SUCCESS != status)
	{
		VMC_ERR("Failed to read 3v3 Pex power");
	}

	/* Divide by 1000 to convert to Watts */
	sensor_glvr.sensor_readings.total_power = (power_12v_pex + power_3v3_pex)/1000.0;
}

void V70_Current_Monitor_Vccint()
{
	u8 status = XST_SUCCESS;
	float currentInA = 0.0;

	status =  ISL68221_ReadVCCINT_Current(i2c_main, SLAVE_ADDRESS_ISL68221, &currentInA);
	if(XST_SUCCESS != status)
	{
		VMC_ERR("Failed to read Vccint Current ");
	}

	sensor_glvr.sensor_readings.current[eVCCINT] = currentInA; //In Amps
}

s8 V70_Asdm_Read_Power(snsrRead_t *snsrData) {

	s8 status = XST_SUCCESS;
	u16 total_power = sensor_glvr.sensor_readings.total_power;

	Cl_SecureMemcpy(&snsrData->snsrValue[0],sizeof(total_power),&total_power,sizeof(total_power));
	snsrData->sensorValueSize = sizeof(total_power);
	snsrData->snsrSatus = Vmc_Snsr_State_Normal;

	return status;
}

s8 V70_Asdm_Read_Voltage_12v(snsrRead_t *snsrData) {

	s8 status = XST_SUCCESS;
	/* Multiply by 1000 to convert to Volts */
	u16 voltage = (sensor_glvr.sensor_readings.voltage[e12V_PEX]* 1000);

	Cl_SecureMemcpy(&snsrData->snsrValue[0],sizeof(voltage),&voltage,sizeof(voltage));
	snsrData->sensorValueSize = sizeof(voltage);
	snsrData->snsrSatus = Vmc_Snsr_State_Normal;

	return status;
}

s8 V70_Asdm_Read_Voltage_3v3(snsrRead_t *snsrData) {

	s8 status = XST_SUCCESS;
	/* Multiply by 1000 to convert to Volts */
	u16 voltage = (sensor_glvr.sensor_readings.voltage[e3V3_PEX]* 1000);

	Cl_SecureMemcpy(&snsrData->snsrValue[0],sizeof(voltage),&voltage,sizeof(voltage));
	snsrData->sensorValueSize = sizeof(voltage);
	snsrData->snsrSatus = Vmc_Snsr_State_Normal;

	return status;
}

s8 V70_Asdm_Read_Current_12v(snsrRead_t *snsrData) {

	s8 status = XST_SUCCESS;
	u16 current = (sensor_glvr.sensor_readings.current[e12V_PEX]* 1000);

	Cl_SecureMemcpy(&snsrData->snsrValue[0],sizeof(current),&current,sizeof(current));
	snsrData->sensorValueSize = sizeof(current);
	snsrData->snsrSatus = Vmc_Snsr_State_Normal;

	return status;
}

s8 V70_Asdm_Read_Current_3v3(snsrRead_t *snsrData) {

	s8 status = XST_SUCCESS;
	u16 current = (sensor_glvr.sensor_readings.current[e3V3_PEX] * 1000);

	Cl_SecureMemcpy(&snsrData->snsrValue[0],sizeof(current),&current,sizeof(current));
	snsrData->sensorValueSize = sizeof(current);
	snsrData->snsrSatus = Vmc_Snsr_State_Normal;

	return status;
}

s8 V70_Asdm_Read_Current_Vccint(snsrRead_t *snsrData)
{
	s8 status = XST_SUCCESS;
	u16 current = (sensor_glvr.sensor_readings.current[eVCCINT] * 1000);

	Cl_SecureMemcpy(&snsrData->snsrValue[0],sizeof(current),&current,sizeof(current));
	snsrData->sensorValueSize = sizeof(current);
	snsrData->snsrSatus = Vmc_Snsr_State_Normal;

	return status;
}

s8 V70_Asdm_Read_Temp_Vccint(snsrRead_t *snsrData)
{
	s8 status = XST_SUCCESS;
	u16 temp = (sensor_glvr.sensor_readings.vccint_temp);

	Cl_SecureMemcpy(&snsrData->snsrValue[0],sizeof(temp),&temp,sizeof(temp));
	snsrData->sensorValueSize = sizeof(temp);
	snsrData->snsrSatus = Vmc_Snsr_State_Normal;

	return status;
}

void V70_Monitor_Sensors()
{
//	snsrRead_t *snsrData = NULL;

	/* Read Temp Sensors */
//	V70_Temperature_Read_Inlet(snsrData);
//	V70_Temperature_Read_Outlet(snsrData);
	V70_Temperature_Monitor();

	/* Read Voltage Sensors */
	V70_Voltage_Monitor_12V_PEX();
	V70_Voltage_Monitor_3v3_PEX();
	V70_Voltage_Monitor_3v3_AUX();

	/* Read Current Sensors */
	V70_Current_Monitor_12V_PEX();
	V70_Current_Monitor_3v3_PEX();
	V70_Current_Monitor_Vccint();

	/* Read Power Sensors */
	V70_Power_Monitor();
}

u8 V70_Init(void)
{
	//s8 status = XST_FAILURE;
	msg_id_handler_ptr = V70_VMC_SC_Comms_Msg;
	set_total_req_size(V70_MAX_MSGID_COUNT);
	Monitor_Sensors = V70_Monitor_Sensors;

	get_supported_sdr_info = V70_Get_Supported_Sdr_Info;
	asdm_update_record_count = V70_Asdm_Update_Record_Count;
	return XST_SUCCESS;
}
