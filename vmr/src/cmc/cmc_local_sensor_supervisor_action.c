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
 *  $Revision: #92 $
 *
 */

#include "cmc_local_sensor_supervisor_thread.h"
#include "cmc_supervisor_message_formatter.h"
#include "cmc_supervisor_message_framer.h"
#include "cmc_transmit_defines.h"
#include "cmc_supervisor_message_parser.h"
#include "cmc_version_support.h"
#include "cmc_peripheral_axi_gpio_hbm.h"
#include "cmc_feature_control.h"
#include "cmc_receive_defines.h"
#include "cmc_clock_throttling.h"
#include <stdio.h>

extern CMC_CLOCK_THROTTLING_CONTEXT_TYPE		    ClockThrottlingAlgorithmContext;
extern SENSOR_SUPERVISOR_PARSER_CONTEXT_TYPE		SensorSupervisorParserContext;
extern CMC_VERSION_SUPPORT_CONTEXT_TYPE				VersionSupportContext;
extern PERIPHERAL_AXI_GPIO_HBM_CONTEXT_TYPE			AXI_GPIO_HBM_Context;
extern PERIPHERAL_AXI_GPIO_QSFP_CONTEXT_TYPE		AXI_GPIO_QSFP_Context;

static void storeQSFPdata(LOCAL_SENSOR_SUPERVISOR_THREAD_CONTEXT_TYPE* pContext, char* payload, uint8_t length)
{
    int i;
    PERIPHERAL_REGMAP_RAM_CONTROLLER_Write(pContext->DataStoreContext.pHardwareRegisterSetContext, (HOST_REGISTER_REMOTE_COMMAND_READWRITE_QSFP_DIAGNOSTICS_LENGTH), (uint32_t)length);

    for (i = 0; i < length; i += 4)
    {
       PERIPHERAL_REGMAP_RAM_CONTROLLER_Write(pContext->DataStoreContext.pHardwareRegisterSetContext, (HOST_REGISTER_REMOTE_COMMAND_READWRITE_QSFP_DIAGNOSTICS_DATA + i),
            (uint32_t)cmcU8ToU32(((uint8_t*)&payload[i])));
    }
}

static void CSDRFailed(LOCAL_SENSOR_SUPERVISOR_THREAD_CONTEXT_TYPE* pContext)
{
    pContext->CSDRResponseWaitCount = CSDR_MESSGAE_TIMEOUT_COUNT;
    pContext->WaitingOnCSDRResponse = IDLE;
    Broker_Announce_UserProxyCSDRFailed(pContext->pBrokerContext);
}


static void storeCSDRdata(LOCAL_SENSOR_SUPERVISOR_THREAD_CONTEXT_TYPE* pContext, char* payload, uint8_t length)
{
    uint8_t i = 0, j = 0, registerIncrement = 0;
    uint8_t header[2], footer[2];
    uint16_t totalRecordSize = (uint16_t)cmcU8ToU16((uint8_t*)payload);
    i = 2;
    uint8_t individualRecordSize = (uint8_t)payload[i++];
    uint8_t record = (uint8_t)payload[i++];
    uint8_t initialRecord = record;
    uint8_t numberOfRecords = (uint8_t)payload[i++];
    uint32_t Offset;
    uint32_t StartingOffset;
    uint32_t RecordOffset;
   
    if (totalRecordSize != CSDR_MESSGAE_TOTAL_RECORD_SIZE)
    {
        Watch_Set(pContext->pWatchPointContext, W_CSDR_ERROR_IN_MESSAGE, CSDR_TOTAL_RECORD_SIZE_ERROR + (totalRecordSize << 8));
        CSDRFailed(pContext);
        return;
    }

    if (individualRecordSize != CSDR_MESSGAE_INDIVIDUAL_RECORD_SIZE)
    {
        Watch_Set(pContext->pWatchPointContext, W_CSDR_ERROR_IN_MESSAGE, CSDR_INDIVIDUAL_RECORD_SIZE_ERROR + (individualRecordSize<<8));
        CSDRFailed(pContext);
        return;
    }

    // Response received mark it
    pContext->WaitingOnCSDRResponse = RESPONSE_RECEIVED;
    pContext->CSDRResponseWaitCount = CSDR_MESSGAE_TIMEOUT_COUNT;
    switch (length)
    {
    case ((CSDR_MESSGAE_INDIVIDUAL_RECORD_SIZE + 4) + 5):
    case ((CSDR_MESSGAE_INDIVIDUAL_RECORD_SIZE + 4)*2 + 5):
    case ((CSDR_MESSGAE_INDIVIDUAL_RECORD_SIZE + 4)*3 + 5):
        {
            for (j = 0; j < numberOfRecords; j++)
            {
                header[0] = (uint8_t)payload[i++];
                header[1] = (uint8_t)payload[i++];
                record = header[0];
                if (record != initialRecord + j)
                {
                    Watch_Set(pContext->pWatchPointContext, W_CSDR_ERROR_IN_MESSAGE, CSDR_HEADER_ERROR);
                    CSDRFailed(pContext);
                    return;
                }

                //Watch_Set(pContext->pWatchPointContext, W_CSDR_ERROR_IN_MESSAGE, CSDR_FOOTER_ERROR + (i << 8));

                // Starting position of records
                if (record < 32)
                {
                    StartingOffset = HOST_REGISTER_REMOTE_COMMAND_FIRST_CSDR_RECORD_RESPONSE;
                }
                else
                {
                    StartingOffset = HOST_REGISTER_REMOTE_COMMAND_ADDITIONAL_CSDR_RECORD_RESPONSE;
                }

                // Work out the offset for this record
                if (record < 32)
                {
                    RecordOffset = record * 0x40;
                }
                else if (record < 64)
                {
                    RecordOffset = (record - 32) * 0x40;
                }
                else if (record < 96)
                {
                    RecordOffset = (record - 64) * 0x40;
                }
                else
                {
                    RecordOffset = (record - 96) * 0x40;
                }

                // Work out the offset for register within the record
                for (registerIncrement = 0; registerIncrement < 64; registerIncrement += 4, i += 4)
                {                
                    Offset = StartingOffset  + RecordOffset + registerIncrement;

                    PERIPHERAL_REGMAP_RAM_CONTROLLER_Write(pContext->DataStoreContext.pHardwareRegisterSetContext,Offset, (uint32_t)cmcU8ToU32((uint8_t*)&payload[i]));
                }

                footer[0] = (uint8_t)payload[i++];
                footer[1] = (uint8_t)payload[i++];
                if (footer[1] != 0xE)
                {
                    Watch_Set(pContext->pWatchPointContext, W_CSDR_ERROR_IN_MESSAGE, CSDR_FOOTER_ERROR + (footer[1] <<8));
                    CSDRFailed(pContext);
                    return;
                }
                if (footer[0] != 0x0F)
                {
                    
                    Watch_Set(pContext->pWatchPointContext, W_CSDR_ERROR_IN_MESSAGE, CSDR_FOOTER_ERROR + (footer[0]  << 16));
                    CSDRFailed(pContext);
                    return;
                }
            }
        }
        // Message received ok - mark it as ok 
        pContext->CSDRResponseWaitCount = CSDR_MESSGAE_TIMEOUT_COUNT+1;
        break;

    default:
        {
            // error
            Watch_Set(pContext->pWatchPointContext, W_CSDR_ERROR_IN_MESSAGE, CSDR_LENGTH_ERROR + (length << 8));
            CSDRFailed(pContext);
            return;
        }
        break;


    }

    if (initialRecord == CSDR_LAST_RECORD_NUMBER_1ST_PART ||
        initialRecord == CSDR_LAST_RECORD_NUMBER_2ND_PART ||
        initialRecord == CSDR_LAST_RECORD_NUMBER_3RD_PART ||
        initialRecord == CSDR_LAST_RECORD_NUMBER_4TH_PART)
    {
        // Signal XRT that message is complete
        if (initialRecord == CSDR_LAST_RECORD_NUMBER_1ST_PART)
        {
            PERIPHERAL_REGMAP_RAM_CONTROLLER_Write(pContext->DataStoreContext.pHardwareRegisterSetContext,
                HOST_REGISTER_REMOTE_COMMAND_FIRST_CSDR_TOTAL_RECORD_SIZE_RESPONSE, (uint32_t)CSDR_MESSGAE_TOTAL_RECORD_SIZE);

            PERIPHERAL_REGMAP_RAM_CONTROLLER_Write(pContext->DataStoreContext.pHardwareRegisterSetContext,
                HOST_REGISTER_REMOTE_COMMAND_FIRST_CSDR_THIS_RECORD_SIZE_RESPONSE, (uint32_t)(CSDR_MESSGAE_INDIVIDUAL_RECORD_SIZE *32));
        }
        else
        {
            PERIPHERAL_REGMAP_RAM_CONTROLLER_Write(pContext->DataStoreContext.pHardwareRegisterSetContext,
                HOST_REGISTER_REMOTE_COMMAND_ADDITIONAL_CSDR_THIS_RECORD_SIZE_RESPONSE, (uint32_t)(CSDR_MESSGAE_INDIVIDUAL_RECORD_SIZE * 32));
        }
        pContext->WaitingOnCSDRResponse = IDLE;
        Broker_Announce_UserProxyCSDRComplete(pContext->pBrokerContext);        
    }


}

static void checkExpectedMessage(LOCAL_SENSOR_SUPERVISOR_THREAD_CONTEXT_TYPE* pContext, SENSOR_SUPERVISOR_EXPECTED_MSG_TYPE thisMessage)
{
    uint32_t watchValue;

    if (pContext->ExpectedMessage != thisMessage)
    {
        Watch_Get(pContext->pWatchPointContext, W_SENSOR_SUPERVISOR_LATE_MESSAGE_ARRIVAL, &watchValue);
        watchValue = ((watchValue & 0x00FFFFFF) | ((thisMessage << 24) & 0xFF000000)) + 1;
        Watch_Set(pContext->pWatchPointContext, W_SENSOR_SUPERVISOR_LATE_MESSAGE_ARRIVAL, watchValue);
    }
    else
    {
        pContext->ExpectedMessage = SS_EXPECTED_MSG_NONE;
    }
}
void SensorSupervisorSendMessage(LOCAL_SENSOR_SUPERVISOR_THREAD_CONTEXT_TYPE* pContext, CMC_REQUEST_MESSAGE_TYPE requestMessage)
{
	LINK_PROTOCOL_TRACE_ELEMENT_TYPE LinkProtocolTraceLogElement;

	if (CMC_VersionSupport_CheckMessageSupport(&VersionSupportContext, requestMessage))
	{
		PROTOCOL_SENSOR_FormatREQMessage(pContext->pProtocol, pContext->TxFormattedMsgBuffer, &(pContext->TxFormattedMsgBufferSize), requestMessage);
		TRANSPORT_SENSOR_FrameMessage(pContext->pTransport, pContext->TxFormattedMsgBuffer, &(pContext->TxFormattedMsgBufferSize), pContext->TxFramedMsgBuffer, &(pContext->TxFramedMsgBufferSize));
		TRANSPORT_SENSOR_SendByteSequenceBlocking(pContext->pTransport, (char*)pContext->TxFramedMsgBuffer, pContext->TxFramedMsgBufferSize);

		/* Trace */
		LinkProtocolTraceLogElement.MsgID = requestMessage;
		LinkProtocolTraceLogElement.FSMCurrentState = pContext->State;
		LinkProtocolTraceLogElement.FSMPrevState = pContext->PreviousState;
		LinkProtocolTrace_TryAdd(pContext->LinkProtocolTraceContext, &LinkProtocolTraceLogElement);

		/* Increment the message coverage counter */
		cmcIncrementReg(pContext->DataStoreContext.pHardwareRegisterSetContext, CMC_SC_MSG_COVERAGE_REG + (requestMessage << 2));
	}
	else
	{
		Watch_Inc(pContext->pWatchPointContext, W_SENSOR_SUPERVISOR_MESSAGE_VERSION_UNSUPPORTED);
        pContext->ExpectedMessage = SS_EXPECTED_MSG_NONE;
	}
}

void checkAllSensorIDsReceived(LOCAL_SENSOR_SUPERVISOR_THREAD_CONTEXT_TYPE* pContext)
{
    uint64_t CMCHeartbeatSensors_0_to_63 = 0;
    uint64_t CMCHeartbeatSensors_64_to_127 = 0;
    uint64_t CMCValidSensors_0_to_63 = 0;
    uint64_t CMCValidSensors_64_to_127 = 0;

    // check for CMS and if so mask off all but common sensors between u2xx/50/nextgen 
    if (pContext->DataStoreContext.pFirmwareVersionContext->Version.Minor == 0x8)
    {
        CMCHeartbeatSensors_0_to_63 = pContext->DataStoreContext.CMCHeartbeatSensors_0_to_63 & 0x00000000030754A3;
        CMCHeartbeatSensors_64_to_127 = pContext->DataStoreContext.CMCHeartbeatSensors_64_to_127 & 0x0000000000000000;
        CMCValidSensors_0_to_63 = pContext->DataStoreContext.CMCValidSensors_0_to_63 & 0x00000000030754A3;
        CMCValidSensors_64_to_127 = pContext->DataStoreContext.CMCValidSensors_64_to_127 & 0x0000000000000000;
    }
    else
    {
        CMCHeartbeatSensors_0_to_63 = pContext->DataStoreContext.CMCHeartbeatSensors_0_to_63 & 0xFFFF00000FFFFFFF;
        CMCHeartbeatSensors_64_to_127 = pContext->DataStoreContext.CMCHeartbeatSensors_64_to_127 & 0xFFFFFFFFFFFFF7BF;
        CMCValidSensors_0_to_63 = pContext->DataStoreContext.CMCValidSensors_0_to_63 & 0xFFFF00000FFFFFFF;
        CMCValidSensors_64_to_127 = pContext->DataStoreContext.CMCValidSensors_64_to_127 & 0xFFFFFFFFFFFFF7BF;
    }

    uint64_t CMCMissedSensors_0_to_63 = CMCHeartbeatSensors_0_to_63 ^ CMCValidSensors_0_to_63;
    uint64_t CMCMissedSensors_64_to_127 = CMCHeartbeatSensors_64_to_127 ^ CMCValidSensors_64_to_127;

    uint8_t Sensor = 0;
    uint8_t Count_0_to_63 = 0;
    uint8_t Count_64_to_127 = 0;
    uint8_t Total_Count = 0;

    // additional check for U30 ZYNQ 
    uint32_t OEMID = PERIPHERAL_REGMAP_RAM_CONTROLLER_Read(pContext->DataStoreContext.pHardwareRegisterSetContext, HOST_REGISTER_OEM_ID);
    if (pContext->DataStoreContext.pFirmwareVersionContext->Version.Minor == 0x4 && OEMID == 0x12EB)
    {
        // U30 ZYNQ AWS mode - No sensors being passed so increment heartbeat
        cmcIncrementReg(pContext->DataStoreContext.pHardwareRegisterSetContext, CMC_HEARTBEAT);
    }
    else
    {
        if (CMCHeartbeatSensors_0_to_63 == CMCValidSensors_0_to_63 && CMCHeartbeatSensors_64_to_127 == CMCValidSensors_64_to_127)
        {
            cmcIncrementReg(pContext->DataStoreContext.pHardwareRegisterSetContext, CMC_HEARTBEAT);
        }
        else
        {
            for (Sensor = 0; Sensor < 64; Sensor++)
            {
                if ((CMCMissedSensors_0_to_63 >> Sensor) & 0x1)
                {
                    Count_0_to_63++;
                    PERIPHERAL_REGMAP_RAM_CONTROLLER_WriteWithMask(pContext->DataStoreContext.pHardwareRegisterSetContext, CMC_HEARTBEAT_ERR_CODE, (((uint32_t)Sensor) << 16), (uint32_t)LAST_SENSOR_NOT_RECEIVED_MASK);
                }
                else if ((CMCMissedSensors_64_to_127 >> Sensor) & 0x1)
                {
                    Count_64_to_127++;
                    PERIPHERAL_REGMAP_RAM_CONTROLLER_WriteWithMask(pContext->DataStoreContext.pHardwareRegisterSetContext, CMC_HEARTBEAT_ERR_CODE, (((uint32_t)(Sensor + 64)) << 16), (uint32_t)LAST_SENSOR_NOT_RECEIVED_MASK);
                }
            }

            Total_Count = Count_0_to_63 + Count_64_to_127;

            if (Total_Count == 1)
            {
                PERIPHERAL_REGMAP_RAM_CONTROLLER_WriteWithMask(pContext->DataStoreContext.pHardwareRegisterSetContext, CMC_HEARTBEAT_ERR_CODE, (uint32_t)SINGLE_EXPECTED_SENSOR_NOT_RECEIVED, (uint32_t)HEARTBEAT_ERROR_CODE_MASK);
                PERIPHERAL_REGMAP_RAM_CONTROLLER_WriteWithMask(pContext->DataStoreContext.pHardwareRegisterSetContext, CMC_HEARTBEAT_ERR_CODE, (((uint32_t)Total_Count) << 24), (uint32_t)SENSOR_NOT_RECEIVED_CNT_MASK);
                Watch_Inc(pContext->pWatchPointContext, W_HEARTBEAT_SINGLE_SNSR_ERR_CNT);
            }
            else if (Total_Count > 1)
            {
                PERIPHERAL_REGMAP_RAM_CONTROLLER_WriteWithMask(pContext->DataStoreContext.pHardwareRegisterSetContext, CMC_HEARTBEAT_ERR_CODE, (uint32_t)MULTIPLE_EXPECTED_SENSORS_NOT_RECEIVED, (uint32_t)HEARTBEAT_ERROR_CODE_MASK);
                PERIPHERAL_REGMAP_RAM_CONTROLLER_WriteWithMask(pContext->DataStoreContext.pHardwareRegisterSetContext, CMC_HEARTBEAT_ERR_CODE, (((uint32_t)Total_Count) << 24), (uint32_t)SENSOR_NOT_RECEIVED_CNT_MASK);
                Watch_Inc(pContext->pWatchPointContext, W_HEARTBEAT_MULTIPLE_SNSR_ERR_CNT);
            }
        }
    }
}

void SensorSupervisorSendSetMessage(LOCAL_SENSOR_SUPERVISOR_THREAD_CONTEXT_TYPE* pContext, CMC_REQUEST_MESSAGE_TYPE requestMessage, uint8_t* pPayload, uint8_t payloadLength)
{
    LINK_PROTOCOL_TRACE_ELEMENT_TYPE LinkProtocolTraceLogElement;

    if (CMC_VersionSupport_CheckMessageSupport(&VersionSupportContext, requestMessage))
    {
        PROTOCOL_SENSOR_FormatSETMessage(pContext->pProtocol, pContext->TxFormattedMsgBuffer, &(pContext->TxFormattedMsgBufferSize), requestMessage, pPayload, payloadLength);
        TRANSPORT_SENSOR_FrameMessage(pContext->pTransport, pContext->TxFormattedMsgBuffer, &(pContext->TxFormattedMsgBufferSize), pContext->TxFramedMsgBuffer, &(pContext->TxFramedMsgBufferSize));
        TRANSPORT_SENSOR_SendByteSequenceBlocking(pContext->pTransport, (char*)pContext->TxFramedMsgBuffer, pContext->TxFramedMsgBufferSize);

        /* Trace */
        LinkProtocolTraceLogElement.MsgID = requestMessage;
        LinkProtocolTraceLogElement.FSMCurrentState = pContext->State;
        LinkProtocolTraceLogElement.FSMPrevState = pContext->PreviousState;
        LinkProtocolTrace_TryAdd(pContext->LinkProtocolTraceContext, &LinkProtocolTraceLogElement);
        /* Increment the message coverage counter */
        cmcIncrementReg(pContext->DataStoreContext.pHardwareRegisterSetContext, CMC_SC_MSG_COVERAGE_REG + (requestMessage << 2));
    }
    else
    {
        Watch_Inc(pContext->pWatchPointContext, W_SENSOR_SUPERVISOR_MESSAGE_VERSION_UNSUPPORTED);
    }
}


void SensorSupervisor_CLEAR_ACTION(LOCAL_SENSOR_SUPERVISOR_THREAD_CONTEXT_TYPE* pContext)
{
    pContext->Action = 0;
}


void SensorSupervisor_ACTION(LOCAL_SENSOR_SUPERVISOR_THREAD_CONTEXT_TYPE* pContext, uint32_t AnyAction)
{
    pContext->Action |= AnyAction;
}

void SensorSupervisor_HandleAction_A_SS_REQUEST_LINK_STATE(LOCAL_SENSOR_SUPERVISOR_THREAD_CONTEXT_TYPE* pContext)
{
    Broker_SensorSupervisorRequestsLinkState(pContext->pBrokerContext);
}

void SensorSupervisor_HandleAction_A_SS_REQUEST_BOARD_INFO(LOCAL_SENSOR_SUPERVISOR_THREAD_CONTEXT_TYPE* pContext)
{
    uint32_t stack_fill_percentage = pContext->pFN_Stack_Check(pContext->pUserContext);
    bool heartbeat = pContext->bCMC_Heartbeat;

    Watch_Set(pContext->pWatchPointContext, W_STACK_FILL_PERCENTAGE, (uint32_t)stack_fill_percentage);

    if (pContext->DataStoreContext.bRenegotiateComms)
    {
        pContext->RenegotiateCommsCounter++;

        if (pContext->RenegotiateCommsCounter == 10)
        {
            pContext->RenegotiateCommsCounter = 0;
            pContext->DataStoreContext.bRenegotiateComms = false;
            Broker_RequestLinkUserIsSensorSupervisor(pContext->pBrokerContext);
        }
    }

    if (heartbeat)
    {
        checkAllSensorIDsReceived(pContext);
    }
    pContext->bCMC_Heartbeat = true;

    if (!pContext->DataStoreContext.BoardInfo.IsValid)
    {
        if (pContext->DataStoreContext.pFirmwareVersionContext->Version.Minor == 0x9) // If U26
        {
            Watch_Set(pContext->pWatchPointContext, W_LINK_RECEIVE_NEGOTIATED_VERSION, pContext->pVersionSupportContext->commsVersion);
            PERIPHERAL_REGMAP_RAM_CONTROLLER_WriteWithMask(pContext->DataStoreContext.pHardwareRegisterSetContext, HOST_REGISTER_STATUS,
                pContext->pVersionSupportContext->commsVersion << HOST_REGISTER_PATTERN_SAT_VER_OFFSET, HOST_REGISTER_PATTERN_SAT_VER_MASK);
        }

        pContext->ExpectedMessage = SS_EXPECTED_MSG_BOARD_INFO_RESP;
        SensorSupervisorSendMessage(pContext, SAT_COMMS_BOARD_INFO_REQ);
    }
    else
    {
        SensorSupervisor_CreateEvent_E_SS_T_SENSOR_EXPIRY(pContext);
    }
}

void SensorSupervisor_HandleAction_A_SS_REQUEST_VOLTAGE_SENSOR_INFORMATION(LOCAL_SENSOR_SUPERVISOR_THREAD_CONTEXT_TYPE *pContext)     
{
    pContext->ExpectedMessage = SS_EXPECTED_MSG_VOLT_SNSR_RESP;
	SensorSupervisorSendMessage(pContext, SAT_COMMS_VOLT_SNSR_REQ);
}

void SensorSupervisor_HandleAction_A_SS_REQUEST_POWER_SENSOR_INFORMATION(LOCAL_SENSOR_SUPERVISOR_THREAD_CONTEXT_TYPE *pContext)
{
    pContext->ExpectedMessage = SS_EXPECTED_MSG_POWER_SNSR_RESP;
    SensorSupervisorSendMessage(pContext, SAT_COMMS_POWER_SNSR_REQ);
}

void SensorSupervisor_HandleAction_A_SS_REQUEST_TEMPERATURE_SENSOR_INFORMATION(LOCAL_SENSOR_SUPERVISOR_THREAD_CONTEXT_TYPE *pContext)         
{
    pContext->ExpectedMessage = SS_EXPECTED_MSG_TEMP_SNSR_RESP;
	SensorSupervisorSendMessage(pContext, SAT_COMMS_TEMP_SNSR_REQ);
}

void SensorSupervisor_HandleAction_A_SS_FETCHING_POWER_THROTTLING_THRESHOLDS(LOCAL_SENSOR_SUPERVISOR_THREAD_CONTEXT_TYPE* pContext)
{
    if (pContext->pFeatureControlContext->pClockThrottlingContext->FeatureEnabled)
    {
        if (pContext->pFeatureControlContext->pClockThrottlingContext->Mode == CT_MODE_POWER_ONLY ||
            pContext->pFeatureControlContext->pClockThrottlingContext->Mode == CT_MODE_BOTH)
        {
            pContext->ExpectedMessage = SS_EXPECTED_MSG_POWER_THROTTLING_THRESHOLDS_RESP;
                SensorSupervisorSendMessage(pContext, SAT_COMMS_SNSR_POWER_THROTTLING_THRESHOLDS_REQ);
        }
        else
        {
            SensorSupervisor_CreateEvent_E_SS_T_SENSOR_EXPIRY(pContext);
        }
    }
    else
    {
        SensorSupervisor_CreateEvent_E_SS_T_SENSOR_EXPIRY(pContext);
    }
}

void SensorSupervisor_HandleAction_A_SS_FETCHING_TEMP_THROTTLING_THRESHOLDS(LOCAL_SENSOR_SUPERVISOR_THREAD_CONTEXT_TYPE* pContext)
{
    if (pContext->pFeatureControlContext->pClockThrottlingContext->FeatureEnabled)
    {
        if (pContext->pFeatureControlContext->pClockThrottlingContext->Mode == CT_MODE_TEMPERATURE_ONLY ||
            pContext->pFeatureControlContext->pClockThrottlingContext->Mode == CT_MODE_BOTH)
        {
            pContext->ExpectedMessage = SS_EXPECTED_MSG_TEMP_THROTTLING_THRESHOLDS_RESP;
                SensorSupervisorSendMessage(pContext, SAT_COMMS_SNSR_TEMP_THROTTLING_THRESHOLDS_REQ);
        }
        else
        {
            SensorSupervisor_CreateEvent_E_SS_T_SENSOR_EXPIRY(pContext);
        }
    }
    else
    {
        SensorSupervisor_CreateEvent_E_SS_T_SENSOR_EXPIRY(pContext);
    }
}

void SensorSupervisor_HandleAction_A_SS_SEND_ALERT_REQUEST(LOCAL_SENSOR_SUPERVISOR_THREAD_CONTEXT_TYPE* pContext)
{
    pContext->ExpectedMessage = SS_EXPECTED_MSG_ALERT_RESP;
    SensorSupervisorSendMessage(pContext, SAT_COMMS_ALERT_REQ);
}

void SensorSupervisor_HandleAction_A_SS_SEND_SENSOR_STATE_REQUEST(LOCAL_SENSOR_SUPERVISOR_THREAD_CONTEXT_TYPE* pContext)
{
    pContext->ExpectedMessage = SS_EXPECTED_MSG_SNSR_STATE_RESP;
	SensorSupervisorSendMessage(pContext, SAT_COMMS_SNSR_STATE_REQ);
}

void SensorSupervisor_HandleAction_A_SS_SEND_OEM_CMD_REQUEST(LOCAL_SENSOR_SUPERVISOR_THREAD_CONTEXT_TYPE* pContext)
{
    if (!pContext->DataStoreContext.OEMID_Available)
    {
        pContext->ExpectedMessage = SS_EXPECTED_MSG_SEND_OEM_CMD_RESP;
        SensorSupervisorSendMessage(pContext, SAT_COMMS_SEND_OEM_CMD);
    }
    else
    {
        SensorSupervisor_CreateEvent_E_SS_T_SENSOR_EXPIRY(pContext);
    }
}

void SensorSupervisor_HandleAction_A_SS_SEND_ENABLE_UART_REQUEST(LOCAL_SENSOR_SUPERVISOR_THREAD_CONTEXT_TYPE* pContext)
{
    
    if (pContext->EnableUartRequest && !pContext->UartEnabled)
    {
        pContext->ExpectedMessage = SS_EXPECTED_MSG_UART_EN_RESP;
        SensorSupervisorSendMessage(pContext, SAT_COMMS_DEBUG_UART_EN);
        pContext->UartEnabled = true;
        pContext->EnableUartRequest = false;
        
    }
    else
    {
        if (pContext->EnableUartCount == 0)
        {
            SensorSupervisor_CreateEvent_E_SS_T_ENABLE_UART_NOT_REQUIRED(pContext);
        }
    }
    pContext->EnableUartCount++;
}


void SensorSupervisor_HandleAction_A_SS_SEND_PCIE_ERRORS(LOCAL_SENSOR_SUPERVISOR_THREAD_CONTEXT_TYPE* pContext)
{
    uint8_t payload[20];
    uint32_t PCIE_Surprise_Down_Error_Count, PCIE_Unsupported_Request_Count, PCIE_Receiver_Error_Count, PCIE_Replay_Timer_Timeout_Count;

    
    if (pContext->PCIeECCSupported)
    {    
        if (pContext->CounterPCIeECC == 0)
        {
            PCIE_Surprise_Down_Error_Count = pContext->pFN_PCIe_Read(pContext->pUserContext, PCIE_ECC_ERROR_PCIE_SURPRISE_DOWN_ERROR_COUNT);
            PERIPHERAL_REGMAP_RAM_CONTROLLER_Write(pContext->DataStoreContext.pHardwareRegisterSetContext, HOST_REGISTER_PCIE_SURPRISE_DOWN_ERROR_COUNT, PCIE_Surprise_Down_Error_Count);

            PCIE_Unsupported_Request_Count = pContext->pFN_PCIe_Read(pContext->pUserContext, PCIE_ECC_ERROR_PCIE_UNSUPPORTED_REQUEST_COUNT);
            PERIPHERAL_REGMAP_RAM_CONTROLLER_Write(pContext->DataStoreContext.pHardwareRegisterSetContext, HOST_REGISTER_PCIE_UNSUPPORTED_REQUEST_COUNT, PCIE_Unsupported_Request_Count);

            PCIE_Receiver_Error_Count = pContext->pFN_PCIe_Read(pContext->pUserContext, PCIE_ECC_ERROR_PCIE_RECEIVER_ERROR_COUNT);
            PERIPHERAL_REGMAP_RAM_CONTROLLER_Write(pContext->DataStoreContext.pHardwareRegisterSetContext, HOST_REGISTER_PCIE_RECEIVER_ERROR_COUNT, PCIE_Receiver_Error_Count);

            PCIE_Replay_Timer_Timeout_Count = pContext->pFN_PCIe_Read(pContext->pUserContext, PCIE_ECC_ERROR_PCIE_REPLAY_TIMER_TIMEOUT_COUNT);
            PERIPHERAL_REGMAP_RAM_CONTROLLER_Write(pContext->DataStoreContext.pHardwareRegisterSetContext, HOST_REGISTER_PCIE_REPLAY_TIMER_TIMEOUT_COUNT, PCIE_Replay_Timer_Timeout_Count);

            payload[0] = CMC_REPORTING_ID_PCIE_SURPRISE_DOWN_ERROR_COUNT;
            cmcU32ToU8(PCIE_Surprise_Down_Error_Count, &payload[1]);
            payload[5] = CMC_REPORTING_ID_PCIE_UNSUPPORTED_REQUEST_COUNT;
            cmcU32ToU8(PCIE_Unsupported_Request_Count, &payload[6]);
            payload[10] = CMC_REPORTING_ID_PCIE_RECEIVER_ERROR_COUNT;
            cmcU32ToU8(PCIE_Receiver_Error_Count, &payload[11]);
            payload[15] = CMC_REPORTING_ID_PCIE_REPLAY_TIMER_TIMEOUT_COUNT;
            cmcU32ToU8(PCIE_Replay_Timer_Timeout_Count, &payload[16]);

            pContext->ExpectedMessage = SS_EXPECTED_MSG_GOOD;
            SensorSupervisorSendSetMessage(pContext, SAT_COMMS_PCIE_ERROR_REPORT, payload, 20);
        }

    }
    else
    {
        SensorSupervisor_CreateEvent_E_SS_T_SENSOR_EXPIRY(pContext);
    }

    return;
}

void SensorSupervisor_HandleAction_A_SS_SEND_ECC_ERRORS(LOCAL_SENSOR_SUPERVISOR_THREAD_CONTEXT_TYPE* pContext)
{

    uint8_t payload[10];
    uint32_t ECC_UE_Errors, ECC_CE_Errors;

    if (pContext->PCIeECCSupported)
    {  
        if (pContext->CounterPCIeECC == 150)
        {
            ECC_UE_Errors = pContext->pFN_ECC_Read(pContext->pUserContext, PCIE_ECC_ERROR_ECC_UE_ERROR_COUNT);
            PERIPHERAL_REGMAP_RAM_CONTROLLER_Write(pContext->DataStoreContext.pHardwareRegisterSetContext, HOST_REGISTER_ECC_UE_ERROR_COUNT, ECC_UE_Errors);

            ECC_CE_Errors = pContext->pFN_ECC_Read(pContext->pUserContext, PCIE_ECC_ERROR_ECC_CE_ERROR_COUNT);
            PERIPHERAL_REGMAP_RAM_CONTROLLER_Write(pContext->DataStoreContext.pHardwareRegisterSetContext, HOST_REGISTER_ECC_CE_ERROR_COUNT, ECC_CE_Errors);

            payload[0] = CMC_REPORTING_ID_ECC_UE_ERROR_COUNT;
            cmcU32ToU8(ECC_UE_Errors, &payload[1]);
            payload[5] = CMC_REPORTING_ID_ECC_CE_ERROR_COUNT;
            cmcU32ToU8(ECC_CE_Errors, &payload[6]);

            pContext->ExpectedMessage = SS_EXPECTED_MSG_GOOD;
            SensorSupervisorSendSetMessage(pContext, SAT_COMMS_ECC_ERROR_REPORT, payload, 10);
        }
        pContext->CounterPCIeECC++;
        if (pContext->CounterPCIeECC == 300) pContext->CounterPCIeECC = 0;
    }
    else
    {
        SensorSupervisor_CreateEvent_E_SS_T_SENSOR_EXPIRY(pContext);
    }

    return;
}

void SensorSupervisor_HandleAction_A_SS_SEND_KEEPALIVE(LOCAL_SENSOR_SUPERVISOR_THREAD_CONTEXT_TYPE* pContext)
{
    uint8_t payload[22];
    uint8_t KeepAliveCount;
    uint16_t ShellVersion;
    
    int i;

    if (pContext->bKeepAliveSupported)
    {
        if (pContext->CounterKeepAlive == 4)
        {
            KeepAliveCount = pContext->pFN_KeepAlive_Read(pContext->pUserContext);
            ShellVersion = pContext->pFN_ShellVersion_Read(pContext->pUserContext);
            pContext->pFN_UUID_Read(pContext->pUserContext, pContext->UUID);

            payload[0] = CMC_SHELL_VERSION;
            cmcU16ToU8(ShellVersion, &payload[1]);
            payload[3] = CMC_KEEPALIVE_COUNTER;
            payload[4] = KeepAliveCount;
            payload[5] = CMC_UUID;

            for (i = 0; i < 16; i++)
            {
                payload[6 + i] = pContext->UUID[i];
            }

            pContext->ExpectedMessage = SS_EXPECTED_MSG_GOOD;
            SensorSupervisorSendSetMessage(pContext, SAT_COMMS_KEEPALIVE, payload, 22);

            pContext->CounterKeepAlive = 0;
        }
        pContext->CounterKeepAlive++;
        
    }
    else
    {
        SensorSupervisor_CreateEvent_E_SS_T_SENSOR_EXPIRY(pContext);
    }
    return;
}
void SensorSupervisor_HandleAction_A_SS_SEND_QSPI_STATUS_REQUEST(LOCAL_SENSOR_SUPERVISOR_THREAD_CONTEXT_TYPE* pContext)
{
    uint8_t GPIO_read = 0;

    

    if (pContext->bKeepAliveSupported)   // We can use this flag to tell if QSPI is supported also
    {
        // Read GPIO register if its non-zero send a QSPI read message to the SC
        GPIO_read = pContext->pFN_GPIO_Read(pContext->pUserContext);

        if (GPIO_read)
        {
            if(GPIO_read != pContext->GPIO_read_previous)
            {
                pContext->ExpectedMessage = SS_EXPECTED_MSG_INTERRUPT_STATUS_RESP;
                SensorSupervisorSendMessage(pContext, SAT_COMMS_INTERRUPT_STATUS_REQ);
            }
        }
        pContext->GPIO_read_previous = GPIO_read;
    }
    else
    {
        SensorSupervisor_CreateEvent_E_SS_T_SENSOR_EXPIRY(pContext);
    }

    return;
}


void SensorSupervisor_HandleAction_A_SS_COLLECT_CSDR(LOCAL_SENSOR_SUPERVISOR_THREAD_CONTEXT_TYPE* pContext)
{
    uint8_t payload[2];
    uint8_t NumberOfCSDRRecords;
    
    // If we need to collect CSDR drop into the if -  otherwise set the sensor expiry so we don't wait unnecessarily
    if (pContext->bCollectCSDR)
    {
        if (pContext->WaitingOnCSDRResponse == WAITING_ON_RESPONSE)
        {
            // Now done in state function
        }
        else if(pContext->WaitingOnCSDRResponse == IDLE)
        {

            if (pContext->CSDRInitialRecordNumber == CSDR_LAST_RECORD_NUMBER_1ST_PART ||
                pContext->CSDRInitialRecordNumber == CSDR_LAST_RECORD_NUMBER_2ND_PART ||
                pContext->CSDRInitialRecordNumber == CSDR_LAST_RECORD_NUMBER_3RD_PART ||
                pContext->CSDRInitialRecordNumber == CSDR_LAST_RECORD_NUMBER_4TH_PART)
            {
                NumberOfCSDRRecords = 2;
            }
            else
            {
                NumberOfCSDRRecords = 3;
            }


            payload[0] = pContext->CSDRInitialRecordNumber;
            payload[1] = NumberOfCSDRRecords;

            pContext->ExpectedMessage = SS_EXPECTED_MSG_CSDR_RESP;
            

            if (CMC_VersionSupport_CheckMessageSupport(&VersionSupportContext, SAT_COMMS_CSDR_REQ))
            {
                SensorSupervisorSendSetMessage(pContext, SAT_COMMS_CSDR_REQ, payload, 2);
                pContext->WaitingOnCSDRResponse = WAITING_ON_RESPONSE;
            }
            else
            {
                CSDRFailed(pContext);
            }

        

            // Check if we have sent all necessary SC messages (11 needed)
            if (pContext->CSDRInitialRecordNumber == CSDR_LAST_RECORD_NUMBER_1ST_PART ||
                pContext->CSDRInitialRecordNumber == CSDR_LAST_RECORD_NUMBER_2ND_PART ||
                pContext->CSDRInitialRecordNumber == CSDR_LAST_RECORD_NUMBER_3RD_PART ||
                pContext->CSDRInitialRecordNumber == CSDR_LAST_RECORD_NUMBER_4TH_PART)
            {
                pContext->CSDRInitialRecordNumber += 2;
            }
            else
            {
                pContext->CSDRInitialRecordNumber += 3;
            }
        }  
        else
        {
            // ?
        }
    }
    else
    {
        pContext->WaitingOnCSDRResponse = IDLE;
        SensorSupervisor_CreateEvent_E_SS_T_SENSOR_EXPIRY(pContext);
    }
    pContext->CSDRResponseWaitCount++;

    return;
}

void SensorSupervisor_HandleAction_A_SS_QSFP_REQUEST(LOCAL_SENSOR_SUPERVISOR_THREAD_CONTEXT_TYPE* pContext)
{
    uint8_t payload[16]; 
    uint8_t  QSFPData[4];
    uint32_t Data;
    uint8_t keep_bits = 0;
    uint8_t new_bit_0 = 0;
    uint8_t new_bit_1 = 0;
    uint8_t QSFP_Read_Value = 0;
    uint8_t new_value = 0;

    // Only one of these booleans can be set at a time
    if (pContext->bReadQSFPDiagnostics == true || pContext->bReadQSFPValidateLowSpeedIO == true || pContext->bWriteQSFPValidateLowSpeedIO == true ||
        pContext->bQSFPReadSingleByte == true || pContext->bQSFPWriteSingleByte == true)
    {
        if (pContext->bQSFPReadSingleByte == true)
        {
            if (pContext->bQSFPRequestSent != true)
            {
                pContext->ExpectedMessage = SS_EXPECTED_MSG_QSFP_READ_SINGLE_BYTE_RESP;
                Data = PERIPHERAL_REGMAP_RAM_CONTROLLER_Read(pContext->DataStoreContext.pHardwareRegisterSetContext, HOST_REGISTER_REMOTE_COMMAND_READWRITE_QSFP_WHICH_QSFP);
                cmcU32ToU8(Data, &QSFPData[0]);
                payload[0] = QSFPData[0];

                Data = PERIPHERAL_REGMAP_RAM_CONTROLLER_Read(pContext->DataStoreContext.pHardwareRegisterSetContext, HOST_REGISTER_REMOTE_COMMAND_READWRITE_QSFP_LOWER_OR_UPPER);
                cmcU32ToU8(Data, &QSFPData[0]);
                payload[1] = QSFPData[0];

                Data = PERIPHERAL_REGMAP_RAM_CONTROLLER_Read(pContext->DataStoreContext.pHardwareRegisterSetContext, HOST_REGISTER_REMOTE_COMMAND_READWRITE_QSFP_WHICH_PAGE);
                cmcU32ToU8(Data, &QSFPData[0]);
                payload[2] = QSFPData[0];

                Data = PERIPHERAL_REGMAP_RAM_CONTROLLER_Read(pContext->DataStoreContext.pHardwareRegisterSetContext, HOST_REGISTER_REMOTE_COMMAND_READWRITE_QSFP_BYTE_OFFSET);
                cmcU32ToU8(Data, &QSFPData[0]);
                payload[3] = QSFPData[0];

                if (CMC_VersionSupport_CheckMessageSupport(&VersionSupportContext, SAT_COMMS_QSFP_READ_SINGLE_BYTE_REQ))
                {
                    SensorSupervisorSendSetMessage(pContext, SAT_COMMS_QSFP_READ_SINGLE_BYTE_REQ, payload, 4);
                    pContext->bQSFPRequestSent = true;
                }
                else
                {
                    Broker_Announce_SensorSupervisor_ReadQSFPSingleByteFailed(pContext->pBrokerContext);
                }
            }
        }
        else if (pContext->bQSFPWriteSingleByte == true)
        {
            if (pContext->bQSFPRequestSent != true)
            {
                pContext->ExpectedMessage = SS_EXPECTED_MSG_GOOD;
                Data = PERIPHERAL_REGMAP_RAM_CONTROLLER_Read(pContext->DataStoreContext.pHardwareRegisterSetContext, HOST_REGISTER_REMOTE_COMMAND_READWRITE_QSFP_WHICH_QSFP);
                cmcU32ToU8(Data, &QSFPData[0]);
                payload[0] = QSFPData[0];

                Data = PERIPHERAL_REGMAP_RAM_CONTROLLER_Read(pContext->DataStoreContext.pHardwareRegisterSetContext, HOST_REGISTER_REMOTE_COMMAND_READWRITE_QSFP_LOWER_OR_UPPER);
                cmcU32ToU8(Data, &QSFPData[0]);
                payload[1] = QSFPData[0];

                Data = PERIPHERAL_REGMAP_RAM_CONTROLLER_Read(pContext->DataStoreContext.pHardwareRegisterSetContext, HOST_REGISTER_REMOTE_COMMAND_READWRITE_QSFP_WHICH_PAGE);
                cmcU32ToU8(Data, &QSFPData[0]);
                payload[2] = QSFPData[0];

                Data = PERIPHERAL_REGMAP_RAM_CONTROLLER_Read(pContext->DataStoreContext.pHardwareRegisterSetContext, HOST_REGISTER_REMOTE_COMMAND_READWRITE_QSFP_BYTE_OFFSET);
                cmcU32ToU8(Data, &QSFPData[0]);
                payload[3] = QSFPData[0];

                Data = PERIPHERAL_REGMAP_RAM_CONTROLLER_Read(pContext->DataStoreContext.pHardwareRegisterSetContext, HOST_REGISTER_REMOTE_COMMAND_READWRITE_QSFP_BYTE_VALUE);
                cmcU32ToU8(Data, &QSFPData[0]);
                payload[4] = QSFPData[0];

                if (CMC_VersionSupport_CheckMessageSupport(&VersionSupportContext, SAT_COMMS_QSFP_WRITE_SINGLE_BYTE_REQ))
                {
                    SensorSupervisorSendSetMessage(pContext, SAT_COMMS_QSFP_WRITE_SINGLE_BYTE_REQ, payload, 5);
                    pContext->bQSFPRequestSent = true;
                }
                else
                {
                    Broker_Announce_SensorSupervisor_WriteQSFPSingleByteFailed(pContext->pBrokerContext);
                }
            }
        }
        else if (pContext->bReadQSFPDiagnostics == true)
        {
            if (pContext->bQSFPRequestSent != true)           
            {
                pContext->ExpectedMessage = SS_EXPECTED_MSG_READ_QSFP_DIAGNOSTICS_RESP;
                Data = PERIPHERAL_REGMAP_RAM_CONTROLLER_Read(pContext->DataStoreContext.pHardwareRegisterSetContext, HOST_REGISTER_REMOTE_COMMAND_READWRITE_QSFP_WHICH_QSFP);
                cmcU32ToU8(Data, &QSFPData[0]);
                payload[0] = QSFPData[0];
            
                Data = PERIPHERAL_REGMAP_RAM_CONTROLLER_Read(pContext->DataStoreContext.pHardwareRegisterSetContext, HOST_REGISTER_REMOTE_COMMAND_READWRITE_QSFP_WHICH_PAGE);
                cmcU32ToU8(Data, &QSFPData[0]);
                payload[2] = QSFPData[0];
           
                Data = PERIPHERAL_REGMAP_RAM_CONTROLLER_Read(pContext->DataStoreContext.pHardwareRegisterSetContext, HOST_REGISTER_REMOTE_COMMAND_READWRITE_QSFP_LOWER_OR_UPPER);
                cmcU32ToU8(Data, &QSFPData[0]);
                payload[1] = QSFPData[0];

                if (CMC_VersionSupport_CheckMessageSupport(&VersionSupportContext, SAT_COMMS_READ_QSFP_DIAGNOSTICS_REQ))
                {
                    SensorSupervisorSendSetMessage(pContext, SAT_COMMS_READ_QSFP_DIAGNOSTICS_REQ, payload, 3);
                    pContext->bQSFPRequestSent = true;
                }
                else
                {
                    Broker_Announce_SensorSupervisor_ReadQSFPDiagnosticsFailed(pContext->pBrokerContext);
                }
            }
        }
        else if(pContext->bReadQSFPValidateLowSpeedIO ==true)
        {
            if (pContext->bQSFPRequestSent != true)
            {
                pContext->ExpectedMessage = SS_EXPECTED_MSG_QSFP_VALIDATE_LOW_SPEED_READ_IO_RESP;
                Data = PERIPHERAL_REGMAP_RAM_CONTROLLER_Read(pContext->DataStoreContext.pHardwareRegisterSetContext, HOST_REGISTER_REMOTE_COMMAND_READWRITE_QSFP_WHICH_QSFP);
                cmcU32ToU8(Data, &QSFPData[0]);
                payload[0] = QSFPData[0];

               if(pContext->DataStoreContext.pFirmwareVersionContext->Version.Minor == 0x0C ||                                            // VCK5000 R5
                  pContext->DataStoreContext.pFirmwareVersionContext->Version.Minor == 0x02 ||                                            // U200 or U250
                  (pContext->DataStoreContext.pFirmwareVersionContext->Version.Minor == 0x08 && pContext->bQSFPReadWriteGPIO == true))    // CMS
                {    
                    // If U200 or U250 intercept the request to the SC and read directly from the GPIO
                    /* Using new profiled QSFP read */
                    /* New profiled QSFP read */
                    pContext->pFN_QSFP_GPIO_Read(pContext->pUserContext, (uint32_t)payload[0], &QSFP_Read_Value);
                    //PERIPHERAL_AXI_GPIO_QSFP_Read_GPIO_for_QSFP(&AXI_GPIO_QSFP_Context, (uint32_t)payload[0], &QSFP_Read_Value);

                    // Swap bits 0 & 1
                    keep_bits = QSFP_Read_Value & 0xFC;
                    new_bit_0 = (QSFP_Read_Value >> 1) & 0x01;
                    new_bit_1 = (QSFP_Read_Value & 0x01) << 1;

                    pContext->bQSFPRequestSent = true;
                    PERIPHERAL_REGMAP_RAM_CONTROLLER_Write(pContext->DataStoreContext.pHardwareRegisterSetContext, HOST_REGISTER_REMOTE_COMMAND_READWRITE_QSFP_VALIDATE_LOW_SPEED_IO_DATA, (uint32_t)(keep_bits | new_bit_0 | new_bit_1));
                    Broker_Announce_SensorSupervisor_ReadQSFPValidateLowSpeedIOComplete(pContext->pBrokerContext);
                }
                else
                {
                    if (CMC_VersionSupport_CheckMessageSupport(&VersionSupportContext, SAT_COMMS_QSFP_VALIDATE_LOW_SPEED_READ_IO_REQ))
                    {
                        SensorSupervisorSendSetMessage(pContext, SAT_COMMS_QSFP_VALIDATE_LOW_SPEED_READ_IO_REQ, payload, 1);
                        pContext->bQSFPRequestSent = true;
                    }
                    else
                    {
                        Broker_Announce_SensorSupervisor_ReadQSFPValidateLowSpeedIOFailed(pContext->pBrokerContext);
                    }
                }
            }
        }
        else if (pContext->bWriteQSFPValidateLowSpeedIO == true)
        {
            if (pContext->bQSFPRequestSent != true)
            {
                Data = PERIPHERAL_REGMAP_RAM_CONTROLLER_Read(pContext->DataStoreContext.pHardwareRegisterSetContext, HOST_REGISTER_REMOTE_COMMAND_READWRITE_QSFP_WHICH_QSFP);
                cmcU32ToU8(Data, &QSFPData[0]);
                payload[0] = QSFPData[0];

                Data = PERIPHERAL_REGMAP_RAM_CONTROLLER_Read(pContext->DataStoreContext.pHardwareRegisterSetContext, HOST_REGISTER_REMOTE_COMMAND_READWRITE_QSFP_VALIDATE_LOW_SPEED_IO_DATA);
                cmcU32ToU8(Data, &QSFPData[0]);
                payload[1] = QSFPData[0];

                // If U200 or U250 intercept the request to the SC and write directly to the GPIO
                if (pContext->DataStoreContext.pFirmwareVersionContext->Version.Minor == 0x0C ||                                            // VCK5000 R5
                    pContext->DataStoreContext.pFirmwareVersionContext->Version.Minor == 0x02 ||                                            // U200 or U250
                    (pContext->DataStoreContext.pFirmwareVersionContext->Version.Minor == 0x08 && pContext->bQSFPReadWriteGPIO == true))    // CMS
                {
                    // Swap bits 0 & 1
                    keep_bits = payload[1] & 0xFC;
                    new_bit_0 = (payload[1] >> 1) & 0x01;
                    new_bit_1 = (payload[1] & 0x01) << 1;

                    new_value = (keep_bits | new_bit_0 | new_bit_1);

                    if (payload[1] & 0x20)
                    {
                        // If bit 5 is set -  hold the QSFP in reset
                        /* New profiled QSFP read */       
                        pContext->pFN_QSFP_GPIO_Write(pContext->pUserContext, (uint32_t)payload[0], (new_value & 0xFD));
                        pContext->bQSFPRequestSent = true;
                        Broker_Announce_SensorSupervisor_WriteQSFPValidateLowSpeedIOComplete(pContext->pBrokerContext);
                    }
                    else
                    {
                        if (pContext->bFirstWrite)
                        {
                            pContext->pFN_QSFP_GPIO_Write(pContext->pUserContext, (uint32_t)payload[0], new_value);
                            pContext->bFirstWrite = false;
                        }
                        else
                        {
                            // To be consistent with other cards setting of RESET_L to zero should initate a toggle of the RESET_L GPIO pin
                            pContext->pFN_QSFP_GPIO_Write(pContext->pUserContext, (uint32_t)payload[0], (new_value | 0x2));
                            pContext->bQSFPRequestSent = true;
                            Broker_Announce_SensorSupervisor_WriteQSFPValidateLowSpeedIOComplete(pContext->pBrokerContext);
                            pContext->bFirstWrite = true;
                        }
                    }
                }
                else
                {
                    if (CMC_VersionSupport_CheckMessageSupport(&VersionSupportContext, SAT_COMMS_QSFP_VALIDATE_LOW_SPEED_WRITE_IO_REQ))
                    {
                        SensorSupervisorSendSetMessage(pContext, SAT_COMMS_QSFP_VALIDATE_LOW_SPEED_WRITE_IO_REQ, payload, 2);
                        pContext->bQSFPRequestSent = true;
                    }
                    else
                    {
                        Broker_Announce_SensorSupervisor_WriteQSFPValidateLowSpeedIOFailed(pContext->pBrokerContext);
                    }
                }
            }
        }

        pContext->QSFPResponseWaitCount++;
    }
    else
    {
        SensorSupervisor_CreateEvent_E_SS_T_SENSOR_EXPIRY(pContext);
    }
}


void SensorSupervisor_HandleAction_A_SS_RELAY_HBM_SENSOR_INFORMATION(LOCAL_SENSOR_SUPERVISOR_THREAD_CONTEXT_TYPE *pContext)        
{
    uint8_t hbmTemp1, hbmTemp2, payload[4];
    uint32_t hbmTemps1and2;

    if (pContext->HBMSupported)
    {
        Watch_Inc(pContext->pWatchPointContext, W_HBM_SUPPORTED);
        // read HBM temperature GPIO to acquire HBM_TEMP1 & HBM_TEMP2 readings
        PERIPHERAL_AXI_GPIO_HBM_Read_GPIO_for_HBM(&AXI_GPIO_HBM_Context, &hbmTemps1and2);
        hbmTemp1 = (uint8_t)hbmTemps1and2;
        hbmTemp2 = (uint8_t)(hbmTemps1and2 >> 8);

        // send HBM_TEMP1 & HBM_TEMP2 temperature to SC in SAT_COMMS_SNSR_PUSH message container
        payload[0] = SNSR_ID_HBM_TEMP1;
        payload[1] = hbmTemp1;
        payload[2] = SNSR_ID_HBM_TEMP2;
        payload[3] = hbmTemp2;

        SensorSupervisorSendSetMessage(pContext, SAT_COMMS_SNSR_PUSH, payload, 4);

        Watch_Set(pContext->pWatchPointContext, W_HBM_PAYLOAD, (cmcU8ToU32(payload)));
    }
    return;
}


void SensorSupervisor_HandleAction_A_SS_INITIALIZE_CAGE_SENSOR_INFORMATION(LOCAL_SENSOR_SUPERVISOR_THREAD_CONTEXT_TYPE* pContext)
{
	uint8_t QSFPCage, payload[2];
	uint32_t iQSFP;

	for (iQSFP = 0; iQSFP < AXI_GPIO_QSFP_Context.MaxCages; iQSFP++)
	{
		// read cage GPIO and send to satellite controller
        if (pContext->DataStoreContext.pFirmwareVersionContext->Version.Minor == 0x0C ||                                            // VCK5000 R5
            pContext->DataStoreContext.pFirmwareVersionContext->Version.Minor == 0x02 ||                                            // U200 or U250
            (pContext->DataStoreContext.pFirmwareVersionContext->Version.Minor == 0x08 && pContext->bQSFPReadWriteGPIO == true))    // CMS
        {
            pContext->pFN_QSFP_GPIO_Read(pContext->pUserContext, (uint8_t)iQSFP, &QSFPCage);
            payload[0] = (uint8_t)iQSFP;
            payload[1] = QSFPCage;
            SensorSupervisorSendSetMessage(pContext, SAT_COMMS_CAGE_IO_EVENT, payload, 2);
        }
	}
}


void SensorSupervisor_HandleAction_A_SS_RELAY_CAGE_SENSOR_INFORMATION(LOCAL_SENSOR_SUPERVISOR_THREAD_CONTEXT_TYPE* pContext)
{
	uint8_t QSFPCage, payload[2];

	// read cage GPIO and send to satellite controller
    if (pContext->DataStoreContext.pFirmwareVersionContext->Version.Minor == 0x0C ||                                            // VCK5000 R5
        pContext->DataStoreContext.pFirmwareVersionContext->Version.Minor == 0x02 ||                                            // U200 or U250
        (pContext->DataStoreContext.pFirmwareVersionContext->Version.Minor == 0x08 && pContext->bQSFPReadWriteGPIO == true))    // CMS
    {
        pContext->pFN_QSFP_GPIO_Read(pContext->pUserContext, pContext->iQSFP, &QSFPCage);
        payload[0] = (uint8_t)pContext->iQSFP;
        payload[1] = QSFPCage;
        SensorSupervisorSendSetMessage(pContext, SAT_COMMS_CAGE_IO_EVENT, payload, 2);
    }
		pContext->iQSFP++;
}


void SensorSupervisor_HandleAction_A_SS_T_SENSOR_START(LOCAL_SENSOR_SUPERVISOR_THREAD_CONTEXT_TYPE *pContext)                  
{
    pContext->CurrentTimerInstance=TimerThread_Start(   pContext->pThreadTimerContext,
                                                                T_SENSOR,
                                                                pContext->SensorSupervisorTimeout,
                                                                pContext,
                                                                LocalSensorSupervisor_SensorTimerCallback);
}

void SensorSupervisor_HandleAction_A_SS_T_SENSOR_RESTART(LOCAL_SENSOR_SUPERVISOR_THREAD_CONTEXT_TYPE *pContext)                      
{
    pContext->CurrentTimerInstance=TimerThread_Restart(pContext->pThreadTimerContext, T_SENSOR);
}


void SensorSupervisor_HandleAction_A_SS_T_SENSOR_STOP(LOCAL_SENSOR_SUPERVISOR_THREAD_CONTEXT_TYPE *pContext)
{
    TimerThread_Stop(pContext->pThreadTimerContext, T_SENSOR);
}



void SensorSupervisor_SeedAction(LOCAL_SENSOR_SUPERVISOR_THREAD_CONTEXT_TYPE *pContext)
{
    SensorSupervisor_HandleAction_A_SS_T_SENSOR_START(pContext);
}


void SensorSupervisor_HandleAction_A_SS_PROCESS_MESSAGE(LOCAL_SENSOR_SUPERVISOR_THREAD_CONTEXT_TYPE* pContext)
{
    if (SensorRelay_TryRemove(pContext->pSensorSupervisorRelayBufferContext, &(pContext->LogElement)))
    {
        switch ((uint8_t)pContext->LogElement.Buffer[0])
        {
        case SAT_COMMS_VOLT_SNSR_RESP:
            checkExpectedMessage(pContext, SS_EXPECTED_MSG_VOLT_SNSR_RESP);
            DataStore_UpdateSensors(&(pContext->DataStoreContext),
                &(pContext->LogElement.Buffer[3]),
                pContext->LogElement.Buffer[2],
                SAT_COMMS_VOLT_SNSR_RESP,
                pContext->DataStoreContext.CMCValidSensors_0_to_63,
                pContext->DataStoreContext.CMCValidSensors_64_to_127,
                pContext->DataStoreContext.bCardSupportsScalingFactor,
                pContext->pVersionSupportContext->commsVersion);
            break;

        case SAT_COMMS_POWER_SNSR_RESP:
            checkExpectedMessage(pContext, SS_EXPECTED_MSG_POWER_SNSR_RESP);
            DataStore_UpdateSensors(&(pContext->DataStoreContext),
                &(pContext->LogElement.Buffer[3]),
                pContext->LogElement.Buffer[2],
                SAT_COMMS_POWER_SNSR_RESP,
                pContext->DataStoreContext.CMCValidSensors_0_to_63,
                pContext->DataStoreContext.CMCValidSensors_64_to_127,
                pContext->DataStoreContext.bCardSupportsScalingFactor,
                pContext->pVersionSupportContext->commsVersion);
            break;

        case SAT_COMMS_TEMP_SNSR_RESP:
            checkExpectedMessage(pContext, SS_EXPECTED_MSG_TEMP_SNSR_RESP);
            DataStore_UpdateSensors(&(pContext->DataStoreContext),
                &(pContext->LogElement.Buffer[3]),
                pContext->LogElement.Buffer[2],
                SAT_COMMS_TEMP_SNSR_RESP,
                pContext->DataStoreContext.CMCValidSensors_0_to_63,
                pContext->DataStoreContext.CMCValidSensors_64_to_127,
                pContext->DataStoreContext.bCardSupportsScalingFactor,
                pContext->pVersionSupportContext->commsVersion);
            break;

        case SAT_COMMS_SNSR_STATE_RESP:
            checkExpectedMessage(pContext, SS_EXPECTED_MSG_SNSR_STATE_RESP);
            DataStore_UpdateSensorState(&(pContext->DataStoreContext),
                &(pContext->LogElement.Buffer[3]),
                pContext->LogElement.Buffer[2]);
            break;

        case SAT_COMMS_BOARD_INFO_RESP:
            checkExpectedMessage(pContext, SS_EXPECTED_MSG_BOARD_INFO_RESP);
            DataStore_UpdateBoardInfo(&(pContext->DataStoreContext),
                (uint8_t*) &(pContext->LogElement.Buffer[3]),
                pContext->LogElement.Buffer[2]);
            Broker_Announce_SensorSupervisor_BoardInfoComplete(pContext->pBrokerContext);
            break;

        case SAT_COMMS_SEND_OEM_CMD_RESP:
            checkExpectedMessage(pContext, SS_EXPECTED_MSG_SEND_OEM_CMD_RESP);
            DataStore_UpdateOEM(&(pContext->DataStoreContext),
                &(pContext->LogElement.Buffer[3]),
                pContext->LogElement.Buffer[2]);
            break;

        case SAT_COMMS_ALERT_RESP:
            checkExpectedMessage(pContext, SS_EXPECTED_MSG_ALERT_RESP);
            DataStore_UpdateAlertResponse(&(pContext->DataStoreContext),
                &(pContext->LogElement.Buffer[3]),
                pContext->LogElement.Buffer[2]);
            break;

        case SAT_COMMS_SNSR_POWER_THROTTLING_THRESHOLDS_RESP:
            checkExpectedMessage(pContext, SS_EXPECTED_MSG_POWER_THROTTLING_THRESHOLDS_RESP);
            DataStore_UpdatePowerThresholdResponse(&(pContext->DataStoreContext),
                &(pContext->LogElement.Buffer[3]),
                pContext->LogElement.Buffer[2],
                pContext->pVersionSupportContext->commsVersion);
            break;

        case SAT_COMMS_SNSR_TEMP_THROTTLING_THRESHOLDS_RESP:
            checkExpectedMessage(pContext, SS_EXPECTED_MSG_TEMP_THROTTLING_THRESHOLDS_RESP);
            DataStore_UpdateTempThresholdResponse(&(pContext->DataStoreContext),
                &(pContext->LogElement.Buffer[3]),
                pContext->LogElement.Buffer[2]);
            break;

        case SAT_COMMS_CSDR_RESP:
            checkExpectedMessage(pContext, SS_EXPECTED_MSG_CSDR_RESP);        
            storeCSDRdata(pContext, &(pContext->LogElement.Buffer[3]), pContext->LogElement.Buffer[2]);
            break;

        case SAT_COMMS_READ_QSFP_DIAGNOSTICS_RESP:
            checkExpectedMessage(pContext, SS_EXPECTED_MSG_READ_QSFP_DIAGNOSTICS_RESP);
            storeQSFPdata(pContext, &(pContext->LogElement.Buffer[3]), pContext->LogElement.Buffer[2]);            
            Broker_Announce_SensorSupervisor_ReadQSFPDiagnosticsComplete(pContext->pBrokerContext);
            break;

        case SAT_COMMS_QSFP_READ_SINGLE_BYTE_RESP:
            checkExpectedMessage(pContext, SS_EXPECTED_MSG_QSFP_READ_SINGLE_BYTE_RESP);
            // Response is a single byte
            PERIPHERAL_REGMAP_RAM_CONTROLLER_Write(pContext->DataStoreContext.pHardwareRegisterSetContext, HOST_REGISTER_REMOTE_COMMAND_READWRITE_QSFP_BYTE_VALUE, ((uint32_t)pContext->LogElement.Buffer[3]) & 0x000000FF);
            Broker_Announce_SensorSupervisor_ReadQSFPSingleByteComplete(pContext->pBrokerContext);
            break;
       
        case SAT_COMMS_QSFP_VALIDATE_LOW_SPEED_READ_IO_RESP:
            
            checkExpectedMessage(pContext, SS_EXPECTED_MSG_QSFP_VALIDATE_LOW_SPEED_READ_IO_RESP);
            // Response is a single byte
            PERIPHERAL_REGMAP_RAM_CONTROLLER_Write(pContext->DataStoreContext.pHardwareRegisterSetContext, HOST_REGISTER_REMOTE_COMMAND_READWRITE_QSFP_VALIDATE_LOW_SPEED_IO_DATA, (uint32_t)pContext->LogElement.Buffer[3]);
            Broker_Announce_SensorSupervisor_ReadQSFPValidateLowSpeedIOComplete(pContext->pBrokerContext);
            break;

        case SAT_COMMS_MSG_GOOD:
            switch ((uint32_t)pContext->LogElement.Buffer[3])
            {
            case SAT_COMMS_QSFP_VALIDATE_LOW_SPEED_WRITE_IO_REQ:
                Broker_Announce_SensorSupervisor_WriteQSFPValidateLowSpeedIOComplete(pContext->pBrokerContext);
                break;

            case SAT_COMMS_QSFP_WRITE_SINGLE_BYTE_REQ:
                Broker_Announce_SensorSupervisor_WriteQSFPSingleByteComplete(pContext->pBrokerContext);
                break;

            case SAT_COMMS_PCIE_ERROR_REPORT:
            case SAT_COMMS_ECC_ERROR_REPORT:
            case SAT_COMMS_KEEPALIVE:
                checkExpectedMessage(pContext, SS_EXPECTED_MSG_GOOD);
                break;
            default:
                break;
            }
            break;

        case SAT_COMMS_MSG_ERR:
            if ((uint32_t)pContext->LogElement.Buffer[4] == SAT_COMMS_QSFP_VALIDATE_LOW_SPEED_WRITE_IO_REQ)
            {
                Broker_Announce_SensorSupervisor_WriteQSFPValidateLowSpeedIOFailed(pContext->pBrokerContext);
            }
            if ((uint32_t)pContext->LogElement.Buffer[4] == SAT_COMMS_READ_QSFP_DIAGNOSTICS_REQ)
            {
                Broker_Announce_SensorSupervisor_ReadQSFPDiagnosticsFailed(pContext->pBrokerContext);
            }
            if ((uint32_t)pContext->LogElement.Buffer[4] == SAT_COMMS_QSFP_VALIDATE_LOW_SPEED_READ_IO_REQ)
            {
                Broker_Announce_SensorSupervisor_ReadQSFPValidateLowSpeedIOFailed(pContext->pBrokerContext);
            }
            if ((uint32_t)pContext->LogElement.Buffer[4] == SAT_COMMS_QSFP_WRITE_SINGLE_BYTE_REQ)
            {
                Broker_Announce_SensorSupervisor_WriteQSFPSingleByteFailed(pContext->pBrokerContext);
            }
            if ((uint32_t)pContext->LogElement.Buffer[4] == SAT_COMMS_QSFP_READ_SINGLE_BYTE_REQ)
            {
                Broker_Announce_SensorSupervisor_ReadQSFPSingleByteFailed(pContext->pBrokerContext);
            }
            if ((uint32_t)pContext->LogElement.Buffer[4] == SAT_COMMS_CSDR_REQ)
            {
                Broker_Announce_UserProxyCSDRFailed(pContext->pBrokerContext);
            }
            if ((uint32_t)pContext->LogElement.Buffer[4] == SAT_COMMS_BOARD_INFO_REQ)
            {
                Broker_Announce_SensorSupervisor_BoardInfoFailed(pContext->pBrokerContext);
            }
            break;

        case SAT_COMMS_QSFP_VALIDATE_LOW_SPEED_READ_IO_REQ:
            break;

        case SAT_COMMS_INTERRUPT_STATUS_RESP:
            checkExpectedMessage(pContext, SS_EXPECTED_MSG_SEND_OEM_CMD_RESP);
            DataStore_Update_Interrupt_Status(&(pContext->DataStoreContext),
                &(pContext->LogElement.Buffer[3]),
                pContext->LogElement.Buffer[2]);
            break;

        default:
            break;

        }
    }
}

void SensorSupervisor_HandleAction_A_SS_KICK_CLOCK_THROTTLING(LOCAL_SENSOR_SUPERVISOR_THREAD_CONTEXT_TYPE* pContext)
{
    UNUSED(pContext);
    cmc_clock_throttling_phase1_preprocessing(&ClockThrottlingAlgorithmContext);
    cmc_clock_throttling_phase2_preprocessing(&ClockThrottlingAlgorithmContext);
}

void SensorSupervisor_HandleAction(LOCAL_SENSOR_SUPERVISOR_THREAD_CONTEXT_TYPE *pContext)
{
    uint32_t i;

    for(i=0;i<MAX_SS_ACTIONS; i++)
    {
        switch((1<<i)&(pContext->Action))
        {

        case    A_SS_NO_ACTION:
                /* Ignore */
                break;

        case    A_SS_REQUEST_LINK_STATE:
                SensorSupervisor_HandleAction_A_SS_REQUEST_LINK_STATE(pContext);
                break;                    

        case    A_SS_REQUEST_BOARD_INFO:                    
                SensorSupervisor_HandleAction_A_SS_REQUEST_BOARD_INFO(pContext);
                break;                    

        case    A_SS_REQUEST_VOLTAGE_SENSOR_INFORMATION:    
                SensorSupervisor_HandleAction_A_SS_REQUEST_VOLTAGE_SENSOR_INFORMATION(pContext);
                break;                    

        case    A_SS_REQUEST_POWER_SENSOR_INFORMATION:      
                SensorSupervisor_HandleAction_A_SS_REQUEST_POWER_SENSOR_INFORMATION(pContext);
                break;                    

        case    A_SS_REQUEST_TEMPERATURE_SENSOR_INFORMATION:
                SensorSupervisor_HandleAction_A_SS_REQUEST_TEMPERATURE_SENSOR_INFORMATION(pContext);
                break;         

        case    A_SS_FETCHING_POWER_THROTTLING_THRESHOLDS:
                SensorSupervisor_HandleAction_A_SS_FETCHING_POWER_THROTTLING_THRESHOLDS(pContext);
                break;

        case    A_SS_FETCHING_TEMP_THROTTLING_THRESHOLDS:
                SensorSupervisor_HandleAction_A_SS_FETCHING_TEMP_THROTTLING_THRESHOLDS(pContext);
                break;

        case    A_SS_RELAY_HBM_SENSOR_INFORMATION:          
                SensorSupervisor_HandleAction_A_SS_RELAY_HBM_SENSOR_INFORMATION(pContext);
                break;                    

        case    A_SS_RELAY_CAGE_SENSOR_INFORMATION:         
                SensorSupervisor_HandleAction_A_SS_RELAY_CAGE_SENSOR_INFORMATION(pContext);
                break;                    

        case    A_SS_T_SENSOR_START:                        
                SensorSupervisor_HandleAction_A_SS_T_SENSOR_START(pContext);
                break;                    

        case    A_SS_T_SENSOR_RESTART:                      
                SensorSupervisor_HandleAction_A_SS_T_SENSOR_RESTART(pContext);
                break;                    

        case    A_SS_T_SENSOR_STOP:                         
                SensorSupervisor_HandleAction_A_SS_T_SENSOR_STOP(pContext);
                break;                    

        case    A_SS_PROCESS_MESSAGE:
                SensorSupervisor_HandleAction_A_SS_PROCESS_MESSAGE(pContext);
                break;

		case    A_SS_INITIALIZE_CAGE_SENSOR_INFORMATION:
				SensorSupervisor_HandleAction_A_SS_INITIALIZE_CAGE_SENSOR_INFORMATION(pContext);
				break;

		case    A_SS_SEND_ALERT_REQUEST:
				SensorSupervisor_HandleAction_A_SS_SEND_ALERT_REQUEST(pContext);
				break;

		case    A_SS_SEND_SENSOR_STATE_REQUEST:
				SensorSupervisor_HandleAction_A_SS_SEND_SENSOR_STATE_REQUEST(pContext);
				break;
				
		case    A_SS_SEND_OEM_CMD_REQUEST:
			    SensorSupervisor_HandleAction_A_SS_SEND_OEM_CMD_REQUEST(pContext);
			    break;

        case    A_SS_KICK_CLOCK_THROTTLING:
                SensorSupervisor_HandleAction_A_SS_KICK_CLOCK_THROTTLING(pContext);
                break;

        case    A_SS_DEBUG_UART_ENABLE:
                SensorSupervisor_HandleAction_A_SS_SEND_ENABLE_UART_REQUEST(pContext);
                break;
        
        case    A_SS_SEND_PCIE_ERRORS:
                SensorSupervisor_HandleAction_A_SS_SEND_PCIE_ERRORS(pContext);
                break;

        case    A_SS_SEND_ECC_ERRORS:
                SensorSupervisor_HandleAction_A_SS_SEND_ECC_ERRORS(pContext);
                break;

        case    A_SS_SEND_KEEPALIVE:
                SensorSupervisor_HandleAction_A_SS_SEND_KEEPALIVE(pContext);
                break;

        case    A_SS_COLLECT_CSDR:
                SensorSupervisor_HandleAction_A_SS_COLLECT_CSDR(pContext);
                break;

        case    A_SS_QSFP_REQUEST:
                SensorSupervisor_HandleAction_A_SS_QSFP_REQUEST(pContext);
                break;

        case    A_SS_INTERRUPT_STATUS_REQUEST:
                SensorSupervisor_HandleAction_A_SS_SEND_QSPI_STATUS_REQUEST(pContext);
                break;
                
         default:
                Watch_Inc(pContext->pWatchPointContext, W_SENSOR_SUPERVISOR_UNEXPECTED_ACTION);
                break;
        }
    }
}
