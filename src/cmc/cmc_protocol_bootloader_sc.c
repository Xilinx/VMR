/*----------------------------------------------------------------------------------------------*/
/*                                                                                              */
/* © Copyright 2020 Xilinx, Inc. All rights reserved.                                           */
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
 *  $Change: 3097075 $
 *  $Date: 2021/01/14 $
 *  $Revision: #4 $
 *
 */





#include "cmc_protocol_bootloader_sc.h"
#include "cmc_bootloader_message_formatter.h"
#include "cmc_bootloader_message_parser.h"


void PROTOCOL_BOOTLOADER_SC_FormatMessage(void* pUserContext, uint8_t msgID, char* pMessage, uint32_t* pMessageLength, uint32_t address, char* pPayload, uint32_t payloadLength)
{
	PROTOCOL_BOOTLOADER_SC_CONTEXT_TYPE *pProtocolContext = (PROTOCOL_BOOTLOADER_SC_CONTEXT_TYPE *)pUserContext;

	switch (msgID)
	{
	case SAT_COMMS_BSL_SYNC:
		cmcFormatBSLSyncMessage(pMessage, pMessageLength);
		break;

	case SAT_COMMS_BSL_PW:
		cmcFormatBSLPasswordMessage(pProtocolContext->pUserEnvironmentContext, pProtocolContext->pFN_Calculate_CRC16_CCITT, pMessage, pMessageLength);
		break;

	case SAT_COMMS_BSL_BAUD:
		cmcFormatBSLBaudMessage(pProtocolContext->pUserEnvironmentContext, pProtocolContext->pFN_Calculate_CRC16_CCITT, pMessage, pMessageLength);
		break;

	case SAT_COMMS_BSL_VERS:
		cmcFormatBSLVersionMessage(pProtocolContext->pUserEnvironmentContext, pProtocolContext->pFN_Calculate_CRC16_CCITT, pMessage, pMessageLength);
		break;

	case SAT_COMMS_BSL_MASS_ERASE:
		cmcFormatBSLMassEraseMessage(pProtocolContext->pUserEnvironmentContext, pProtocolContext->pFN_Calculate_CRC16_CCITT, pMessage, pMessageLength);
		break;

	case SAT_COMMS_BSL_RX_BLOCK_32:
		cmcFormatBSLRxBlock32Message(pProtocolContext->pUserEnvironmentContext, pProtocolContext->pFN_Calculate_CRC16_CCITT, pMessage, pMessageLength, address, pPayload, payloadLength);
		break;

	case SAT_COMMS_BSL_CRC_CHECK_32:
		cmcFormatBSLCRCCheck32Message(pProtocolContext->pUserEnvironmentContext, pProtocolContext->pFN_Calculate_CRC16_CCITT, pMessage, pMessageLength, address, pPayload, payloadLength);
		break;

	case SAT_COMMS_BSL_LOAD_PC_32:
		cmcFormatBSLLoadPC32Message(pProtocolContext->pUserEnvironmentContext, pProtocolContext->pFN_Calculate_CRC16_CCITT, pMessage, pMessageLength, address);
		break;

	case SAT_COMMS_BSL_RESET:
	case SAT_COMMS_BSL_TX_BLOCK_32:
	case SAT_COMMS_BSL_ERASE_SECTOR_32:
	default:
		return;
	}

	return;
}


bool PROTOCOL_BOOTLOADER_SC_ProcessMessage(void* pUserContext, char AnyCharacter, uint8_t lastCommandSent, uint8_t* msgID, BOOTLOADER_PARSER_RESULT_TYPE* msgResult)
{
    bool Result=false;
	PROTOCOL_BOOTLOADER_SC_CONTEXT_TYPE *protocolContext = (PROTOCOL_BOOTLOADER_SC_CONTEXT_TYPE *)pUserContext;

	Result = cmc_bootloader_message_parse(protocolContext->pParserContext, AnyCharacter, lastCommandSent);

	if (Result)
	{
		*msgID = protocolContext->pParserContext->rxBootloaderMsgIDReceived;
		*msgResult = protocolContext->pParserContext->parserResult;
	}
    return Result;
}


void PROTOCOL_BOOTLOADER_SC_Initialize(void *pUserContext)
{
	PROTOCOL_BOOTLOADER_SC_CONTEXT_TYPE* protocolContext = (PROTOCOL_BOOTLOADER_SC_CONTEXT_TYPE*)pUserContext;
	cmc_bootloader_parser_initialize(protocolContext->pParserContext, protocolContext->pFN_Calculate_CRC16_CCITT, protocolContext->pUserEnvironmentContext);
}


