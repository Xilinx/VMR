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
 *  $Change: 2755078 $
 *  $Date: 2020/01/17 $
 *  $Revision: #18 $
 *
 */




#ifndef _CMC_BOOTLOADER_PARSER_H_
#define _CMC_BOOTLOADER_PARSER_H_


#ifdef __cplusplus
extern "C"
{
#endif

#include "cmc_common.h"
#include "cmc_user_supplied_environment.h"
#include "cmc_watchpoint.h"

#define BOOTLOADER_NORMAL_MODE_ESC 0x5C
#define BOOTLOADER_NORMAL_MODE_SOP 0x02
#define BOOTLOADER_NORMAL_MODE_EOP 0x03
#define BOOTLOADER_NORMAL_MODE_SUCCESS_RESPONSE_MSGID 0xFE
#define BOOTLOADER_NORMAL_MODE_FAIL_RESPONSE_MSGID 0xFF
#define BOOTLOADER_BSL_MODE_FLAGS 0x00
#define BOOTLOADER_BSL_MODE_PAYLOAD_LENGTH 0x01
#define BOOTLOADER_BSL_MODE_PAYLOAD 0x01
#define BOOTLOADER_BSL_MODE_CHECKSUM_LSB 0x30
#define BOOTLOADER_BSL_MODE_CHECKSUM_MSB 0x03

#define BOOTLOADER_BSL_ACK					0x00
#define BOOTLOADER_BSL_HEADER_INCORRECT		0x51
#define BOOTLOADER_BSL_CHECKSUM_INCORRECT	0x52
#define BOOTLOADER_BSL_PACKET_SIZE_ZERO		0x53
#define BOOTLOADER_BSL_PACKET_SIZE_TOO_BIG	0x54
#define BOOTLOADER_BSL_UNKNOWN_ERROR		0x55
#define BOOTLOADER_BSL_UNKNOWN_BAUD_RATE	0x56

#define BOOTLOADER_HDR 0x80

#define BOOTLOADER_DATA_CMD 0x3A
#define BOOTLOADER_MSG_CMD  0x3B

#define MAX_BOOTLOADER_PARSER_MSG_SIZE 17


#define BOOTLOADER_MSG_OPERATION_SUCCESSFUL 0x00
#define BOOTLOADER_MSG_BSL_LOCKED			0x04
#define BOOTLOADER_MSG_BSL_PASSWORD_ERROR	0x05
#define BOOTLOADER_MSG_UNNOWN_COMMAND		0x07


typedef enum BOOTLOADER_PARSER_STATE_TYPE
{
	S_BOOTLOADER_AWAITING_ACK = 0,
	S_BOOTLOADER_AWAITING_HDR,
	S_BOOTLOADER_AWAITING_LENGTH_L,
	S_BOOTLOADER_AWAITING_LENGTH_H,
	S_BOOTLOADER_AWAITING_CMD,
	S_BOOTLOADER_AWAITING_MSG,
	S_BOOTLOADER_AWAITING_CHECKSUM_L,
	S_BOOTLOADER_AWAITING_CHECKSUM_H,
	S_BOOTLOADER_AWAITING_DATA,

	S_BOOTLOADER_AWAITING_NORMAL_MODE_SOP,
	S_BOOTLOADER_AWAITING_NORMAL_MODE_SUCCESS_RESPONSE_MSGID,
	S_BOOTLOADER_AWAITING_NORMAL_MODE_FLAGS,
	S_BOOTLOADER_AWAITING_NORMAL_MODE_PAYLOAD_LENGTH,
	S_BOOTLOADER_AWAITING_NORMAL_MODE_PAYLOAD,
	S_BOOTLOADER_AWAITING_NORMAL_MODE_CHECKSUM_LSB,
	S_BOOTLOADER_AWAITING_NORMAL_MODE_CHECKSUM_MSB,
	S_BOOTLOADER_AWAITING_NORMAL_MODE_ESC2,
	S_BOOTLOADER_AWAITING_NORMAL_MODE_EOP,

	S_BOOTLOADER_MAX_STATES

} BOOTLOADER_PARSER_STATE_TYPE;

typedef enum BOOTLOADER_PARSER_RESULT_TYPE
{
	BOOTLOADER_BAD = 0,
	BOOTLOADER_OK,
	BOOTLOADER_RESULT_BSL_MODE_UNEXPECTED_CHARACTER_RECEIVED,
	BOOTLOADER_RESULT_BSL_MODE_HDR_NOT_RECEIVED,
	BOOTLOADER_RESULT_BSL_MODE_CMD_INVALID,
	BOOTLOADER_MSG,
	BOOTLOADER_CHECKSUM_L,
	BOOTLOADER_CHECKSUM_H,
	BOOTLOADER_DATA,
	BOOTLOADER_RESULT_NORMAL_MODE_ACK_RECEIVED,
	BOOTLOADER_RESULT_NORMAL_MODE_NACK_RECEIVED,
	BOOTLOADER_RESULT_OPERATION_SUCCESSFUL,
	BOOTLOADER_RESULT_BSL_MODE_ACK_RECEIVED,
	BOOTLOADER_RESULT_BSL_MODE_BSL_LOCKED,
	BOOTLOADER_RESULT_BSL_MODE_PASSWORD_ERROR,
	BOOTLOADER_RESULT_BSL_MODE_UNKNOWN_COMMAND,
	BOOTLOADER_RESULT_BSL_MODE_RECEIVED_INCORRECT_HEADER,
	BOOTLOADER_RESULT_BSL_MODE_CHECKSUM_INCORRECT,
	BOOTLOADER_RESULT_BSL_MODE_PACKET_SIZE_ZERO,
	BOOTLOADER_RESULT_BSL_MODE_PACKET_SIZE_TOO_BIG,
	BOOTLOADER_RESULT_BSL_MODE_UNKNOWN_ERROR,
	BOOTLOADER_RESULT_BSL_MODE_UNKNOWN_BAUD_RATE,
	BOOTLOADER_RESULT_BSL_BOOTLOADER_MAX_STATES

} BOOTLOADER_PARSER_RESULT_TYPE;

typedef enum BOOTLOADER_PARSER_MSG_TYPE
{
	BOOTLOADER_PARSER_MSG_OPERATION_SUCCESSFUL = 0,
	BOOTLOADER_PARSER_MSG_BSL_LOCKED,
	BOOTLOADER_PARSER_MSG_BSL_PASSWORD_ERROR,
	BOOTLOADER_PARSER_MSG_UNKNOWN_COMMAND

} BOOTLOADER_PARSER_MSG_TYPE;




typedef struct BOOTLOADER_PARSER_CONTEXT_TYPE
{
	BOOTLOADER_PARSER_STATE_TYPE	bootloaderParserState;
	char							rxBootloaderMsg[MAX_BOOTLOADER_PARSER_MSG_SIZE];
	uint8_t							rxBootloaderMsgIndex;
	uint16_t						rxBootloaderMsgBodyLength;
	uint16_t						rxBootloaderChecksum;
	uint16_t						rxBootloaderCalculatedChecksum;
	uint8_t							payloadSize;
	uint8_t							rxBootloaderMsgIDReceived;
	BOOTLOADER_PARSER_MSG_TYPE		rxBootloaderMsgType;
	BOOTLOADER_PARSER_RESULT_TYPE	parserResult;
    USER_SUPPLIED_ENVIRONMENT_CALCULATE_CRC16_CCITT_TYPE pFN_Calculate_CRC16_CCITT;
	void*							pUserContext;
    uint8_t                         escapeCount;

} BOOTLOADER_PARSER_CONTEXT_TYPE;

	
void cmc_bootloader_parser_initialize(BOOTLOADER_PARSER_CONTEXT_TYPE* pContext, USER_SUPPLIED_ENVIRONMENT_CALCULATE_CRC16_CCITT_TYPE pFN_Calculate_CRC16_CCITT, void* pUserContext);
bool cmc_bootloader_message_parse(BOOTLOADER_PARSER_CONTEXT_TYPE* pContext, char AnyCharacter, uint8_t lastCommandSent);


	

#ifdef __cplusplus
}
#endif


#endif /* _CMC_BOOTLOADER_PARSER_H_ */








