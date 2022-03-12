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
 *  $Change: 3095780 $
 *  $Date: 2021/01/13 $
 *  $Revision: #44 $
 *
 */




#include "cmc_local_bootloader_thread.h"
#include "cmc_protocol_bootloader.h"

void Bootloader_CLEAR_ACTION(LOCAL_BOOTLOADER_THREAD_CONTEXT_TYPE *pContext)
{
    pContext->Action=0;    
}


void Bootloader_ACTION(LOCAL_BOOTLOADER_THREAD_CONTEXT_TYPE *pContext, uint32_t AnyAction)
{
    pContext->Action|=AnyAction;
}



void Bootloader_HandleAction_A_LB_T_RESPONSE_START(LOCAL_BOOTLOADER_THREAD_CONTEXT_TYPE *pContext)
{
    pContext->CurrentResponseTimerInstance=TimerThread_Start(   pContext->pThreadTimerContext,
                                                                T_BOOTLOADER_RESPONSE_TIMER,
                                                                BOOTLOADER_RESPONSE_TIMER_DURATION_MS,
                                                                pContext,
                                                                LocalBootloader_ResponseTimerCallback);
}


void Bootloader_HandleAction_A_LB_T_RESPONSE_RESTART(LOCAL_BOOTLOADER_THREAD_CONTEXT_TYPE *pContext)
{
    pContext->CurrentResponseTimerInstance=TimerThread_Restart(pContext->pThreadTimerContext, T_BOOTLOADER_RESPONSE_TIMER);
}


void Bootloader_HandleAction_A_LB_T_RESPONSE_STOP(LOCAL_BOOTLOADER_THREAD_CONTEXT_TYPE *pContext)
{
    TimerThread_Stop(pContext->pThreadTimerContext, T_BOOTLOADER_RESPONSE_TIMER);
}


void Bootloader_HandleAction_A_LB_T_RESTART_DELAY_START(LOCAL_BOOTLOADER_THREAD_CONTEXT_TYPE *pContext)
{
    pContext->CurrentDelayTimerInstance=TimerThread_Start(      pContext->pThreadTimerContext,
                                                                T_BOOTLOADER_DELAY_TIMER,
                                                                BOOTLOADER_RESTART_DELAY_TIMER_DURATION_MS,
                                                                pContext,
                                                                LocalBootloader_DelayTimerCallback);
}


void Bootloader_HandleAction_A_LB_T_RESTART_DELAY_RESTART(LOCAL_BOOTLOADER_THREAD_CONTEXT_TYPE *pContext)
{
    pContext->CurrentResponseTimerInstance=TimerThread_Restart(pContext->pThreadTimerContext, T_BOOTLOADER_DELAY_TIMER);
}


void Bootloader_HandleAction_A_LB_T_RESTART_DELAY_STOP(LOCAL_BOOTLOADER_THREAD_CONTEXT_TYPE *pContext)
{
    TimerThread_Stop(pContext->pThreadTimerContext, T_BOOTLOADER_DELAY_TIMER);
}


void Bootloader_HandleAction_A_LB_SEND_PASSWORD(LOCAL_BOOTLOADER_THREAD_CONTEXT_TYPE *pContext)
{
	/* Format Message into Buffer */
	PROTOCOL_BOOTLOADER_FormatMessage(pContext->pProtocol, SAT_COMMS_BSL_PW, pContext->TxMsgBuffer, &(pContext->TxMsgBufferSize), 0, NULL, 0);

	PERIPHERAL_AXI_UART_LITE_SATELLITE_SendByteSequenceBlocking(pContext->pAXI_UART_LITE_Satellite_Context, pContext->TxMsgBuffer, pContext->TxMsgBufferSize);
	
	/* Increment the message coverage counter */
	cmcIncrementReg(pContext->pPeripheralRegMapContext, CMC_SC_MSG_COVERAGE_REG + (SAT_COMMS_BSL_PW << 2));
	
	pContext->lastCommandSent = BSL_PW_MESSAGE_ID;
}


void Bootloader_HandleAction_A_LB_SEND_ERASE_FIRMWARE(LOCAL_BOOTLOADER_THREAD_CONTEXT_TYPE *pContext)
{
	PROTOCOL_BOOTLOADER_FormatMessage(pContext->pProtocol, SAT_COMMS_BSL_MASS_ERASE, pContext->TxMsgBuffer, &(pContext->TxMsgBufferSize), 0, NULL, 0);

	PERIPHERAL_AXI_UART_LITE_SATELLITE_SendByteSequenceBlocking(pContext->pAXI_UART_LITE_Satellite_Context, pContext->TxMsgBuffer, pContext->TxMsgBufferSize);
	
	/* Increment the message coverage counter */
	cmcIncrementReg(pContext->pPeripheralRegMapContext, CMC_SC_MSG_COVERAGE_REG + (SAT_COMMS_BSL_MASS_ERASE << 2));

	pContext->lastCommandSent = BSL_MASS_ERASE_MESSAGE_ID;
}


void Bootloader_HandleAction_A_LB_ANNOUNCE_COMMAND_DONE(LOCAL_BOOTLOADER_THREAD_CONTEXT_TYPE *pContext)
{
	Broker_Announce_UserProxyRemoteBootloaderOperationSuccess(pContext->pBrokerContext);
}

void Bootloader_HandleAction_A_LB_SEND_ACK_FINAL_DECISION(LOCAL_BOOTLOADER_THREAD_CONTEXT_TYPE* pContext)
{
	if (Bootloader_FSM_Helper_FramesRemaining(pContext))
	{
		Bootloader_CreateEvent_E_LB_REMOTE_ACK_FRAMES_REMAINING(pContext);
	}
	else
	{
		Bootloader_CreateEvent_E_LB_REMOTE_ACK_NO_FRAMES_REMAINING(pContext);
	}
}


void Bootloader_HandleAction_A_LB_ANNOUNCE_COMMAND_FAILED(LOCAL_BOOTLOADER_THREAD_CONTEXT_TYPE *pContext)
{
	Broker_Announce_UserProxyRemoteBootloaderOperationFailed(pContext->pBrokerContext);
	pContext->ChunkCount = 0;

}

void Bootloader_HandleAction_A_LB_SEND_FIRMWARE_SEGMENT(LOCAL_BOOTLOADER_THREAD_CONTEXT_TYPE* pContext)
{
    
    
	uint8_t* pDataSegment; /* points to data segment in RAM Controller Register Map */
    uint32_t totalMessageLength;

	/* Read the opcode to see if this is a first segment */
	uint32_t RemoteMessageRegisterValue = PERIPHERAL_REGMAP_RAM_CONTROLLER_Read(pContext->pPeripheralRegMapContext, HOST_REGISTER_REMOTE_COMMAND_REGISTER);

	uint8_t msgOpCode = (RemoteMessageRegisterValue & 0xFF000000) >> 24;

	if (msgOpCode == CMC_OP_MSP432_FW_SEC)
	{
		/* If this is the first chunk to send store the address and total segment length */
		if(pContext->ChunkCount == 0)
		{
			pContext->DestinationAddress = PERIPHERAL_REGMAP_RAM_CONTROLLER_Read(pContext->pPeripheralRegMapContext, HOST_REGISTER_REMOTE_COMMAND_ADDRESS_REGISTER_FIRST_SEGMENT);
			PERIPHERAL_REGMAP_RAM_CONTROLLER_GetDataSegmentFirstRecord(pContext->pPeripheralRegMapContext, &pDataSegment, &totalMessageLength);
			pContext->pPayload = pDataSegment;
			pContext->IndexAddress = pContext->DestinationAddress;
			pContext->FirmwareDownloadRemaining = true;
			pContext->Length = totalMessageLength - 8;
			pContext->lastCommandSent = 0;
		}
	}
	else if (msgOpCode == CMC_OP_MSP432_FW_DATA)
	{
		/* If this is the first chunk to send store the address and total segment length */
		if (pContext->ChunkCount == 0)
		{
			PERIPHERAL_REGMAP_RAM_CONTROLLER_GetDataSegmentSubsequentRecord(pContext->pPeripheralRegMapContext, &pDataSegment, &totalMessageLength);
			pContext->pPayload = pDataSegment;
			pContext->FirmwareDownloadRemaining = true;
			pContext->Length = totalMessageLength;
			pContext->lastCommandSent = 0;
		}

	}
    

	if ((pContext->Length - (pContext->ChunkCount * 256)) >= 256) // full 256 byte message
	{
		PROTOCOL_BOOTLOADER_FormatMessage(pContext->pProtocol, SAT_COMMS_BSL_RX_BLOCK_32, pContext->TxMsgBuffer, &(pContext->TxMsgBufferSize), pContext->IndexAddress, (char*)(pContext->pPayload), 256);

        //TimerThread_TimeDifference_UpdateEnter(pContext->pThreadTimerContext);
		PERIPHERAL_AXI_UART_LITE_SATELLITE_SendByteSequenceBlocking(pContext->pAXI_UART_LITE_Satellite_Context, pContext->TxMsgBuffer, pContext->TxMsgBufferSize);
        //TimerThread_TimeDifference_UpdateExit(pContext->pThreadTimerContext);
		/* Increment the message coverage counter */
		cmcIncrementReg(pContext->pPeripheralRegMapContext, CMC_SC_MSG_COVERAGE_REG + (SAT_COMMS_BSL_RX_BLOCK_32 << 2));

		pContext->lastCommandSent = BSL_RX_BLOCK_32_MESSAGE_ID;
        
	}
	else 
	{
		PROTOCOL_BOOTLOADER_FormatMessage(pContext->pProtocol, SAT_COMMS_BSL_RX_BLOCK_32, pContext->TxMsgBuffer, &(pContext->TxMsgBufferSize), pContext->IndexAddress, (char*)(pContext->pPayload), (pContext->Length - (pContext->ChunkCount * 256)));

		PERIPHERAL_AXI_UART_LITE_SATELLITE_SendByteSequenceBlocking(pContext->pAXI_UART_LITE_Satellite_Context, pContext->TxMsgBuffer, pContext->TxMsgBufferSize);	
		
		/* Increment the message coverage counter */
		cmcIncrementReg(pContext->pPeripheralRegMapContext, CMC_SC_MSG_COVERAGE_REG + (SAT_COMMS_BSL_RX_BLOCK_32 << 2));
		
		pContext->lastCommandSent = BSL_RX_BLOCK_32_MESSAGE_ID;
	}

    

}


void Bootloader_HandleAction_A_LB_SEND_CHECKSUM(LOCAL_BOOTLOADER_THREAD_CONTEXT_TYPE *pContext)
{
	/* We need to know the data to send
	   The first 4 are the 32-bit address
	   The next 2 are length of the CRC check */
	uint32_t bytesRemaining;
	
	/* Read the address to write the data segment to */
	if ((pContext->Length - (pContext->ChunkCount * 256)) >= 256) // full 256 byte message
	{
		pContext->FirmwareDownloadRemaining = true;
		/* Calculate the checksum for this chunk and save it in the context */
		pContext->calculatedChecksum = pContext->pFN_Calculate_CRC16_CCITT(pContext->pUserContext, 0xFFFF, (char*)(pContext->pPayload), 256);

		/* First 4 bytes are the address*/
		*(pContext->checksumBody) = 0;
		*(pContext->checksumBody + 1) = 1;
		PROTOCOL_BOOTLOADER_FormatMessage(pContext->pProtocol, SAT_COMMS_BSL_CRC_CHECK_32, pContext->TxMsgBuffer, &(pContext->TxMsgBufferSize), pContext->IndexAddress, (char*)(pContext->checksumBody), 2);

		PERIPHERAL_AXI_UART_LITE_SATELLITE_SendByteSequenceBlocking(pContext->pAXI_UART_LITE_Satellite_Context, pContext->TxMsgBuffer, pContext->TxMsgBufferSize);
		
		/* Increment the message coverage counter */
		cmcIncrementReg(pContext->pPeripheralRegMapContext, CMC_SC_MSG_COVERAGE_REG + (SAT_COMMS_BSL_CRC_CHECK_32 << 2));

		pContext->lastCommandSent = BSL_CRC_CHECK_32_MESSAGE_ID;
		

		/* Last one so set the counter back to zero */
		if ((pContext->Length - (pContext->ChunkCount * 256)) == 256)
		{
			/* Last one so set the counter back to zero */
			pContext->ChunkCount = 0;
			pContext->FirmwareDownloadRemaining = false;
		}
		else
		{
			pContext->ChunkCount++;
		}	

		/* Increment the payload pointer */
		pContext->pPayload = pContext->pPayload + 0x100;
		pContext->IndexAddress = pContext->IndexAddress + 0x100;
	}
	else 
	{
		/* Calculate the checksum for this chunk and save it in the context */
		pContext->calculatedChecksum = pContext->pFN_Calculate_CRC16_CCITT(pContext->pUserContext, 0xFFFF, (char*)(pContext->pPayload), (pContext->Length - (pContext->ChunkCount * 256)));

		bytesRemaining = (pContext->Length - (pContext->ChunkCount * 256));
		*(pContext->checksumBody) = (uint8_t)bytesRemaining;
		*(pContext->checksumBody + 1) = (uint8_t)(bytesRemaining >> 8);

		PROTOCOL_BOOTLOADER_FormatMessage(pContext->pProtocol, SAT_COMMS_BSL_CRC_CHECK_32, pContext->TxMsgBuffer, &(pContext->TxMsgBufferSize), pContext->IndexAddress, (char*)(pContext->checksumBody), 2);

		PERIPHERAL_AXI_UART_LITE_SATELLITE_SendByteSequenceBlocking(pContext->pAXI_UART_LITE_Satellite_Context, pContext->TxMsgBuffer, pContext->TxMsgBufferSize);
		
		/* Increment the message coverage counter */
		cmcIncrementReg(pContext->pPeripheralRegMapContext, CMC_SC_MSG_COVERAGE_REG + (SAT_COMMS_BSL_CRC_CHECK_32 << 2));

		pContext->lastCommandSent = BSL_CRC_CHECK_32_MESSAGE_ID;
		/* Increment the payload pointer */
		pContext->pPayload = pContext->pPayload + (pContext->Length - (pContext->ChunkCount * 256));
		pContext->IndexAddress = pContext->IndexAddress + ((pContext->Length - (pContext->ChunkCount * 256)));

		/* Last one so set the counter back to zero */
		pContext->FirmwareDownloadRemaining = false;
		pContext->ChunkCount = 0;		
	}
}


void Bootloader_HandleAction_A_LB_SEND_RESTART_FIRMWARE(LOCAL_BOOTLOADER_THREAD_CONTEXT_TYPE *pContext)
{
	uint32_t LoadAddress = PERIPHERAL_REGMAP_RAM_CONTROLLER_Read(pContext->pPeripheralRegMapContext, HOST_REGISTER_REMOTE_COMMAND_ADDRESS_REGISTER_FIRST_SEGMENT);;
	PROTOCOL_BOOTLOADER_FormatMessage(pContext->pProtocol, SAT_COMMS_BSL_LOAD_PC_32, pContext->TxMsgBuffer, &(pContext->TxMsgBufferSize), LoadAddress, NULL, 0);

	PERIPHERAL_AXI_UART_LITE_SATELLITE_SendByteSequenceBlocking(pContext->pAXI_UART_LITE_Satellite_Context, pContext->TxMsgBuffer, pContext->TxMsgBufferSize);
	
	/* Increment the message coverage counter */
	cmcIncrementReg(pContext->pPeripheralRegMapContext, CMC_SC_MSG_COVERAGE_REG + (SAT_COMMS_BSL_LOAD_PC_32 << 2));
	
	pContext->lastCommandSent = BSL_LOAD_PC_32_MESSAGE_ID;
}


void Bootloader_HandleAction_A_LB_REQUEST_LINK_USER(LOCAL_BOOTLOADER_THREAD_CONTEXT_TYPE *pContext)
{
    Broker_DetermineLinkState(pContext->pBrokerContext);
}



void Bootloader_HandleAction(LOCAL_BOOTLOADER_THREAD_CONTEXT_TYPE *pContext)
{
    uint32_t i;

    for(i=0;i<MAX_LB_ACTIONS; i++)
    {
        switch((1<<i)&(pContext->Action))
        {

            case    A_LB_NO_ACTION:
                    /* Ignore */
                    break;

            case    A_LB_T_RESPONSE_START:
                    Bootloader_HandleAction_A_LB_T_RESPONSE_START(pContext);
                    break;
       
            case    A_LB_T_RESPONSE_RESTART:
                    Bootloader_HandleAction_A_LB_T_RESPONSE_RESTART(pContext);
                    break;
     
            case    A_LB_T_RESPONSE_STOP:
                    Bootloader_HandleAction_A_LB_T_RESPONSE_STOP(pContext);
                    break;
        
            case    A_LB_T_RESTART_DELAY_START:
                    Bootloader_HandleAction_A_LB_T_RESTART_DELAY_START(pContext);
                    break;
  
            case    A_LB_T_RESTART_DELAY_RESTART:
                    Bootloader_HandleAction_A_LB_T_RESTART_DELAY_RESTART(pContext);
                    break;

            case    A_LB_T_RESTART_DELAY_STOP:
                    Bootloader_HandleAction_A_LB_T_RESTART_DELAY_STOP(pContext); 
                    break;
  
            case    A_LB_SEND_PASSWORD:
                    Bootloader_HandleAction_A_LB_SEND_PASSWORD(pContext);
                    break;
          
            case    A_LB_SEND_ERASE_FIRMWARE:
                    Bootloader_HandleAction_A_LB_SEND_ERASE_FIRMWARE(pContext);
                    break;
    
            case    A_LB_ANNOUNCE_COMMAND_DONE:
                    Bootloader_HandleAction_A_LB_ANNOUNCE_COMMAND_DONE(pContext);
                    break;
  
            case    A_LB_ANNOUNCE_COMMAND_FAILED:
                    Bootloader_HandleAction_A_LB_ANNOUNCE_COMMAND_FAILED(pContext);
                    break;
  
            case    A_LB_SEND_FIRMWARE_SEGMENT:
                    Bootloader_HandleAction_A_LB_SEND_FIRMWARE_SEGMENT(pContext);
                    break;
  
            case    A_LB_SEND_CHECKSUM:
                    Bootloader_HandleAction_A_LB_SEND_CHECKSUM(pContext);
                    break;
          
            case    A_LB_SEND_RESTART_FIRMWARE:
                    Bootloader_HandleAction_A_LB_SEND_RESTART_FIRMWARE(pContext);
                    break;
  
            case    A_LB_REQUEST_LINK_USER:
                    Bootloader_HandleAction_A_LB_REQUEST_LINK_USER(pContext);
                    break;      

			case	A_LB_SEND_ACK_FINAL_DECISION:
					Bootloader_HandleAction_A_LB_SEND_ACK_FINAL_DECISION(pContext);
					break;
            
            default:
                    Watch_Inc(pContext->pWatchPointContext, W_BOOTLOADER_UNEXPECTED_ACTION);
                    break;
        }
    }
}





