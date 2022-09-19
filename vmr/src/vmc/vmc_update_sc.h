/******************************************************************************
 * Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
 * SPDX-License-Identifier: MIT
 *******************************************************************************/

#ifndef SRC_VMC_VMC_UPDATE_SC_H_
#define SRC_VMC_VMC_UPDATE_SC_H_


#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>

#define ULONG_MAX			0xFFFFFFFFUL
#define FW_UPDATE_TRIGGER_0		( 0x01UL )

#define DELAY_MS(x)			((x) /portTICK_PERIOD_MS )
#define RCV_TIMEOUT_MS(x)		((x) /portTICK_PERIOD_MS )

#define SC_UPDATE_MAX_RETRY_COUNT	(5u)
#define MODE_VER_MAX_RETRY_COUNT	(3u)

#define BSL_VERSION_REQ_SIZE		(0x06)
#define BSL_VERSION_RESP_SIZE		(0x11)
#define BSL_REBOOT_RESET_REQ_SIZE	(0x06)
#define BSL_LOAD_PC_REQ_SIZE		(0x0A)
#define BSL_DATA_TX_32_RESP_SIZE	(0x08)
#define BSL_MASS_ERASE_REQ_SIZE		(0x06)
#define BSL_MASS_ERASE_RESP_SIZE	(0x08)
#define BSL_UNLOCK_PASSWORD_REQ_SIZE	(0x3E)
#define BSL_UNLOCK_PASSWORD_RESP_SIZE	(0x08)
#define BSL_SYNCED_REQ_SIZE		(0x01)
#define BSL_SYNCED_RESP_SIZE		(0x01)
#define BSL_CRC_REQ_SIZE		(0x0C)
#define BSL_CRC_RESP_SIZE		(0x09)
#define BSL_RESP_CMD_CHAR		(0x3B)
#define BSL_RESP_MSG_CHAR		(0x00)
#define BSL_CRC_SUCCESS_RESP		(0x3A)
#define BSL_SYNC_REQ_CHAR		(0xFF)
#define BSL_SYNC_SUCCESS		(0x00)
#define SC_BSL_SYNCED_REQ_SIZE		(0x01)
#define SC_BSL_SYNCED_RESP_SIZE		(0x0B)
#define BSL_UNKNOWN_MSG_RESP_SIZE	(0x08)
#define BSL_LOAD_PC_RESP_SIZE		(0x01)
#define BSL_LOAD_PC_SUCCESS_RESP	(0x00)

#define SC_ENABLE_BSL_REQ_SIZE		(0x09)
#define SC_ENABLE_BSL_RESP_SIZE		(0x0B)

#define CMD_RX_DATA_BLOCK_32		(0x20)
#define CMD_RX_PASSWORD			(0x21)
#define CMD_MASS_ERASE			(0x15)
#define CMD_REBOOT_RESET		(0x25)
#define CMD_CRC_CHECK_32		(0x26)
#define CMD_LOAD_PC_32			(0x27)
#define CMD_MSP_GET_STATUS		(0x31)
#define CMD_SC_ENABLE_BSL		(0x32)
#define CMD_TX_BSL_VERSION		(0x19)

#define MSP_IN_SC_MODE			(0x02)
#define MSP_IN_BSL_MODE			(0x01)

#define BSL_MSG_HEADER			(0x80)
#define BSL_RESP_CMD_OFFSET		(4u)
#define BSL_RESP_MSG_OFFSET		(5u)
#define BSL_RESP_BUILD_ID_OFFSET_H	(13u)
#define BSL_RESP_BUILD_ID_OFFSET_L	(14u)
#define BSL_RESP_LEN_OFFSET_L		(2u)
#define BSL_RESP_LEN_OFFSET_H		(3u)
#define BSL_RESP_CAL_CHKSUM_H		(6u)
#define BSL_RESP_CAL_CHKSUM_L		(5u)

#define SECTION_START			(0x40)
#define FILE_TERMINATION_CHAR		(0x71)
#define SPACE				(0x20)
#define NEW_LINE			(0x0A)

#define MAJOR				(0x00)
#define MINOR				(0x01)
#define REVISION			(0x02)

#define UPDATE_REQUIRED			(0x01)
#define NO_UPDATE_REQUIRED		(0x00)

#define BSL_MAX_DATA_SIZE		(266u)
#define BSL_MAX_RCV_DATA_SIZE		(32u)
#define BSL_VERSION_BYTE_L		(0x0D)
#define BSL_VERSION_BYTE_H		(0x00)

#define SC_HEADER_SIZE			(0x0B)
#define SC_RES_HEADER_SIZE		(0x05)
#define SC_TOT_HEADER_SIZE		(SC_HEADER_SIZE + SC_RES_HEADER_SIZE)
#define SC_HEADER_MSG			("VERSAL_SCFW")

#define BYTES_TO_READ			(0x01)
#define BSL_DATA_PACKET_CRC_SIZE	(0x02)

#define MAX_SC_VERSION_SIZE		(0x03)

#define SC_VER_ADDR_W_CHKSUM		(0x195)
#define SC_VER_MAJ_START_ADDR		(0x7E000)
#define SC_VER_MIN_START_ADDR		(0x7E002)
#define SC_VER_REV_START_ADDR		(0x7E004)

#define SC_VALID			(0x00)
#define SC_INVALID			(0x01)
#define FAILED				(-1)
#define SUCCESS				(0)

typedef struct efpt_sc
{
	u32 start_address;
	u32 size;
} efpt_sc_t;

typedef enum sc_parse_states_e
{
	eParse_Address = 0,
	eParse_Data
} sc_parse_states_t;

typedef enum sc_update_status_e
{
	eStatus_Success = 0,
	eStatus_Failure,
	eStatus_In_Progress
} sc_update_status_t;

typedef enum sc_update_state_e
{
	eSc_State_Idle = 0,
	eSc_Enable_Bsl,
	eBsl_Synced,
	eBsl_Unlock_Password,
	eBsl_Mass_Erase,
	eBsl_Sector_Erase,
	eBsl_Data_Tx_32,
	eBsl_Crc_Rx_32,
	eBsl_Load_Pc_32,
	eMsp_Get_Status,
	eBsl_Reboot_Reset,
	eTx_Bsl_Version,
	eSc_Bsl_Sync
} sc_update_state_t;

typedef enum sc_update_error_e
{
	eSc_Update_No_Error = 0x00,
	eSc_Up_To_Date_No_Update_Req = 0xE0,
	eSc_Update_Error_Sc_Bsl_Sync_Failed,
	eSc_Update_Error_En_Bsl_Failed,
	eSc_Update_Error_Vmc_Bsl_Sync_Failed,
	eSc_Update_Error_Bsl_Unlock_Password_Failed,
	eSc_Update_Error_Erase_Failed,
	eSc_Update_Error_Invalid_Sc_Symbol_Found,
	eSc_Update_Error_Post_Crc_Failed,
	eSc_Update_Error_Invalid_Bsl_Resp,
	eSc_Update_Error_No_Valid_Fpt_Sc_Found,
	eSc_Update_Error_Failed_To_Get_Bsl_Version,
	eSc_Update_Error_Operation_Timedout,
} sc_update_error_t;


#endif /* SRC_VMC_VMC_UPDATE_SC_H_ */
