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
 *  $Change: 3121031 $
 *  $Date: 2021/02/11 $
 *  $Revision: #23 $
 *
 */




#include "cmc_bootloader_message_parser.h"
#include "cmc_bootloader_message_formatter.h"


void cmc_bootloader_parser_initialize(BOOTLOADER_PARSER_CONTEXT_TYPE* pContext, USER_SUPPLIED_ENVIRONMENT_CALCULATE_CRC16_CCITT_TYPE pFN_Calculate_CRC16_CCITT, void* pUserContext)
{
	pContext->bootloaderParserState = S_BOOTLOADER_AWAITING_ACK;
	pContext->rxBootloaderMsgIndex = 0;
	pContext->rxBootloaderMsgBodyLength = 0;
	pContext->rxBootloaderChecksum = 0;
	pContext->rxBootloaderCalculatedChecksum = 0;
	pContext->payloadSize = 0;
	pContext->rxBootloaderMsgIDReceived = 0;
	pContext->rxBootloaderMsgType = BOOTLOADER_PARSER_MSG_OPERATION_SUCCESSFUL;
	pContext->parserResult = BOOTLOADER_BAD;
    pContext->pFN_Calculate_CRC16_CCITT=pFN_Calculate_CRC16_CCITT;
	pContext->pUserContext = pUserContext;

	int i;
		
	for (i = 0; i < MAX_BOOTLOADER_PARSER_MSG_SIZE; i++)
	{
		pContext->rxBootloaderMsg[i] = 0;
	}
}


void cmc_bootloader_setParserResult(BOOTLOADER_PARSER_RESULT_TYPE* pCurrentParserResult, BOOTLOADER_PARSER_RESULT_TYPE newParserResult)
{
	if (*pCurrentParserResult == BOOTLOADER_BAD)
	{
		*pCurrentParserResult = newParserResult;
	}
}

bool cmc_bootloader_message_parse(BOOTLOADER_PARSER_CONTEXT_TYPE* pContext, char AnyCharacter, uint8_t lastCommandSent)
{
	bool Result =  false;
	BOOTLOADER_PARSER_STATE_TYPE next_state = S_BOOTLOADER_AWAITING_ACK;
    UNUSED(lastCommandSent);

	switch (pContext->bootloaderParserState)
	{
		case S_BOOTLOADER_AWAITING_ACK:
			if ((uint8_t)AnyCharacter == BOOTLOADER_BSL_ACK)
			{
				pContext->rxBootloaderMsgIndex = 0;
				pContext->rxBootloaderMsg[pContext->rxBootloaderMsgIndex++] = AnyCharacter;
				next_state = S_BOOTLOADER_AWAITING_HDR;
				pContext->parserResult = BOOTLOADER_RESULT_BSL_MODE_ACK_RECEIVED;
				//if (lastCommandSent == BSL_SYNC_ID || lastCommandSent == BSL_BAUD_MESSAGE_ID)
				//{
				//	/* Expect a single 0x00 as the response */
				//	next_state = S_BOOTLOADER_AWAITING_ACK;
				//	Result = true;
				//}
			}
			else if ((uint8_t)AnyCharacter == BOOTLOADER_BSL_HEADER_INCORRECT)
			{
				pContext->rxBootloaderMsgIndex = 0;
				pContext->rxBootloaderMsg[pContext->rxBootloaderMsgIndex++] = AnyCharacter;
				next_state = S_BOOTLOADER_AWAITING_ACK;
				pContext->parserResult = BOOTLOADER_RESULT_BSL_MODE_RECEIVED_INCORRECT_HEADER;
				Result = true;
			}
			else if ((uint8_t)AnyCharacter == BOOTLOADER_BSL_CHECKSUM_INCORRECT)
			{
				pContext->rxBootloaderMsgIndex = 0;
				pContext->rxBootloaderMsg[pContext->rxBootloaderMsgIndex++] = AnyCharacter;
				next_state = S_BOOTLOADER_AWAITING_ACK;
				pContext->parserResult = BOOTLOADER_RESULT_BSL_MODE_CHECKSUM_INCORRECT;
				Result = true;
			}
			else if ((uint8_t)AnyCharacter == BOOTLOADER_BSL_PACKET_SIZE_ZERO)
			{
				pContext->rxBootloaderMsgIndex = 0;
				pContext->rxBootloaderMsg[pContext->rxBootloaderMsgIndex++] = AnyCharacter;
				next_state = S_BOOTLOADER_AWAITING_ACK;
				pContext->parserResult = BOOTLOADER_RESULT_BSL_MODE_PACKET_SIZE_ZERO;
				Result = true;
			}
			else if ((uint8_t)AnyCharacter == BOOTLOADER_BSL_PACKET_SIZE_TOO_BIG)
			{
				pContext->rxBootloaderMsgIndex = 0;
				pContext->rxBootloaderMsg[pContext->rxBootloaderMsgIndex++] = AnyCharacter;
				next_state = S_BOOTLOADER_AWAITING_ACK;
				pContext->parserResult = BOOTLOADER_RESULT_BSL_MODE_PACKET_SIZE_TOO_BIG;
				Result = true;
			}
			else if ((uint8_t)AnyCharacter == BOOTLOADER_BSL_UNKNOWN_ERROR)
			{
				pContext->rxBootloaderMsgIndex = 0;
				pContext->rxBootloaderMsg[pContext->rxBootloaderMsgIndex++] = AnyCharacter;
				next_state = S_BOOTLOADER_AWAITING_ACK;
				pContext->parserResult = BOOTLOADER_RESULT_BSL_MODE_UNKNOWN_ERROR;
				Result = true;
			}
			else if ((uint8_t)AnyCharacter == BOOTLOADER_BSL_UNKNOWN_BAUD_RATE)
			{
				pContext->rxBootloaderMsgIndex = 0;
				pContext->rxBootloaderMsg[pContext->rxBootloaderMsgIndex++] = AnyCharacter;
				next_state = S_BOOTLOADER_AWAITING_ACK;
				pContext->parserResult = BOOTLOADER_RESULT_BSL_MODE_UNKNOWN_BAUD_RATE;
				Result = true;
			}		
			else if ((uint8_t)AnyCharacter == BOOTLOADER_NORMAL_MODE_ESC)
			{
				pContext->rxBootloaderMsg[pContext->rxBootloaderMsgIndex++] = AnyCharacter;
				next_state = S_BOOTLOADER_AWAITING_NORMAL_MODE_SOP;
			}
			else
			{
				pContext->parserResult = BOOTLOADER_RESULT_BSL_MODE_UNEXPECTED_CHARACTER_RECEIVED;
			}
			break;

		case S_BOOTLOADER_AWAITING_HDR:
			if ((uint8_t)AnyCharacter == BOOTLOADER_HDR)
			{
				pContext->rxBootloaderMsg[pContext->rxBootloaderMsgIndex++] = AnyCharacter;
				next_state = S_BOOTLOADER_AWAITING_LENGTH_L;
			}
			else if((uint8_t)AnyCharacter == BOOTLOADER_BSL_ACK)
			{
				pContext->rxBootloaderMsg[pContext->rxBootloaderMsgIndex++] = AnyCharacter;
				next_state = S_BOOTLOADER_AWAITING_HDR;
				pContext->parserResult = BOOTLOADER_RESULT_BSL_MODE_ACK_RECEIVED;
			}
			else
			{
				pContext->parserResult = BOOTLOADER_RESULT_BSL_MODE_HDR_NOT_RECEIVED;
			}
			break;

		case S_BOOTLOADER_AWAITING_LENGTH_L:
			pContext->rxBootloaderMsg[pContext->rxBootloaderMsgIndex++] = AnyCharacter;
			next_state = S_BOOTLOADER_AWAITING_LENGTH_H;
			break;

		case S_BOOTLOADER_AWAITING_LENGTH_H:
			pContext->rxBootloaderMsg[pContext->rxBootloaderMsgIndex++] = AnyCharacter;
			pContext->rxBootloaderMsgBodyLength = (pContext->rxBootloaderMsg[pContext->rxBootloaderMsgIndex - 1] << 8) | (pContext->rxBootloaderMsg[pContext->rxBootloaderMsgIndex - 2]);
			next_state = S_BOOTLOADER_AWAITING_CMD;
			break;

		case S_BOOTLOADER_AWAITING_CMD:
			pContext->rxBootloaderMsg[pContext->rxBootloaderMsgIndex++] = AnyCharacter;
			if ((uint8_t)AnyCharacter == BOOTLOADER_MSG_CMD)
			{
				pContext->payloadSize = 1;
				pContext->rxBootloaderMsgIDReceived = AnyCharacter;
				next_state = S_BOOTLOADER_AWAITING_MSG;
			}
			else if ((uint8_t)AnyCharacter == BOOTLOADER_DATA_CMD)
			{
				pContext->rxBootloaderMsgBodyLength--;
				pContext->rxBootloaderMsgIDReceived = AnyCharacter;
				next_state = S_BOOTLOADER_AWAITING_DATA;
			}
			else
			{
				next_state = S_BOOTLOADER_AWAITING_ACK;
				pContext->parserResult = BOOTLOADER_RESULT_BSL_MODE_CMD_INVALID;
			}
			break;

		case S_BOOTLOADER_AWAITING_MSG:
			switch (AnyCharacter)
			{
			case BOOTLOADER_MSG_OPERATION_SUCCESSFUL:
				pContext->rxBootloaderMsgType = BOOTLOADER_PARSER_MSG_OPERATION_SUCCESSFUL;
				break;

			case BOOTLOADER_MSG_BSL_LOCKED:
				pContext->rxBootloaderMsgType = BOOTLOADER_PARSER_MSG_BSL_LOCKED;
				break;

			case BOOTLOADER_MSG_BSL_PASSWORD_ERROR:
				pContext->rxBootloaderMsgType = BOOTLOADER_PARSER_MSG_BSL_PASSWORD_ERROR;
				break;

			case BOOTLOADER_MSG_UNNOWN_COMMAND:
				pContext->rxBootloaderMsgType = BOOTLOADER_PARSER_MSG_UNKNOWN_COMMAND;
				break;

			default:
				break;

			}
			pContext->rxBootloaderMsg[pContext->rxBootloaderMsgIndex++] = AnyCharacter;
			next_state = S_BOOTLOADER_AWAITING_CHECKSUM_L;
			break;

		case S_BOOTLOADER_AWAITING_DATA:
			pContext->rxBootloaderMsg[pContext->rxBootloaderMsgIndex++] = AnyCharacter;
			pContext->rxBootloaderMsgBodyLength--;
			pContext->payloadSize++;
			if (pContext->rxBootloaderMsgBodyLength > 0)
			{			
				next_state = S_BOOTLOADER_AWAITING_DATA;
			}
			else
			{
				next_state = S_BOOTLOADER_AWAITING_CHECKSUM_L;
			}
			break;

		case S_BOOTLOADER_AWAITING_CHECKSUM_L:
			pContext->rxBootloaderMsg[pContext->rxBootloaderMsgIndex++] = AnyCharacter;
			next_state = S_BOOTLOADER_AWAITING_CHECKSUM_H;
			break;

		case S_BOOTLOADER_AWAITING_CHECKSUM_H:
			pContext->rxBootloaderMsg[pContext->rxBootloaderMsgIndex] = AnyCharacter;

			pContext->rxBootloaderChecksum = (uint8_t)(pContext->rxBootloaderMsg[pContext->rxBootloaderMsgIndex]);
			pContext->rxBootloaderChecksum = pContext->rxBootloaderChecksum << 8;
			pContext->rxBootloaderChecksum = pContext->rxBootloaderChecksum | (uint8_t)pContext->rxBootloaderMsg[pContext->rxBootloaderMsgIndex - 1];
			
			pContext->rxBootloaderCalculatedChecksum = pContext->pFN_Calculate_CRC16_CCITT(pContext->pUserContext, 0xFFFF, &(pContext->rxBootloaderMsg[4]), pContext->payloadSize + 1);
			if (pContext->rxBootloaderCalculatedChecksum != pContext->rxBootloaderChecksum)
			{
				Result = true;
				pContext->parserResult = BOOTLOADER_RESULT_BSL_MODE_CHECKSUM_INCORRECT;
			}
			else
			{
				/* Success -  Message parsed correctly */
				Result = true;
				switch (pContext->rxBootloaderMsgType)
				{
				case BOOTLOADER_PARSER_MSG_OPERATION_SUCCESSFUL:
					pContext->parserResult = BOOTLOADER_RESULT_BSL_MODE_ACK_RECEIVED;
					break;

				case BOOTLOADER_PARSER_MSG_BSL_LOCKED:
					pContext->parserResult = BOOTLOADER_RESULT_BSL_MODE_BSL_LOCKED;
					break;

				case BOOTLOADER_PARSER_MSG_BSL_PASSWORD_ERROR:
					pContext->parserResult = BOOTLOADER_RESULT_BSL_MODE_PASSWORD_ERROR;
					break;

				case BOOTLOADER_PARSER_MSG_UNKNOWN_COMMAND:
					pContext->parserResult = BOOTLOADER_RESULT_BSL_MODE_UNKNOWN_COMMAND;
					break;

				default:
					pContext->parserResult = BOOTLOADER_RESULT_BSL_MODE_UNKNOWN_COMMAND;
					break;
				}
			}			
			break;



		case S_BOOTLOADER_AWAITING_NORMAL_MODE_SOP:
			if ((uint8_t)AnyCharacter == BOOTLOADER_NORMAL_MODE_SOP)
			{
				pContext->rxBootloaderMsg[pContext->rxBootloaderMsgIndex++] = AnyCharacter;
				next_state = S_BOOTLOADER_AWAITING_NORMAL_MODE_SUCCESS_RESPONSE_MSGID;
			}
			else
			{
				pContext->parserResult = BOOTLOADER_RESULT_BSL_MODE_UNEXPECTED_CHARACTER_RECEIVED;
                next_state = S_BOOTLOADER_AWAITING_ACK;
                Result = true;
			}
			break;

		case S_BOOTLOADER_AWAITING_NORMAL_MODE_SUCCESS_RESPONSE_MSGID:
            if ((uint8_t)AnyCharacter == BOOTLOADER_NORMAL_MODE_ESC)
            {
                pContext->escapeCount++;
            }
            else
            {
                // Reset escape count
                pContext->escapeCount = 0;
            }

            if (pContext->escapeCount < 2)
            {
			    if ((uint8_t)AnyCharacter == BOOTLOADER_NORMAL_MODE_SUCCESS_RESPONSE_MSGID)
			    {
				    pContext->rxBootloaderMsg[pContext->rxBootloaderMsgIndex++] = AnyCharacter;
				    next_state = S_BOOTLOADER_AWAITING_NORMAL_MODE_FLAGS;
			    }
			    else if ((uint8_t)AnyCharacter == BOOTLOADER_NORMAL_MODE_FAIL_RESPONSE_MSGID)
			    {
				    pContext->rxBootloaderMsg[pContext->rxBootloaderMsgIndex++] = AnyCharacter;
				    next_state = S_BOOTLOADER_AWAITING_NORMAL_MODE_FLAGS;
				    pContext->parserResult = BOOTLOADER_RESULT_NORMAL_MODE_NACK_RECEIVED;
			    }	
			    else
			    {
				    pContext->parserResult = BOOTLOADER_RESULT_BSL_MODE_UNEXPECTED_CHARACTER_RECEIVED;
                    next_state = S_BOOTLOADER_AWAITING_ACK;
                    Result = true;
			    }
            }
            else
            {
                // Disregard this character and reset escape count
                pContext->escapeCount = 0;
            }
			break;

		case S_BOOTLOADER_AWAITING_NORMAL_MODE_FLAGS:
            if ((uint8_t)AnyCharacter == BOOTLOADER_NORMAL_MODE_ESC)
            {
                pContext->escapeCount++;
            }
            else
            {
                // Reset escape count
                pContext->escapeCount = 0;
            }

            if (pContext->escapeCount < 2)
            {
			    if ((uint8_t)AnyCharacter == BOOTLOADER_BSL_MODE_FLAGS)
			    {
				    pContext->rxBootloaderMsg[pContext->rxBootloaderMsgIndex++] = AnyCharacter;
				    next_state = S_BOOTLOADER_AWAITING_NORMAL_MODE_PAYLOAD_LENGTH;
			    }
			    else
			    {
				    pContext->parserResult = BOOTLOADER_RESULT_BSL_MODE_UNEXPECTED_CHARACTER_RECEIVED;
                    next_state = S_BOOTLOADER_AWAITING_ACK;
                    Result = true;
			    }
            }
            else
            {
                // Disregard this character and reset escape count
                pContext->escapeCount = 0;
                next_state = S_BOOTLOADER_AWAITING_NORMAL_MODE_FLAGS;
            }
			break;

		case S_BOOTLOADER_AWAITING_NORMAL_MODE_PAYLOAD_LENGTH:
            if ((uint8_t)AnyCharacter == BOOTLOADER_NORMAL_MODE_ESC)
            {
                pContext->escapeCount++;
            }
            else
            {
                // Reset escape count
                pContext->escapeCount = 0;
            }
        
            if(pContext->escapeCount < 2)
            {  
			    pContext->payloadSize = (uint8_t)AnyCharacter;
			    pContext->rxBootloaderMsg[pContext->rxBootloaderMsgIndex++] = AnyCharacter;
			    next_state = S_BOOTLOADER_AWAITING_NORMAL_MODE_PAYLOAD;
            }
            else
            {
                // Disregard this character and reset escape count
                pContext->escapeCount = 0;
                next_state = S_BOOTLOADER_AWAITING_NORMAL_MODE_PAYLOAD_LENGTH;
            }
			break;

		case S_BOOTLOADER_AWAITING_NORMAL_MODE_PAYLOAD:
            if ((uint8_t)AnyCharacter == BOOTLOADER_NORMAL_MODE_ESC)
            {
                pContext->escapeCount++;
            }
            else
            {
                // Reset escape count
                pContext->escapeCount = 0;
            }
            if (pContext->escapeCount < 2)
            {
			    pContext->rxBootloaderMsg[pContext->rxBootloaderMsgIndex++] = AnyCharacter;
			    pContext->payloadSize = pContext->payloadSize - 1;
			    if (pContext->payloadSize > 0)
			    {
				    next_state = S_BOOTLOADER_AWAITING_NORMAL_MODE_PAYLOAD;
			    }
			    else
			    {
				    next_state = S_BOOTLOADER_AWAITING_NORMAL_MODE_CHECKSUM_LSB;
			    }
            }
            else
            {
                // Disregard this character and reset escape count
                pContext->escapeCount = 0;
                next_state = S_BOOTLOADER_AWAITING_NORMAL_MODE_PAYLOAD;
            }
			break;

		case S_BOOTLOADER_AWAITING_NORMAL_MODE_CHECKSUM_LSB:
            if ((uint8_t)AnyCharacter == BOOTLOADER_NORMAL_MODE_ESC)
            {
                pContext->escapeCount++;
            }
            else
            {
                // Reset escape count
                pContext->escapeCount = 0;
            }

            if (pContext->escapeCount < 2)
            {
				pContext->rxBootloaderMsg[pContext->rxBootloaderMsgIndex++] = AnyCharacter;
				next_state = S_BOOTLOADER_AWAITING_NORMAL_MODE_CHECKSUM_MSB;
            }
            else
            {
                // Disregard this character and reset escape count
                pContext->escapeCount = 0;
                next_state = S_BOOTLOADER_AWAITING_NORMAL_MODE_CHECKSUM_LSB;
            }

			break;

		case S_BOOTLOADER_AWAITING_NORMAL_MODE_CHECKSUM_MSB:
            if ((uint8_t)AnyCharacter == BOOTLOADER_NORMAL_MODE_ESC)
            {
                pContext->escapeCount++;
            }
            else
            {
                // Reset escape count
                pContext->escapeCount = 0;
            }

            if (pContext->escapeCount < 2)
            {
				pContext->rxBootloaderMsg[pContext->rxBootloaderMsgIndex++] = AnyCharacter;
				next_state = S_BOOTLOADER_AWAITING_NORMAL_MODE_ESC2;

            }
            else
            {
                // Disregard this character and reset escape count
                pContext->escapeCount = 0;
                next_state = S_BOOTLOADER_AWAITING_NORMAL_MODE_CHECKSUM_MSB;
            }
			break;

		case S_BOOTLOADER_AWAITING_NORMAL_MODE_ESC2:
			if ((uint8_t)AnyCharacter == BOOTLOADER_NORMAL_MODE_ESC)
			{
				pContext->rxBootloaderMsg[pContext->rxBootloaderMsgIndex++] = AnyCharacter;
				next_state = S_BOOTLOADER_AWAITING_NORMAL_MODE_EOP;
			}
			else
			{
				pContext->parserResult = BOOTLOADER_RESULT_BSL_MODE_UNEXPECTED_CHARACTER_RECEIVED;
                next_state = S_BOOTLOADER_AWAITING_ACK;
                Result = true;
			}
			break;

		case S_BOOTLOADER_AWAITING_NORMAL_MODE_EOP:
			if ((uint8_t)AnyCharacter == BOOTLOADER_NORMAL_MODE_EOP)
			{
				pContext->rxBootloaderMsg[pContext->rxBootloaderMsgIndex++] = AnyCharacter;
				next_state = S_BOOTLOADER_AWAITING_ACK;

				/* Success -  Message parsed correctly */
				Result = true;
				if(pContext->parserResult == BOOTLOADER_RESULT_NORMAL_MODE_NACK_RECEIVED)
				{ 
				}
				else
				{ 
					pContext->parserResult = BOOTLOADER_RESULT_NORMAL_MODE_ACK_RECEIVED;
				}
			}
			else
			{
				pContext->parserResult = BOOTLOADER_RESULT_BSL_MODE_UNEXPECTED_CHARACTER_RECEIVED;
			}
			break;


		default:
			pContext->parserResult = BOOTLOADER_BAD;
            next_state = S_BOOTLOADER_AWAITING_ACK;
            Result = true;
			break;

	}

	pContext->bootloaderParserState = next_state;
	return Result;
}
