/******************************************************************************
 * Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
 * SPDX-License-Identifier: MIT
 *******************************************************************************/

#ifndef SRC_VMC_VMC_UPDATE_SC_H_
#define SRC_VMC_VMC_UPDATE_SC_H_


#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include "cl_msg.h"

#define ULONG_MAX 					0xFFFFFFFFUL
#define FW_UPDATE_TRIGGER_0				( 0x01UL )

#define DELAY_MS(x)   					((x)     /portTICK_PERIOD_MS )
#define RCV_TIMEOUT_MS(x)  				((x)     /portTICK_PERIOD_MS )

#define SC_UPDATE_MAX_RETRY_COUNT		(10u)
#define MODE_VER_MAX_RETRY_COUNT		(3u)

#define BSL_VERSION_REQ					(0x06)
#define BSL_VERSION_RESP				(0x11)
#define BSL_REBOOT_RESET_REQ			(0x06)
#define BSL_LOAD_PC_REQ					(0x0A)
#define BSL_DATA_TX_32_RESP				(0x08)
#define BSL_MASS_ERASE_REQ				(0x06)
#define BSL_MASS_ERASE_RESP				(0x08)
#define BSL_UNLOCK_PASSWORD_REQ			(0x3E)
#define BSL_UNLOCK_PASSWORD_RESP		(0x08)
#define BSL_SYNCED_REQ					(0x01)
#define BSL_SYNCED_RESP					(0x01)
#define BSL_CRC_RESQ					(0x0C)
#define BSL_CRC_RESP					(0x09)
#define BSL_DATA_WRITE_RESP_FIRST_CHAR	(0x3B)
#define BSL_DATA_WRITE_RESP_SEC_CHAR	(0x00)
#define BSL_CRC_SUCCESS_RESP			(0x3A)
#define BSL_SYNC_SUCCESS				(0x00)
#define SC_BSL_SYNCED_REQ				(0x01)
/* Size of comms error message response */
#define SC_BSL_SYNCED_RESP				(0x0B)

#define SC_ENABLE_BSL_REQ				(0x09)
#define SC_ENABLE_BSL_RESP				(0x0A)

#define CMD_RX_DATA_BLOCK_32			(0x20)
#define CMD_RX_PASSWORD					(0x21)
#define CMD_MASS_ERASE					(0x15)
#define CMD_REBOOT_RESET				(0x25)
#define CMD_CRC_CHECK_32				(0x26)
#define CMD_LOAD_PC_32					(0x27)
#define CMD_MSP_GET_STATUS				(0x31)
#define CMD_SC_ENABLE_BSL				(0x32)
#define CMD_TX_BSL_VERSION				(0x19)

#define MSP_IN_SC_MODE					(0x02)
#define MSP_IN_BSL_MODE					(0x01)

#define BSL_MSG_HEADER					(0x80)

#define SECTION_START					(0x40)
#define FILE_TERMINATION_CHAR			(0x71)
#define SPACE							(0x20)
#define NEW_LINE						(0x0A)

#define MAJOR							(0x00)
#define MINOR							(0x01)
#define REVISION						(0x02)

#define UPDATE_REQUIRED					(0x01)
#define NO_UPDATE_REQUIRED				(0x00)

#define BSL_MAX_DATA_SIZE				(266u)

#define SC_HEADER_SIZE					(0x0B)
#define SC_RES_HEADER_SIZE				(0x05)
#define SC_TOT_HEADER_SIZE				(SC_HEADER_SIZE + SC_RES_HEADER_SIZE)
#define SC_HEADER_MSG					("VERSAL_SCFW")

#define BYTES_TO_READ					(0x01)
#define BSL_DATA_PACKET_CRC_SIZE		(0x02)


typedef struct efpt_sc
{
	u32 start_address;
	u32 size;
} efpt_sc_t;


typedef enum
{
	PARSE_ADDRESS = 0,
	PARSE_DATA
}sc_parse_states;


typedef enum
{
	STATUS_SUCCESS = 0,
	STATUS_FAILURE,
	STATUS_IN_PROGRESS
}upgrade_status_t;

typedef enum
{
	SC_STATE_IDLE = 0,
	SC_ENABLE_BSL,
	BSL_SYNCED,
	BSL_UNLOCK_PASSWORD,
	BSL_MASS_ERASE,
	BSL_SECTOR_ERASE,
	BSL_DATA_TX_32,
	BSL_CRC_RX_32,
	BSL_LOAD_PC_32,
	MSP_GET_STATUS,
	BSL_REBOOT_RESET,
	TX_BSL_VERSION,
	SC_BSL_SYNC
}upgrade_state_t;


typedef enum scUpateError_e
{
	SC_UPDATE_NO_ERROR = 0xE0,
	SC_UP_TO_DATE_NO_UPDATE_REQ,
	SC_UPDATE_ERROR_SC_BSL_SYNC_FAILED,
	SC_UPDATE_ERROR_EN_BSL_FAILED,
	SC_UPDATE_ERROR_VMC_BSL_SYNC_FAILED,
	SC_UPDATE_ERROR_BSL_UNLOCK_PASSWORD_FAILED,
	SC_UPDATE_ERROR_ERASE_FAILED,
	SC_UPDATE_ERROR_INVALID_SC_SYMBOL_FOUND,
	SC_UPDATE_ERROR_POST_CRC_FAILED,
	SC_UPDATE_ERROR_INVALID_BSL_RESP,
	SC_UPDATE_ERROR_NO_VALID_FPT_SC_FOUND,
	SC_UPDATE_ERROR_FAILED_TO_GET_BSL_VERSION,
	SC_UPDATE_ERROR_OPEARTION_TIMEDOUT
}scUpateError_t;


void VMC_Parse_Fpt_SC(u32 addr_location, u8 *bsl_send_data_pkt , u16 *pkt_length);
void VMC_Parse_Fpt_SC_Version(u32 addr_location, u8 *bsl_send_data_pkt);
bool VMC_Read_SC_FW(void);
u8 Get_SC_Checksum(void);
u8 Check_Received_SC_Header(void *ptr1, void *ptr2, u8 len);
upgrade_status_t matchCRC_postWrite(unsigned int writeAdd);
void UpdateSCFW();
void VMC_Get_Fpt_SC_Version(cl_msg_t *msg);

#endif /* SRC_VMC_VMC_UPDATE_SC_H_ */
