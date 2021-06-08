/*----------------------------------------------------------------------------------------------*/
/*                                                                                              */
/* © Copyright 2019 Xilinx, Inc. All rights reserved.                                           */
/*                                                                                              */
/* This file contains confidential and proprietary information of Xilinx, Inc.                  */
/* and is protected under U.S. and international copyright and other intellectual               */
/* property laws.                                                                               */
/*                                                                                              */
/*                                                                                              */
/* DISCLAIMER                                                                                   */
/*                                                                                              */
/* This disclaimer is not a license and does not grant any rights to the materials              */
/* distributed herewith. Except as otherwise provided in a valid license issued                 */
/* to you by Xilinx, and to the maximum extent permitted by applicable law:                     */
/*                                                                                              */
/* (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND WITH ALL FAULTS,                          */
/* AND XILINX HEREBY DISCLAIMS ALL WARRANTIES AND CONDITIONS, EXPRESS, IMPLIED,                 */
/* OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY,                    */
/* NON-INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE; and                                 */
/*                                                                                              */
/* (2) Xilinx shall not be liable (whether in contract or tort, including negligence,           */
/* or under any other theory of liability) for any loss or damage of any kind or                */
/* nature related to, arising under or in connection with these materials,                      */
/* including for any direct, or any indirect, special, incidental, or consequential             */
/* loss or damage (including loss of data, profits, goodwill, or any type of loss or            */
/* damage suffered as a result of any action brought by a third party) even if such             */
/* damage or loss was reasonably foreseeable or Xilinx had been advised of the                  */
/* possibility of the same.                                                                     */
/*                                                                                              */
/*                                                                                              */
/* CRITICAL APPLICATIONS                                                                        */
/*                                                                                              */
/* Xilinx products are not designed or intended to be fail-safe, or for use in                  */
/* any application requiring fail-safe performance, such as life-support or safety              */
/* devices or systems, Class III medical devices, nuclear facilities, applications              */
/* related to the deployment of airbags, or any other applications that could lead              */
/* to death, personal injury, or severe property or environmental damage (individually          */
/* and collectively, "Critical Applications"). Customer assumes the sole risk and               */
/* liability of any use of Xilinx products in Critical Applications, subject                    */
/* only to applicable laws and regulations governing limitations on product liability.          */
/*                                                                                              */
/*                                                                                              */
/* THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS PART OF THIS FILE AT ALL TIMES.     */
/*                                                                                              */
/*----------------------------------------------------------------------------------------------*/


/* COMMON_DIST */ 





/*
 *
 *  RCS Keyword Metadata
 *
 *  $Change: 3163114 $
 *  $Date: 2021/03/29 $
 *  $Revision: #60 $
 *
 */




#include "cmc_data_store.h"

void DataStoreWriteMACAddressToRegister(CMC_DATA_STORE_CONTEXT_TYPE* pContext, uint64_t input, uint32_t HostRegister)
{
    PERIPHERAL_REGMAP_RAM_CONTROLLER_Write(pContext->pHardwareRegisterSetContext, HostRegister, (uint32_t)((input & 0xFFFFFFFF00000000) >> 32));
    PERIPHERAL_REGMAP_RAM_CONTROLLER_Write(pContext->pHardwareRegisterSetContext, HostRegister + 4, (uint32_t)(input & 0xFFFFFFFF));
}

uint64_t cmcU8ToU64(uint8_t * payload)
{
    return (uint64_t)payload[5] | (((uint64_t)payload[4]) << 8) | (((uint64_t)payload[3]) << 16) | (((uint64_t)payload[2]) << 24) |
         (((uint64_t)payload[1]) << 32) | (((uint64_t)payload[0]) << 40);
}


void cmcU8ToMacString(uint8_t* payload, char* macString)
{
    uint8_t i;
    uint8_t macNibble;

    for (i = 0; i < DATA_STORE_MAC_ADDRESS_SIZE; i += 3)
    {
        macNibble = (payload[i / 3] >> 4) & 0x0F;
        macString[i] = macNibble < 10 ? macNibble + 48 : macNibble + 55;
        macNibble = payload[i / 3] & 0x0F;
        macString[i + 1] = macNibble < 10 ? macNibble + 48 : macNibble + 55;
        macString[i + 2] = ':';
    }

    macString[DATA_STORE_MAC_ADDRESS_SIZE] = '\0';
}

void cmcMemClear(uint8_t* ptr, size_t length)
{
    size_t i;

    for (i = 0; i < length; i++)
    {
        ptr[i] = 0;
    }
}

uint8_t cmcStrLength(const char* str)
{
    uint8_t i;

    for (i = 0; str[i] != '\0'; i++)
    {
        continue;
    }

    return i;
}

void cmcStrCopy(char* destination, const char* source)
{
    while ((*(destination++) = *(source++)))
    {
        continue;
    }
}

void cmcIncrementReg(PERIPHERAL_REGMAP_RAM_CONTROLLER_CONTEXT_TYPE* pHardwareRegisterSetContext, uint32_t regAddr)
{
    uint32_t regValCurrent;

    regValCurrent = PERIPHERAL_REGMAP_RAM_CONTROLLER_Read(pHardwareRegisterSetContext, regAddr);
    PERIPHERAL_REGMAP_RAM_CONTROLLER_Write(pHardwareRegisterSetContext, regAddr, ++regValCurrent);
}

void DataStore_ResetOEMID(CMC_DATA_STORE_CONTEXT_TYPE* pContext)
{
    pContext->OEMID_Available = false;
    pContext->OEMID = 0;

    PERIPHERAL_REGMAP_RAM_CONTROLLER_Write(pContext->pHardwareRegisterSetContext, HOST_REGISTER_OEM_ID, pContext->OEMID);

}

void DataStore_ResetSensors(CMC_DATA_STORE_CONTEXT_TYPE* pContext)
{
    uint8_t i;
    uint32_t regValCurrent;

    // set average/max to instantaneous for all sensors
    for (i = 0; i < CMC_MAX_NUM_SNSRS; ++i)
    {
        regValCurrent = PERIPHERAL_REGMAP_RAM_CONTROLLER_Read(pContext->pHardwareRegisterSetContext, i * 12 + HOST_REGISTER_SNSR_INS_REG_OFFSET);
        PERIPHERAL_REGMAP_RAM_CONTROLLER_Write(pContext->pHardwareRegisterSetContext, i * 12 + HOST_REGISTER_SNSR_MAX_REG_OFFSET, regValCurrent);
        PERIPHERAL_REGMAP_RAM_CONTROLLER_Write(pContext->pHardwareRegisterSetContext, i * 12 + HOST_REGISTER_SNSR_AVG_REG_OFFSET, regValCurrent);
    }

    // sample count for ever present sensor readings
    pContext->SensorInfo.ProcessSensorCount = 1;
}

void DataStore_BindBroker(CMC_DATA_STORE_CONTEXT_TYPE* pContext, struct BROKER_CONTEXT_TYPE* pBrokerContext)
{
    pContext->pBrokerContext = pBrokerContext;
}


void DataStore_Initialize(  CMC_DATA_STORE_CONTEXT_TYPE *pContext, 
                            PERIPHERAL_REGMAP_RAM_CONTROLLER_CONTEXT_TYPE * pHardwareRegisterSetContext, 
                            CMC_WATCHPOINT_CONTEXT_TYPE* pWatchPointContext,
                            CMC_FIRMWARE_VERSION_CONTEXT_TYPE * pFirmwareVersionContext,
                            bool bSensorsGatedByPowerGood,
                            bool bZync1Device,
                            uint64_t CMCValidSensors_0_to_63,
                            uint64_t CMCValidSensors_64_to_127,
                            bool bCardSupportsScalingFactor,
                            bool bCardSupportsSCUpgrade
    )
{
    int i;
    pContext->pHardwareRegisterSetContext=pHardwareRegisterSetContext;
    pContext->pWatchPointContext = pWatchPointContext;
    pContext->pFirmwareVersionContext = pFirmwareVersionContext;
    DataStore_InitializeBoardInfo(&(pContext->BoardInfo));
    DataStore_ResetSensors(pContext);
    DataStore_ResetOEMID(pContext);
    pContext->bSensorsGatedByPowerGood = bSensorsGatedByPowerGood;
    pContext->bPowerGood = false;
    pContext->bZync1Device = bZync1Device;
    pContext->CMCValidSensors_0_to_63 = CMCValidSensors_0_to_63;
    pContext->CMCValidSensors_64_to_127 = CMCValidSensors_64_to_127;
    pContext->bCardSupportsScalingFactor = bCardSupportsScalingFactor;
    pContext->bCardSupportsSCUpgrade = bCardSupportsSCUpgrade;
    pContext->CMCHeartbeatSensors_0_to_63 = 0;
    pContext->CMCHeartbeatSensors_64_to_127 = 0;
    // sample count for ever present sensor readings
    //pContext->SensorInfo.ProcessSensorCount = 1;
    for (i = 0; i < (DATA_STORE_SATELLITE_CONTROLLER_FW_REVISION_SIZE + 1); i++)
    {     
        pContext->BoardInfo.SatelliteController_FirmwarePreviousRevision[i] = '\0';
    }
    pContext->bRenegotiateComms = false;
}

void DataStore_StoreSensorValue(CMC_DATA_STORE_CONTEXT_TYPE* pContext, uint8_t id, uint32_t value, bool bCardSupportsScalingFactor, uint8_t CommsVersion)
{
    uint32_t sampleCount, runningAverage;
    bool CalculateAverage = true;
    sampleCount = pContext->SensorInfo.ProcessSensorCount;
    runningAverage = 0;

    // sensor reading conditional processing
    switch (id)
    {
    case SNSR_ID_POWER_GOOD:
        pContext->bPowerGood = value ? true : false;
        break;

    case SNSR_ID_CAGE_TEMP0:
    case SNSR_ID_CAGE_TEMP1:
    case SNSR_ID_CAGE_TEMP2:
    case SNSR_ID_CAGE_TEMP3:
        
        // Workaround for SC occasionally sending burst of zero value cage temperatures on module insertion
        if (0 == value)
        {
            CalculateAverage = false;
        }

        break;
    case SNSR_ID_VCCINT_I:
        if (CommsVersion >= 8)
        {
            break;
        }
        else
        {
            if (bCardSupportsScalingFactor)
            {
                // scale sensor reading for VCCINT current depending on power available (0:75W, 1:150W, 2:225W)
                if (1 == pContext->BoardInfo.AvailablePower)
                {
                    value *= 4;
                }
                else if (2 == pContext->BoardInfo.AvailablePower)
                {
                    value *= 6;
                }
                else if (3 == pContext->BoardInfo.AvailablePower)
                {
                    value *= 8;
                }
                break;
            }
        }

    default:
        break;
    }


    // Gate storing the sensor values on POWER GOOD being ok. But always store Power Good 
    // TODO - AP. Uncomment the line below after the SC has added the power good sensor
    if ((!pContext->bSensorsGatedByPowerGood || pContext->bPowerGood) || (id == SNSR_ID_POWER_GOOD))
    {
        // store instantaneous
        PERIPHERAL_REGMAP_RAM_CONTROLLER_Write(pContext->pHardwareRegisterSetContext, id * 12 + HOST_REGISTER_SNSR_INS_REG_OFFSET, value);

        if(CalculateAverage)
        {
            // calculate and store average
            double currentAverage = (double)pContext->SensorInfo.fSensorAverage[id];
            pContext->SensorInfo.fSensorAverage[id] = (float)(((currentAverage * (float)(sampleCount - 1)) + (float)value) / (float)sampleCount);
        
        
            runningAverage = (uint32_t)pContext->SensorInfo.fSensorAverage[id];
        
        
            PERIPHERAL_REGMAP_RAM_CONTROLLER_Write(pContext->pHardwareRegisterSetContext, id * 12 + HOST_REGISTER_SNSR_AVG_REG_OFFSET, runningAverage);
        }

        // calculate and store maximum      
        if (value > PERIPHERAL_REGMAP_RAM_CONTROLLER_Read(pContext->pHardwareRegisterSetContext, id * 12 + HOST_REGISTER_SNSR_MAX_REG_OFFSET))
        {
            PERIPHERAL_REGMAP_RAM_CONTROLLER_Write(pContext->pHardwareRegisterSetContext, id * 12 + HOST_REGISTER_SNSR_MAX_REG_OFFSET, value);
        }
    }
    else
    {
        // store instantaneous as a zero
        PERIPHERAL_REGMAP_RAM_CONTROLLER_Write(pContext->pHardwareRegisterSetContext, id * 12 + HOST_REGISTER_SNSR_INS_REG_OFFSET, 0);
    }
}

void DataStore_CheckInvalidID(CMC_DATA_STORE_CONTEXT_TYPE* pContext, uint8_t SensorID)
{
    uint32_t watchValue;

    if (SensorID >= SENSOR_IDS_3)
    {
        Watch_Get(pContext->pWatchPointContext, W_INVALID_SENSOR_ID_FOR_PROFILE_96_127_RECEIVED, &watchValue);
        watchValue = watchValue | ((uint32_t)1 << (SensorID - 96));
        Watch_Set(pContext->pWatchPointContext, W_INVALID_SENSOR_ID_FOR_PROFILE_96_127_RECEIVED, watchValue);
    }
    else if (SensorID >= SENSOR_IDS_2)
    {
        Watch_Get(pContext->pWatchPointContext, W_INVALID_SENSOR_ID_FOR_PROFILE_64_95_RECEIVED, &watchValue);
        watchValue = watchValue | ((uint32_t)1 << (SensorID - 64));
        Watch_Set(pContext->pWatchPointContext, W_INVALID_SENSOR_ID_FOR_PROFILE_64_95_RECEIVED, watchValue);
    }
    else if (SensorID >= SENSOR_IDS_1)
    {
        Watch_Get(pContext->pWatchPointContext, W_INVALID_SENSOR_ID_FOR_PROFILE_32_63_RECEIVED, &watchValue);
        watchValue = watchValue | ((uint32_t)1 << (SensorID - 32));
        Watch_Set(pContext->pWatchPointContext, W_INVALID_SENSOR_ID_FOR_PROFILE_32_63_RECEIVED, watchValue);
    }
    else
    {
        Watch_Get(pContext->pWatchPointContext, W_INVALID_SENSOR_ID_FOR_PROFILE_0_31_RECEIVED, &watchValue);
        watchValue = watchValue | ((uint32_t)1 << (SensorID));
        Watch_Set(pContext->pWatchPointContext, W_INVALID_SENSOR_ID_FOR_PROFILE_0_31_RECEIVED, watchValue);
    }
}

void DataStore_SensorIDReceived(CMC_DATA_STORE_CONTEXT_TYPE* pContext, uint8_t SensorID)
{
    uint64_t watchValue;

    if (SensorID >= SENSOR_IDS_2)
    {
        watchValue = pContext->CMCHeartbeatSensors_64_to_127;
        watchValue = watchValue | ((uint64_t)1 << (SensorID - 64));
        pContext->CMCHeartbeatSensors_64_to_127 = watchValue;
    }
    else
    {
        watchValue = pContext->CMCHeartbeatSensors_0_to_63;
        watchValue = watchValue | ((uint64_t)1 << (SensorID));
        pContext->CMCHeartbeatSensors_0_to_63 = watchValue;
    }
}

void DataStore_UpdateSensors_Pre_Version_7(CMC_DATA_STORE_CONTEXT_TYPE* pContext, char* payload, uint16_t length, uint8_t messageID, uint64_t CMCValidSensors_0_to_63, uint64_t CMCValidSensors_64_to_127, bool bCardSupportsScalingFactor, uint8_t CommsVersion)
{
    uint8_t i;
    uint64_t SensorHasTwoBytePayload = (uint64_t)1 << SNSR_ID_FPGA_TEMP |
        (uint64_t)1 << SNSR_ID_FAN_TEMP |
        (uint64_t)1 << SNSR_ID_DIMM_TEMP0 |
        (uint64_t)1 << SNSR_ID_DIMM_TEMP1 |
        (uint64_t)1 << SNSR_ID_DIMM_TEMP2 |
        (uint64_t)1 << SNSR_ID_DIMM_TEMP3 |
        (uint64_t)1 << SNSR_ID_SE98_TEMP0 |
        (uint64_t)1 << SNSR_ID_SE98_TEMP1 |
        (uint64_t)1 << SNSR_ID_SE98_TEMP2 |
        (uint64_t)1 << SNSR_ID_CAGE_TEMP0 |
        (uint64_t)1 << SNSR_ID_CAGE_TEMP1 |
        (uint64_t)1 << SNSR_ID_CAGE_TEMP2 |
        (uint64_t)1 << SNSR_ID_CAGE_TEMP3 |
        (uint64_t)1 << SNSR_ID_HBM_TEMP1 |
        (uint64_t)1 << SNSR_ID_HBM_TEMP2 |
        (uint64_t)1 << SNSR_ID_VCCINT_TEMP;

    for (i = 0; i < length;)
    {
        // coverage counters (with boundary check to prevent memory corruption)
        if (payload[i] < CMC_MAX_NUM_SNSRS)
        {
            cmcIncrementReg(pContext->pHardwareRegisterSetContext, CMC_SC_SNSR_COVERAGE_REG + (payload[i] << 2));


            if (SensorHasTwoBytePayload & ((uint64_t)1 << payload[i]))
            {
                if (CMCValidSensors_0_to_63 & ((uint64_t)1 << payload[i]))
                {
                    DataStore_StoreSensorValue(pContext, (uint8_t)payload[i], (uint32_t)payload[i + 1], bCardSupportsScalingFactor, CommsVersion);
                    DataStore_SensorIDReceived(pContext, payload[i]);
                }
                else if (CMCValidSensors_64_to_127 & ((uint64_t)1 << (payload[i] - 64)))
                {
                    DataStore_StoreSensorValue(pContext, (uint8_t)payload[i], (uint32_t)payload[i + 1], bCardSupportsScalingFactor, CommsVersion);
                    DataStore_SensorIDReceived(pContext, payload[i]);
                }
                else
                {
                    DataStore_CheckInvalidID(pContext, payload[i]);
                }

                i += 2;
            }
            else /* 3 byte payload */
            {
                if (CMCValidSensors_0_to_63 & ((uint64_t)1 << payload[i]))
                {
                    DataStore_StoreSensorValue(pContext, (uint8_t)payload[i], (uint32_t)cmcU8ToU16((uint8_t*)&payload[i + 1]), bCardSupportsScalingFactor, CommsVersion);
                    DataStore_SensorIDReceived(pContext, payload[i]);
                }
                else if (CMCValidSensors_64_to_127 & ((uint64_t)1 << (payload[i] - 64)))
                {
                    DataStore_StoreSensorValue(pContext, (uint8_t)payload[i], (uint32_t)cmcU8ToU16((uint8_t*)&payload[i + 1]), bCardSupportsScalingFactor, CommsVersion);
                    DataStore_SensorIDReceived(pContext, payload[i]);
                }
                else
                {
                    DataStore_CheckInvalidID(pContext, payload[i]);
                }

                i += 3;
            }
        }
        else
        {
            // We cannot parse the message any further
            Watch_Set(pContext->pWatchPointContext, W_INVALID_SENSOR_ID_RECEIVED, (((messageID << 24) & 0xFF000000) | (payload[i] & 0x000000FF)));
            return;
        }
    }
}



void DataStore_UpdateSensors_Post_Version_7(CMC_DATA_STORE_CONTEXT_TYPE* pContext, char* payload, uint16_t length, uint8_t messageID, uint64_t CMCValidSensors_0_to_63, uint64_t CMCValidSensors_64_to_127, bool bCardSupportsScalingFactor, uint8_t CommsVersion)
{
    uint8_t i;

    for (i = 0; i < length;)
    {
        // coverage counters (with boundary check to prevent memory corruption)
        if (payload[i] < CMC_MAX_NUM_SNSRS)
        {
            cmcIncrementReg(pContext->pHardwareRegisterSetContext, CMC_SC_SNSR_COVERAGE_REG + (payload[i] << 2));

            if (payload[i + 1] == 1) /* 1 byte payload sensor value*/
            {
                if (CMCValidSensors_0_to_63 & ((uint64_t)1 << payload[i]))
                {
                    DataStore_StoreSensorValue(pContext, (uint8_t)payload[i], (uint32_t)payload[i + 2], bCardSupportsScalingFactor, CommsVersion);
                    DataStore_SensorIDReceived(pContext, payload[i]);
                }
                else if (CMCValidSensors_64_to_127 & ((uint64_t)1 << (payload[i] - 64)))
                {
                    DataStore_StoreSensorValue(pContext, (uint8_t)payload[i], (uint32_t)payload[i + 2], bCardSupportsScalingFactor, CommsVersion);
                    DataStore_SensorIDReceived(pContext, payload[i]);
                }
                else
                {
                    DataStore_CheckInvalidID(pContext, payload[i]);
                }
            }
            else if (payload[i + 1] == 2) /* 2 byte payload sensor value*/
            {
                if (CMCValidSensors_0_to_63 & ((uint64_t)1 << payload[i]))
                {
                    DataStore_StoreSensorValue(pContext, (uint8_t)payload[i], (uint32_t)cmcU8ToU16((uint8_t*)&payload[i + 2]), bCardSupportsScalingFactor, CommsVersion);
                    DataStore_SensorIDReceived(pContext, payload[i]);
                }
                else if (CMCValidSensors_64_to_127 & ((uint64_t)1 << (payload[i] - 64)))
                {
                    DataStore_StoreSensorValue(pContext, (uint8_t)payload[i], (uint32_t)cmcU8ToU16((uint8_t*)&payload[i + 2]), bCardSupportsScalingFactor, CommsVersion);
                    DataStore_SensorIDReceived(pContext, payload[i]);
                }
                else
                {
                    DataStore_CheckInvalidID(pContext, payload[i]);
                }
            }
            else if (payload[i + 1] == 4) /* 4 byte payload sensor value*/
            {
                if (CMCValidSensors_0_to_63 & ((uint64_t)1 << payload[i]))
                {
                    DataStore_StoreSensorValue(pContext, (uint8_t)payload[i], (uint32_t)cmcU8ToU32((uint8_t*)&payload[i + 2]), bCardSupportsScalingFactor, CommsVersion);
                    DataStore_SensorIDReceived(pContext, payload[i]);
                }
                else if (CMCValidSensors_64_to_127 & ((uint64_t)1 << (payload[i] - 64)))
                {
                    DataStore_StoreSensorValue(pContext, (uint8_t)payload[i], (uint32_t)cmcU8ToU32((uint8_t*)&payload[i + 2]), bCardSupportsScalingFactor, CommsVersion);
                    DataStore_SensorIDReceived(pContext, payload[i]);
                }
                else
                {
                    DataStore_CheckInvalidID(pContext, payload[i]);
                }
            }
            else
            {
                // We cannot parse the message any further
                Watch_Set(pContext->pWatchPointContext, W_INVALID_SENSOR_SIZE, (((messageID << 24) & 0xFF000000) | (payload[i] & 0x000000FF)));
                return;
            }
            i = i + 2 + payload[i + 1];
        }
        else
        {
            // We cannot parse the message any further
            Watch_Set(pContext->pWatchPointContext, W_INVALID_SENSOR_ID_RECEIVED, (((messageID << 24) & 0xFF000000) | (payload[i] & 0x000000FF)));
            return;
        }
    }
}

void DataStore_UpdateSensors(CMC_DATA_STORE_CONTEXT_TYPE* pContext, char* payload, uint16_t length, uint8_t messageID, uint64_t CMCValidSensors_0_to_63, uint64_t CMCValidSensors_64_to_127, bool bCardSupportsScalingFactor, uint8_t CommsVersion)
{
    // Check what the interface version is
    if (CommsVersion >= 7)
    {
        DataStore_UpdateSensors_Post_Version_7(pContext, payload, length, messageID, CMCValidSensors_0_to_63, CMCValidSensors_64_to_127, bCardSupportsScalingFactor, CommsVersion);
    }
    else
    {
        DataStore_UpdateSensors_Pre_Version_7(pContext, payload, length, messageID, CMCValidSensors_0_to_63, CMCValidSensors_64_to_127, bCardSupportsScalingFactor, CommsVersion);
    }


}

void DataStore_UpdateAlertResponse(CMC_DATA_STORE_CONTEXT_TYPE* pContext, char* payload, uint16_t length)
{
    uint8_t i;
    uint32_t value;
    uint32_t WarningStateValue;
    uint32_t CriticalStateValue;

    for (i = 0; i < length; i+=4)
    {
        value = (uint32_t)cmcU8ToU32((uint8_t*)payload + i);

        WarningStateValue = value & 0xFFFF;
        CriticalStateValue = (value & 0xFFFF0000) >> 16;
        PERIPHERAL_REGMAP_RAM_CONTROLLER_Write(pContext->pHardwareRegisterSetContext, HOST_REGISTER_ALERT_RESP_WARNING_REG, WarningStateValue);
        PERIPHERAL_REGMAP_RAM_CONTROLLER_Write(pContext->pHardwareRegisterSetContext, HOST_REGISTER_ALERT_RESP_CRITICAL_REG, CriticalStateValue);
    }
}

void DataStore_UpdateSensorState(CMC_DATA_STORE_CONTEXT_TYPE* pContext, char* payload, uint16_t length)
{
    uint8_t i;
    uint8_t id;
    uint32_t value;

    for (i = 0; i < length;)
    {
        id = (uint8_t)payload[i];
        value = (uint32_t)payload[i + 1];
        // Ensure we only store values for allowed sensor ids
        if((id <= SNSR_ID_VCC1V2_BTM) || (id == SNSR_ID_VCCINT))
        {
            PERIPHERAL_REGMAP_RAM_CONTROLLER_Write(pContext->pHardwareRegisterSetContext, id * 4 + HOST_REGISTER_SNSR_STATE_BASE_REG, value);
        }
        i += 2;
    }
}

void DataStore_UpdateOEM(CMC_DATA_STORE_CONTEXT_TYPE* pContext, char* payload, uint16_t length)
{
    /* Make sure the OEM ID is exactly 4 bytes*/
    if (length == 4)
    {
        pContext->OEMID = (uint32_t)cmcU8ToU32((uint8_t*)payload);
        PERIPHERAL_REGMAP_RAM_CONTROLLER_Write(pContext->pHardwareRegisterSetContext, HOST_REGISTER_OEM_ID, pContext->OEMID);
        pContext->OEMID_Available = true;
    }
}

void DataStore_Update_Interrupt_Status(CMC_DATA_STORE_CONTEXT_TYPE* pContext, char* payload, uint16_t length)
{
    uint32_t QSPI_status;
    /* Make sure the QSPI status is exactly 3 bytes*/
    if ((length == 3) || (length == 5))
    {     
        if (payload[0] == 0x1)
        {
            // Interrupt is due to a QSPI Write Protect
            QSPI_status = (uint32_t)cmcU8ToU16((uint8_t*)& payload[1]);
            PERIPHERAL_REGMAP_RAM_CONTROLLER_Write(pContext->pHardwareRegisterSetContext, HOST_REGISTER_QSPI_STATUS, QSPI_status);
        }          
    }
    
    if (length == 5)
    {
        if (payload[3] == 0x1)
        {
            // Interrupt is due to a ZYNC UART change required
            PERIPHERAL_REGMAP_RAM_CONTROLLER_Write(pContext->pHardwareRegisterSetContext, HOST_REGISTER_ZYNC_UART_STATUS, (uint32_t)payload[4]);   
        }
    }
}

void DataStore_UpdatePowerThresholdResponse(CMC_DATA_STORE_CONTEXT_TYPE* pContext, char* payload, uint16_t length, uint8_t CommsVersion)
{
    uint8_t i;
    uint8_t id;
    uint32_t offset = 0; 
    for (i = 0; i < length;)
    {
        id = (uint8_t)payload[i];

        PERIPHERAL_REGMAP_RAM_CONTROLLER_Write(pContext->pHardwareRegisterSetContext, (offset++) * 4 + HOST_REGISTER_POWER_THRESHOLD_BASE_REG, (uint32_t)id);

        if (CommsVersion >= 7)
        {
            if (payload[i + 1] == 1) /* 1 byte payload sensor value*/
            {
                PERIPHERAL_REGMAP_RAM_CONTROLLER_Write(pContext->pHardwareRegisterSetContext, (offset++) * 4 + HOST_REGISTER_POWER_THRESHOLD_BASE_REG, (uint32_t)(payload[i + 2]));
            }
            else if (payload[i + 1] == 2) /* 2 byte payload sensor value*/
            {
                PERIPHERAL_REGMAP_RAM_CONTROLLER_Write(pContext->pHardwareRegisterSetContext, (offset++) * 4 + HOST_REGISTER_POWER_THRESHOLD_BASE_REG, (uint32_t)cmcU8ToU16((uint8_t*)payload + i + 2));
            }
            else if (payload[i + 1] == 4) /* 4 byte payload sensor value*/
            {
                PERIPHERAL_REGMAP_RAM_CONTROLLER_Write(pContext->pHardwareRegisterSetContext, (offset++) * 4 + HOST_REGISTER_POWER_THRESHOLD_BASE_REG, (uint32_t)cmcU8ToU32((uint8_t*)payload + i + 2));
            }
            i = i + 2 + payload[i + 1];
        }
        else
        {
            PERIPHERAL_REGMAP_RAM_CONTROLLER_Write(pContext->pHardwareRegisterSetContext, (offset++) * 4 + HOST_REGISTER_POWER_THRESHOLD_BASE_REG, (uint32_t)cmcU8ToU16((uint8_t*)payload + i + 1));
            i += 3;
        }      
    }
}

void DataStore_UpdateTempThresholdResponse(CMC_DATA_STORE_CONTEXT_TYPE* pContext, char* payload, uint16_t length)
{
    uint8_t i;
    uint8_t id;
    uint32_t value;
    uint32_t offset = 0;

    for (i = 0; i < length;)
    {
        id = (uint8_t)payload[i];
        value = (uint32_t)payload[i + 1];
        PERIPHERAL_REGMAP_RAM_CONTROLLER_Write(pContext->pHardwareRegisterSetContext, (offset++) * 4 + HOST_REGISTER_TEMP_THRESHOLD_BASE_REG, (uint32_t)id);
        PERIPHERAL_REGMAP_RAM_CONTROLLER_Write(pContext->pHardwareRegisterSetContext, (offset++) * 4 + HOST_REGISTER_TEMP_THRESHOLD_BASE_REG, value);
        i += 2;
    }
}