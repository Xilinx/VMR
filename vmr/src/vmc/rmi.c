/******************************************************************************
* Copyright (C) 2023 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/
#include "cl_config.h"

#ifdef BUILD_FOR_RMI
#include <stdint.h>

#include "FreeRTOS.h"
#include "RMI/rmi_api.h"
#include "RMI/rmi_sensors.h"
#include "rmi.h"
#include "vmc_asdm.h"
#include "cl_mem.h"
#include "cl_log.h"
#include <semphr.h>

extern SemaphoreHandle_t sdr_lock;

rmi_error_codes_t xGet_Sdr_Api(uint8_t *pucPayload, uint16_t *pusPayload_Size)
{
    rmi_error_codes_t xErr = eRMI_SUCCESS;

    uint8_t ucMax_Repo_Count = Get_Asdm_SDR_Repo_Size();

    if( NULL == pucPayload || NULL == pusPayload_Size )
    {
        xErr = eRMI_ERROR_INVALID_ARGUMENT;
    }
    else
    {
        if( pdTRUE != xSemaphoreTake( sdr_lock , portMAX_DELAY ) )
        {
            xErr = eRMI_ERROR_GENERIC;
        }
        else
        {
            uint32_t ulTotal_records = 0;

            for( int i = 1; i < ucMax_Repo_Count; ++i )
            {
                ulTotal_records += sdrInfo[i].header.no_of_records;
            }

            size_t xOffset = 0;

            rmi_sdr_header_t xTemp_Header = { 0 };

            xTemp_Header.repository_type = eRmiSDR;
            xTemp_Header.repository_version = 0x00;
            xTemp_Header.no_of_records = ulTotal_records;

            Cl_SecureMemcpy( pucPayload, sizeof(rmi_sdr_header_t), &xTemp_Header, sizeof(rmi_sdr_header_t));

            xOffset += sizeof(rmi_sdr_header_t);

            // Start from 1 to skip board info at index 0
            for(int i = 1; i < ucMax_Repo_Count; ++i)
            {
                for(int j = 0; j < sdrInfo[i].header.no_of_records; ++j)
                {
                    size_t sensor_id_size = sizeof(sdrInfo[i].sensorRecord[j].sensor_id);
                    Cl_SecureMemcpy( pucPayload + xOffset, sensor_id_size, &sdrInfo[i].sensorRecord[j].sensor_id, sensor_id_size);
                    xOffset += sensor_id_size;

                    size_t sensor_name_type_size = sizeof(sdrInfo[i].sensorRecord[j].sensor_name_type_length);
                    Cl_SecureMemcpy( pucPayload + xOffset, sensor_name_type_size, &sdrInfo[i].sensorRecord[j].sensor_name_type_length, sensor_name_type_size);
                    xOffset += sensor_name_type_size;

                    uint8_t sensor_name_length = sdrInfo[i].sensorRecord[j].sensor_name_type_length & LENGTH_BITMASK;
                    Cl_SecureMemcpy( pucPayload + xOffset, sensor_name_length, sdrInfo[i].sensorRecord[j].sensor_name, sensor_name_length);
                    xOffset += sensor_name_length;

                    size_t sensor_value_type_size = sizeof(sdrInfo[i].sensorRecord[j].sensor_value_type_length);
                    Cl_SecureMemcpy( pucPayload + xOffset, sensor_value_type_size, &sdrInfo[i].sensorRecord[j].sensor_value_type_length, sensor_value_type_size);
                    xOffset += sensor_value_type_size;

                    uint8_t sensor_value_length = sdrInfo[i].sensorRecord[j].sensor_value_type_length & LENGTH_BITMASK;
                    Cl_SecureMemcpy( pucPayload + xOffset, sensor_value_length, sdrInfo[i].sensorRecord[j].sensor_value, sensor_value_length);
                    xOffset += sensor_value_length;

                    size_t sensor_base_unit_type_size = sizeof(sdrInfo[i].sensorRecord[j].sensor_base_unit_type_length);
                    Cl_SecureMemcpy( pucPayload + xOffset, sensor_base_unit_type_size, &sdrInfo[i].sensorRecord[j].sensor_base_unit_type_length,
                                        sensor_base_unit_type_size);
                    xOffset += sensor_base_unit_type_size;

                    if( NULL != sdrInfo[i].sensorRecord[j].sensor_base_unit )
                    {
                        uint8_t sensor_base_unit_length = sdrInfo[i].sensorRecord[j].sensor_base_unit_type_length & LENGTH_BITMASK;
                        Cl_SecureMemcpy( pucPayload + xOffset, sensor_base_unit_length, sdrInfo[i].sensorRecord[j].sensor_base_unit, sensor_base_unit_length);
                        xOffset += sensor_base_unit_length;
                    }

                    size_t sensor_unit_modifier_type_byte_size = sizeof(sdrInfo[i].sensorRecord[j].sensor_unit_modifier_byte);
                    Cl_SecureMemcpy( pucPayload + xOffset, sensor_unit_modifier_type_byte_size, &sdrInfo[i].sensorRecord[j].sensor_unit_modifier_byte,
                                        sensor_unit_modifier_type_byte_size);
                    xOffset += sensor_unit_modifier_type_byte_size;

                    size_t threshold_support_byte_size = sizeof(sdrInfo[i].sensorRecord[j].threshold_support_byte);
                    Cl_SecureMemcpy( pucPayload + xOffset, threshold_support_byte_size, &sdrInfo[i].sensorRecord[j].threshold_support_byte, threshold_support_byte_size);
                    xOffset += threshold_support_byte_size;

                    if( NULL != sdrInfo[i].sensorRecord[j].lower_fatal_limit )
                    {
                        Cl_SecureMemcpy( pucPayload + xOffset, sensor_value_length, sdrInfo[i].sensorRecord[j].lower_fatal_limit, sensor_value_length);
                        xOffset += sensor_value_length;
                    }

                    if( NULL != sdrInfo[i].sensorRecord[j].lower_critical_limit )
                    {
                        Cl_SecureMemcpy( pucPayload + xOffset, sensor_value_length, sdrInfo[i].sensorRecord[j].lower_critical_limit, sensor_value_length);
                        xOffset += sensor_value_length;
                    }

                    if( NULL != sdrInfo[i].sensorRecord[j].lower_warning_limit )
                    {
                        Cl_SecureMemcpy( pucPayload + xOffset, sensor_value_length, sdrInfo[i].sensorRecord[j].lower_warning_limit, sensor_value_length);
                        xOffset += sensor_value_length;
                    }

                    if( NULL != sdrInfo[i].sensorRecord[j].upper_fatal_limit )
                    {
                        Cl_SecureMemcpy( pucPayload + xOffset, sensor_value_length, sdrInfo[i].sensorRecord[j].upper_fatal_limit, sensor_value_length);
                        xOffset += sensor_value_length;
                    }

                    if( NULL != sdrInfo[i].sensorRecord[j].upper_critical_limit )
                    {
                        Cl_SecureMemcpy( pucPayload + xOffset, sensor_value_length, sdrInfo[i].sensorRecord[j].upper_critical_limit, sensor_value_length);
                        xOffset += sensor_value_length;
                    }

                    if( NULL != sdrInfo[i].sensorRecord[j].upper_warning_limit )
                    {
                        Cl_SecureMemcpy( pucPayload + xOffset, sensor_value_length, sdrInfo[i].sensorRecord[j].upper_warning_limit, sensor_value_length);
                        xOffset += sensor_value_length;
                    }

                    size_t sensor_status_size = sizeof(sdrInfo[i].sensorRecord[j].sensor_status);
                    Cl_SecureMemcpy( pucPayload + xOffset, sensor_status_size, &sdrInfo[i].sensorRecord[j].sensor_status, sensor_status_size);
                    xOffset += sensor_status_size;
                }
            }

            ((rmi_sdr_header_t *) pucPayload)->no_of_bytes = xOffset;
            *pusPayload_Size = sizeof(rmi_sdr_header_t) + xOffset;

            if( pdTRUE == xSemaphoreGive( sdr_lock ) )
            {
                xErr = eRMI_SUCCESS;
            }
            else
            {
                xErr = eRMI_ERROR_GENERIC;
            }
        }
    }

    return xErr;
}

rmi_error_codes_t xGet_All_Sensor_Data_Api(uint8_t *pucPayload, uint16_t *pusPayload_Size)
{
    rmi_error_codes_t xErr = eRMI_ERROR_GENERIC;
    uint8_t ucMax_Repo_Count = Get_Asdm_SDR_Repo_Size();

    if( NULL == pucPayload || NULL == pusPayload_Size )
    {
        xErr = eRMI_ERROR_INVALID_ARGUMENT;
    }
    else
    {
        if( pdTRUE == xSemaphoreTake( sdr_lock, portMAX_DELAY ) )
        {
            size_t xOffset = 0;

            for(int i = 1; i < ucMax_Repo_Count; ++i)
            {
                for(int j = 0; j < sdrInfo[i].header.no_of_records; ++j)
                {
                    sensors_ds_t xTemp_Sensor = { 0 };

                    Cl_SecureMemcpy(&xTemp_Sensor.id, sizeof(xTemp_Sensor.id), &sdrInfo[i].sensorRecord[j].sensor_id, sizeof(sdrInfo[i].sensorRecord[j].sensor_id));
                    Cl_SecureMemcpy(&xTemp_Sensor.size, sizeof(xTemp_Sensor.size), &sdrInfo[i].sensorRecord[j].sensor_value_type_length, sizeof(sdrInfo[i].sensorRecord[j].sensor_value_type_length) & LENGTH_BITMASK);
                    Cl_SecureMemcpy(&xTemp_Sensor.value, sizeof(xTemp_Sensor.value), sdrInfo[i].sensorRecord[j].sensor_value, sizeof(sdrInfo[i].sensorRecord[j].sensor_value_type_length & LENGTH_BITMASK));
                    Cl_SecureMemcpy(&xTemp_Sensor.status, sizeof(xTemp_Sensor.status), &sdrInfo[i].sensorRecord[j].sensor_status, sizeof(sdrInfo[i].sensorRecord[j].sensor_status));
                    //TODO do we need this?
                    //Cl_SecureMemcpy(&xTemp_Sensor.tag, sizeof(xTemp_Sensor.tag),,);

                    Cl_SecureMemcpy( pucPayload + xOffset, sizeof(xTemp_Sensor), &xTemp_Sensor, sizeof(xTemp_Sensor));

                    xOffset += sizeof(sensors_ds_t);
                }
            }

            *pusPayload_Size = xOffset;

            if( pdTRUE == xSemaphoreGive(sdr_lock) )
            {
                xErr = eRMI_SUCCESS;
            }
            else
            {
                xErr = eRMI_ERROR_GENERIC;
            }
        }
        else
        {
            xErr = eRMI_ERROR_GENERIC;
        }
    }

    return xErr;
}

rmi_error_codes_t xRmi_Request_Handler(uint8_t* pucReq, uint16_t* pusReq_size, uint8_t* pucResp, uint16_t* pusResp_size)
{
    rmi_error_codes_t xErr = eRMI_ERROR_GENERIC;
    uint8_t ucApi_id = 0;
    uint16_t usPayload_Size = 0;

    if( NULL != pusReq_size && apiREQUEST_HEADER_SIZE <= *pusReq_size && NULL != pusResp_size && NULL != pucResp )
    {
        if( NULL != pucReq )
        {
            ucApi_id = pucReq[apiID_INDEX];

            switch( ucApi_id )
            {
                case apiCMD_GET_SDR_API_ID:

                    xErr = xGet_Sdr_Api( pucResp + apiRESPONSE_HEADER_SIZE, &usPayload_Size );

                    if( eRMI_SUCCESS == xErr )
                    {
                            VMR_ERR("e\r\n");
                            pucResp[apiID_INDEX] = ucApi_id;
                            pucResp[apiCOMPLETION_CODE_INDEX] = eRmi_CC_Operation_Success;
                            pucResp[apiRESPONSE_REPO_TYPE_INDEX] = eRmiSDR;
                            pucResp[apiRESPONSE_PAYLOAD_SIZE_LSB] = (uint8_t) usPayload_Size;
                            pucResp[apiRESPONSE_PAYLOAD_SIZE_MSB] = (uint8_t) (usPayload_Size >> 8);
                            *pusResp_size = apiRESPONSE_HEADER_SIZE + usPayload_Size;
                    }
                    else
                    {
                        xErr = eRMI_ERROR_GENERIC;
                    }

                break;

                case apiCMD_GET_ALL_SENSOR_DATA_API_ID:

                    if( NULL != pucResp )
                    {
                        xErr = xGet_All_Sensor_Data_Api( pucResp + apiRESPONSE_HEADER_SIZE, &usPayload_Size );

                        if( eRMI_SUCCESS == xErr )
                        {
                            pucResp[apiID_INDEX] = ucApi_id;
                            pucResp[apiCOMPLETION_CODE_INDEX] = eRmi_CC_Operation_Success;
                            pucResp[apiRESPONSE_REPO_TYPE_INDEX] = eRmiSDR;
                            pucResp[apiRESPONSE_PAYLOAD_SIZE_LSB] = (uint8_t) usPayload_Size;
                            pucResp[apiRESPONSE_PAYLOAD_SIZE_MSB] = (uint8_t) (usPayload_Size >> 8);
                            *pusResp_size = apiRESPONSE_HEADER_SIZE + usPayload_Size;
                        }
                        else
                        {
                            xErr = eRMI_ERROR_GENERIC;
                        }
                    }
                    else
                    {
                        xErr = eRMI_ERROR_INVALID_ARGUMENT;
                    }

                break;

                default:

                    xErr = eRMI_ERROR_INVALID_API_ID;

                break;
            }
        }
        else
        {
            xErr = eRMI_ERROR_INVALID_ARGUMENT;
        }
    }
    else
    {
        xErr = eRMI_ERROR_INVALID_ARGUMENT;
    }

    return xErr;
}

#endif /* BUILD_FOR_RMI */