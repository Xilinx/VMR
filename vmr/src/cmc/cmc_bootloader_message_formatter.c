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
 *  $Change: 2770123 $
 *  $Date: 2020/02/04 $
 *  $Revision: #18 $
 *
 */




#include "cmc_bootloader_message_formatter.h"
#include "cmc_user_supplied_environment.h"

static void cmcFormatBSLMessage(void* pUserContext, USER_SUPPLIED_ENVIRONMENT_CALCULATE_CRC16_CCITT_TYPE pFN_Calculate_CRC16_CCITT, char* pTxBuffer, uint32_t* pTxBufferSize, char* pPayload, uint32_t payloadSize, char command)
{
	uint16_t checksum;
	char* checksumstartpoint;

	/* Header */
	*(pTxBuffer++) = BSL_HEADER;

	/* Length */
	*(pTxBuffer++) = ((payloadSize+1) & 0xFF);
	*(pTxBuffer++) = (((payloadSize+1) >> 8) & 0xFF);

	/* Mark where to start calculating the checksum */
	checksumstartpoint = pTxBuffer;

	/* Command */
	*(pTxBuffer++) = command;

	/* Payload */
	cmcMemCopy(pTxBuffer, pPayload, payloadSize);
	pTxBuffer += payloadSize;

	/* Checksum is done over Command + payload */
	checksum = pFN_Calculate_CRC16_CCITT(pUserContext, 0xFFFF, checksumstartpoint, payloadSize + 1);
	cmcMemCopy(pTxBuffer, (char*)& checksum, 2);

	/* HEADER + LENGTH_L + LENGTH_H + CMD + PAYLOAD + CHECKSUM_L + CHECKSUM_H*/
	*pTxBufferSize = payloadSize + 6;
}


static void cmcPopulateBSLRxBlock32orCRCCheckMMessage(void* pUserContext, USER_SUPPLIED_ENVIRONMENT_CALCULATE_CRC16_CCITT_TYPE pFN_Calculate_CRC16_CCITT, char* pTxBuffer, uint32_t* pTxBufferSize, uint32_t DestinationAddress, char* pPayload, uint32_t payloadSize, char command)
{
	uint16_t checksum;
	char* checksumstartpoint;

	/* Header */
	*(pTxBuffer++) = BSL_HEADER;

	/* Length */
	*(pTxBuffer++) = ((payloadSize + 5) & 0xFF);
	*(pTxBuffer++) = (((payloadSize + 5) >> 8) & 0xFF);

	/* Mark where to start calculating the checksum */
	checksumstartpoint = pTxBuffer;

	/* Command */
	*(pTxBuffer++) = command;

	/* Payload Address */
	cmcMemCopy(pTxBuffer, (char*)&DestinationAddress, 4);
	pTxBuffer += 4;


	/* Payload Data */
	cmcMemCopy(pTxBuffer, pPayload, payloadSize);
	pTxBuffer += payloadSize;

	/* Checksum is done over Command + payload */
	checksum = pFN_Calculate_CRC16_CCITT(pUserContext, 0xFFFF, checksumstartpoint, payloadSize + 5);
	cmcMemCopy(pTxBuffer, (char*)& checksum, 2);

	/* HEADER + LENGTH_L + LENGTH_H + CMD + PAYLOAD + CHECKSUM_L + CHECKSUM_H*/
	*pTxBufferSize = payloadSize + 10;
}

void cmcFormatBSLResetMessage(void* pUserContext, USER_SUPPLIED_ENVIRONMENT_CALCULATE_CRC16_CCITT_TYPE pFN_Calculate_CRC16_CCITT,
	char* pTxBuffer, uint32_t* pTxBufferSize)
{
	char* pPayload = NULL;
	uint16_t payloadSize = 0;
	uint8_t command = BSL_REBOOT_RESET_MESSAGE_ID;

	cmcFormatBSLMessage(pUserContext, pFN_Calculate_CRC16_CCITT, pTxBuffer, pTxBufferSize, pPayload, payloadSize, command);
}

void cmcFormatBSLPasswordMessage(void* pUserContext, USER_SUPPLIED_ENVIRONMENT_CALCULATE_CRC16_CCITT_TYPE pFN_Calculate_CRC16_CCITT, 
								char* pTxBuffer, uint32_t* pTxBufferSize)
{
	uint16_t payloadSize = BSL_PW_LEN;
	uint8_t command = BSL_PW_MESSAGE_ID;

	cmcFormatBSLMessage(pUserContext, pFN_Calculate_CRC16_CCITT, pTxBuffer, pTxBufferSize, bsl_password, payloadSize, command);
}

void cmcFormatBSLBaudMessage(void* pUserContext, USER_SUPPLIED_ENVIRONMENT_CALCULATE_CRC16_CCITT_TYPE pFN_Calculate_CRC16_CCITT,
	char* pTxBuffer, uint32_t* pTxBufferSize)
{
	char payload = BSL_BAUD_RATE_115200;
	uint16_t payloadSize = BSL_BAUD_LEN;
	uint8_t command = BSL_BAUD_MESSAGE_ID;

	cmcFormatBSLMessage(pUserContext, pFN_Calculate_CRC16_CCITT, pTxBuffer, pTxBufferSize, &payload, payloadSize, command);
}

void cmcFormatBSLVersionMessage(void* pUserContext, USER_SUPPLIED_ENVIRONMENT_CALCULATE_CRC16_CCITT_TYPE pFN_Calculate_CRC16_CCITT,
	char* pTxBuffer, uint32_t* pTxBufferSize)
{
	char* pPayload = NULL;
	uint16_t payloadSize = BSL_VERS_LEN;
	uint8_t command = BSL_VERS_MESSAGE_ID;

	cmcFormatBSLMessage(pUserContext, pFN_Calculate_CRC16_CCITT, pTxBuffer, pTxBufferSize, pPayload, payloadSize, command);
}


void cmcFormatBSLMassEraseMessage(void* pUserContext, USER_SUPPLIED_ENVIRONMENT_CALCULATE_CRC16_CCITT_TYPE pFN_Calculate_CRC16_CCITT,
	char* pTxBuffer, uint32_t* pTxBufferSize)
{
	char* pPayload = NULL;
	uint16_t payloadSize = BSL_MASS_ERASE_LEN;
	uint8_t command = BSL_MASS_ERASE_MESSAGE_ID;

	cmcFormatBSLMessage(pUserContext, pFN_Calculate_CRC16_CCITT, pTxBuffer, pTxBufferSize, pPayload, payloadSize, command);
}


void cmcFormatBSLRxBlock32Message(void* pUserContext, USER_SUPPLIED_ENVIRONMENT_CALCULATE_CRC16_CCITT_TYPE pFN_Calculate_CRC16_CCITT, char* pTxBuffer, uint32_t* pTxBufferSize, uint32_t DestinationAddress, char* pPayloadFromHost, uint32_t payloadFromHostSize)
{
	uint8_t command = BSL_RX_BLOCK_32_MESSAGE_ID;

	cmcPopulateBSLRxBlock32orCRCCheckMMessage(pUserContext, pFN_Calculate_CRC16_CCITT, pTxBuffer, pTxBufferSize, DestinationAddress, pPayloadFromHost, payloadFromHostSize, command);
}


void cmcFormatBSLCRCCheck32Message(void* pUserContext, USER_SUPPLIED_ENVIRONMENT_CALCULATE_CRC16_CCITT_TYPE pFN_Calculate_CRC16_CCITT, char* pTxBuffer, uint32_t* pTxBufferSize, uint32_t DestinationAddress, char* pPayloadFromHost, uint32_t payloadFromHostSize)
{
	uint8_t command = BSL_CRC_CHECK_32_MESSAGE_ID;

	cmcPopulateBSLRxBlock32orCRCCheckMMessage(pUserContext, pFN_Calculate_CRC16_CCITT, pTxBuffer, pTxBufferSize, DestinationAddress, pPayloadFromHost, payloadFromHostSize, command);
}

void cmcFormatBSLLoadPC32Message(void* pUserContext, USER_SUPPLIED_ENVIRONMENT_CALCULATE_CRC16_CCITT_TYPE pFN_Calculate_CRC16_CCITT,
	char* pTxBuffer, uint32_t* pTxBufferSize, uint32_t LoadAddress)
{
	uint8_t command = BSL_LOAD_PC_32_MESSAGE_ID;
	char* pPayloadFromHost = NULL;
	uint32_t payloadFromHostSize = 0;

	//assert(payloadSize != BSL_LOAD_PC_32_LEN);

	cmcPopulateBSLRxBlock32orCRCCheckMMessage(pUserContext, pFN_Calculate_CRC16_CCITT, pTxBuffer, pTxBufferSize, LoadAddress, pPayloadFromHost, payloadFromHostSize, command);
}


void cmcFormatBSLSyncMessage(char* pTxBuffer, uint32_t* pTxBufferSize)
{
	*pTxBuffer = BSL_SYNC_ID;
	*pTxBufferSize = BSL_SYNC_LEN;
}
