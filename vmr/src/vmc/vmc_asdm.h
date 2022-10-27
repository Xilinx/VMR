/******************************************************************************
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/
#ifndef INC_VMC_ASDM_H_
#define INC_VMC_ASDM_H_

#include "xil_types.h"

#define SENSOR_NAME_MAX (30)
#define TEMP_CAGE0_NAME    "cage_temp_0\0"
#define TEMP_CAGE1_NAME    "cage_temp_1\0"

typedef enum VMC_Sensor_State_e
{
    Vmc_Snsr_State_Unavailable,
    Vmc_Snsr_State_Normal,
    Vmc_Snsr_State_Comms_failure,
    Vmc_Snsr_State_Warninig,
    Vmc_Snsr_State_Critical,
    Vmc_Snsr_State_Fatal,
    Vmc_Snsr_State_LowerWarning,
    Vmc_Snsr_State_LowerCritical,
    Vmc_Snsr_State_LowerFatal,
    Vmc_Snsr_State_UpperWarning,
    Vmc_Snsr_State_UpperCritical,
    Vmc_Snsr_State_UpperFatal
} VMC_Sensor_State_t;

typedef enum Asdm_Repository_Type_Enum_e
{
    BoardInfoSDR   = 0xC0,
    TemperatureSDR = 0xC1,
    VoltageSDR     = 0xC2,
    CurrentSDR	   = 0xC3,
    PowerSDR       = 0xC4,
    QSFPControlSDR = 0xC5,
    VPDPCIeSDR     = 0xD0,
    IPMIFRUSDR     = 0xD1,
    CSDRLogDataSDR = 0xE0,
    VMCLogDataSDR  = 0xE1,
}Asdm_RepositoryTypeEnum_t;

typedef enum
{
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
	eVoltage_Group_Sensors,
	eVoltage_Sysmon_Vccint,

	/* Current SDR */
	eCurrent_Group_Sensors,
	eCurrent_SC_Vccint,
	
	/* Power SDR */
	ePower_Total,

	eSdr_Sensor_Max
}Asdm_Sensor_PDR_List_t;

typedef enum Threshold_Support_Enum
{
    Upper_Warning_Threshold  = 0x01,
    Upper_Critical_Threshold = (0x01 << 1),
    Upper_Fatal_Threshold    = (0x01 << 2),
    Lower_Warning_Threshold  = (0x01 << 3),
    Lower_Critical_Threshold = (0x01 << 4),
    Lower_Fatal_Threshold    = (0x01 << 5),
    Sensor_Avg_Val_Support   = (0x01 << 6),
    Sensor_Max_Val_Support   = (0x01 << 7),
}SDR_Threshold_Support_Enum;

typedef enum Asdm_Completion_Code_e
{
    Asdm_CC_Not_Available		= 0x00,
    Asdm_CC_Operation_Success 		= 0x01,
    Asdm_CC_Operation_Failed 		= 0x02,
    Asdm_CC_Flow_Control_Read_Stale	= 0x03,
    Asdm_CC_Flow_Control_Write_Error    = 0x04,
    Asdm_CC_Invalid_Sensor_ID 		= 0x05,
}Asdm_Completion_Code_t;

typedef enum Asdm_CMD_Id_e {
	ASDM_CMD_GET_SIZE		= 0x1,
	ASDM_CMD_GET_SDR		= 0x2,
	ASDM_CMD_GET_SINGLE_SENSOR_DATA	= 0x3,
	ASDM_CMD_GET_ALL_SENSOR_DATA	= 0x4,
}Asdm_CMD_Id_t;

/*typedef s8 (*Asdm_GetRepository_func)(u8 *respData, u16 respSize);
  typedef s8 (*Asdm_GetSensorData_func)(u8 snsrIndex, u8 *respData, u16 respSize);


  typedef struct Asdm_ApiHandle_s
  {
  Asdm_RepositoryTypeEnum_t repoType;
  Asdm_GetRepository_func asdmGetRepoHandle;
  Asdm_GetSensorData_func asdmGetSnsrDataHandle;

  }Asdm_ApiHandle_t;
  */
typedef struct __attribute__((packed)) Asdm_Sensor_Unit_s
{
    u8 sensorUnitTypeLen;
    char8 *baseUnit;
}Asdm_Sensor_Unit_t;

typedef struct __attribute__((packed)) Asdm_Sensor_Thresholds_s
{
    char8 *sensorName;

    u16 lowerWarnLimit;
    u16 lowerCritLimit;
    u16 lowerFatalLimit;
    u16 upperWarnLimit;
    u16 upperCritLimit;
    u16 upperFatalLimit;

}Asdm_Sensor_Thresholds_t;


typedef struct __attribute__((packed)) snsrRead_s
{
    u8 sensorInstance;
    u8 mspSensorIndex;
    u8 snsrValue[4];
    u8 sensorValueSize;
    u8 snsrSatus;

}snsrRead_t;

typedef s8 (*sensorMonitorFunc)(snsrRead_t *snsrData);

typedef s8 (*snsrNameFunc)(u8 index, char8* snsrName, u8* snsrId, sensorMonitorFunc *sensor_handler );


typedef struct __attribute__((packed)) Asdm_SDRCount_s
{
    u8 recordIndex;
    Asdm_RepositoryTypeEnum_t repoType;
    u8 sensorIdCount;
}Asdm_SDRCount_t;

typedef struct __attribute__((packed)) Asdm_Sensor_MetaData_s
{
    Asdm_RepositoryTypeEnum_t repoType;
    char8 sensorName[20];
    u8 snsrValTypeLength;
    u8 *defaultValue;
    s8 snsrUnitModifier;
    u8 supportedThreshold;

    /*Multiple Instance of Similar Sensors */
    u8 sensorInstance;
    snsrNameFunc getSensorName;

    u8 sensorListTbl;
    u8 sampleCount;
    sensorMonitorFunc monitorFunc;

}Asdm_Sensor_MetaData_t;

typedef struct __attribute__((packed)) Asdm_Header_s
{
    u8 repository_type;
    u8 repository_version;
    u8 no_of_records;
    u8 no_of_bytes;
}Asdm_Header_t;

typedef struct  Asdm_SensorRecord_s
{
    u8 sensor_id;
    u8 sensor_name_type_length;
    char8 * sensor_name;
    u8 sensor_value_type_length;
    u8 * sensor_value;
    u8 sensor_base_unit_type_length;
    u8 * sensor_base_unit;
    s8 sensor_unit_modifier_byte;
    u8 threshold_support_byte;
    u8 * lower_fatal_limit;
    u8 * lower_critical_limit;
    u8 * lower_warning_limit;
    u8 * upper_fatal_limit;
    u8 * upper_critical_limit;
    u8 * upper_warning_limit;
    u8 sensor_status;
    u8 * sensorAverageValue;
    u8 * sensorMaxValue;

    /* For Internal usage */
    u8 mspSensorId;
    u32 sampleCount;
    u8 sensorInstance;
    sensorMonitorFunc snsrReadFunc;
}Asdm_SensorRecord_t;

typedef struct __attribute__((packed)) Asdm_EOR_s
{
    u8 EndOfRepoMarker[4];
}Asdm_EOR_t;

#define ASDM_EOR_MAX_SIZE		(4u)

typedef struct __attribute__((packed)) SDR_s
{
    Asdm_Header_t header;
    Asdm_SensorRecord_t *sensorRecord;
    u8 asdmEOR[ASDM_EOR_MAX_SIZE];

}SDR_t;


typedef enum Sensor_Status_Enum_e
{
    Sensor_Not_Present          = 0x00,
    Sensor_Present_And_Valid    = 0x01,
    Data_Not_Available          = 0x02,
    Sensor_Status_Not_Available = 0x7F,
}SDR_Sensor_Status_Enum_t;

typedef struct __attribute__((packed)) {
	Asdm_RepositoryTypeEnum_t record_type;
	u8 record_count;
}AsdmHeader_info_t;

s8 Init_Asdm();
void Asdm_Update_Sensors(void);
s8 Asdm_Process_Sensor_Request(u8 *req, u8 *resp, u16 *respSize);
s8 PMBUS_SC_Sensor_Read(snsrRead_t *snsrData);
s8 Temperature_Read_QSFP(snsrRead_t *snsrData);
s8 Temperature_Read_VCCINT(snsrRead_t *snsrData);


typedef void (*asdm_update_record_count_ptr) (Asdm_Header_t *headerInfo );

#endif

