/******************************************************************************
* Copyright (C) 2023 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/
#include <stdint.h>

#include "FreeRTOS.h"
#include "RMI/rmi_api.h"
#include "rmi.h"
#include "vmc_asdm.h"
#include "cl_mem.h"
#include "cl_log.h"

rmi_error_codes_t xGet_Sdr_Api(uint8_t **pucPayload, uint16_t *pusPayload_Size)
{
    rmi_error_codes_t xErr = eRMI_SUCCESS;
    uint8_t ucMax_Repo_Count = Get_Asdm_SDR_Repo_Size();
    uint16_t usPayload_Size = sizeof(Rmi_asdm_header_t);
    uint8_t *pucBuffer = NULL;

    if( NULL == pucPayload || NULL == pusPayload_Size )
    {
        xErr = eRMI_ERROR_INVALID_ARGUMENT;
    }
    else
    {
        uint32_t ulTotal_records = 0;

        for( int i = 0; i < ucMax_Repo_Count; ++i )
        {
            usPayload_Size += sdrInfo[i].header.no_of_records * sizeof(Rmi_asdm_sdr_t);
            ulTotal_records += sdrInfo[i].header.no_of_records;
        }

        pucBuffer = pvPortMalloc(usPayload_Size);

        if( NULL == pucBuffer )
        {
            xErr = eRMI_ERROR_GENERIC;
        }
        else
        {
            Cl_SecureMemset(pucBuffer, 0, usPayload_Size);

            size_t xOffset = 0;

            Rmi_asdm_header_t xTemp_Header = { 0 };
            Rmi_asdm_sdr_t xTemp_Sensor = { 0 };

            xTemp_Header.repository_type = RmiSDR;
            xTemp_Header.repository_version = 0x00;
            xTemp_Header.no_of_records = ulTotal_records;

            Cl_SecureMemcpy(pucBuffer, usPayload_Size, &xTemp_Header, sizeof(xTemp_Header));

            xOffset += sizeof(Rmi_asdm_header_t);

            for(int i = 0; i < ucMax_Repo_Count; ++i)
            {
                for(int j = 0; j < sdrInfo[i].header.no_of_records; ++j)
                {
                    xTemp_Sensor.sensor_id = sdrInfo[i].sensorRecord[j].sensor_id;

                    xTemp_Sensor.sensor_name_type_length = sdrInfo[i].sensorRecord[j].sensor_name_type_length;
                    uint8_t ucSensorname_Size = xTemp_Sensor.sensor_name_type_length;
                    //TODO Change to copy data into response buffer directly rather than allocating memory for each member
                    xTemp_Sensor.sensor_name = pvPortMalloc(ucSensorname_Size);
                    Cl_SecureMemcpy(xTemp_Sensor.sensor_name, ucSensorname_Size, sdrInfo[i].sensorRecord[j].sensor_name, ucSensorname_Size);

                    xTemp_Sensor.sensor_value_type_length = sdrInfo[i].sensorRecord[j].sensor_value_type_length;
                    uint8_t ucSensorvalue_Size = xTemp_Sensor.sensor_value_type_length;
                    xTemp_Sensor.sensor_value = pvPortMalloc(ucSensorvalue_Size);
                    Cl_SecureMemcpy(xTemp_Sensor.sensor_value, ucSensorvalue_Size, sdrInfo[i].sensorRecord[j].sensor_value, ucSensorvalue_Size);

                    xTemp_Sensor.sensor_base_unit_type_length = sdrInfo[i].sensorRecord[j].sensor_base_unit_type_length;
                    uint8_t ucSensorbase_Size = xTemp_Sensor.sensor_value_type_length;
                    xTemp_Sensor.sensor_base_unit = pvPortMalloc(ucSensorbase_Size);
                    Cl_SecureMemcpy(xTemp_Sensor.sensor_base_unit, ucSensorbase_Size, sdrInfo[i].sensorRecord[j].sensor_base_unit, ucSensorbase_Size);


                    xTemp_Sensor.sensor_unit_modifier_byte = sdrInfo[i].sensorRecord[j].sensor_unit_modifier_byte;
                    xTemp_Sensor.threshold_support_byte = sdrInfo[i].sensorRecord[j].threshold_support_byte;

                    if( NULL != sdrInfo[i].sensorRecord[j].lower_fatal_limit )
                    {
                        xTemp_Sensor.lower_fatal_limit = pvPortMalloc(ucSensorvalue_Size);
                        Cl_SecureMemcpy(xTemp_Sensor.lower_fatal_limit, ucSensorvalue_Size, sdrInfo[i].sensorRecord[j].lower_fatal_limit, ucSensorvalue_Size);
                    }

                    if( NULL != sdrInfo[i].sensorRecord[j].lower_critical_limit )
                    {
                        xTemp_Sensor.lower_critical_limit = pvPortMalloc(ucSensorvalue_Size);
                        Cl_SecureMemcpy(xTemp_Sensor.lower_critical_limit, ucSensorvalue_Size, sdrInfo[i].sensorRecord[j].lower_critical_limit, ucSensorvalue_Size);
                    }

                    if( NULL != sdrInfo[i].sensorRecord[j].lower_warning_limit )
                    {
                        xTemp_Sensor.lower_warning_limit = pvPortMalloc(ucSensorvalue_Size);
                        Cl_SecureMemcpy(xTemp_Sensor.lower_warning_limit, ucSensorvalue_Size, sdrInfo[i].sensorRecord[j].lower_warning_limit, ucSensorvalue_Size);
                    }

                    if( NULL != sdrInfo[i].sensorRecord[j].upper_fatal_limit )
                    {
                        xTemp_Sensor.upper_fatal_limit = pvPortMalloc(ucSensorvalue_Size);
                        Cl_SecureMemcpy(xTemp_Sensor.upper_fatal_limit, ucSensorvalue_Size, sdrInfo[i].sensorRecord[j].upper_fatal_limit, ucSensorvalue_Size);
                    }

                    if( NULL != sdrInfo[i].sensorRecord[j].upper_critical_limit )
                    {
                        xTemp_Sensor.upper_critical_limit = pvPortMalloc(ucSensorvalue_Size);
                        Cl_SecureMemcpy(xTemp_Sensor.upper_critical_limit, ucSensorvalue_Size, sdrInfo[i].sensorRecord[j].upper_critical_limit, ucSensorvalue_Size);
                    }

                    if( NULL != sdrInfo[i].sensorRecord[j].upper_warning_limit )
                    {
                        xTemp_Sensor.upper_warning_limit = pvPortMalloc(ucSensorvalue_Size);
                        Cl_SecureMemcpy(xTemp_Sensor.upper_warning_limit, ucSensorvalue_Size, sdrInfo[i].sensorRecord[j].upper_warning_limit, ucSensorvalue_Size);
                    }

                    xTemp_Sensor.sensor_status = sdrInfo[i].sensorRecord[j].sensor_status;

                    Cl_SecureMemcpy(pucBuffer + xOffset, sizeof(xTemp_Sensor), &xTemp_Sensor, sizeof(xTemp_Sensor));
                    xOffset += sizeof(xTemp_Sensor);
                }
            }

            *pucPayload = pucBuffer;
            *pusPayload_Size = usPayload_Size;
        }
    }

    return xErr;
}

rmi_error_codes_t xGet_All_Sensor_Data_Api(void)
{
    //TODO
    return eRMI_SUCCESS;
}

rmi_error_codes_t xRmi_Request_Handler(uint8_t* pucReq, uint16_t* pusReq_size, uint8_t* pucResp, uint16_t* pusResp_size)
{
    rmi_error_codes_t xErr = eRMI_ERROR_GENERIC;
    uint8_t ucApi_id = 0;
    uint8_t *pucPayload = NULL;
    uint16_t usPayload_Size = 0;

    if( NULL != pusReq_size && RMI_API_REQUEST_HEADER_SIZE <= *pusReq_size && NULL != pusResp_size )
    {
        if( NULL != pucReq )
        {
            ucApi_id = pucReq[RMI_API_ID_INDEX];

            switch( ucApi_id )
            {
                case ASDM_CMD_GET_SDR_API_ID:

                    xErr = xGet_Sdr_Api( &pucPayload, &usPayload_Size );

                    if( eRMI_SUCCESS == xErr )
                    {
                        //TODO Change so pucResp is allocated by RMI
                        uint8_t *pucResp = pvPortMalloc( usPayload_Size + RMI_API_RESPONSE_HEADER_SIZE );

                        if( NULL != pucResp )
                        {
                            pucResp[RMI_API_ID_INDEX] = ucApi_id;
                            //TODO Add named completion codes to RMI
                            pucResp[RMI_API_COMPLETION_CODE_INDEX] = 0x01;
                            pucResp[RMI_API_RESPONSE_REPO_TYPE_INDEX] = RmiSDR;
                            pucResp[RMI_API_RESPONSE_PAYLOAD_SIZE_LSB] = (uint8_t) usPayload_Size;
                            pucResp[RMI_API_RESPONSE_PAYLOAD_SIZE_MSB] = (uint8_t) (usPayload_Size >> 8);
                            Cl_SecureMemcpy(pucResp + RMI_API_RESPONSE_HEADER_SIZE, usPayload_Size, pucPayload, usPayload_Size);

                            *pusResp_size = RMI_API_REQUEST_HEADER_SIZE + usPayload_Size;
                        }
                        else
                        {
                            xErr = eRMI_ERROR_GENERIC;
                        }

                        vPortFree(pucPayload);
                    }

                break;

                case ASDM_CMD_GET_ALL_SENSOR_DATA_API_ID:

                    xErr = xGet_All_Sensor_Data_Api();

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
