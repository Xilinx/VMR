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
#include "../vmc_sc_comms.h"
#include "../clock_throttling.h"
#include "../vmc_asdm.h"

#define NOMINAL_VOLTAGE 12000
#define VCK5000_IDLE_POWER 21000000
#define VCK5000_NUM_POWER_RAILS 3

#define VCK5000_TEMP_GAIN_KP_FPGA		3.000e+06    //0x4a371b00;
#define VCK5000_TEMP_GAIN_KI			2.500e+03    //0x451c4000;
#define VCK5000_TEMP_GAIN_KP_VCCINT 	1.500e+07    //0x4b64e1c0;
#define VCK5000_TEMP_GAIN_KAW			5.500e-04    //0x3a102de2;
#define VCK5000_INTEGRATION_SUM_INITIAL	1.150e+08

#define VCK5000_NUM_BOARD_INFO_SENSORS		(12)
#define VCK5000_NUM_TEMPERATURE_SENSORS		(5)
#define VCK5000_NUM_SC_VOLTAGE_SENSORS		(5)
#define VCK5000_NUM_SYSMON_VOLTAGE_SENSORS	(1)
#define VCK5000_NUM_SC_CURRENT_SENSORS		(4)
#define VCK5000_NUM_POWER_SENSORS		(1)

AsdmHeader_info_t vck5000_asdmHeader_info[] = {
		{BoardInfoSDR, VCK5000_NUM_BOARD_INFO_SENSORS},
		{TemperatureSDR, VCK5000_NUM_TEMPERATURE_SENSORS},
		{VoltageSDR, VCK5000_NUM_SC_VOLTAGE_SENSORS + VCK5000_NUM_SYSMON_VOLTAGE_SENSORS},
		{CurrentSDR, VCK5000_NUM_SC_CURRENT_SENSORS},
		{PowerSDR, VCK5000_NUM_POWER_SENSORS},
};
#define MAX_SDR_REPO 	(sizeof(vck5000_asdmHeader_info)/sizeof(vck5000_asdmHeader_info[0]))

extern Vmc_Sensors_Gl_t sensor_glvr;
extern msg_id_ptr msg_id_handler_ptr;

extern Fetch_BoardInfo_Func fetch_boardinfo_ptr;

extern supported_sdr_info_ptr get_supported_sdr_info;
extern asdm_update_record_count_ptr asdm_update_record_count;

Build_Clock_Throttling_Profile clock_throttling_vck5000;
Clock_Throttling_Algorithm clock_throttling_std_algorithm;

static u8 i2c_num = LPD_I2C_0;

u8 VCK5000_VMC_SC_Comms_Msg[] = {
		SC_COMMS_RX_VOLT_SNSR,
		SC_COMMS_RX_POWER_SNSR,
		SC_COMMS_RX_TEMP_SNSR,
		SC_COMMS_TX_I2C_SNSR,
};

#define VCK5000_MAX_MSGID_COUNT     (sizeof(VCK5000_VMC_SC_Comms_Msg)/sizeof(VCK5000_VMC_SC_Comms_Msg[0]))
#define TEMP_THROTTLING_THRESHOLD       95
#define PWR_THROTTLING_THRESHOLD        290

typedef enum {
        eCLK_SCALING_MODE_PWR  = 0x0,
	eCLK_SCALING_MODE_TEMP = 0x1,
	eCLK_SCALING_MODE_BOTH = 0x2
}eClk_scaling_mode;

extern clk_throttling_params_t g_clk_trottling_params;

u32 vck5000_Supported_Sensors[] = {
	eProduct_Name,
	eSerial_Number,
	ePart_Number,
	eRevision,
	eMfg_Date,
	eUUID,
	eMAC_0,
	eMAC_1,
	eFpga_Fan_1,
	eActive_SC_Ver,
	eTarget_SC_Ver,
	eOEM_Id,

	/* Temperature SDR */
	eTemp_Board,
	eTemp_Sysmon_Fpga,
	eTemp_Vccint,
	eTemp_Qsfp,

	/* Voltage SDR */
	eVolatge_SC_Sensors,
	eVoltage_Sysmon_Vccint,

	/* Current SDR */
	eCurrent_SC_Sensors,
	eCurrent_SC_Vccint,

	/* Power SDR */
	ePower_Total
};

void Vck5000_Get_Supported_Sdr_Info(u32 *platform_Supported_Sensors, u32 *sdr_count)
{
	*sdr_count = (sizeof(vck5000_Supported_Sensors) / sizeof(vck5000_Supported_Sensors[0]));
	Cl_SecureMemcpy(platform_Supported_Sensors, sizeof(vck5000_Supported_Sensors),
			vck5000_Supported_Sensors,sizeof(vck5000_Supported_Sensors));

	return;
}

void Vck5000_Asdm_Update_Record_Count(Asdm_Header_t *headerInfo)
{
	u8 i = 0;
	for(i=0; i < MAX_SDR_REPO; i++)
	{
		if(headerInfo[i].repository_type == vck5000_asdmHeader_info[i].record_type) {
			headerInfo[i].no_of_records = vck5000_asdmHeader_info[i].record_count;
		}
	}
	return;
}

void Build_clock_throttling_profile_VCK5000(Build_Clock_Throttling_Profile * pProfile)
{
	pProfile->NumberOfSensors = VCK5000_NUM_POWER_RAILS;

	pProfile->VoltageSensorID[0] = eSC_PEX_12V;
	pProfile->VoltageSensorID[1] = eSC_AUX_12V;
	pProfile->VoltageSensorID[2] = eSC_AUX1_12V;

	pProfile->CurrentSensorID[0] = eSC_PEX_12V_I_IN;
	pProfile->CurrentSensorID[1] = eSC_V12_IN_AUX0_I;
	pProfile->CurrentSensorID[2] = eSC_V12_IN_AUX1_I;

	pProfile->throttlingThresholdCurrent[0] = VCK5000_PEX_12V_I_IN_THROTTLING_LIMIT;
	pProfile->throttlingThresholdCurrent[1] = VCK5000_AUX_12V_I_IN_THROTTLING_LIMIT_2X4;
	pProfile->throttlingThresholdCurrent[2] = VCK5000_AUX_12V_I_IN_THROTTLING_LIMIT_2X3;

	pProfile->NominalVoltage[0] = NOMINAL_VOLTAGE;
	pProfile->NominalVoltage[1] = NOMINAL_VOLTAGE;
	pProfile->NominalVoltage[2] = NOMINAL_VOLTAGE;
	pProfile->IdlePower = VCK5000_IDLE_POWER;

	pProfile->bVCCIntThermalThrottling = true;
	pProfile->TempGainKpFPGA    = VCK5000_TEMP_GAIN_KP_FPGA;
	pProfile->TempGainKi        = VCK5000_TEMP_GAIN_KI;
	pProfile->TempGainKpVCCInt  = VCK5000_TEMP_GAIN_KP_VCCINT;
	pProfile->TempGainKaw       = VCK5000_TEMP_GAIN_KAW;

	pProfile->IntegrataionSumInitial = VCK5000_INTEGRATION_SUM_INITIAL;

}


void clk_scaling_params_init() {
	g_clk_trottling_params.is_clk_scaling_supported = true;
	g_clk_trottling_params.clk_scaling_mode = eCLK_SCALING_MODE_BOTH;
	g_clk_trottling_params.clk_scaling_enable = false;
	g_clk_trottling_params.limits.shutdown_limit_temp = TEMP_FPGA_CRITICAL_THRESHOLD;
	g_clk_trottling_params.limits.shutdown_limit_pwr = POWER_CRITICAL_THRESHOLD;
	g_clk_trottling_params.limits.throttle_limit_temp = TEMP_THROTTLING_THRESHOLD;
	g_clk_trottling_params.limits.throttle_limit_pwr = POWER_THROTTLING_THRESOLD_LIMIT;
	return;
}


u8 Vck5000_Init(void)
{
	//s8 status = XST_FAILURE;

	/* Retry till fan controller is programmed */
	while (max6639_init(1, 0x2E));  // only for vck5000

	msg_id_handler_ptr = VCK5000_VMC_SC_Comms_Msg;
	set_total_req_size(VCK5000_MAX_MSGID_COUNT);
	fetch_boardinfo_ptr = &Vck5000_VMC_Fetch_BoardInfo;

	/* platform specific initialization */
	Build_clock_throttling_profile_VCK5000(&clock_throttling_vck5000);

	/* clock throttling initialization */
	ClockThrottling_Initialize(&clock_throttling_std_algorithm, &clock_throttling_vck5000);

	clk_scaling_params_init();

	get_supported_sdr_info = Vck5000_Get_Supported_Sdr_Info;
	asdm_update_record_count = Vck5000_Asdm_Update_Record_Count;

	return XST_SUCCESS;
}

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
		status = SE98A_ReadTemperature(i2c_num, SLAVE_ADDRESS_SE98A_0 + i, &sensor_glvr.sensor_readings.board_temp[i]);
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
	sensor_glvr.sensor_readings.remote_temp = TempReading;

	status = max6639_ReadDDRTemperature(i2c_num, SLAVE_ADDRESS_MAX6639, &TempReading);
	if (status == XST_FAILURE)
	{
			VMC_DBG( "Failed to read MAX6639 \n\r");
			return;
	}
	//CL_LOG (APP_VMC,"local temp %f",TempReading);
	sensor_glvr.sensor_readings.local_temp = TempReading;

	status = max6639_ReadFanTach(i2c_num, SLAVE_ADDRESS_MAX6639, 1, &fanSpeed);
	fanRpm1 = MAX6639_FAN_TACHO_TO_RPM(fanSpeed);

	fanSpeed = 0;
	status = max6639_ReadFanTach(i2c_num, SLAVE_ADDRESS_MAX6639, 2, &fanSpeed);
   	fanRpm2 = MAX6639_FAN_TACHO_TO_RPM(fanSpeed);

	sensor_glvr.sensor_readings.fanRpm = (fanRpm1 + fanRpm2)/2;

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
			sensor_glvr.sensor_readings.qsfp_temp[snsrIndex] = TemperatureValue;
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

u8 Vck5000_Vmc_Sc_Comms(void)
{
	/* Fetch the Volt & power Sensor length */
	if (!vmc_get_snsr_resp_status()) {
		VMC_SC_COMMS_Tx_Rx(MSP432_COMMS_VMC_GET_RESP_SIZE_REQ);
	}

	return XST_SUCCESS;
}

s32 Vck5000_VMC_Fetch_BoardInfo(u8 *board_snsr_data)
{
	Versal_BoardInfo board_info = { 0 };
	/* byte_count will indicate the length of the response payload being generated */
	u32 byte_count = 0;
	u32 bufr_ptr = 0;

	(void) VMC_Get_BoardInfo(&board_info);
	Cl_SecureMemcpy(board_info.DIMM_size, (EEPROM_V2_0_DIMM_SIZE_SIZE + 1), board_info.Memory_size, (EEPROM_V2_0_DIMM_SIZE_SIZE + 1));

	Cl_SecureMemcpy(&board_snsr_data[bufr_ptr], (EEPROM_V2_0_PRODUCT_NAME_SIZE + 1), board_info.product_name, (EEPROM_V2_0_PRODUCT_NAME_SIZE + 1));
	bufr_ptr += (EEPROM_V2_0_PRODUCT_NAME_SIZE + 1);
	Cl_SecureMemcpy(&board_snsr_data[bufr_ptr], (EEPROM_V2_0_BOARD_REV_SIZE + 1), board_info.board_rev, (EEPROM_V2_0_BOARD_REV_SIZE + 1));
	bufr_ptr += (EEPROM_V2_0_BOARD_REV_SIZE + 1);
	Cl_SecureMemcpy(&board_snsr_data[bufr_ptr], (EEPROM_V2_0_BOARD_SERIAL_SIZE + 1), board_info.board_serial, (EEPROM_V2_0_BOARD_SERIAL_SIZE + 1));
	bufr_ptr += (EEPROM_V2_0_BOARD_SERIAL_SIZE + 1);
	Cl_SecureMemcpy(&board_snsr_data[bufr_ptr], (EEPROM_VERSION_SIZE + 1), board_info.eeprom_version, (EEPROM_VERSION_SIZE + 1));
	bufr_ptr += (EEPROM_VERSION_SIZE + 1);
	Cl_SecureMemcpy(&board_snsr_data[bufr_ptr], 28, board_info.board_mac, 28);
	bufr_ptr += 28;
	Cl_SecureMemcpy(&board_snsr_data[bufr_ptr], (EEPROM_V2_0_BOARD_ACT_PAS_SIZE + 1), board_info.board_act_pas, (EEPROM_V2_0_BOARD_ACT_PAS_SIZE + 1));
	bufr_ptr += (EEPROM_V2_0_BOARD_ACT_PAS_SIZE + 1);
	Cl_SecureMemcpy(&board_snsr_data[bufr_ptr], (EEPROM_V2_0_BOARD_CONFIG_MODE_SIZE + 1), board_info.board_config_mode, (EEPROM_V2_0_BOARD_CONFIG_MODE_SIZE + 1));
	bufr_ptr += (EEPROM_V2_0_BOARD_CONFIG_MODE_SIZE + 1);
	Cl_SecureMemcpy(&board_snsr_data[bufr_ptr], (EEPROM_V2_0_MFG_DATE_SIZE + 1), board_info.board_mfg_date, (EEPROM_V2_0_MFG_DATE_SIZE + 1));
	bufr_ptr += (EEPROM_V2_0_MFG_DATE_SIZE + 1);
	Cl_SecureMemcpy(&board_snsr_data[bufr_ptr], (EEPROM_V2_0_PART_NUM_SIZE + 1), board_info.board_part_num, (EEPROM_V2_0_PART_NUM_SIZE + 1));
	bufr_ptr += (EEPROM_V2_0_PART_NUM_SIZE + 1);
	Cl_SecureMemcpy(&board_snsr_data[bufr_ptr], (EEPROM_V2_0_UUID_SIZE + 1), board_info.board_uuid, (EEPROM_V2_0_UUID_SIZE + 1));
	bufr_ptr += (EEPROM_V2_0_UUID_SIZE + 1);
	Cl_SecureMemcpy(&board_snsr_data[bufr_ptr], (EEPROM_V2_0_PCIE_INFO_SIZE + 1), board_info.board_pcie_info, (EEPROM_V2_0_PCIE_INFO_SIZE + 1));
	bufr_ptr += (EEPROM_V2_0_PCIE_INFO_SIZE + 1);
	Cl_SecureMemcpy(&board_snsr_data[bufr_ptr], (EEPROM_V2_0_MAX_POWER_MODE_SIZE + 1), board_info.board_max_power_mode, (EEPROM_V2_0_MAX_POWER_MODE_SIZE + 1));
	bufr_ptr += (EEPROM_V2_0_MAX_POWER_MODE_SIZE + 1);
	Cl_SecureMemcpy(&board_snsr_data[bufr_ptr], (EEPROM_V2_0_DIMM_SIZE_SIZE + 1), board_info.Memory_size, (EEPROM_V2_0_DIMM_SIZE_SIZE + 1));
	bufr_ptr += (EEPROM_V2_0_DIMM_SIZE_SIZE + 1);
	Cl_SecureMemcpy(&board_snsr_data[bufr_ptr], (EEPROM_V2_0_OEMID_SIZE + 1), board_info.OEM_ID, (EEPROM_V2_0_OEMID_SIZE + 1));
	bufr_ptr += (EEPROM_V2_0_OEMID_SIZE + 1);
	Cl_SecureMemcpy(&board_snsr_data[bufr_ptr], (EEPROM_V2_0_DIMM_SIZE_SIZE + 1), board_info.DIMM_size, (EEPROM_V2_0_DIMM_SIZE_SIZE + 1));
	bufr_ptr += (EEPROM_V2_0_DIMM_SIZE_SIZE + 1);

	byte_count = bufr_ptr;

	/* Check and return -1 if size of response is > 256 */
	return ((byte_count <= MAX_VMC_SC_UART_BUF_SIZE) ? (byte_count) : (-1));
}

