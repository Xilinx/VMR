/******************************************************************************
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#include "vmc_sensors.h"
#include "vmc_asdm.h"
#include "cl_log.h"
#include "vmc_api.h"
#include <semphr.h>

#define LENGTH_BITMASK   (0x3F)
#define GET_REPOSITORY_REQUEST		(0xF1)

SDR_t *sdrInfo;
SemaphoreHandle_t sdr_lock;
static u8 asdmInitSuccess = false;
extern Versal_BoardInfo board_info;

extern s8 Temperature_Read_Inlet(snsrRead_t *snsrData);
extern s8 Temperature_Read_Outlet(snsrRead_t *snsrData);
extern s8 Temperature_Read_Board(snsrRead_t *snsrData);
extern s8 Fan_RPM_Read(snsrRead_t *snsrData);

Asdm_Header_t asdmHeaderInfo[] = {
    /* Record Type	| Hdr Version | Record Count | NumBytes */
    {BoardInfoSDR ,  	 	0x1  ,		 0x2, 		0x7f},
    {TemperatureSDR , 		0x1  ,		 0x4,		0x7f},
};

#define MAX_SDR_REPO 	(sizeof(asdmHeaderInfo)/sizeof(asdmHeaderInfo[0]))

/****************************************************************
 * Note:-
 * Add New sensors in below snsrMetaData [].
 * Update the "Record Count" for each New sensor in asdmHeaderInfo.
 * New sensor to be added based on Repo type sequence.
 *****************************************************************/
void getSDRMetaData(Asdm_Sensor_MetaData_t **pMetaData, u16 *sdrMetaDataCount)
{
    u32 snsrDefaultVal = 0x0;

    Asdm_Sensor_MetaData_t snsrMetaData [] = {
	{
	    .repoType = BoardInfoSDR,
	    .sensorName = "Product Name\0",
	    .snsrValTypeLength = (0xC << 0x4) | sizeof(board_info.product_name),
	    .defaultValue = &board_info.product_name[0],
	    .snsrBaseUnitTypeLength = 00,
	    .snsrUnitModifier = 0x0,
	    .supportedThreshold = 0,
	    .snsrBaseUnit = "NA",
	    .supportedThreshold = 0x0,
	    .sampleCount = 0x0,
	},
	{
	    .repoType = BoardInfoSDR,
	    .sensorName = "Board Serial\0",
	    .snsrValTypeLength = (0xC << 0x4) | sizeof(board_info.board_serial),
	    .defaultValue = &board_info.board_serial[0],
	    .snsrBaseUnitTypeLength = 0x00,
	    .snsrUnitModifier = 0x0,
	    .supportedThreshold = 0,
	    .sampleCount = 0x0,
	},
	{
	    .repoType = TemperatureSDR,
	    .sensorName = "Inlet Temp\0",
	    .snsrValTypeLength = 0x01,
	    .snsrBaseUnitTypeLength = 0xC8,
	    .snsrBaseUnit = "Celcius\0",
	    .snsrUnitModifier = 0x0,
	    .supportedThreshold = 0x07,
	    .upperWarnLimit = 90,
	    .upperCritLimit = 100,
	    .upperFatalLimit = 110,
	    .sampleCount = 0x1,
	    .mointorFunc = &Temperature_Read_Inlet,

	},
	{
	    .repoType = TemperatureSDR,
	    .sensorName = "Outlet Temp\0",
	    .snsrValTypeLength = 0x01,
	    .snsrBaseUnitTypeLength = 0xC8,
	    .snsrBaseUnit = "Celcius\0",
	    .snsrUnitModifier = 0x0,
	    .supportedThreshold = 0x07,
	    .upperWarnLimit = 90,
	    .upperCritLimit = 100,
	    .upperFatalLimit = 110,
	    .sampleCount = 0x1,
	    .mointorFunc = &Temperature_Read_Outlet,
	},
	{
	    .repoType = TemperatureSDR,
	    .sensorName = "Board Temp\0",
	    .snsrValTypeLength = 0x04,
	    .snsrBaseUnitTypeLength = 0xC8,
	    .snsrBaseUnit = "Celcius\0",
	    .snsrUnitModifier = 0x0,
	    .supportedThreshold = 0x07,
	    .upperWarnLimit = 90,
	    .upperCritLimit = 100,
	    .upperFatalLimit = 110,
	    .sampleCount = 0x1,
	    .mointorFunc = &Temperature_Read_Board,
	},
	{
	    .repoType = TemperatureSDR,
	    .sensorName = "Fan RPM\0",
	    .snsrValTypeLength = 0x02,
	    .snsrBaseUnitTypeLength = 0xC4,
	    .snsrBaseUnit = "RPM\0",
	    .snsrUnitModifier = 0x0,
	    .supportedThreshold = 0x00,
	    .sampleCount = 0x1,
	    .mointorFunc = &Fan_RPM_Read,
	},
    };

    /* Get Record Count */
    *sdrMetaDataCount = (sizeof(snsrMetaData) / sizeof(snsrMetaData[0]));

    *pMetaData = (Asdm_Sensor_MetaData_t *) pvPortMalloc( sizeof(Asdm_Sensor_MetaData_t) * (*sdrMetaDataCount));

    if(*pMetaData != NULL)
    {
	memset(*pMetaData,0x00,sizeof(snsrMetaData));
	memcpy(*pMetaData,snsrMetaData,sizeof(snsrMetaData));
    }
    else
    {
	VMC_ERR(" pvPortMalloc Failed\n\r");
	return;
    }
}


u8 Update_Sensor_Value(Asdm_RepositoryTypeEnum_t repoType, u8 sensorIdx, snsrRead_t *snsrInfo)
{
    /* Extract the Repo index from Repotype */
    u8 repoIndex = (((repoType >> 4 ) - 0xC ) | (repoType & 0x0F));
    Asdm_SensorRecord_t *sensorRecord = &sdrInfo[repoIndex].sensorRecord[sensorIdx];

    if(NULL != sensorRecord)
    {
	if (xSemaphoreTake(sdr_lock, portMAX_DELAY))
	{
	    memcpy(sensorRecord->sensor_value,snsrInfo->snsrValue,snsrInfo->sensorValueSize);

	    /* Update only if the sensor is not Static */
	    if(sensorRecord->sampleCount > 0)
	    {
		/* Update Sensor Status */
		sensorRecord->sensor_status = snsrInfo->snsrSatus;

		/* Update Max Sensor Value */
		if ( *((float *)sensorRecord->sensor_value) > *((float *)sensorRecord->sensorMaxValue))
		{
		    memcpy(sensorRecord->sensorMaxValue,sensorRecord->sensor_value,
			    snsrInfo->sensorValueSize);
		}

		/* Calculate and Update Average Value */
		*((float *)sensorRecord->sensorAverageValue) = *((float *)sensorRecord->sensorAverageValue)
		    				- (*((float *)sensorRecord->sensorAverageValue)/sensorRecord->sampleCount)
		    				+ (*((float *)sensorRecord->sensor_value)/sensorRecord->sampleCount);

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
    u8 snsrValueLen = 0;
    u8 baseUnitLen = 0;
    u16 sdrMetaDataCount = 0;
    u16 totalRecords = 0;
    Asdm_Sensor_MetaData_t *pSdrMetaData = NULL;
    u16 idx = 0;


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
	return -1;
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
	memset(sdrInfo,0x00,(MAX_SDR_REPO * sizeof(SDR_t)));

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

	    memset(sdrInfo[sdrCount].sensorRecord,0x00,allocateSize);

	    Asdm_SensorRecord_t *tmp = sdrInfo[sdrCount].sensorRecord;

	    /* Initialize each Sensor Info */
	    for(idx = 0; idx < asdmHeaderInfo[sdrCount].no_of_records; idx++)
	    {
		/* Assign  the Sensor Id Count for this RepoType  */
		tmp[idx].sensor_id = idx;

		/* Sensor Name
		 * Based on Name Type and Length allocate Memory for Sensor Name
		 */

		snsrNameLen = strlen(pSdrMetaData[totalRecords].sensorName) + 1;
		tmp[idx].sensor_name_type_length = (0xC << 4) | (snsrNameLen & LENGTH_BITMASK);

		tmp[idx].sensor_name = (char8 *) pvPortMalloc(snsrNameLen);
		if(NULL != tmp[idx].sensor_name)
		{
		    memset(tmp[idx].sensor_name, 0x00, snsrNameLen);
		    memcpy(tmp[idx].sensor_name,
			    pSdrMetaData[totalRecords].sensorName, snsrNameLen);
		}
		else
		{
		    VMC_ERR("Failed to allocate Memory for Sensor Name !!!\n\r");
		    return -1;
		}

		/* Sensor Value */
		tmp[idx].sensor_value_type_length = pSdrMetaData[totalRecords].snsrValTypeLength;

		snsrValueLen = (pSdrMetaData[totalRecords].snsrValTypeLength & LENGTH_BITMASK);


		/* Only allocate the Memory, Value will be updated while Monitoring */
		tmp[idx].sensor_value = (u8 *) pvPortMalloc(snsrValueLen);
		if(NULL != tmp[idx].sensor_value)
		{
		    memset(tmp[idx].sensor_value, 0x00, snsrValueLen);
		    /* Value */
		    memcpy(tmp[idx].sensor_value,pSdrMetaData[totalRecords].defaultValue,snsrValueLen);
		}
		else
		{
		    VMC_ERR("Failed to allocate Memory for Sensor Value !!!\n\r");
		    return -1;
		}

		/* Sensor Units */
		if(pSdrMetaData[totalRecords].snsrBaseUnitTypeLength != 0)
		{
		    tmp[idx].sensor_base_unit_type_length = pSdrMetaData[totalRecords].snsrBaseUnitTypeLength;

		    baseUnitLen = (pSdrMetaData[totalRecords].snsrBaseUnitTypeLength & LENGTH_BITMASK);

		    tmp[idx].sensor_base_unit = (u8 *) pvPortMalloc(baseUnitLen);
		    if(NULL != tmp[idx].sensor_base_unit)
		    {
			memset(tmp[idx].sensor_base_unit, 0x00, baseUnitLen);
			memcpy(tmp[idx].sensor_base_unit,
				pSdrMetaData[totalRecords].snsrBaseUnit, baseUnitLen);
		    }
		    else
		    {
			VMC_ERR("Failed to allocate Memory for BaseUnit !!");
			return -1;
		    }
		}

		/* Sensor unit modifier */
		tmp[idx].sensor_unit_modifier_byte = pSdrMetaData[totalRecords].snsrUnitModifier;

		/* Sensor Threshold support byte */
		tmp[idx].threshold_support_byte = pSdrMetaData[totalRecords].supportedThreshold;

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
		    memset(tmp[idx].lower_fatal_limit,0x00, snsrValueLen);
		    memcpy(tmp[idx].lower_fatal_limit,&pSdrMetaData[totalRecords].lowerFatalLimit,snsrValueLen);
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
		    memset(tmp[idx].lower_critical_limit,0x00, snsrValueLen);
		    memcpy(tmp[idx].lower_critical_limit,&pSdrMetaData[totalRecords].lowerCritLimit,snsrValueLen);
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
		    memset(tmp[idx].lower_warning_limit,0x00, snsrValueLen);
		    memcpy(tmp[idx].lower_warning_limit,&pSdrMetaData[totalRecords].lowerWarnLimit,snsrValueLen);
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
		    memset(tmp[idx].upper_fatal_limit,0x00, snsrValueLen);
		    memcpy(tmp[idx].upper_fatal_limit, &pSdrMetaData[totalRecords].upperFatalLimit, snsrValueLen);
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
		    memset(tmp[idx].upper_critical_limit,0x00, snsrValueLen);
		    memcpy(tmp[idx].upper_critical_limit, &pSdrMetaData[totalRecords].upperCritLimit, snsrValueLen);
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
		    memset(tmp[idx].upper_warning_limit,0x00, snsrValueLen);
		    memcpy(tmp[idx].upper_warning_limit, &pSdrMetaData[totalRecords].upperWarnLimit, snsrValueLen);
		}

		/* Sensor Status */
		tmp[idx].sensor_status = Sensor_Present_And_Valid;

		/* Update the Sample count */
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
		    memset(tmp[idx].sensorMaxValue,0x00, snsrValueLen);

		    tmp[idx].sensorAverageValue = (u8 *) pvPortMalloc(snsrValueLen);
		    if(NULL == tmp[idx].sensorAverageValue)
		    {
			/* LOG Error and return */
			VMC_ERR("Failed to allocate Memory for sensorAverageValue !!!\n\r");
			return -1;
		    }
		    memset(tmp[idx].sensorAverageValue,0x00, snsrValueLen);
		}
		/* Update the Monitor Function for the Sensor */
		tmp[idx].snsrReadFunc = pSdrMetaData[totalRecords].mointorFunc;

		/* One Record is initialized, move to the Next SDRMetaData */
		totalRecords++;

	    }

	    /* Fill the EOR */
	    memcpy(sdrInfo[sdrCount].asdmEOR,"END",sizeof(u8) * strlen("END"));
	}

	vPortFree(pSdrMetaData);
    }
    else
    {
	/* Log Error ASDM Init Failed */
	VMC_LOG("ASDM Init Failed, MetaData not available !!!\n\r");
	return -1;
    }

    /* sdr_lock */
    sdr_lock = xSemaphoreCreateMutex();
    if(sdr_lock == NULL){
	VMC_ERR("sdr_lock creation failed \n\r");
	return XST_FAILURE;
    }


    asdmInitSuccess = true;
    VMC_LOG("ASDM Init success !!\n\r");

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

    if(asdmInitSuccess != true)
    {
	VMC_ERR(" ASDM Data not Initialized yet or has Failed \n\r");
	return -1;
    }

    sdrIndex = (((req[0] >> 4 ) - 0xC ) | (req[0] & 0x0F));

    if (xSemaphoreTake(sdr_lock, portMAX_DELAY))
    {
	/* Start from Offset 1 , 0 is for completion code, we fill it at the End */
	respIndex = 1;
	/* Populate the Header */
	memcpy(&resp[respIndex], &sdrInfo[sdrIndex].header, sizeof(Asdm_Header_t));
	respIndex += sizeof(Asdm_Header_t);

	Asdm_SensorRecord_t *tmp = sdrInfo[sdrIndex].sensorRecord;
	/* Populate the Sensor Records */
	for(idx = 0; idx < sdrInfo[sdrIndex].header.no_of_records; idx++)
	{
	    resp[respIndex++] = tmp[idx].sensor_id;
	    resp[respIndex++] = tmp[idx].sensor_name_type_length;

	    /* Copy Sensor Name */
	    snsrNameLen = (tmp[idx].sensor_name_type_length & LENGTH_BITMASK);
	    memcpy(&resp[respIndex], tmp[idx].sensor_name, snsrNameLen);
	    respIndex += snsrNameLen;

	    /*  Sensor Value */
	    resp[respIndex++] = tmp[idx].sensor_value_type_length;

	    snsrValueLen = (tmp[idx].sensor_value_type_length & LENGTH_BITMASK);
	    if(snsrValueLen != 0)
	    {
		memcpy(&resp[respIndex], tmp[idx].sensor_value, snsrValueLen);
		respIndex += snsrValueLen;
	    }

	    /* Sensor Base unit */
	    resp[respIndex++] = tmp[idx].sensor_base_unit_type_length;

	    baseUnitLen = (tmp[idx].sensor_base_unit_type_length & LENGTH_BITMASK);
	    if(baseUnitLen != 0)
	    {
		memcpy(&resp[respIndex], tmp[idx].sensor_base_unit, baseUnitLen);
		respIndex += baseUnitLen;
	    }

	    resp[respIndex++] = tmp[idx].sensor_unit_modifier_byte;

	    /* Sensor Thresholds */
	    resp[respIndex++] = tmp[idx].threshold_support_byte;

	    if(tmp[idx].threshold_support_byte & Lower_Fatal_Threshold)
	    {
		memcpy(&resp[respIndex], tmp[idx].lower_fatal_limit,snsrValueLen);
		respIndex += snsrValueLen;
	    }
	    if(tmp[idx].threshold_support_byte & Lower_Critical_Threshold)
	    {
		memcpy(&resp[respIndex], tmp[idx].lower_critical_limit,snsrValueLen);
		respIndex += snsrValueLen;
	    }
	    if(tmp[idx].threshold_support_byte & Lower_Warning_Threshold)
	    {
		memcpy(&resp[respIndex], tmp[idx].lower_warning_limit,snsrValueLen);
		respIndex += snsrValueLen;
	    }
	    if(tmp[idx].threshold_support_byte & Upper_Fatal_Threshold)
	    {
		memcpy(&resp[respIndex], tmp[idx].upper_fatal_limit,snsrValueLen);
		respIndex += snsrValueLen;
	    }
	    if(tmp[idx].threshold_support_byte & Upper_Critical_Threshold)
	    {
		memcpy(&resp[respIndex], tmp[idx].upper_critical_limit,snsrValueLen);
		respIndex += snsrValueLen;
	    }
	    if(tmp[idx].threshold_support_byte & Upper_Warning_Threshold)
	    {
		memcpy(&resp[respIndex], tmp[idx].upper_warning_limit,snsrValueLen);
		respIndex += snsrValueLen;
	    }

	    resp[respIndex++] =  tmp[idx].sensor_status;
	}

	/* Populate the EOR "END" */
	memcpy(&resp[respIndex], sdrInfo[sdrIndex].asdmEOR, (sizeof(u8) * strlen("END")));
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

/********************************************************************
 * Caller - Sensor_Monitor Task
 *
 * This Functions traverse through all sensors in ASDM repo
 * and call the registered monitoring functions.
 *
 * Sensor Data read from the devices are then updated to the
 * global SDR record for external consumption
 *******************************************************************/

s8 Monitor_Sensors(void)
{
    snsrRead_t snsrData = {0};
    u8 idx = 0;
    u8 sdrIndex = 0;

    if(asdmInitSuccess == true)
    {
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
			memset(&snsrData, 0x00, sizeof(snsrRead_t));
			if(!sensorRecord[idx].snsrReadFunc(&snsrData))
			{
			    /* Update the Sensor Data in SDR records */
			    Update_Sensor_Value(sdrInfo[sdrIndex].header.repository_type,
				    sensorRecord[idx].sensor_id, &snsrData);

			}
			else
			{
			    VMC_ERR("Failed to read : %s \n\r",sensorRecord[idx].sensor_name);
			}
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

    VMC_PRNT("|   Sensor Name    |   Value     |    Status  | Running Average | \n\r");
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
		    VMC_PRNT(" %s %18s %8s       NA \n\r",(sensorRecord[idx].sensor_name),
			    				(sensorRecord[idx].sensor_value),
			   				 ((sensorRecord[idx].sensor_status == Vmc_Snsr_State_Normal)
							 ? "Ok":"Error"));
		}
		else if((sensorRecord[idx].sensor_value_type_length & 0x3F) < 4)
		{
		    VMC_PRNT(" %s           %d             %s            %d \n\r",(sensorRecord[idx].sensor_name),
					*((u16 *)sensorRecord[idx].sensor_value),
					((sensorRecord[idx].sensor_status == Vmc_Snsr_State_Normal) ? "Ok":"Error"),
					*((u16 *)(sensorRecord[idx].sensorAverageValue)));
		}
		else if((sensorRecord[idx].sensor_value_type_length & 0x3F) == sizeof(float))
		{

		    VMC_PRNT(" %10s           %0.3f           %2s          %0.3f \n\r",(sensorRecord[idx].sensor_name),
					*((float *)sensorRecord[idx].sensor_value),
					((sensorRecord[idx].sensor_status == Vmc_Snsr_State_Normal) ? "Ok":"Error"),
					*((float *)(sensorRecord[idx].sensorAverageValue)));
		}
	    }
	}
    }

    VMC_PRNT("\n\r");


}
