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
 *  $Revision: #16 $
 *
 */

#ifndef _CMC_BOOTLOADER_FORMATTER_H_
#define _CMC_BOOTLOADER_FORMATTER_H_


#ifdef __cplusplus
extern "C"
{
#endif

#include "cmc_common.h"
#include "cmc_user_supplied_environment.h"

#define MAX_BOOTLOADER_MSG_SIZE 266
#define BSL_PASSWORD_SIZE 57

extern char bsl_password[BSL_PASSWORD_SIZE];


	// satellite controller bsl definitions
#define SAT_COMMS_BSL_FLAGS     0
#define BSL_CRC_GEN_FSL_INDEX   0
#define BSL_ACK                 0x00
#define BSL_HEADER              0x80
#define BSL_BAUD_RATE_115200    0x06

// satellite controller bsl messages
#define BSL_HEADER_SIZE         1
#define BSL_MSG_LENGTH_INDEX    (BSL_HEADER_SIZE)
#define BSL_MSG_LENGTH_SIZE     2
#define BSL_MSG_ID_INDEX        (BSL_HEADER_SIZE+BSL_MSG_LENGTH_SIZE)
#define BSL_MSG_CHKSUM_START    (BSL_MSG_ID_INDEX)
#define BSL_MSG_PAYLOAD_INDEX   (BSL_MSG_ID_INDEX+1)

// satellite controller bsl message lengths
#define DEBUG_BSL_MSG_LEN       12
#define BSL_SYNC_LEN            1
#define BSL_PW_LEN              56
#define BSL_RST_LEN             0
#define BSL_VERS_LEN            0
#define BSL_BAUD_LEN            1
#define BSL_MASS_ERASE_LEN      0
#define BSL_CRC_CHECK_32_LEN    6
#define BSL_TX_BLOCK_32_LEN     6
#define BSL_LOAD_PC_32_LEN      4

// satellite controller bsl message ids
#define BSL_PW_MESSAGE_ID				0x21				
#define BSL_REBOOT_RESET_MESSAGE_ID		0x25
#define BSL_VERS_MESSAGE_ID				0x19
#define BSL_BAUD_MESSAGE_ID				0x52
#define BSL_MASS_ERASE_MESSAGE_ID		0x15
#define BSL_ERASE_SECTOR_32_MESSAGE_ID  0x22
#define BSL_RX_BLOCK_32_MESSAGE_ID		0x20
#define BSL_CRC_CHECK_32_MESSAGE_ID		0x26
#define BSL_TX_BLOCK_32_MESSAGE_ID		0x28
#define BSL_LOAD_PC_32_MESSAGE_ID		0x27

#define BSL_SYNC_ID						0xFF


typedef enum BSL_MESSAGE_TYPE
{
	SAT_COMMS_BSL_SYNC = 0x42,
	SAT_COMMS_BSL_RESET,
	SAT_COMMS_BSL_PW,
	SAT_COMMS_BSL_BAUD,
	SAT_COMMS_BSL_VERS,
	SAT_COMMS_BSL_MASS_ERASE,
	SAT_COMMS_BSL_RX_BLOCK_32,
	SAT_COMMS_BSL_CRC_CHECK_32,
	SAT_COMMS_BSL_TX_BLOCK_32,
	SAT_COMMS_BSL_LOAD_PC_32,
	SAT_COMMS_BSL_ERASE_SECTOR_32,

} BSL_MESSAGE_TYPE;


void cmcFormatBSLPasswordMessage(void* pUserContext, USER_SUPPLIED_ENVIRONMENT_CALCULATE_CRC16_CCITT_TYPE pFN_Calculate_CRC16_CCITT, char* pTxBuffer, uint32_t* pTxBufferSize);
void cmcFormatBSLBaudMessage(void* pUserContext, USER_SUPPLIED_ENVIRONMENT_CALCULATE_CRC16_CCITT_TYPE pFN_Calculate_CRC16_CCITT, char* pTxBuffer, uint32_t* pTxBufferSize);
void cmcFormatBSLVersionMessage(void* pUserContext, USER_SUPPLIED_ENVIRONMENT_CALCULATE_CRC16_CCITT_TYPE pFN_Calculate_CRC16_CCITT, char* pTxBuffer, uint32_t* pTxBufferSize);
void cmcFormatBSLMassEraseMessage(void* pUserContext, USER_SUPPLIED_ENVIRONMENT_CALCULATE_CRC16_CCITT_TYPE pFN_Calculate_CRC16_CCITT, char* pTxBuffer, uint32_t* pTxBufferSize);
void cmcFormatBSLRxBlock32Message(void* pUserContext, USER_SUPPLIED_ENVIRONMENT_CALCULATE_CRC16_CCITT_TYPE pFN_Calculate_CRC16_CCITT, char* pTxBuffer, uint32_t* pTxBufferSize, uint32_t DestinationAddress, char* pPayloadFromHost, uint32_t payloadFromHostSize);
void cmcFormatBSLCRCCheck32Message(void* pUserContext, USER_SUPPLIED_ENVIRONMENT_CALCULATE_CRC16_CCITT_TYPE pFN_Calculate_CRC16_CCITT, char* pTxBuffer, uint32_t* pTxBufferSize, uint32_t DestinationAddress, char* pPayloadFromHost, uint32_t payloadFromHostSize);
void cmcFormatBSLLoadPC32Message(void* pUserContext, USER_SUPPLIED_ENVIRONMENT_CALCULATE_CRC16_CCITT_TYPE pFN_Calculate_CRC16_CCITT, char* pTxBuffer, uint32_t* pTxBufferSize, uint32_t LoadAddress);
void cmcFormatBSLSyncMessage(char* pTxBuffer, uint32_t* pTxBufferSize);

#ifdef __cplusplus
}
#endif


#endif /* _CMC_BOOTLOADER_FORMATTER_H_ */







