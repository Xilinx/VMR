/******************************************************************************
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#include "vmc_sensors.h"
#include "vmc_asdm.h"
#include "cl_log.h"
#include "vmc_api.h"
#include "vmc_sc_comms.h"
#include "cl_mem.h"
#include <semphr.h>

#define ASDM_GET_SDR_SIZE_REQ (0x00)

#define SENSOR_NAME_MAX (30)
#define SENSOR_TYPE_NUM	(0x0)
#define SENSOR_TYPE_ASCII	(0xC << 4)

#define SENSOR_SIZE_1B	(0x1)
#define SENSOR_SIZE_2B	(0x2)
#define SENSOR_SIZE_4B	(0x4)

#define NO_THRESHOLDS	(0x0)
#define SNSR_MAX_VAL	(0x1 << 7)
#define SNSR_AVG_VAL	(0x1 << 6)
#define HAS_LOWER_THRESHOLDS	(0x7 << 3)
#define HAS_UPPER_THRESHOLDS	(0x7)

#define LENGTH_BITMASK   (0x3F)
#define SENSOR_REPOSITORY_REQUEST	(0xF1)
#define THRESHOLDS_UNSUPPORTED	(0x00)

#define QSFP_MAX_NUM    (2)

#define ASDM_HEADER_VER     (0x1)
#define NUM_BOARD_INFO_SENSORS  (12)
#define NUM_TEMPERATURE_SENSORS  (5)
#define NUM_SC_VOLTAGE_SENSORS  (5)
#define NUM_SYSMON_VOLTAGE_SENSORS	(1)
#define NUM_SC_CURRENT_SENSORS  (4)
#define NUM_POWER_SENSORS       (1)
#define CURRENT_SENSORS_INSTANCES (3)

#define SNSRNAME_ACTIVE_SC_VER "Active SC Ver\0"
/* Temp Sensor Names */
#define TEMP_BOARD_NAME    "PCB\0"
#define TEMP_ACAP_NAME     "device\0"
#define TEMP_VCCINT_NAME   "vccint\0"
#define TEMP_CAGE0_NAME    "cage_temp_0\0"
#define TEMP_CAGE1_NAME    "cage_temp_1\0"

/* Voltage and Current Sensor Name */
#define VCCINT_NAME    "vccint\0"

/* ASDM API Req/Resp Offsets */
#define ASDM_REQ_BYTE_CMD_CODE		(0)
#define ASDM_REQ_BYTE_REPO_TYPE		(1)
#define ASDM_REQ_BYTE_SNSR_ID		(2)

/* CC - Completion Code */
#define ASDM_RESP_BYTE_CC		(0)
#define ASDM_RESP_BYTE_REPO_TYPE	(1)
#define ASDM_RESP_BYTE_SNSR_SIZE	(2)

SDR_t *sdrInfo;
extern SemaphoreHandle_t sdr_lock;
static u8 asdmInitSuccess = false;
extern Versal_BoardInfo board_info;
extern SC_VMC_Data sc_vmc_data;
extern u8 fpt_sc_version[3];

void Asdm_Update_Active_MSP_sensor();
s8 Temperature_Read_Inlet(snsrRead_t *snsrData);
s8 Temperature_Read_Outlet(snsrRead_t *snsrData);
s8 Temperature_Read_Board(snsrRead_t *snsrData);
s8 Temperature_Read_QSFP(snsrRead_t *snsrData);
s8 Temperature_Read_ACAP_Device_Sysmon(snsrRead_t *snsrData);
//s8 Fan_RPM_Read(snsrRead_t *snsrData);
s8 PMBUS_SC_Sensor_Read(snsrRead_t *snsrData);
s8 Power_Monitor(snsrRead_t *snsrData);
s8 PMBUS_SC_Vccint_Read(snsrRead_t *snsrData);
s8 VCCINT_Read_ACAP_Device_Sysmon(snsrRead_t *snsrData);

Asdm_Sensor_Thresholds_t thresholds_limit_tbl[]= {
    /*  Name           LW   LC   LF    UW   UC  UF  */
    { TEMP_BOARD_NAME,	0,   0,  0,     80,  85, 95 },
    { TEMP_ACAP_NAME,	0,   0,  0,     88,  97, 107 },
    { TEMP_VCCINT_NAME,	0,   0,  0,     100, 110, 125 },
    { TEMP_CAGE0_NAME,  0,   0,  0,     80,  85, 90 },
    { TEMP_CAGE1_NAME,  0,   0,  0,     80,  85, 90 },

};

Asdm_Sensor_Unit_t sensor_unit_tbl[] = {
	[BoardInfoSDR]   = { 0x0 , "NA" },
	[TemperatureSDR] = { 0xC8, "Celsius\0" },
	[VoltageSDR]     = { 0xC6, "Volts\0" },
	[CurrentSDR]     = { 0xC5, "Amps\0" },
	[PowerSDR]       = { 0xC6, "Watts\0" },
};
#define THRESHOLD_TBL_SIZE 	(sizeof(thresholds_limit_tbl)/sizeof(thresholds_limit_tbl[0]))


void getCurrentNames(u8 index, char8* snsrName, u8 *sensorId)
{
    struct sensorData
    {
        u8 sensorId;
        char8 sensorName[SENSOR_NAME_MAX];
    };

    struct sensorData snsrData[] =    {
        { PEX_12V_I_IN,   "12v_pex\0"  },
        { V12_IN_AUX0_I,  "12v_aux_0\0" },
        { V12_IN_AUX1_I,  "12v_aux_1\0" },
    };

    if(NULL != snsrName)
    {
        Cl_SecureMemcpy(snsrName,SENSOR_NAME_MAX,&snsrData[index].sensorName[0],SENSOR_NAME_MAX);

    }
    if(NULL != sensorId)
    {
        *sensorId = snsrData[index].sensorId;
    }
}

void getVoltagesName(u8 index, char8* snsrName, u8 *sensorId)
{
    struct sensorData
    {
        u8 sensorId;
        char8 sensorName[SENSOR_NAME_MAX];
    };

    struct sensorData voltageData[] =
    {
        { PEX_12V,    "12v_pex\0" },
        { PEX_3V3,    "3v3_pex\0" },
        { AUX_3V3,     "3v3_aux\0" },
        { AUX_12V,     "12v_aux_0\0" },
        { AUX1_12V,    "12v_aux_1\0" }
    };

    if(NULL != snsrName)
    {
        Cl_SecureMemcpy(snsrName,SENSOR_NAME_MAX,&voltageData[index].sensorName[0],SENSOR_NAME_MAX);
    }
    if(NULL != sensorId)
    {
        *sensorId = voltageData[index].sensorId;
    }
}

void getQSFPName(u8 index, char8* snsrName, u8 *sensorId)
{
    struct sensorData
    {
        u8 sensorId;
        char8 sensorName[SENSOR_NAME_MAX];
    };

    struct sensorData qsfpData[] =
    {
        { CAGE_TEMP0,  TEMP_CAGE0_NAME },
        { CAGE_TEMP1,  TEMP_CAGE1_NAME }
    };

    if(NULL != snsrName)
    {
        Cl_SecureMemcpy(snsrName,SENSOR_NAME_MAX,&qsfpData[index].sensorName[0],SENSOR_NAME_MAX);
    }
    if(NULL != sensorId)
    {
        *sensorId = qsfpData[index].sensorId;
    }
}

Asdm_Header_t asdmHeaderInfo[] = {
    /* Record Type	| Hdr Version | Record Count | NumBytes */
    {BoardInfoSDR ,  	ASDM_HEADER_VER  ,  NUM_BOARD_INFO_SENSORS, 	0x7f},
    {TemperatureSDR, 	ASDM_HEADER_VER  ,  NUM_TEMPERATURE_SENSORS,	0x7f},
    {VoltageSDR,  	ASDM_HEADER_VER  ,  NUM_SC_VOLTAGE_SENSORS+NUM_SYSMON_VOLTAGE_SENSORS, 	0x7f},
    {CurrentSDR, 	ASDM_HEADER_VER  ,  NUM_SC_CURRENT_SENSORS,    0x7f},
    {PowerSDR, 		ASDM_HEADER_VER  ,  NUM_POWER_SENSORS,	        0x7f},
};

#define MAX_SDR_REPO 	(sizeof(asdmHeaderInfo)/sizeof(asdmHeaderInfo[0]))

/****************************************************************
 * Note:-
 * Add New sensors in below snsrMetaData [].
 * Update the "Record Count" for each New sensor in above asdmHeaderInfo 
 * structure.
 * 
 * New sensor to be added based on Repo type sequence i.e, if its a
 * new BoardInfo(0xc0) sensor, that has to be append before the next
 * TemperatureSDR(0xc1) starts.
 *****************************************************************/
void getSDRMetaData(Asdm_Sensor_MetaData_t **pMetaData, u16 *sdrMetaDataCount)
{
    Asdm_Sensor_MetaData_t snsrMetaData[] = {
	{
	    .repoType = BoardInfoSDR,
	    .sensorName = "Product Name\0",
	    .snsrValTypeLength = SENSOR_TYPE_ASCII | sizeof(board_info.product_name),
	    .defaultValue = &board_info.product_name[0],
	},
	{
	    .repoType = BoardInfoSDR,
	    .sensorName = "Serial Num\0",
	    .snsrValTypeLength = SENSOR_TYPE_ASCII | sizeof(board_info.board_serial),
	    .defaultValue = &board_info.board_serial[0],
	},
	{
	    .repoType = BoardInfoSDR,
	    .sensorName = "Part Num\0",
	    .snsrValTypeLength = SENSOR_TYPE_ASCII | sizeof(board_info.board_part_num),
	    .defaultValue = &board_info.board_part_num[0],
	},
	{
	    .repoType = BoardInfoSDR,
	    .sensorName = "Revision\0",
	    .snsrValTypeLength = SENSOR_TYPE_ASCII | sizeof(board_info.board_rev),
	    .defaultValue = &board_info.board_rev[0],
	},
	{
	    .repoType = BoardInfoSDR,
	    .sensorName = "MFG Date\0",
	    .snsrValTypeLength = SENSOR_TYPE_NUM | sizeof(board_info.board_mfg_date),
	    .defaultValue = &board_info.board_mfg_date[0],
	},
	{
	    .repoType = BoardInfoSDR,
	    .sensorName = "UUID\0",
	    .snsrValTypeLength = SENSOR_TYPE_NUM | sizeof(board_info.board_uuid),
	    .defaultValue = &board_info.board_uuid[0],
	},
	{
	    .repoType = BoardInfoSDR,
	    .sensorName = "MAC 0\0",
	    .snsrValTypeLength = SENSOR_TYPE_NUM | sizeof(board_info.board_mac[0]),
	    .defaultValue = board_info.board_mac[0],
	},
	{
	    .repoType = BoardInfoSDR,
	    .sensorName = "MAC 1\0",
	    .snsrValTypeLength = SENSOR_TYPE_NUM | sizeof(board_info.board_mac[1]),
	    .defaultValue = board_info.board_mac[1],
        },
    {
        .repoType = BoardInfoSDR,
        .sensorName = "fpga_fan_1\0",
        .snsrValTypeLength = SENSOR_TYPE_ASCII | sizeof(board_info.board_act_pas),
        .defaultValue = &board_info.board_act_pas[0],
    },
    {
        .repoType = BoardInfoSDR,
        .sensorName = SNSRNAME_ACTIVE_SC_VER,
        .snsrValTypeLength = SENSOR_TYPE_NUM | sizeof(sc_vmc_data.scVersion),
        .defaultValue = &sc_vmc_data.scVersion[0],
    },
    {
        .repoType = BoardInfoSDR,
        .sensorName = "Target SC Ver\0",
        .snsrValTypeLength = SENSOR_TYPE_NUM | sizeof(fpt_sc_version),
        .defaultValue = &fpt_sc_version[0],
    },
	{
	    .repoType = BoardInfoSDR,
	    .sensorName = "OEM ID\0",
	    .snsrValTypeLength = SENSOR_TYPE_NUM | sizeof(board_info.OEM_ID),
	    .defaultValue = &board_info.OEM_ID[0],
	},
/*	{
	    .repoType = TemperatureSDR,
	    .sensorName = "Inlet Temp\0",
	    .snsrValTypeLength = SENSOR_TYPE_NUM | SENSOR_SIZE_2B,
	    .snsrUnitModifier = 0x0,
	    .supportedThreshold = SNSR_MAX_VAL | SNSR_AVG_VAL | HAS_UPPER_THRESHOLDS,
	    .sesnorListTbl = SE98_TEMP0,
	    .sampleCount = 0x1,
	    .monitorFunc = &Temperature_Read_Inlet,

	},
	{
	    .repoType = TemperatureSDR,
	    .sensorName = "Outlet Temp\0",
	    .snsrValTypeLength = SENSOR_TYPE_NUM | SENSOR_SIZE_2B,
	    .snsrUnitModifier = 0x0,
	    .supportedThreshold = SNSR_MAX_VAL | SNSR_AVG_VAL | HAS_UPPER_THRESHOLDS,
	    .sampleCount = 0x1,
	    .sesnorListTbl = SE98_TEMP1,
	    .monitorFunc = &Temperature_Read_Outlet,
	},
*/
	{
	    .repoType = TemperatureSDR,
	    .sensorName = TEMP_BOARD_NAME,
	    .snsrValTypeLength = SENSOR_TYPE_NUM | SENSOR_SIZE_2B,
	    .snsrUnitModifier = 0x0,
	    .supportedThreshold = SNSR_MAX_VAL | SNSR_AVG_VAL | HAS_UPPER_THRESHOLDS,
	    .sampleCount = 0x1,
	    .sesnorListTbl = FAN_TEMP,
	    .monitorFunc = &Temperature_Read_Board,
	},
	{
	    .repoType = TemperatureSDR,
	    .sensorName = TEMP_ACAP_NAME,
	    .snsrValTypeLength = SENSOR_TYPE_NUM | SENSOR_SIZE_2B,
	    .snsrUnitModifier = 0x0,
	    .supportedThreshold = SNSR_MAX_VAL | SNSR_AVG_VAL | HAS_UPPER_THRESHOLDS,
	    .sampleCount = 0x1,
	    .sesnorListTbl = FPGA_TEMP,
	    .monitorFunc = &Temperature_Read_ACAP_Device_Sysmon,
	},
	{
	    .repoType = TemperatureSDR,
	    .sensorName = TEMP_VCCINT_NAME,
	    .snsrValTypeLength = SENSOR_TYPE_NUM | SENSOR_SIZE_2B,
	    .snsrUnitModifier = 0x0,
	    .supportedThreshold = SNSR_MAX_VAL | SNSR_AVG_VAL | HAS_UPPER_THRESHOLDS,
	    .sampleCount = 0x1,
	    .sesnorListTbl = VCCINT_TEMP,
	    .monitorFunc = &PMBUS_SC_Sensor_Read,
	},
	{
	    .repoType = TemperatureSDR,
	    .getSensorName = &getQSFPName,
	    .snsrValTypeLength = SENSOR_TYPE_NUM | SENSOR_SIZE_2B,
	    .snsrUnitModifier = 0x0,
	    .supportedThreshold = SNSR_MAX_VAL | SNSR_AVG_VAL | HAS_UPPER_THRESHOLDS,
	    .sampleCount = 0x1,
	    .sensorInstance = QSFP_MAX_NUM,
	    .monitorFunc = &Temperature_Read_QSFP,
	},
	{
	    .repoType = VoltageSDR,
	    .getSensorName = &getVoltagesName,
	    .snsrValTypeLength = SENSOR_TYPE_NUM | SENSOR_SIZE_2B,
	    .snsrUnitModifier = -3,
	    .supportedThreshold = SNSR_MAX_VAL | SNSR_AVG_VAL ,
	    .sampleCount = 0x1,
	    .sensorInstance = NUM_SC_VOLTAGE_SENSORS,
	    .monitorFunc = &PMBUS_SC_Sensor_Read,
	},
	{
		.repoType = VoltageSDR,
		.sensorName = VCCINT_NAME,
		.snsrValTypeLength = SENSOR_TYPE_NUM | SENSOR_SIZE_4B,
		.snsrUnitModifier = -3,
		.supportedThreshold = SNSR_MAX_VAL | SNSR_AVG_VAL ,
		.sampleCount = 0x1,
		.sesnorListTbl = VCCINT,
		.monitorFunc = &VCCINT_Read_ACAP_Device_Sysmon,
	},
	{
	    .repoType = CurrentSDR,
	    .getSensorName = &getCurrentNames,
	    .snsrValTypeLength = SENSOR_TYPE_NUM | SENSOR_SIZE_2B,
	    .snsrUnitModifier = -3,
	    .supportedThreshold = SNSR_MAX_VAL | SNSR_AVG_VAL ,
	    .sampleCount = 0x1,
	    .sensorInstance = CURRENT_SENSORS_INSTANCES,
	    .monitorFunc = &PMBUS_SC_Sensor_Read,
	},
	{
	    .repoType = CurrentSDR,
	    .sensorName = VCCINT_NAME,
	    .snsrValTypeLength = SENSOR_TYPE_NUM | SENSOR_SIZE_4B,
	    .snsrUnitModifier = -3,
	    .supportedThreshold = SNSR_MAX_VAL | SNSR_AVG_VAL ,
	    .sampleCount = 0x1,
	    .sesnorListTbl = VCCINT_I,
	    .monitorFunc = &PMBUS_SC_Vccint_Read,
	},
	{
	    .repoType = PowerSDR,
	    .sensorName = "Total Power\0",
	    .snsrValTypeLength = SENSOR_TYPE_NUM | SENSOR_SIZE_2B,
	    .snsrUnitModifier = 0,
	    .supportedThreshold = SNSR_MAX_VAL | SNSR_AVG_VAL ,
	    .sampleCount = 0x1,
	    .monitorFunc = &Power_Monitor,
	},
    };

    /* Get Record Count */
    *sdrMetaDataCount = (sizeof(snsrMetaData) / sizeof(snsrMetaData[0]));

    *pMetaData = (Asdm_Sensor_MetaData_t *) pvPortMalloc( sizeof(Asdm_Sensor_MetaData_t) * (*sdrMetaDataCount));

    if(*pMetaData != NULL)
    {
    	Cl_SecureMemset(*pMetaData,0x00,sizeof(snsrMetaData));
    	Cl_SecureMemcpy(*pMetaData,sizeof(snsrMetaData),snsrMetaData,sizeof(snsrMetaData));
    }
    else
    {
	VMC_ERR(" pvPortMalloc Failed\n\r");
	return;
    }
}

u8 getThresholdIdx(u8 *threshIdx,char8 *snsrName,u8 snsrNameLen)
{
	u8 i=0;
	for(i=0; i<THRESHOLD_TBL_SIZE; i++)
	{
		if(!Cl_SecureStrncmp(snsrName,snsrNameLen-2,thresholds_limit_tbl[i].sensorName,snsrNameLen-2))
		{
			*threshIdx = i;
			break;
		}
	}
	return 0;
}

u8 isRepoTypeSupported(u32 sensorType)
{
	u8 i=0;

	for(i=0;i<MAX_SDR_REPO;i++)
	{
		if(asdmHeaderInfo[i].repository_type == sensorType)
		{
			return true;
		}
	}
	return false;
}

/*
 * This Function converts the Standard RepoTypes to the
 * corresponding Record/sensor Index stored in VMC memory.
 *
 * Sensor Data in VMC is stored in a 2D array format:
 * RepoType -> Sensors Index
 *        ---------------------------------------------
 * 0xC0 -| SensorId_1 | SensorId_2 | .... | SensorId_n |
 * 0xC1 -| SensorId_1 | SensorId_2 | .... | SensorId_n |
 *        ---------------------------------------------
 */
s8 getSDRIndex(u8 repoType)
{
	s8 i=-1;

	for(i=0;i<MAX_SDR_REPO;i++)
	{
		if(asdmHeaderInfo[i].repository_type == repoType)
		{
			return i;
		}
	}

	return i;
}

u8 Update_Sensor_Value(Asdm_RepositoryTypeEnum_t repoType, u8 sensorIdx, snsrRead_t *snsrInfo)
{
    /* Get the Repo index from Repotype */
    u8 repoIndex = getSDRIndex(repoType);
    /* SensorIdx is 0 indexed in memory vs 1 index in ASDM SDR, so substracting by 1 */
    Asdm_SensorRecord_t *sensorRecord = &sdrInfo[repoIndex].sensorRecord[sensorIdx - 1];

    if(NULL != sensorRecord)
    {
	if (xSemaphoreTake(sdr_lock, portMAX_DELAY))
	{
	  if( (0 != snsrInfo->sensorValueSize) && (NULL != &sensorRecord->sensor_value) ){
	    Cl_SecureMemcpy(sensorRecord->sensor_value,snsrInfo->sensorValueSize,&snsrInfo->snsrValue[0],snsrInfo->sensorValueSize);
	  }
	    /* Update only if the sensor is not Static */
	    if(sensorRecord->sampleCount > 0)
	    {
		/* Update Sensor Status */
		sensorRecord->sensor_status = snsrInfo->snsrSatus;

		if(snsrInfo->sensorValueSize == sizeof(u32))
		{
			/* Update Max Sensor Value */
			if ( *((u32 *)sensorRecord->sensor_value) > *((u32 *)sensorRecord->sensorMaxValue))
			{
				Cl_SecureMemcpy(sensorRecord->sensorMaxValue,snsrInfo->sensorValueSize,sensorRecord->sensor_value,
				    snsrInfo->sensorValueSize);
			}

			/* Calculate and Update Average Value */
			*((u32 *)sensorRecord->sensorAverageValue) = *((u32 *)sensorRecord->sensorAverageValue)
			    				- (*((u32 *)sensorRecord->sensorAverageValue)/sensorRecord->sampleCount)
			    				+ (*((u32 *)sensorRecord->sensor_value)/sensorRecord->sampleCount);
		}
		else if(snsrInfo->sensorValueSize == sizeof(u16))
		{
			/* Update Max Sensor Value */
			if ( *((u16 *)sensorRecord->sensor_value) > *((u16 *)sensorRecord->sensorMaxValue))
			{
				Cl_SecureMemcpy(sensorRecord->sensorMaxValue,snsrInfo->sensorValueSize,sensorRecord->sensor_value,
						snsrInfo->sensorValueSize);
			}

			/* Calculate and Update Average Value */
			*((u16 *)sensorRecord->sensorAverageValue) = (*((u16 *)sensorRecord->sensorAverageValue)
					    				- (*((u16 *)sensorRecord->sensorAverageValue)/sensorRecord->sampleCount))
									+ (*((u16 *)sensorRecord->sensor_value)/sensorRecord->sampleCount);
		}
		else if(snsrInfo->sensorValueSize == sizeof(u8))
		{
			/* Update Max Sensor Value */
			if ( *((u8 *)sensorRecord->sensor_value) > *((u8 *)sensorRecord->sensorMaxValue))
			{
				Cl_SecureMemcpy(sensorRecord->sensorMaxValue,snsrInfo->sensorValueSize,sensorRecord->sensor_value,
						snsrInfo->sensorValueSize);
			}

			/* Calculate and Update Average Value */
			*((u8 *)sensorRecord->sensorAverageValue) = (*((u8 *)sensorRecord->sensorAverageValue)
					    				- (*((u8 *)sensorRecord->sensorAverageValue)/sensorRecord->sampleCount))
									+ (*((u8 *)sensorRecord->sensor_value)/sensorRecord->sampleCount);
		}
		/* Increment the Sample Count after a Successful read*/
		sensorRecord->sampleCount++;
	    }
	    xSemaphoreGive(sdr_lock);
	}
	else
	{
	    VMC_ERR("Failed to Acquire sdr lock\n\r");
	    return -1;
	}
    }
    else
    {
	VMC_ERR("InValid SDR Data\n\r");
	return -1;
    }

    return 0;
}

s8 Init_Asdm()
{
    u8 sdrCount = 0;
    u16 allocateSize = 0;
    u8 snsrNameLen = 0;
    char8 snsrName[SENSOR_NAME_MAX] = {0};
    u8 snsrValueLen = 0;
    u8 baseUnitLen = 0;
    u16 sdrMetaDataCount = 0;
    u16 totalRecords = 0;
    Asdm_Sensor_MetaData_t *pSdrMetaData = NULL;
    u16 idx = 0;
    u16 byteCount = 0;
    u8 threshIdx = 0;
    u8 currentRepoType = 0;
    u8 totalSensorInstances = 0;
    u8 sensorInstance = 0;

    /* Fetch the ASDM SDR Repository */
    getSDRMetaData(&pSdrMetaData, &sdrMetaDataCount);

    /* Validation check if SDR Data Count is Valid */
    while(idx < MAX_SDR_REPO)
    {
	totalRecords += asdmHeaderInfo[idx].no_of_records;
	idx++;
    }

    if(sdrMetaDataCount != totalRecords)
    {
	/* LoG Error, don't proceed for Init */
	VMC_ERR("Records Count Mismatch  !!!\n\r");
	//return -1;
    }

    /* for future use */
    totalRecords = 0;

    if(pSdrMetaData != NULL)
    {
	/* Allocate Memory for Number of Repo Type */
	sdrInfo = (SDR_t *) pvPortMalloc(MAX_SDR_REPO * sizeof(SDR_t));
	if(sdrInfo == NULL)
	{
	    VMC_ERR("Failed to allocate Memory for SDR !!!\n\r");
	    return -1;
	}
	Cl_SecureMemset(sdrInfo,0x00,(MAX_SDR_REPO * sizeof(SDR_t)));

	for(sdrCount = 0; sdrCount < MAX_SDR_REPO; sdrCount++)
	{
	    /* Fill ASDM Header */
	    sdrInfo[sdrCount].header.repository_type = asdmHeaderInfo[sdrCount].repository_type;
	    sdrInfo[sdrCount].header.repository_version = asdmHeaderInfo[sdrCount].repository_version;
	    sdrInfo[sdrCount].header.no_of_records = asdmHeaderInfo[sdrCount].no_of_records;
	    sdrInfo[sdrCount].header.no_of_bytes = asdmHeaderInfo[sdrCount].no_of_bytes;

	    /* Based on Number of Records allocate memory of Sensor Records*/
	    allocateSize = sizeof(Asdm_SensorRecord_t) * asdmHeaderInfo[sdrCount].no_of_records;
	    sdrInfo[sdrCount].sensorRecord = (Asdm_SensorRecord_t *)pvPortMalloc(allocateSize);
	    if(NULL == sdrInfo[sdrCount].sensorRecord)
	    {
		VMC_ERR("Failed to allocate Memory for SDR Record !!!\n\r");
		return -1;
	    }

	    Cl_SecureMemset(sdrInfo[sdrCount].sensorRecord,0x00,allocateSize);

	    Asdm_SensorRecord_t *tmp = sdrInfo[sdrCount].sensorRecord;
	    currentRepoType = asdmHeaderInfo[sdrCount].repository_type;

	    byteCount = 0;
	    /* Initialize each Sensor Info, idx incremented below */
	    for(idx = 0; idx < asdmHeaderInfo[sdrCount].no_of_records; )
	    {
		totalSensorInstances = pSdrMetaData[totalRecords].sensorInstance;
		/* By default totalSensorInstance is considered as 1,
		 * even if its uninitialized
		 */
		totalSensorInstances = (totalSensorInstances <= 1) ? 1 : totalSensorInstances;
		/* If Sensor Instance > 1, initialize it in loop */
		sensorInstance = 0;
		while(sensorInstance < totalSensorInstances)
		{
		/* Assign  the Sensor Id Count for this RepoType  */
		tmp[idx].sensor_id = idx + 1;
		byteCount += sizeof(tmp[idx].sensor_id);

		/* Sensor Name
		 * Based on Name Type and Length allocate Memory for Sensor Name
		 */
		if(totalSensorInstances > 1)
		{
		    Cl_SecureMemset(snsrName,0x00,SENSOR_NAME_MAX);
		    pSdrMetaData[totalRecords].getSensorName(sensorInstance,snsrName, &tmp[idx].mspSensorId);
		    snsrNameLen = strlen(snsrName) + 1;
		}
		else
		{
		    snsrNameLen = strlen(pSdrMetaData[totalRecords].sensorName) + 1;
		}
		tmp[idx].sensor_name_type_length = SENSOR_TYPE_ASCII | (snsrNameLen & LENGTH_BITMASK);
		byteCount += sizeof(tmp[idx].sensor_name_type_length);

		tmp[idx].sensor_name = (char8 *) pvPortMalloc(snsrNameLen);
		if(NULL != tmp[idx].sensor_name)
		{
			Cl_SecureMemset(tmp[idx].sensor_name, 0x00, snsrNameLen);
		    if(totalSensorInstances > 1)
		    {
		    	Cl_SecureMemcpy(tmp[idx].sensor_name,SENSOR_NAME_MAX,snsrName,snsrNameLen);
		    }
		    else
		    {
		    	Cl_SecureMemcpy(tmp[idx].sensor_name,SENSOR_NAME_MAX,
		            pSdrMetaData[totalRecords].sensorName, snsrNameLen);
		    }
		}
		else
		{
		    VMC_ERR("Failed to allocate Memory for Sensor Name !!!\n\r");
		    return -1;
		}

		byteCount += snsrNameLen;

		/* Sensor Value */
		tmp[idx].sensor_value_type_length = pSdrMetaData[totalRecords].snsrValTypeLength;
		byteCount += sizeof(tmp[idx].sensor_value_type_length);

		snsrValueLen = (pSdrMetaData[totalRecords].snsrValTypeLength & LENGTH_BITMASK);


		/* Only allocate the Memory, Value will be updated while Monitoring */
		tmp[idx].sensor_value = (u8 *) pvPortMalloc(snsrValueLen);
		if(NULL != tmp[idx].sensor_value)
		{
			Cl_SecureMemset(tmp[idx].sensor_value, 0x00, snsrValueLen);
			if(NULL != pSdrMetaData[totalRecords].defaultValue) {
				/* Value */
				Cl_SecureMemcpy(tmp[idx].sensor_value,snsrValueLen,pSdrMetaData[totalRecords].defaultValue,snsrValueLen);
			}
		}
		else
		{
		    VMC_ERR("Failed to allocate Memory for Sensor Value !!!\n\r");
		    return -1;
		}
		byteCount += snsrValueLen;

		/* Sensor Units */
		tmp[idx].sensor_base_unit_type_length = sensor_unit_tbl[currentRepoType].sensorUnitTypeLen;
		byteCount += sizeof(tmp[idx].sensor_base_unit_type_length);

		if(tmp[idx].sensor_base_unit_type_length != 0)
		{
		    baseUnitLen = (tmp[idx].sensor_base_unit_type_length & LENGTH_BITMASK);

		    tmp[idx].sensor_base_unit = (u8 *) pvPortMalloc(baseUnitLen);
		    if(NULL != tmp[idx].sensor_base_unit)
		    {
		    	Cl_SecureMemset(tmp[idx].sensor_base_unit, 0x00, baseUnitLen);
		    	Cl_SecureMemcpy(tmp[idx].sensor_base_unit,baseUnitLen,
					sensor_unit_tbl[currentRepoType].baseUnit, baseUnitLen);
		    }
		    else
		    {
			VMC_ERR("Failed to allocate Memory for BaseUnit !!");
			return -1;
		    }
		    byteCount += baseUnitLen;
		}

		/* Sensor unit modifier */
		tmp[idx].sensor_unit_modifier_byte = pSdrMetaData[totalRecords].snsrUnitModifier;
		byteCount += sizeof(tmp[idx].sensor_unit_modifier_byte) ;

		/* Sensor Threshold support byte */
		tmp[idx].threshold_support_byte = pSdrMetaData[totalRecords].supportedThreshold;
		byteCount += sizeof(tmp[idx].threshold_support_byte);

		/* Do not bother to check for BOARD INFO Sensor */
		if(sdrInfo[sdrCount].header.repository_type != BoardInfoSDR)
		{
			getThresholdIdx(&threshIdx, tmp[idx].sensor_name, snsrNameLen);

			/* Allocate the Supported Thresholds */
			if(tmp[idx].threshold_support_byte & Lower_Fatal_Threshold)
			{
				tmp[idx].lower_fatal_limit = (u8 *) pvPortMalloc(snsrValueLen);
				if(NULL == tmp[idx].lower_fatal_limit)
				{
					/* LOG Error and return */
					VMC_ERR("Failed to allocate Memory for lower_fatal_limit !!!\n\r");
					return -1;
				}
				Cl_SecureMemset(tmp[idx].lower_fatal_limit,0x00, snsrValueLen);
				Cl_SecureMemcpy(tmp[idx].lower_fatal_limit,snsrValueLen,&thresholds_limit_tbl[threshIdx].lowerFatalLimit,snsrValueLen);
				byteCount += snsrValueLen;
			}

			if(tmp[idx].threshold_support_byte & Lower_Critical_Threshold)
			{
				tmp[idx].lower_critical_limit = (u8 *) pvPortMalloc(snsrValueLen);
				if(NULL == tmp[idx].lower_critical_limit)
				{
					/* LOG Error and return */
					VMC_ERR("Failed to allocate Memory for lower_critical_limit !!!\n\r");
					return -1;
				}
				Cl_SecureMemset(tmp[idx].lower_critical_limit,0x00, snsrValueLen);
				Cl_SecureMemcpy(tmp[idx].lower_critical_limit,snsrValueLen,&thresholds_limit_tbl[threshIdx].lowerCritLimit,snsrValueLen);
				byteCount += snsrValueLen;
			}

			if(tmp[idx].threshold_support_byte & Lower_Warning_Threshold)
			{
				tmp[idx].lower_warning_limit = (u8 *) pvPortMalloc(snsrValueLen);
				if(NULL == tmp[idx].lower_warning_limit)
				{
					/* LOG Error and return */
					VMC_ERR("Failed to allocate Memory for lower_warning_limit !!!\n\r");
					return -1;
				}
				Cl_SecureMemset(tmp[idx].lower_warning_limit,0x00, snsrValueLen);
				Cl_SecureMemcpy(tmp[idx].lower_warning_limit,snsrValueLen,&thresholds_limit_tbl[threshIdx].lowerWarnLimit,snsrValueLen);
				byteCount += snsrValueLen;
			}

			if(tmp[idx].threshold_support_byte & Upper_Fatal_Threshold)
			{
				tmp[idx].upper_fatal_limit = (u8 *) pvPortMalloc(snsrValueLen);
				if(NULL == tmp[idx].upper_fatal_limit)
				{
					/* LOG Error and return */
					VMC_ERR("Failed to allocate Memory for upper_fatal_limit !!!\n\r");
					return -1;
				}
				Cl_SecureMemset(tmp[idx].upper_fatal_limit,0x00, snsrValueLen);
				Cl_SecureMemcpy(tmp[idx].upper_fatal_limit,snsrValueLen,&thresholds_limit_tbl[threshIdx].upperFatalLimit, snsrValueLen);
				byteCount += snsrValueLen;
			}

			if(tmp[idx].threshold_support_byte & Upper_Critical_Threshold)
			{
				tmp[idx].upper_critical_limit = (u8 *) pvPortMalloc(snsrValueLen);
				if(NULL == tmp[idx].upper_critical_limit)
				{
					/* LOG Error and return */
					VMC_ERR("Failed to allocate Memory for upper_critical_limit !!!\n\r");
					return -1;
				}
				Cl_SecureMemset(tmp[idx].upper_critical_limit,0x00, snsrValueLen);
				Cl_SecureMemcpy(tmp[idx].upper_critical_limit,snsrValueLen,&thresholds_limit_tbl[threshIdx].upperCritLimit, snsrValueLen);
				byteCount += snsrValueLen;
			}

			if(tmp[idx].threshold_support_byte & Upper_Warning_Threshold)
			{
				tmp[idx].upper_warning_limit = (u8 *) pvPortMalloc(snsrValueLen);
				if(NULL == tmp[idx].upper_warning_limit)
				{
					/* LOG Error and return */
					VMC_ERR("Failed to allocate Memory for upper_warning_limit !!!\n\r");
					return -1;
				}
				Cl_SecureMemset(tmp[idx].upper_warning_limit,0x00, snsrValueLen);
				Cl_SecureMemcpy(tmp[idx].upper_warning_limit,snsrValueLen,&thresholds_limit_tbl[threshIdx].upperWarnLimit, snsrValueLen);
				byteCount += snsrValueLen;
			}
		}

		/* Sensor Status */
		tmp[idx].sensor_status = Sensor_Present_And_Valid;
		byteCount += sizeof(tmp[idx].sensor_status);

		/* Update the Sample count, byteCount update not required (internal element) */
		tmp[idx].sampleCount = pSdrMetaData[totalRecords].sampleCount;

		/* Sample Count > 0, for only for Dynamic sensors */
		if( pSdrMetaData[totalRecords].sampleCount > 0)
		{
		    tmp[idx].sensorMaxValue = (u8 *) pvPortMalloc(snsrValueLen);
		    if(NULL == tmp[idx].sensorMaxValue)
		    {
			/* LOG Error and return */
			VMC_ERR("Failed to allocate Memory for sensorMaxValue !!!\n\r");
			return -1;
		    }
		    Cl_SecureMemset(tmp[idx].sensorMaxValue,0x00, snsrValueLen);
		    byteCount += snsrValueLen;

		    tmp[idx].sensorAverageValue = (u8 *) pvPortMalloc(snsrValueLen);
		    if(NULL == tmp[idx].sensorAverageValue)
		    {
			/* LOG Error and return */
			VMC_ERR("Failed to allocate Memory for sensorAverageValue !!!\n\r");
			return -1;
		    }
		    Cl_SecureMemset(tmp[idx].sensorAverageValue,0x00, snsrValueLen);
		    byteCount += snsrValueLen;
		}

		tmp[idx].sensorInstance = sensorInstance;
		/* Assign the Sensor ID Mapped to external  MSP (Internal)*/
		if(totalSensorInstances > 1)
		{
		    /* Gets updated during Sensor Name initialization */
		}
		else
		{
		    tmp[idx].mspSensorId = pSdrMetaData[totalRecords].sesnorListTbl;
		}

		/* Update the Monitor Function for the Sensor (Internal)*/
		tmp[idx].snsrReadFunc = pSdrMetaData[totalRecords].monitorFunc;

		idx++;
		sensorInstance++;
		}
		/* One Record is initialized, move to the Next SDRMetaData */
		totalRecords++;

	    }

	    /* Fill the EOR */
	    Cl_SecureMemcpy(sdrInfo[sdrCount].asdmEOR,ASDM_EOR_MAX_SIZE,"END",sizeof(u8) * strlen("END"));
	    byteCount += (sizeof(u8) * strlen("END"));

	    /* Update the Byte Count for this record in Multiple of 8
	     * if not an exact multiple, add pad bytes to round it off */
	    if((byteCount % 8) != 0)
	    {
	    	u8 tmp = (byteCount % 8);
	    	byteCount += (8 - tmp);
	    }
	    sdrInfo[sdrCount].header.no_of_bytes = (byteCount/8);
	}

	vPortFree(pSdrMetaData);
    }
    else
    {
	/* Log Error ASDM Init Failed */
	VMC_LOG("ASDM Init Failed, MetaData not available !!!\n\r");
	return -1;
    }

    asdmInitSuccess = true;
    VMC_LOG("ASDM Init success !!\n\r");

    return 0;
}


s8 Asdm_Get_SDR_Size(u8 *req, u8 *resp, u16 *respSize)
{
    u8 sdrIndex = 0;
    u16 sdrSize = 0;

    if((req == NULL) || (resp == NULL) || (respSize == NULL))
    {
	VMC_ERR(" Buffers received are NULL.\n\r req : %p, resp : %p, respSize : %p \n\r", \
		req, resp,respSize);
	return -1;
    }

    if(asdmInitSuccess != true)
    {
	VMC_ERR(" ASDM Data not Initialized yet or has Failed \n\r");
	return -1;
    }

    /* Get the SDR index from the request */
    sdrIndex = getSDRIndex(req[1]);

    /*Fill the Completion Code */
    resp[0] = Asdm_CC_Operation_Success;
    *respSize += sizeof(u8);

    /*Fill the Repo Type */
    resp[1] = req[1];
    *respSize += sizeof(u8);

    /* Fill the Size of the SDR */
    sdrSize = sizeof(Asdm_Header_t) + (sdrInfo[sdrIndex].header.no_of_bytes * 8)
    		 + sizeof(Asdm_EOR_t);
    Cl_SecureMemcpy(&resp[2],sizeof(sdrSize),&sdrSize, sizeof(sdrSize));

    *respSize +=  sizeof(sdrSize);

    return 0;

}

s8 Asdm_Get_Sensor_Repository(u8 *req, u8 *resp, u16 *respSize)
{
    s8 retStatus = -1;
    u8 sdrIndex = 0;
    u8 idx = 0;
    u8 snsrNameLen = 0;
    u8 snsrValueLen = 0;
    u8 baseUnitLen = 0;

    u16 respIndex = 0;

    if((req == NULL) || (resp == NULL) || (respSize == NULL))
    {
	VMC_ERR(" Buffers received are NULL.\n\r req : %p, resp : %p, respSize : %p \n\r",
		req, resp,respIndex);
	return -1;
    }

    sdrIndex = getSDRIndex(req[1]);

    if (xSemaphoreTake(sdr_lock, portMAX_DELAY))
    {
	/* Start from Offset 1 , 0 is for completion code, we fill it at the End */
	respIndex = 1;
	/* Populate the Header */
	Cl_SecureMemcpy(&resp[respIndex], sizeof(Asdm_Header_t), &sdrInfo[sdrIndex].header, sizeof(Asdm_Header_t));
	respIndex += sizeof(Asdm_Header_t);

	Asdm_SensorRecord_t *tmp = sdrInfo[sdrIndex].sensorRecord;
	/* Populate the Sensor Records */
	for(idx = 0; idx < sdrInfo[sdrIndex].header.no_of_records; idx++)
	{
	    resp[respIndex++] = tmp[idx].sensor_id;
	    resp[respIndex++] = tmp[idx].sensor_name_type_length;

	    /* Copy Sensor Name */
	    snsrNameLen = (tmp[idx].sensor_name_type_length & LENGTH_BITMASK);
	    Cl_SecureMemcpy(&resp[respIndex],snsrNameLen, tmp[idx].sensor_name, snsrNameLen);
	    respIndex += snsrNameLen;

	    /*  Sensor Value */
	    resp[respIndex++] = tmp[idx].sensor_value_type_length;

	    snsrValueLen = (tmp[idx].sensor_value_type_length & LENGTH_BITMASK);
	    if(snsrValueLen != 0)
	    {
		Cl_SecureMemcpy(&resp[respIndex],snsrValueLen, tmp[idx].sensor_value, snsrValueLen);
		respIndex += snsrValueLen;
	    }

	    /* Sensor Base unit */
	    resp[respIndex++] = tmp[idx].sensor_base_unit_type_length;

	    baseUnitLen = (tmp[idx].sensor_base_unit_type_length & LENGTH_BITMASK);
	    if(baseUnitLen != 0)
	    {
		Cl_SecureMemcpy(&resp[respIndex],baseUnitLen, tmp[idx].sensor_base_unit, baseUnitLen);
		respIndex += baseUnitLen;
	    }

	    resp[respIndex++] = tmp[idx].sensor_unit_modifier_byte;

	    /* Sensor Thresholds */
	    resp[respIndex++] = tmp[idx].threshold_support_byte;

	    if(tmp[idx].threshold_support_byte & Lower_Fatal_Threshold)
	    {
	    Cl_SecureMemcpy(&resp[respIndex],snsrValueLen, tmp[idx].lower_fatal_limit,snsrValueLen);
		respIndex += snsrValueLen;
	    }
	    if(tmp[idx].threshold_support_byte & Lower_Critical_Threshold)
	    {
		Cl_SecureMemcpy(&resp[respIndex], snsrValueLen,tmp[idx].lower_critical_limit,snsrValueLen);
		respIndex += snsrValueLen;
	    }
	    if(tmp[idx].threshold_support_byte & Lower_Warning_Threshold)
	    {
		Cl_SecureMemcpy(&resp[respIndex], snsrValueLen,tmp[idx].lower_warning_limit,snsrValueLen);
		respIndex += snsrValueLen;
	    }
	    if(tmp[idx].threshold_support_byte & Upper_Fatal_Threshold)
	    {
		Cl_SecureMemcpy(&resp[respIndex],snsrValueLen,tmp[idx].upper_fatal_limit,snsrValueLen);
		respIndex += snsrValueLen;
	    }
	    if(tmp[idx].threshold_support_byte & Upper_Critical_Threshold)
	    {
		Cl_SecureMemcpy(&resp[respIndex], snsrValueLen,tmp[idx].upper_critical_limit,snsrValueLen);
		respIndex += snsrValueLen;
	    }
	    if(tmp[idx].threshold_support_byte & Upper_Warning_Threshold)
	    {
		Cl_SecureMemcpy(&resp[respIndex], snsrValueLen,tmp[idx].upper_warning_limit,snsrValueLen);
		respIndex += snsrValueLen;
	    }

	    resp[respIndex++] =  tmp[idx].sensor_status;

	    if(tmp[idx].threshold_support_byte & Sensor_Avg_Val_Support)
	    {
		Cl_SecureMemcpy(&resp[respIndex], snsrValueLen,tmp[idx].sensorAverageValue,snsrValueLen);
		respIndex += snsrValueLen;
	    }

	    if(tmp[idx].threshold_support_byte & Sensor_Max_Val_Support)
	    {
		Cl_SecureMemcpy(&resp[respIndex], snsrValueLen,tmp[idx].sensorMaxValue,snsrValueLen);
		respIndex += snsrValueLen;
	    }
	}

	/* Populate the EOR "END" */
	Cl_SecureMemcpy(&resp[respIndex],ASDM_EOR_MAX_SIZE, sdrInfo[sdrIndex].asdmEOR, (sizeof(u8) * strlen("END")));
	respIndex += (sizeof(u8) * strlen("END"));

	/* Release the Lock */
	xSemaphoreGive(sdr_lock);

	resp[0] = Asdm_CC_Operation_Success;
	*respSize = respIndex;

	retStatus = 0;
    }
    else
    {
	retStatus = -1;
	VMC_ERR("Unable to Acquire Sdr Lock \n\r");
	resp[0] = Asdm_CC_Operation_Failed;
	*respSize = 1;
    }


    return retStatus;
}

s8 Asdm_Get_All_Sensor_Data(u8 *req, u8 *resp, u16 *respSize)
{
	u8 sdrIndex = 0;
	u8 snsrIndex = 0;
	u8 total_records = 0;
	u8 payload_size = 0;
	u8 snsrValueLen = 0;
	u8 snsrStatusLen = 0;

	u16 respIndex = 0;

	if((req == NULL) || (resp == NULL) || (respSize == NULL)) {
		VMC_ERR(" Buffers received are NULL.\n\r req : %p, resp : %p, respSize : %p \n\r",
				req, resp,respSize);
		return -1;
	}

	if(asdmInitSuccess != true) {
		VMC_ERR(" ASDM Data not Initialized yet or has Failed \n\r");
		return -1;
	}

	sdrIndex = getSDRIndex(req[ASDM_REQ_BYTE_REPO_TYPE]);
	total_records = sdrInfo[sdrIndex].header.no_of_records;

	*respSize = 0;
	payload_size = 0;

	if (xSemaphoreTake(sdr_lock, portMAX_DELAY)) {
		/* Fetch the Sensor Record to extract the Value */
		Asdm_SensorRecord_t *tmp = sdrInfo[sdrIndex].sensorRecord;

		resp[ASDM_RESP_BYTE_CC] = Asdm_CC_Operation_Success;

		/* Copy the Repo Type */
		resp[ASDM_RESP_BYTE_REPO_TYPE] = req[ASDM_REQ_BYTE_REPO_TYPE];

		/* Send the Index to copy the Payload */
		respIndex = ASDM_RESP_BYTE_SNSR_SIZE + 1;

		/* Sensor Id are 1 indexed, but we have 0 indexed */
		for(snsrIndex = 0; snsrIndex < total_records; snsrIndex++) {

			/* Size of sensor Value * 3( Snsr Val + Max Snsr Val + Snsr Avg) */
			snsrValueLen = (tmp[snsrIndex].sensor_value_type_length & LENGTH_BITMASK);
			snsrStatusLen = sizeof(tmp[snsrIndex].sensor_status);

			payload_size += ( sizeof(snsrValueLen) + (3 * snsrValueLen) + snsrStatusLen);

			resp[respIndex++] = snsrValueLen;

			Cl_SecureMemcpy(&resp[respIndex],snsrValueLen,tmp[snsrIndex].sensor_value,snsrValueLen);
			respIndex += snsrValueLen;
			Cl_SecureMemcpy(&resp[respIndex],snsrValueLen,tmp[snsrIndex].sensorMaxValue,snsrValueLen);
			respIndex += snsrValueLen;
			Cl_SecureMemcpy(&resp[respIndex],snsrValueLen,tmp[snsrIndex].sensorAverageValue,snsrValueLen);
			respIndex += snsrValueLen;
			Cl_SecureMemcpy(&resp[respIndex],snsrStatusLen,&tmp[snsrIndex].sensor_status,snsrStatusLen);
			respIndex += snsrStatusLen;
		}
		xSemaphoreGive(sdr_lock);

		resp[ASDM_RESP_BYTE_SNSR_SIZE] = payload_size;
		/* 3 -> Length of (CC + REPOTYPE + Payload_Size) */
		*respSize = 3 + payload_size;

	} else {
		resp[ASDM_RESP_BYTE_CC] = Asdm_CC_Operation_Failed;
		*respSize = 1;
	}

	return 0;
}

s8 Asdm_Get_Sensor_Data(u8 *req, u8 *resp, u16 *respSize)
{
    u8 sdrIndex = 0;
    u8 snsrIndex = 0;
    u8 snsrValueLen = 0;
    u8 snsrStatusLen = 0;

    u16 respIndex = 0;

    if((req == NULL) || (resp == NULL) || (respSize == NULL))
    {
	VMC_ERR(" Buffers received are NULL.\n\r req : %p, resp : %p, respSize : %p \n\r",
		req, resp,respSize);
	return -1;
    }

    if(asdmInitSuccess != true)
    {
	VMC_ERR(" ASDM Data not Initialized yet or has Failed \n\r");
	return -1;
    }

    sdrIndex = getSDRIndex(req[ASDM_REQ_BYTE_REPO_TYPE]);
    /* Sensor Id are 1 indexed, but we have 0 indexed */
    snsrIndex = req[ASDM_REQ_BYTE_SNSR_ID] - 1;
    *respSize = 0;

    if (xSemaphoreTake(sdr_lock, portMAX_DELAY))
    {
    	/* Fetch the Sensor Record to extract the Value */
    	Asdm_SensorRecord_t *tmp = sdrInfo[sdrIndex].sensorRecord;

    	resp[ASDM_RESP_BYTE_CC] = Asdm_CC_Operation_Success;
    	respIndex = ASDM_RESP_BYTE_CC + 1;

    	/* Copy the Repo Type */
    	resp[ASDM_RESP_BYTE_REPO_TYPE] = req[ASDM_REQ_BYTE_REPO_TYPE];
    	respIndex = ASDM_RESP_BYTE_REPO_TYPE + 1;

    	/* Size of sensor Value * 3( Snsr Val + Max Snsr Val + Snsr Avg) */
    	snsrValueLen = (tmp[snsrIndex].sensor_value_type_length & LENGTH_BITMASK);
    	snsrStatusLen = sizeof(tmp[snsrIndex].sensor_status);

    	resp[respIndex++] = snsrValueLen;

    	Cl_SecureMemcpy(&resp[respIndex],snsrValueLen,tmp[snsrIndex].sensor_value,snsrValueLen);
    	respIndex += snsrValueLen;
    	Cl_SecureMemcpy(&resp[respIndex],snsrValueLen,tmp[snsrIndex].sensorMaxValue,snsrValueLen);
    	respIndex += snsrValueLen;
    	Cl_SecureMemcpy(&resp[respIndex],snsrValueLen,tmp[snsrIndex].sensorAverageValue,snsrValueLen);
    	respIndex += snsrValueLen;
    	Cl_SecureMemcpy(&resp[respIndex],snsrStatusLen,&tmp[snsrIndex].sensor_status,snsrStatusLen);
    	respIndex += snsrStatusLen;

    	xSemaphoreGive(sdr_lock);
    	*respSize = respIndex;
    }
    else
    {
	resp[ASDM_RESP_BYTE_CC] = Asdm_CC_Operation_Failed;
	*respSize = 1;
    }

    return 0;
}

s8 Asdm_Process_Sensor_Request(u8 *req, u8 *resp, u16 *respSize)
{
    if(asdmInitSuccess != true)
    {
        VMC_ERR(" ASDM Data not Initialized yet or has Failed \n\r");
        return -1;
    }

    if(req[0] == ASDM_CMD_GET_SIZE)
    {
        return Asdm_Get_SDR_Size(req, resp, respSize);
    }

    if(!isRepoTypeSupported(req[1]))
    {
        resp[0] = Asdm_CC_Not_Available;
        resp[1] = req[0];
        *respSize = 0x2;
        return 0;
    }
    else
    {
        if(req[0] == ASDM_CMD_GET_SDR)
        {
            return Asdm_Get_Sensor_Repository(req, resp, respSize);
        }
        else if(req[0] == ASDM_CMD_GET_SINGLE_SENSOR_DATA)
        {
            return Asdm_Get_Sensor_Data(req, resp, respSize);
        }
        else if(req[0] == ASDM_CMD_GET_ALL_SENSOR_DATA)
        {
            return Asdm_Get_All_Sensor_Data(req, resp, respSize);
        }
    }
    return 0;
}
/********************************************************************
 * Caller - Sensor_Monitor Task
 *
 * This Functions traverse through all sensors in ASDM repo
 * and call the registered monitoring functions.
 *
 * Sensor Data read from the devices are then updated to the
 * global SDR record for external consumption
 *******************************************************************/

void Monitor_Sensors(void)
{
    snsrRead_t snsrData = {0};
    u8 idx = 0;
    u8 sdrIndex = 0;

    if(asdmInitSuccess == true)
    {
        /* Update the MSP FW version */
        Asdm_Update_Active_MSP_sensor();

		for(sdrIndex = 0; sdrIndex < MAX_SDR_REPO ; sdrIndex++)
		{
			Asdm_SensorRecord_t *sensorRecord = sdrInfo[sdrIndex].sensorRecord;

			if(NULL != sensorRecord)
			{
				for(idx = 0; idx < sdrInfo[sdrIndex].header.no_of_records; idx++)
				{
					/* Check if a Monitor Function is registered for this sensor */
					if(NULL != sensorRecord[idx].snsrReadFunc)
					{
						Cl_SecureMemset(&snsrData, 0x00, sizeof(snsrRead_t));

						snsrData.sensorInstance = sensorRecord[idx].sensorInstance;
						snsrData.mspSensorIndex = sensorRecord[idx].mspSensorId;

						sensorRecord[idx].snsrReadFunc(&snsrData);

							/* Update the Sensor Data in SDR records */
						Update_Sensor_Value(sdrInfo[sdrIndex].header.repository_type,
								sensorRecord[idx].sensor_id, &snsrData);

					}
				}
			}
			else
			{
				VMC_ERR("Invalid SDR Data \n\r");
			}
		}
    }
    else
    {
    	VMC_ERR("ASDM not yet Initialized \n\r");
    }
}


/********************************************************************
 * Caller - DemoMenu Task
 *
 * Displays the Sensor Data in ASDM repo
 *******************************************************************/
void AsdmSensor_Display(void)
{
    VMC_PRNT("------------------------------------------------------------------\n\r");
    VMC_PRNT("------------------          ASDM Sensor Data    ------------------\n\r");
    VMC_PRNT("------------------------------------------------------------------\n\r");
    u8 idx = 0;
    u8 sdrIndex = 0;
    u8 stringCount = 0;

    VMC_PRNT("|   Sensor Name    |   Value     | Status |    Max    |   Average | \n\r");
    for(sdrIndex = 0; sdrIndex < MAX_SDR_REPO ; sdrIndex++)
    {
	Asdm_SensorRecord_t *sensorRecord = sdrInfo[sdrIndex].sensorRecord;

	VMC_PRNT("\n\rRecord Type : 0x%x \n\r",sdrInfo[sdrIndex].header.repository_type);
	VMC_PRNT("----------------------------------------------------------------\n\r");
	if(NULL != sensorRecord)
	{
	    for(idx = 0; idx < sdrInfo[sdrIndex].header.no_of_records; idx++)
	    {
		if(sensorRecord[idx].sampleCount < 1)
		{
			if(sensorRecord[idx].sensor_value_type_length & SENSOR_TYPE_ASCII)
			{
				VMC_PRNT(" %s		%s	%s	NA	NA \n\r",(sensorRecord[idx].sensor_name),
					(sensorRecord[idx].sensor_value),
					((sensorRecord[idx].sensor_status == Vmc_Snsr_State_Normal)
					 ? "Ok":"Error"));
			}
			else
			{
				u8 stringLen = (sensorRecord[idx].sensor_value_type_length & LENGTH_BITMASK);
				VMC_PRNT(" %s		",(sensorRecord[idx].sensor_name),
					((sensorRecord[idx].sensor_status == Vmc_Snsr_State_Normal)
					? "Ok":"Error"));
				for(stringCount = 0; stringCount< stringLen ; stringCount++)
				{
					VMC_PRNT("%x",sensorRecord[idx].sensor_value[stringCount]);
				}
				VMC_PRNT("	%s	NA	NA \n\r",					((sensorRecord[idx].sensor_status == Vmc_Snsr_State_Normal)
					? "Ok":"Error"));
			}
		}
		else if((sensorRecord[idx].sensor_value_type_length & LENGTH_BITMASK) < 4)
		{

			VMC_PRNT(" %s		%d		%s	%d	%d \n\r",(sensorRecord[idx].sensor_name),
					*((u16 *)sensorRecord[idx].sensor_value),
					((sensorRecord[idx].sensor_status == Vmc_Snsr_State_Normal) ? "Ok":"Error"),
					*((u16 *)(sensorRecord[idx].sensorMaxValue)),
					*((u16 *)(sensorRecord[idx].sensorAverageValue)));

		}
		else if((sensorRecord[idx].sensor_value_type_length & LENGTH_BITMASK) == sizeof(u32))
		{

			VMC_PRNT(" %s		%d		%s	%d	%d \n\r",(sensorRecord[idx].sensor_name),
				*((u32 *)sensorRecord[idx].sensor_value),
				((sensorRecord[idx].sensor_status == Vmc_Snsr_State_Normal) ? "Ok":"Error"),
				*((u32 *)(sensorRecord[idx].sensorMaxValue)),
				*((u32 *)(sensorRecord[idx].sensorAverageValue)));
		}
	    }
	}
    }

    VMC_PRNT("\n\r");

}

u8 Asdm_Send_I2C_Sensors_SC(u8 *scPayload)
{
	u8 i = 0;
	u8 dataIndex = 0;
	/* Fix for CR-1129775 */
	u16 sensorVal = 0;
	float temp_value = 0.0;
	u8 sensorSize = sizeof(float);
	s8 tempSensorIdx = getSDRIndex(TemperatureSDR);
	if(tempSensorIdx < 0)
	{
	    VMC_ERR("Failed to Get TemperatureSDR Index\n\r");
	    return 0;
	}

	if(false == asdmInitSuccess)
	{
	    VMC_ERR("Asdm not yet Initialized !! \n\r");
	    return 0;
	}

	Asdm_SensorRecord_t *sensorRecord = sdrInfo[tempSensorIdx].sensorRecord;
	/* We need to Send all the Temperature sensors to SC for OOB */
	for(i = 0 ; i< sdrInfo[tempSensorIdx].header.no_of_records; i++)
	{
	    temp_value = 0.0;
	    /* We do not need send VCCINT_TEMP to SC,
 	       as we are receiving it from SC
	    */
	    if(sensorRecord[i].mspSensorId == VCCINT_TEMP)
	    {
		continue;
	    }
	    //Add SC sensor Index for the Sensor
	    scPayload[dataIndex++] = sensorRecord[i].mspSensorId;

	    //Add Size of the Sensor
	    //sensorSize = (sensorRecord[i].sensor_value_type_length & LENGTH_BITMASK);
	    scPayload[dataIndex++] = sensorSize;

	    // Add Sensor Value
	    Cl_SecureMemcpy(&sensorVal,sizeof(u16),sensorRecord[i].sensor_value,
	                    (sensorRecord[i].sensor_value_type_length & LENGTH_BITMASK));
	    
	    /* SC v4.4.33 expects sensors in float. */
	    temp_value = sensorVal;
	    Cl_SecureMemcpy(&scPayload[dataIndex],sensorSize,&temp_value,sensorSize);

	    dataIndex += sensorSize;

	}
	return dataIndex;
}

void Asdm_Update_Active_MSP_sensor()
{
	s8 boardInfoSensorIdx = getSDRIndex(BoardInfoSDR);
	s8 idx = 0;

	Asdm_SensorRecord_t *sensorRecord = sdrInfo[boardInfoSensorIdx].sensorRecord;
	for(idx = sdrInfo[boardInfoSensorIdx].header.no_of_records -1 ; idx >= 0 ; idx--)
	{
		if(!Cl_SecureMemcmp(sensorRecord[idx].sensor_name,SENSOR_NAME_MAX,SNSRNAME_ACTIVE_SC_VER,strlen(SNSRNAME_ACTIVE_SC_VER)))
		{
			if (xSemaphoreTake(sdr_lock, portMAX_DELAY))
			{
				Cl_SecureMemcpy(sensorRecord[idx].sensor_value,sizeof(sc_vmc_data.scVersion),
						&sc_vmc_data.scVersion[0],sizeof(sc_vmc_data.scVersion));
				xSemaphoreGive(sdr_lock);
				break;
			}
		}
	}
}
