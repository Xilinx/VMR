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
 *  $Change: 3217619 $
 *  $Date: 2021/05/13 $
 *  $Revision: #76 $
 *
 */




#include "cmc_link_receive_thread.h"
#include "cmc_receive_defines.h"
#include "cmc_transmit_defines.h"

void cmcDumpStartupDebugBuffer(LINK_RECEIVE_THREAD_CONTEXT_TYPE* pContext)
{
    uint32_t* pWord;
    uint32_t RegOffset = CMC_DEBUG_BUFFER_REG;
    int i;

    for (i = 0; i < DEBUG_BUFFER_SIZE; i += 4)
    {
        pWord = (uint32_t*) & (pContext->StartupDebugBuffer[i]);
        PERIPHERAL_REGMAP_RAM_CONTROLLER_Write(pContext->pLocalSensorSupervisorContext->DataStoreContext.pHardwareRegisterSetContext, RegOffset, *pWord);
        RegOffset += 4;
    }

    pContext->bPacketCaptureEnabled = false;
}



void cmcDumpDebugBuffer(LINK_RECEIVE_THREAD_CONTEXT_TYPE* pContext)
{
    uint32_t* pWord;
    uint32_t RegOffset = CMC_DEBUG_BUFFER_REG;
    int i;

    for (i = 0; i < DEBUG_BUFFER_SIZE; i += 4)
    {
        pWord = (uint32_t*) & (pContext->DebugBuffer[i]);
        PERIPHERAL_REGMAP_RAM_CONTROLLER_Write(pContext->pLocalSensorSupervisorContext->DataStoreContext.pHardwareRegisterSetContext, RegOffset, *pWord);
        RegOffset += 4;
    }

    Watch_Set(pContext->pWatchPointContext, W_LINK_RECEIVE_DEBUG_CAPTURE_POSITION, (uint32_t)pContext->DebugBufferPosition);

    pContext->bPacketCaptureEnabled = false;
}

static void LinkReceiveThread_SensorSupervisorPacketCaptureTest(LINK_RECEIVE_THREAD_CONTEXT_TYPE* pContext)
{
	if (pContext->bPacketCaptureEnabled)
	{
		if (pContext->PacketCaptureMsgID == pContext->msgIDReceived)
		{
            cmcDumpDebugBuffer(pContext);
		}
	}
}

static void LinkReceiveThread_BootloaderPacketCaptureTest(LINK_RECEIVE_THREAD_CONTEXT_TYPE* pContext)
{
	if (pContext->bPacketCaptureEnabled)
	{
		if (pContext->PacketCaptureMsgID == pContext->BootloaderParserContext->rxBootloaderMsgIDReceived)
		{
            cmcDumpDebugBuffer(pContext);
		}
	}
}


void LinkReceiveThread_Packet_Capture_Enable(LINK_RECEIVE_THREAD_CONTEXT_TYPE* pContext)
{
	pContext->bPacketCaptureEnabled = true;
}

void LinkReceiveThread_Store_Debug_Capture_MsgID(LINK_RECEIVE_THREAD_CONTEXT_TYPE* pContext, uint8_t MsgID)
{
	pContext->PacketCaptureMsgID = MsgID;
}

void LinkReceiveThread_Packet_Capture_Disable(LINK_RECEIVE_THREAD_CONTEXT_TYPE* pContext)
{
	pContext->bPacketCaptureEnabled = false;
}

bool DoChecksumsMatch(LINK_RECEIVE_THREAD_CONTEXT_TYPE* pContext)
{
	uint16_t receivedChecksum;
	uint16_t rxBootloaderMsg = (uint8_t)pContext->BootloaderParserContext->rxBootloaderMsg[6];
	rxBootloaderMsg = rxBootloaderMsg << 8;
	receivedChecksum = rxBootloaderMsg | (uint8_t)pContext->BootloaderParserContext->rxBootloaderMsg[5];

	if (pContext->pBrokerContext->pBootloaderThreadContext->calculatedChecksum == receivedChecksum)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void LinkReceiveThread_LinkProtocolArbiter(LINK_RECEIVE_THREAD_CONTEXT_TYPE *pContext, char AnyOctet)
{
	bool messageParsed = false;
	bool messageDeframed = false;
	uint8_t minVersion, maxVersion;
	char *message = NULL;
	LINK_PROTOCOL_TRACE_ELEMENT_TYPE LinkProtocolTraceLogElement;
	uint8_t msgID;
	BOOTLOADER_PARSER_RESULT_TYPE msgResult;
    LINK_STATE_TYPE CurrentState;

    CurrentState = LinkState_FSM_GetState(pContext->pLinkStateContext);

	Watch_Inc(pContext->pWatchPointContext, W_SUC_BYTES_RX);

    // Constantly log raw received characters
    pContext->DebugBuffer[pContext->DebugBufferPosition++] = AnyOctet;
    if(pContext->DebugBufferPosition >= DEBUG_BUFFER_SIZE) pContext->DebugBufferPosition = 0;
    
    if(CurrentState != S_LINK_USER_IS_REMOTE_BOOTLOADER)
    {
        // Constantly log raw received characters
        if (pContext->StartupLogBufferPosition < DEBUG_BUFFER_SIZE)
        {
            pContext->StartupDebugBuffer[pContext->StartupLogBufferPosition++] = AnyOctet;
            pContext->StartupDebugBuffer[pContext->StartupLogBufferPosition++] = pContext->pBrokerContext->pBootloaderThreadContext->lastCommandSent;

            cmcDumpStartupDebugBuffer(pContext);
        }
    }

    switch(CurrentState)
    {
        case    S_INITIAL:
                /* Drop Octet - no more work to do */
                break;

        case    S_DETERMINE_LINK_USER:
                /* Pass Octet to Bootloader Parser */
			messageParsed = PROTOCOL_BOOTLOADER_ProcessMessage(pContext->pProtocolBootloader, AnyOctet, pContext->pBrokerContext->pBootloaderThreadContext->lastCommandSent, &msgID, &msgResult);
				if (messageParsed)
				{                 
                    LinkReceiveThread_BootloaderPacketCaptureTest(pContext);

					/* Trace */
					LinkProtocolTraceLogElement.MsgID = pContext->BootloaderParserContext->rxBootloaderMsgIDReceived;
					LinkProtocolTraceLogElement.FSMCurrentState = 0;
					LinkProtocolTrace_TryAdd(pContext->LinkProtocolTraceContext, &LinkProtocolTraceLogElement);
					
                    // In response to a BSL_SYNC we may get BSL response 00 80 02 00 3b 07 87 b4   
                    // The 07 is UNKNOWN_COMMAND but that does still tell us that the SC is in BSL mode
                    // 04  BSL_LOCKED is another possibility  00 80 02 00 3b 04 e4 84
                    // 05 PASSWORD_ERROR also 
                    if (pContext->BootloaderParserContext->parserResult == BOOTLOADER_RESULT_BSL_MODE_ACK_RECEIVED ||
                        pContext->BootloaderParserContext->parserResult == BOOTLOADER_RESULT_BSL_MODE_UNKNOWN_COMMAND ||
                        pContext->BootloaderParserContext->parserResult == BOOTLOADER_RESULT_BSL_MODE_BSL_LOCKED ||
                        pContext->BootloaderParserContext->parserResult == BOOTLOADER_RESULT_BSL_MODE_PASSWORD_ERROR)
					{
						LinkState_CreateEvent_E_LS_LINK_USER_IS_REMOTE_BOOTLOADER(pContext->pLinkStateContext);
					}
					else if ((pContext->BootloaderParserContext->parserResult == BOOTLOADER_RESULT_NORMAL_MODE_ACK_RECEIVED) ||
							 (pContext->BootloaderParserContext->parserResult == BOOTLOADER_RESULT_NORMAL_MODE_NACK_RECEIVED))
					{
						LinkState_CreateEvent_E_LS_LINK_USER_IS_REMOTE_SENSOR_SUPERVISOR(pContext->pLinkStateContext);
					}
					else
					{
						Watch_Inc(pContext->pWatchPointContext, W_LINK_RECEIVE_MSG_RECEIVED_UNKNOWN_MSG_ID); // TODO Add a new watchpoint to log errors
					}
					PROTOCOL_BOOTLOADER_Initialize(pContext->pProtocolBootloader);
				}
                break;

        case    S_LINK_USER_IS_REMOTE_SENSOR_SUPERVISOR:
		case    S_LINK_USER_IS_REMOTE_SENSOR_SUPERVISOR_SUC:
			/* Pass Octet to Sensor Supervisor Parser */
			messageDeframed = TRANSPORT_SENSOR_DeframeTryAdd(pContext->pTransport, AnyOctet);
			if (messageDeframed)
			{
				message = TRANSPORT_SENSOR_DeframeGetMessage(pContext->pTransport);

				if (!PROTOCOL_SENSOR_ProcessMessage(pContext->pProtocol, message, &pContext->payloadLength, &pContext->msgIDReceived))
				{
					/* Trace */
					LinkProtocolTraceLogElement.MsgID = pContext->msgIDReceived;
					LinkProtocolTraceLogElement.FSMCurrentState = pContext->pLocalSensorSupervisorContext->State;
					LinkProtocolTraceLogElement.FSMPrevState = pContext->pLocalSensorSupervisorContext->PreviousState;
					LinkProtocolTrace_TryAdd(pContext->LinkProtocolTraceContext, &LinkProtocolTraceLogElement);

					/* Increment the message coverage counter */
					cmcIncrementReg(pContext->pLocalSensorSupervisorContext->DataStoreContext.pHardwareRegisterSetContext, CMC_SC_MSG_COVERAGE_REG + (pContext->msgIDReceived << 2));

					LinkReceiveThread_SensorSupervisorPacketCaptureTest(pContext);

					switch (pContext->msgIDReceived)
					{
						case SAT_COMMS_MSG_GOOD:
							if (message[3] == SAT_COMMS_SET_VERS)
							{
								Broker_Announce_LinkstateVersionAcknowledge(pContext->pBrokerContext);
							}
                            else if (message[3] == SAT_COMMS_FPGA_I2C_BUS_ARB)
                            {
                                Broker_Announce_LinkstateI2CAcknowledge(pContext->pBrokerContext);
                            }
							break;

						case SAT_COMMS_MSG_ERR:
							if (message[4] == SAT_COMMS_SET_VERS)
							{
								Broker_Announce_LinkstateVersionDeclined(pContext->pBrokerContext);
							}
							else if (message[4] == SAT_COMMS_VERS_REQ)
							{
								Broker_Announce_LinkstateVersionResponseFailed(pContext->pBrokerContext);
							}
                            else if (message[4] == SAT_COMMS_FPGA_I2C_BUS_ARB)
                            {
                                Broker_Announce_LinkstateI2CResponseFailed(pContext->pBrokerContext);
                            }
							else
                            {
                                Watch_Set(pContext->pWatchPointContext, W_LINK_RECEIVE_ERR_MSG_UNHANDLED_MSG_ID,
                                    message[3] << 24 |         // Payload Error
                                    message[4] << 16 |         // Payload (Tx msg id of Errored message)
                                    message[5] << 8 |          // Any payload OR Checksum
                                    message[6]);               // Any more payload OR Checksum
                            }
							break;
						case SAT_COMMS_VERS_RESP:
							//LinkState_CreateEvent_E_LS_LINK_USER_IS_REMOTE_SENSOR_SUPERVISOR(pContext->pLinkStateContext);
							minVersion = message[3];
							maxVersion = message[4];

							pContext->pLocalSensorSupervisorContext->pVersionSupportContext->commsVersion = MIN(pContext->pLocalSensorSupervisorContext->pVersionSupportContext->CMCOfferedInterfaceVersion, maxVersion);

							if (pContext->pLocalSensorSupervisorContext->pVersionSupportContext->commsVersion < CMC_COMMS_MIN_VERSION ||
								pContext->pLocalSensorSupervisorContext->pVersionSupportContext->commsVersion < minVersion)
							{
								pContext->pLocalSensorSupervisorContext->pVersionSupportContext->commsVersion = 0;
							}
							Watch_Set(pContext->pWatchPointContext, W_LINK_RECEIVE_NEGOTIATED_VERSION, pContext->pLocalSensorSupervisorContext->pVersionSupportContext->commsVersion);
							PERIPHERAL_REGMAP_RAM_CONTROLLER_WriteWithMask(pContext->pLocalSensorSupervisorContext->DataStoreContext.pHardwareRegisterSetContext, HOST_REGISTER_STATUS,
								pContext->pLocalSensorSupervisorContext->pVersionSupportContext->commsVersion << HOST_REGISTER_PATTERN_SAT_VER_OFFSET, HOST_REGISTER_PATTERN_SAT_VER_MASK);



							Broker_Announce_LinkstateVersionResponse(pContext->pBrokerContext);
							break;

						case SAT_COMMS_ALERT_RESP:						
						case SAT_COMMS_GET_MCTP_MSG_RESP:
						case SAT_COMMS_SNSR_STATE_RESP:
						case SAT_COMMS_GET_VDM_RESP_RESP:
						case SAT_COMMS_FEATURE_RESP:
						case SAT_COMMS_SNSR_POLL_FREQ_RESP:
						case SAT_COMMS_SEND_OEM_CMD_RESP:
							break;
						default:
							Watch_Inc(pContext->pWatchPointContext, W_LINK_RECEIVE_MSG_RECEIVED_UNKNOWN_MSG_ID);
							PERIPHERAL_REGMAP_RAM_CONTROLLER_WriteWithMask(pContext->pBrokerContext->pHostUserProxyThreadContext->pPeripheralRegMapRamControllerContext, HOST_REGISTER_ERROR_REG, HOST_ERROR_REG_SAT_RX_ERROR_MASK, SAT_COMMS_BAD_MSG_ID << HOST_ERROR_REG_SAT_RX_ERROR_OFFSET);
							PERIPHERAL_REGMAP_RAM_CONTROLLER_WriteWithMask(pContext->pBrokerContext->pHostUserProxyThreadContext->pPeripheralRegMapRamControllerContext, HOST_REGISTER_ERROR_REG, HOST_ERROR_REG_SAT_COMMS_ERROR, HOST_ERROR_REG_SAT_COMMS_ERROR);
							break;
					}
					
				}
				else
				{
                    cmcDumpDebugBuffer(pContext);
				}
				TRANSPORT_SENSOR_DeframeReset(pContext->pTransport);
			}
                break;

        case    S_LINK_USER_IS_REMOTE_SENSOR_SUPERVISOR_LINK_OPERATING:
                /* Pass Octet to Sensor Supervisor Parser */
			messageDeframed = TRANSPORT_SENSOR_DeframeTryAdd(pContext->pTransport, AnyOctet);
			if (messageDeframed)
			{
				message = TRANSPORT_SENSOR_DeframeGetMessage(pContext->pTransport);

				if (!PROTOCOL_SENSOR_ProcessMessage(pContext->pProtocol, message, &pContext->payloadLength, &pContext->msgIDReceived))
				{
					/* Trace */
					LinkProtocolTraceLogElement.MsgID = pContext->msgIDReceived;
					LinkProtocolTraceLogElement.FSMCurrentState = pContext->pLocalSensorSupervisorContext->State;
					LinkProtocolTraceLogElement.FSMPrevState = pContext->pLocalSensorSupervisorContext->PreviousState;
					LinkProtocolTrace_TryAdd(pContext->LinkProtocolTraceContext, &LinkProtocolTraceLogElement);

					/* Increment the message coverage counter */
					cmcIncrementReg(pContext->pLocalSensorSupervisorContext->DataStoreContext.pHardwareRegisterSetContext, CMC_SC_MSG_COVERAGE_REG + (pContext->msgIDReceived << 2));
					
					LinkReceiveThread_SensorSupervisorPacketCaptureTest(pContext);

					switch (pContext->msgIDReceived)
					{		    
						case SAT_COMMS_BOARD_INFO_RESP:
                        case SAT_COMMS_VOLT_SNSR_RESP:
                        case SAT_COMMS_POWER_SNSR_RESP:
                        case SAT_COMMS_TEMP_SNSR_RESP:
                        case SAT_COMMS_SNSR_STATE_RESP:
                        case SAT_COMMS_SEND_OEM_CMD_RESP:
                        case SAT_COMMS_ALERT_RESP:
                        case SAT_COMMS_SNSR_POWER_THROTTLING_THRESHOLDS_RESP:
                        case SAT_COMMS_SNSR_TEMP_THROTTLING_THRESHOLDS_RESP:
                        case SAT_COMMS_CSDR_RESP:
                        case SAT_COMMS_READ_QSFP_DIAGNOSTICS_RESP:
                        case SAT_COMMS_QSFP_VALIDATE_LOW_SPEED_READ_IO_RESP:
                        case SAT_COMMS_INTERRUPT_STATUS_RESP:
                        case SAT_COMMS_QSFP_READ_SINGLE_BYTE_RESP:
                            if (SensorRelay_TryAdd(pContext->pSensorSupervisorRelayBufferContext, (CMC_SENSOR_RELAY_ELEMENT_TYPE*)message))
                            {
                                Broker_SensorSupervisor_Message_Arrival(pContext->pBrokerContext);
                            }
                            else
                            {
                                Watch_Inc(pContext->pWatchPointContext, W_SENSOR_RELAY_TRY_ADD_FAIL);
                            }
                            break;

						case SAT_COMMS_MSG_GOOD:
							/* Check which message we are getting response to */
							switch (message[3])
							{
								case SAT_COMMS_CAGE_IO_EVENT:
									Broker_SensorSupervisor_Cage_Info_Arrival(pContext->pBrokerContext);
									break;
								case SAT_COMMS_SNSR_PUSH:
									break;
								case SAT_COMMS_EN_BSL:
									LinkState_CreateEvent_E_LS_ENABLE_BSL_RESPONSE(pContext->pLinkStateContext);								
									break;
                                case SAT_COMMS_PCIE_ERROR_REPORT:
                                case SAT_COMMS_ECC_ERROR_REPORT:
								case SAT_COMMS_KEEPALIVE:
                                case SAT_COMMS_QSFP_VALIDATE_LOW_SPEED_WRITE_IO_REQ:
                                case SAT_COMMS_QSFP_WRITE_SINGLE_BYTE_REQ:
                                    if (SensorRelay_TryAdd(pContext->pSensorSupervisorRelayBufferContext, (CMC_SENSOR_RELAY_ELEMENT_TYPE*)message))
                                    {
                                        Broker_SensorSupervisor_Message_Arrival(pContext->pBrokerContext);
                                    }
                                    else
                                    {
                                        Watch_Inc(pContext->pWatchPointContext, W_SENSOR_RELAY_TRY_ADD_FAIL);
                                    }
                                    break;
								default:
									Watch_Inc(pContext->pWatchPointContext, W_LINK_RECEIVE_GOOD_MSG_UNHANDLED_MSG_ID);
									break;
							}													
							break;
						case SAT_COMMS_MSG_ERR:         
							/* Check which message we are getting response to */
							switch (message[4])
							{
							case SAT_COMMS_CAGE_IO_EVENT:
								break;
							case SAT_COMMS_SNSR_PUSH:
								break;
							case SAT_COMMS_EN_BSL:
                                LinkState_CreateEvent_E_LS_ENABLE_BSL_RESPONSE_FAILED(pContext->pLinkStateContext);
								break;
							case SAT_COMMS_SEND_OEM_CMD:
								break;
                            case SAT_COMMS_DEBUG_UART_EN:
                                //Broker_Announce_LinkstateDebugUartEnableResponse(pContext->pBrokerContext);
                                break;
							case SAT_COMMS_PCIE_ERROR_REPORT:
								break;
							case SAT_COMMS_ECC_ERROR_REPORT:
								break;
							case SAT_COMMS_KEEPALIVE:
								break;
                            case SAT_COMMS_CSDR_REQ:
                            case SAT_COMMS_READ_QSFP_DIAGNOSTICS_REQ:
                            case SAT_COMMS_QSFP_VALIDATE_LOW_SPEED_WRITE_IO_REQ:
                            case SAT_COMMS_QSFP_VALIDATE_LOW_SPEED_READ_IO_REQ:
                            case SAT_COMMS_QSFP_READ_SINGLE_BYTE_REQ:
                            case SAT_COMMS_QSFP_WRITE_SINGLE_BYTE_REQ:
                                if (SensorRelay_TryAdd(pContext->pSensorSupervisorRelayBufferContext, (CMC_SENSOR_RELAY_ELEMENT_TYPE*)message))
                                {
                                    Broker_SensorSupervisor_Message_Arrival(pContext->pBrokerContext);
                                }
                                else
                                {
                                    Watch_Inc(pContext->pWatchPointContext, W_SENSOR_RELAY_TRY_ADD_FAIL);
                                }
                                break;

							default:
                                Watch_Set(pContext->pWatchPointContext, W_LINK_RECEIVE_ERR_MSG_UNHANDLED_MSG_ID,
                                    message[2] << 24 |         // Payload Length
                                    message[3] << 16 |         // Payload (Tx msg id of Errored message)
                                    message[4] << 8 |          // Any payload OR Checksum
                                    message[5]);               // Any more payload OR Checksum
								break;
							}
							break;

                        case SAT_COMMS_DEBUG_UART_EN_RESP:
                            Broker_SensorSupervisor_Debug_Uart_Response_Arrival(pContext->pBrokerContext);
                            break;

						case SAT_COMMS_VERS_RESP:
						case SAT_COMMS_GET_MCTP_MSG_RESP:
						case SAT_COMMS_GET_VDM_RESP_RESP:
						case SAT_COMMS_FEATURE_RESP:
						case SAT_COMMS_SNSR_POLL_FREQ_RESP:
							break;                  

						default:
							Watch_Inc(pContext->pWatchPointContext, W_LINK_RECEIVE_MSG_RECEIVED_UNKNOWN_MSG_ID);
							PERIPHERAL_REGMAP_RAM_CONTROLLER_WriteWithMask(pContext->pBrokerContext->pHostUserProxyThreadContext->pPeripheralRegMapRamControllerContext, HOST_REGISTER_ERROR_REG, HOST_ERROR_REG_SAT_RX_ERROR_MASK, SAT_COMMS_BAD_MSG_ID << HOST_ERROR_REG_SAT_RX_ERROR_OFFSET);
							PERIPHERAL_REGMAP_RAM_CONTROLLER_WriteWithMask(pContext->pBrokerContext->pHostUserProxyThreadContext->pPeripheralRegMapRamControllerContext, HOST_REGISTER_ERROR_REG, HOST_ERROR_REG_SAT_COMMS_ERROR, HOST_ERROR_REG_SAT_COMMS_ERROR);
							break;
					}
				}
				else
				{
                    cmcDumpDebugBuffer(pContext);
				}
				TRANSPORT_SENSOR_DeframeReset(pContext->pTransport);
			}
                break;

        case    S_WAITING_FOR_REMOTE_BOOTLOADER_RESPONSE:
				/* We arrive here when we've sent an BSL_SYNC - SC should respond with a 0x00  */
   			    messageParsed = PROTOCOL_BOOTLOADER_ProcessMessage(pContext->pProtocolBootloader, AnyOctet, pContext->pBrokerContext->pBootloaderThreadContext->lastCommandSent, &msgID, &msgResult);
				if (messageParsed)
				{
                    LinkReceiveThread_BootloaderPacketCaptureTest(pContext);

					/* Trace */
					LinkProtocolTraceLogElement.MsgID = pContext->BootloaderParserContext->rxBootloaderMsgIDReceived;
					LinkProtocolTraceLogElement.FSMCurrentState = 0;
					LinkProtocolTraceLogElement.FSMPrevState = 0;;
					LinkProtocolTrace_TryAdd(pContext->LinkProtocolTraceContext, &LinkProtocolTraceLogElement);
					
                    // In response to a BSL_SYNC we may get BSL response 00 80 02 00 3b 07 87 b4   
                    // The 07 is UNKNOWN_COMMAND but that does still tell us that the SC is in BSL mode
                    if (pContext->BootloaderParserContext->parserResult == BOOTLOADER_RESULT_BSL_MODE_ACK_RECEIVED ||
                        pContext->BootloaderParserContext->parserResult == BOOTLOADER_RESULT_BSL_MODE_UNKNOWN_COMMAND ||
                        pContext->BootloaderParserContext->parserResult == BOOTLOADER_RESULT_BSL_MODE_BSL_LOCKED ||
                        pContext->BootloaderParserContext->parserResult == BOOTLOADER_RESULT_BSL_MODE_PASSWORD_ERROR)
                    {
						LinkState_CreateEvent_E_LS_LINK_USER_IS_REMOTE_BOOTLOADER(pContext->pLinkStateContext);
					}
					PROTOCOL_BOOTLOADER_Initialize(pContext->pProtocolBootloader);
				}
                break;

        case    S_LINK_USER_IS_REMOTE_BOOTLOADER:
                /* Pass Octet to Bootloader Parser */
				messageParsed = PROTOCOL_BOOTLOADER_ProcessMessage(pContext->pProtocolBootloader, AnyOctet, pContext->pBrokerContext->pBootloaderThreadContext->lastCommandSent, &msgID, &msgResult);
				if (messageParsed)
				{
                    LinkReceiveThread_BootloaderPacketCaptureTest(pContext);
					if (pContext->BootloaderParserContext->parserResult == BOOTLOADER_RESULT_BSL_MODE_ACK_RECEIVED)
					{
						/* Check if the last command was a CRC check and if it was - do the checksums match */
						if (pContext->pBrokerContext->pBootloaderThreadContext->lastCommandSent == BSL_CRC_CHECK_32_MESSAGE_ID)
						{
							if (DoChecksumsMatch(pContext))
							{
								Broker_Announce_RemoteBootloaderOperationSuccess(pContext->pBrokerContext);
							}
							else
							{
								Broker_Announce_RemoteBootloaderOperationFailed(pContext->pBrokerContext);
							}
						}
						else
						{
							Broker_Announce_RemoteBootloaderOperationSuccess(pContext->pBrokerContext);
						}
						
					}
					else
					{
						Broker_Announce_RemoteBootloaderOperationFailed(pContext->pBrokerContext);
					}

					PROTOCOL_BOOTLOADER_Initialize(pContext->pProtocolBootloader);
				}
                break;

        default:
                /* Drop Octet - no more work to do */
                break;
            
    }
}

bool LinkReceiveThread_Schedule(void *pContext)
{
    LINK_RECEIVE_THREAD_CONTEXT_TYPE *pThreadContext=(LINK_RECEIVE_THREAD_CONTEXT_TYPE *)pContext;
    char AnyOctet;

    Watch_Inc(pThreadContext->pWatchPointContext, W_THREAD_LINK_RECEIVE);


    while(CircularBuffer_TryRead(&(pThreadContext->EventSourceCircularBuffer), &AnyOctet))
    {
        LinkReceiveThread_LinkProtocolArbiter(pThreadContext, AnyOctet);
    }

    return true;
}







