/******************************************************************************
 * Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
 * SPDX-License-Identifier: MIT
 *******************************************************************************/

#include "FreeRTOS.h"

#include "cl_mem.h"
#include "cl_vmc.h"
#include "cl_msg.h"
#include "cl_rmgmt.h"
#include "vmc_api.h"
#include "cl_uart_rtos.h"
#include "vmc_update_sc.h"
#include "vmc_sc_comms.h"

static sc_update_status_t update_status = eStatus_Success;
static sc_update_state_t update_state = eSc_State_Idle;
static sc_update_error_t update_error = eSc_Update_No_Error;

extern uart_rtos_handle_t uart_vmcsc_log;

/* Global structures */
static efpt_sc_t fpt_sc_loc;

u8 fpt_sc_version[MAX_SC_VERSION_SIZE] = {0x00}; /* Buffer to keep parsed SC version */
static u8 bsl_send_data_pkt[BSL_MAX_DATA_SIZE] = {0x00}; /* UART Transmit buffer */
static u32 data_ptr = 0; /* Current data pointer from which we should parse from the sc start address*/
static u8 addr_flag = 0x00; /* To check if we are parsing an address from fpt sc raw data */
static u16 data_crc = 0x00; /* Data packet CRC (per transaction) */
static u32 prev_length = 0x00; /* Length of a last data packet that we sent to BSL. */
static u32 total_length = 0x00; /* Length of a data packet (per transaction) that we send to BSL. */
static bool all_pkt_sent = false; /* To keep track of packet completion */
static bool fpt_sc_valid = false; /* To keep track of fpt sc validity */
static u32 parsed_sc_flash_addr = 0x00; /* SC flash section start address, to which we are going to write the data */

/* Variables will keep track of SC update progress */
static s32 update_progress = 0;
static u32 curr_prog = 0;
static u32 max_data_slot = 0;

static u8 rcv_bufr[BSL_MAX_RCV_DATA_SIZE] = {0x00}; /* UART Receiver buffer */
static u32 rcvd_byte_count = 0x00; /* UART received byte count */

//extern void rmgmt_extension_fpt_query(struct cl_msg *msg);


int cl_vmc_scfw_program_progress(void)
{
	return (int)update_progress;
}

static inline void vmc_read_data32(u32 *addr, u32 *data, size_t sz)
{
	u32 i = 0;

	for (i = 0; i < sz; ++i) {
		*(data + i) = IO_SYNC_READ32((UINTPTR )(addr + i));
	}
}

static u16 crc16(u8 *pMsg, u32 size)
{
	u16 crc = 0xFFFF;
	u32 i = 0;

	for (i = 0; i < size; i++) {
		u16 x = 0;

		x = ((crc >> 8) ^ pMsg[i]) & 0xff;
		x ^= x >> 4;
		crc = (crc << 8) ^ (x << 12) ^ (x << 5) ^ x;
	}

	return crc;
}

static inline u8 char_to_hex_addr(u8 C)
{
	if (C >= '0' && C <= '9') {
		C = C - '0';
	} else if (C >= 'a' && C <= 'f') {
		C = C - 'a' + 10;
	}

	return (C & 0x0F);
}

static inline u8 char_to_hex(u8 C)
{
	if (C >= '0' && C <= '9') {
		C = C - '0';
	} else if (C >= 'A' && C <= 'F') {
		C = C - 'A' + 10;
	}

	return (C & 0x0F);
}

/**
  * @brief  Validates BSL response.
  * @param  buffer: Pointer to response buffer.
  * @param  length: Total length of received data.
  * @retval Status
  *
  */

static s32 vmc_validate_bsl_resp(u8 *buffer, u16 length)
{
	s32 status = FAILED;
	u16 data_len = 0;
	u16 rcvd_crc = 0;
	u16 calc_crc = 0;

	if (buffer[0] == 0x00 && buffer[1] == BSL_MSG_HEADER) {
		data_len = ((buffer[BSL_RESP_LEN_OFFSET_H] << 8) | buffer[BSL_RESP_LEN_OFFSET_L]);

		rcvd_crc = ((buffer[length - 1] << 8) | (buffer[length - 2]));
		calc_crc = crc16(&buffer[BSL_RESP_CMD_OFFSET], data_len);

		if (rcvd_crc == calc_crc) {
			status = SUCCESS;
		}
	}

	return status;
}

/**
  * @brief  Validates BSL response.
  * @param  ptr1: Pointer to string_1.
  * @param  ptr2: Pointer to string_2.
  * @param  len: Total length to compare.
  * @retval Status
  *
  */

static u8 check_received_sc_header(void *ptr1, void *ptr2, u8 len)
{
	u8 *buf1 = (u8 *)ptr1;
	u8 *buf2 = (u8 *)ptr2;

	while (len > 0) {
		if (*buf1 != *buf2) {
			return (*buf1 - *buf2);
		}

		len--;
		buf1++;
		buf2++;
	}

	return 0;
}

static u8 get_sc_checksum(void)
{
	/* Get the SC version and compare with the backup one and return accordingly */
	u8 ret_val = UPDATE_REQUIRED;

	return ret_val;
}

/**
  * @brief  Parses the raw SC from fpt and gets the available SC version in shell.
  * @param  addr_location: FPT sc address version location.
  * @param  versionbuff: Pointer to data buffer
  * @retval None
  *
  * NOTE: Please refer to the parse_fpt_sc function description for the parsing details.
  */

void parse_fpt_sc_version(u32 addr_location, u8 *versionbuff)
{
	u8 state = eParse_Address;
	u8 complete_flag = 0;
	u8 version_ptr = 0x00;
	u8 Msg = 0x00;
	bool version_flag = false;

	while (complete_flag == 0) {
		switch (state) {
		case eParse_Address:

			Msg = IO_SYNC_READ8(addr_location + data_ptr);
			data_ptr += BYTES_TO_READ;

			if ((addr_flag == true) && (Msg != SECTION_START)
					&& (Msg != FILE_TERMINATION_CHAR)) {
				addr_flag = false;
				parsed_sc_flash_addr = 0;
				while (Msg != NEW_LINE) {
					Msg = char_to_hex_addr(Msg);
					parsed_sc_flash_addr = ((parsed_sc_flash_addr << (4)) | (Msg & 0x0F));
					Msg = IO_SYNC_READ8(addr_location + data_ptr);
					data_ptr += BYTES_TO_READ;
				}

				if ((parsed_sc_flash_addr == SC_VER_MAJ_START_ADDR)
						|| (parsed_sc_flash_addr == SC_VER_MIN_START_ADDR)
						|| (parsed_sc_flash_addr == SC_VER_REV_START_ADDR)) {
					version_flag = true;
				}

				state = eParse_Data;
			} else {
				if (Msg == SECTION_START) {
					addr_flag = true;
				} else if (Msg == FILE_TERMINATION_CHAR) {
					complete_flag = 1;
				} else {
					state = eParse_Data;
				}
			}
			break;

		case eParse_Data:

			Msg = IO_SYNC_READ8(addr_location + data_ptr);
			data_ptr += BYTES_TO_READ;

			while ((Msg != SECTION_START) && (Msg != FILE_TERMINATION_CHAR)) {
				if (version_ptr >= sizeof(struct fpt_sc_version)) {
					VMC_ERR("ptr %d is overflow fpt_sc_version", version_ptr);
					break;
				}
				if ((Msg != NEW_LINE) && (Msg != SPACE)) {
					Msg = char_to_hex(Msg);
					if ((version_flag) && (version_ptr <= (MAX_SC_VERSION_SIZE - 1)))
						versionbuff[version_ptr] = (Msg & 0x0F);

					if ((Msg == NEW_LINE) || (Msg == SPACE)
							|| (Msg == SECTION_START)) {
						addr_flag = true;
						version_flag = false;
						state = eParse_Address;
						break;
					}

					Msg = IO_SYNC_READ8(addr_location + data_ptr);
					data_ptr += BYTES_TO_READ;
					Msg = char_to_hex(Msg);
					if ((version_flag) && (version_ptr <= (MAX_SC_VERSION_SIZE - 1))) {
						versionbuff[version_ptr] = ((versionbuff[version_ptr] << 4) | (Msg & 0x0F));
						version_ptr++;
					}
				}

				Msg = IO_SYNC_READ8(addr_location + data_ptr);
				data_ptr += BYTES_TO_READ;
			}
			if (Msg == SECTION_START) {
				addr_flag = true;
				state = eParse_Address;
				version_flag = false;
			} else if (Msg == FILE_TERMINATION_CHAR) {
				complete_flag = 1;
			}
			break;

		default:
			VMC_ERR("Must not come here. ");
			break;
		}
	}
}

/**
  *   ==============================================================================
  *          ##### A Detailed Description of What SC Looks Like in FPT. #####
  *   ==============================================================================
  *
  *   TI-TXT Hex Format:
  *
  *   1. The TI-TXT hex format supports 16-bit hexadecimal data.
  *   2. It consists of section start addresses, data byte, and an end-of-file character.
  *
  *  These restrictions apply:
  *   1. The number of sections is unlimited.
  *   2. Each hexadecimal start address must be even.
  *   3. Each line must have 16 data bytes, except the last line of a section.
  *   4. Data bytes are separated by a single space.
  *   5. The end-of-file termination tag q is mandatory.
  *
  *  The data record contains the following information:
  *
  *	Item	Description
  *	@ADDR	Hexadecimal start address of a section
  *	DATAn	Hexadecimal data byte
  * 	 q	End-of-file termination character
  *
  *    	Section Start ----> @ADDR1
  *			 +- DATA01 DATA02 ........ DATA16
  *	  Data Bytes --->|  DATA17 DATA18 ........ DATA32
  *			 +- DATAm ........ DATAn
  *
  *	Section Start ----> @ADDR2
  *	Data Bytes    ----> DATA01 ............... DATAn
  *	End-of-file char -> q
  *
  *Example:
  *
  *	Raw SC in FPT:
  *	@0200
  *	0C 48 08 B5 80 F3 08 88 09 49 08 68 40 F4 70 00
  *	08 60 00 BF 00 BF 20 F0 A1 F8 08 B1 1E F0 18 FC
  *	@7f000
  *	38 11 31 51 00 00 00 00 00 00 00 00 68 1C 02 00
  *	78 03 00 00 00 00 00 00
  *	q
  *
  *	The binary file looks like:
  *	40303230300A3043203438203038204235203830204633203038203838203039203439203038203638203430204634203730203030200A
  *	3038203630203030204246203030204246203230204630204131204638203038204231203145204630203138204643200A403766303030200A
  *	3338203131203331203531203030203030203030203030203030203030203030203030203638203143203032203030200A
  *	3738203033203030203030203030203030203030203030200A71
  */

/**
  * @brief  Parse the raw SC from fpt and frame packets which can be sent to BSL.
  * @param  addr_location: FPT sc address start location
  * @param  bsl_send_data_pkt: Pointer to data buffer
  * @param  pkt_length: Pointer to the length of the current data packet. 
  * @retval None
  *
  * NOTE: Each packet contains 256Bytes of data and 10Bytes of packet header
  * and footer (which contains the packet crc, address, cmd and length etc).
  * Please refer to the TI SLAU622I Data Sheet for BSL packet descriptions.
  */

void parse_fpt_sc(u32 addr_location, u8 *bsl_send_data_pkt, u32 *pkt_length)
{
	u8 state = eParse_Address;
	u8 complete_flag = 0;
	u8 Msg = 0x00;
	u16 crc_val = 0x00;
	u32 length = 0;

	while (complete_flag == 0) {
		switch (state) {
		case eParse_Address:

			Msg = IO_SYNC_READ8(addr_location + data_ptr);
			data_ptr += BYTES_TO_READ;
			curr_prog += 1;
			if (curr_prog >= max_data_slot) {
				update_progress += 1;
				curr_prog = 0;
			}

			if ((addr_flag == true) && (Msg != SECTION_START)
					&& (Msg != FILE_TERMINATION_CHAR)) {
				addr_flag = false;
				parsed_sc_flash_addr = 0;
				while (Msg != NEW_LINE) {
					Msg = char_to_hex_addr(Msg);
					parsed_sc_flash_addr = ((parsed_sc_flash_addr << (4)) | (Msg & 0x0F));
					Msg = IO_SYNC_READ8(addr_location + data_ptr);
					data_ptr += BYTES_TO_READ;
					curr_prog += 1;
					if (curr_prog >= max_data_slot) {
						update_progress += 1;
						curr_prog = 0;
					}
				}
				VMC_LOG("Addr : %x", parsed_sc_flash_addr);
				bsl_send_data_pkt[4] = parsed_sc_flash_addr & 0xFF;
				bsl_send_data_pkt[5] = parsed_sc_flash_addr >> 8 & 0xFF;
				bsl_send_data_pkt[6] = parsed_sc_flash_addr >> 16 & 0xFF;
				bsl_send_data_pkt[7] = parsed_sc_flash_addr >> 24 & 0xFF;
				length += 8;

				state = eParse_Data;
			} else {
				if (Msg == SECTION_START) {
					addr_flag = true;
				} else if (Msg == FILE_TERMINATION_CHAR) {
					complete_flag = 1;
					all_pkt_sent = true;
					update_progress = 99;
				} else {
					parsed_sc_flash_addr = parsed_sc_flash_addr + prev_length;
					bsl_send_data_pkt[4] = parsed_sc_flash_addr & 0xFF;
					bsl_send_data_pkt[5] = parsed_sc_flash_addr >> 8 & 0xFF;
					bsl_send_data_pkt[6] = parsed_sc_flash_addr >> 16 & 0xFF;
					bsl_send_data_pkt[7] = parsed_sc_flash_addr >> 24 & 0xFF;
					length += 8;
					state = eParse_Data;
				}
			}
			break;

		case eParse_Data:

			Msg = IO_SYNC_READ8(addr_location + data_ptr);
			data_ptr += BYTES_TO_READ;
			curr_prog += 1;
			if (curr_prog >= max_data_slot) {
				update_progress += 1;
				curr_prog = 0;
			}

			while ((Msg != SECTION_START)
					&& (length < (BSL_MAX_DATA_SIZE - BSL_DATA_PACKET_CRC_SIZE))
					&& (Msg != FILE_TERMINATION_CHAR)) {
				if ((Msg != NEW_LINE) && (Msg != SPACE)) {
					Msg = char_to_hex(Msg);
					bsl_send_data_pkt[length] = (Msg & 0x0F);

					Msg = IO_SYNC_READ8(addr_location + data_ptr);
					data_ptr += BYTES_TO_READ;
					curr_prog += 1;
					if (curr_prog >= max_data_slot) {
						update_progress += 1;
						curr_prog = 0;
					}
					Msg = char_to_hex(Msg);
					bsl_send_data_pkt[length] = ((bsl_send_data_pkt[length] << 4) | (Msg & 0x0F));
					length++;
				}

				Msg = IO_SYNC_READ8(addr_location + data_ptr);
				data_ptr += BYTES_TO_READ;
				curr_prog += 1;
				if (curr_prog >= max_data_slot) {
					update_progress += 1;
					curr_prog = 0;
				}
			}
			if (Msg == SECTION_START) {
				addr_flag = true;
			} else if (Msg == FILE_TERMINATION_CHAR) {
				all_pkt_sent = true;
				update_progress = 99;
			}

			complete_flag = 1;
			*pkt_length = length + 2;
			length -= 3;
			bsl_send_data_pkt[0] = BSL_MSG_HEADER;
			bsl_send_data_pkt[1] = (length & 0xFF);
			bsl_send_data_pkt[2] = ((length >> 8) & 0xFF);
			bsl_send_data_pkt[3] = CMD_RX_DATA_BLOCK_32;

			prev_length = length - 5;

			crc_val = crc16(&bsl_send_data_pkt[3], length);
			length += 3;
			bsl_send_data_pkt[length] = crc_val & 0xFF;
			bsl_send_data_pkt[length + 1] = ((crc_val >> 8) & 0xFF);
			data_crc = crc16(&bsl_send_data_pkt[8], prev_length);

			break;

		default:
			VMC_ERR("Must not come here. ");
			break;
		}
	}
}

/**
  * @brief  Verifies the fpt sc validity and parses the version.
  * @param  msg: pointer to a cl_msg_t structure that contains
  *              the fpt information for the sc base address and size.
  * @retval None
  */

static int get_fpt_sc_version(cl_msg_t *msg, struct fpt_sc_version *version)
{
	u8 read_buffer[SC_TOT_HEADER_SIZE] = {0};
	u8 header[] = SC_HEADER_MSG;
	u8 fpt_sc_status = SC_INVALID;
	u32 fpt_scfw_end_addr = 0;
	data_ptr = 0x00;
	int ret = 0;

	cl_rmgmt_fpt_query(msg);

	fpt_sc_loc.start_address = msg->multiboot_payload.scfw_offset;
	fpt_sc_loc.size = msg->multiboot_payload.scfw_size;

	VMC_LOG("Fpt SC base addr: 0x%x ", fpt_sc_loc.start_address);
	VMC_LOG("Fpt SC size: 0x%x ", fpt_sc_loc.size);

	/* Data is read in 4-byte format. */
	vmc_read_data32((u32 *) fpt_sc_loc.start_address, (u32 *)read_buffer, ((SC_TOT_HEADER_SIZE)/4));

	fpt_sc_status = check_received_sc_header(read_buffer, header, SC_HEADER_SIZE);
	if(fpt_sc_status == SC_VALID)
	{
		fpt_sc_valid = true;
		VMC_LOG("SC Identification: Successful !! ");
		fpt_scfw_end_addr = fpt_sc_loc.start_address + SC_TOT_HEADER_SIZE + fpt_sc_loc.size;
		portENTER_CRITICAL();
		parse_fpt_sc_version((fpt_scfw_end_addr - SC_VER_ADDR_WO_CHKSUM), (u8 *)version);
		portEXIT_CRITICAL();

		VMC_LOG("Fpt SC version: v%d.%d.%d ",
			version->fsv_major, version->fsv_minor, version->fsv_revision);
	}
	else
	{
		fpt_sc_valid = false;
		VMC_ERR("SC Identification: Failed !! ");
		ret = -1;
	}

	return ret;
}

/**
  * @brief  Sends and receives an amount of data in non blocking mode.
  * @param  send_buf: Pointer to send data buffer.
  * @param  bytes_to_send: Amount of data to be send.
  * @param  recv_resp: Check if receive is needed. 
  * @param  bytes_to_recv: Amount of data to be received.
  * @retval Status
  */

static bool do_uart_transaction(u8 *send_buf, u32 bytes_to_send, bool recv_resp, u32 bytes_to_recv)
{
	bool ret_value = false;

	/* Clean up the receiver buffer and count before every transaction. */
	Cl_SecureMemset(rcv_bufr, 0xFF, sizeof(rcv_bufr));
	rcvd_byte_count = 0x00;

	if (UART_SUCCESS == UART_RTOS_Send(&uart_vmcsc_log, &send_buf[0], bytes_to_send)) {
		ret_value = true;
	} else {
		ret_value = false;
		VMC_ERR("Uart Send Error. Retrying !!");
	}
	if ((recv_resp) && (ret_value)) {
		if (UART_SUCCESS == UART_RTOS_Receive(&uart_vmcsc_log, &rcv_bufr[0],
				bytes_to_recv, &rcvd_byte_count, RCV_TIMEOUT_MS(500))) {
			(bytes_to_recv == rcvd_byte_count) ? (ret_value = true) : (ret_value = false);
		} else {
			VMC_ERR("Uart Receive Error. Retrying !!");
			ret_value = false;
		}
	}
	return ret_value;
}

/**
  * @brief  It checks the CRC after every data packet is sent to BSL.
  * @param  write_add: Perform a CRC check from address.
  * @retval Update status
  */

sc_update_status_t matchcrc_postwrite(u32 write_addr)
{
	sc_update_status_t status = eStatus_Success;

	u16 read_crc = 0;
	u8 crc_pkt[12] = {0};

	crc_pkt[0] = BSL_MSG_HEADER;

	/* Pkt len */
	crc_pkt[1] = 0x7;
	crc_pkt[2] = 0;

	/* CMD Id */
	crc_pkt[3] = CMD_CRC_CHECK_32;

	/* Add the Address byte */
	crc_pkt[4] = write_addr & 0xFF;
	crc_pkt[5] = (write_addr >> 8)  & 0xFF;
	crc_pkt[6] = (write_addr >> 16) & 0xFF;
	crc_pkt[7] = (write_addr >> 24) & 0xFF;

	/* Length to calculate the Data */
	crc_pkt[8] = prev_length & 0xFF;
	crc_pkt[9] = (prev_length >> 8) & 0xFF;

	/* Append the CRC */
	u16 checksum = crc16(&crc_pkt[3],7);
	crc_pkt[10] = checksum & 0xFF;
	crc_pkt[11] = (checksum >> 8) & 0xFF;

	if (do_uart_transaction(&crc_pkt[0], BSL_CRC_REQ_SIZE, TRUE, BSL_CRC_RESP_SIZE)) {
		/* Validate BSL response */
		if (!vmc_validate_bsl_resp(&rcv_bufr[0], BSL_CRC_RESP_SIZE)) {
			/* Check if we received a successful/expected response. */
			if (rcv_bufr[4] == BSL_CRC_SUCCESS_RESP) {
				read_crc = ((rcv_bufr[BSL_RESP_CAL_CHKSUM_H] << 8) | (rcv_bufr[BSL_RESP_CAL_CHKSUM_L]));
				if (data_crc != read_crc) {
					status = eStatus_Failure;
					VMC_ERR("CRC Mismatch !! Expected: %x Received : %x", data_crc, read_crc);
				}
			} else {
				status = eStatus_Failure;
			}
		}
	}

	return status;
}

/**
  * @brief  Check whether MSP is in SC mode or BSL.
  * @param  None
  * @retval None
  */

static void sync_sc_bsl(void)
{
	static u8 mode_retry_cnt = 0;
	static u8 retry_count = 0;
	static u32 bytes_to_rcv = SC_BSL_SYNCED_RESP_SIZE;
	u8 sc_bsl_sync[SC_BSL_SYNCED_REQ_SIZE] = { BSL_SYNC_REQ_CHAR };

	VMC_LOG("CMD : SC BSL SYNC ");
	/* Here we are trying to receive packets from MSP and decide whether MSP is in SC or BSL mode based on the response received */
	if (do_uart_transaction(&sc_bsl_sync[0], SC_BSL_SYNCED_REQ_SIZE, TRUE, bytes_to_rcv)) {
		/* Check if MSP is in SC/BSL mode. */
		if ((rcv_bufr[0] == ESCAPE_CHAR) && (rcv_bufr[1] == STX)) {
			mode_retry_cnt = 0;
			retry_count = 0;
			update_state = eSc_Enable_Bsl;
			VMC_LOG("MSP is in SC mode !! ");
		}
		/* Check if BSL is sending an unknown message as a response. */
		else if ((bytes_to_rcv == BSL_UNKNOWN_MSG_RESP_SIZE)
					&& (!vmc_validate_bsl_resp(&rcv_bufr[0], (u16)bytes_to_rcv))
					&& (mode_retry_cnt <= MODE_VER_MAX_RETRY_COUNT)) {
			mode_retry_cnt += 1;
			/* Make sure MSP is in BSL mode. */
			if (mode_retry_cnt >= MODE_VER_MAX_RETRY_COUNT) {
				mode_retry_cnt = 0;
				retry_count = 0;
				update_state = eBsl_Unlock_Password;
				bytes_to_rcv = SC_BSL_SYNCED_RESP_SIZE;
				VMC_LOG("MSP is in BSL mode !! ");
			}
		}
		/* Check if BSL is trying to sync with VMC. */
		else if ((bytes_to_rcv == BSL_SYNCED_RESP_SIZE)
					&& (rcv_bufr[0] == BSL_SYNC_SUCCESS)
					&& (mode_retry_cnt <= MODE_VER_MAX_RETRY_COUNT)) {
			mode_retry_cnt += 1;
			/* Make sure MSP is in BSL mode. */
			if (mode_retry_cnt >= MODE_VER_MAX_RETRY_COUNT) {
				mode_retry_cnt = 0;
				retry_count = 0;
				update_state = eBsl_Unlock_Password;
				bytes_to_rcv = SC_BSL_SYNCED_RESP_SIZE;
				VMC_LOG("MSP is in BSL mode !! ");
			}
		} else {
			retry_count += 1;
			mode_retry_cnt = 0;
			update_state = eSc_Bsl_Sync;
			VMC_ERR("SC BSL Sync Failure.. Retrying !! ");
		}
	} else {
		retry_count += 1;
	}

	if (retry_count >= SC_UPDATE_MAX_RETRY_COUNT) {
		if ((retry_count >= SC_UPDATE_MAX_RETRY_COUNT)
				&& (bytes_to_rcv == BSL_SYNCED_RESP_SIZE)) {
			retry_count = 0;
			mode_retry_cnt = 0;
			update_status = eStatus_Failure;
			update_state = eSc_State_Idle;
			update_error = eSc_Update_Error_Sc_Bsl_Sync_Failed;
			bytes_to_rcv = SC_BSL_SYNCED_RESP_SIZE;
			VMC_ERR("Update failure: Retry !! ");
			return;
		}
		if (bytes_to_rcv == SC_BSL_SYNCED_RESP_SIZE) {
			bytes_to_rcv = BSL_UNKNOWN_MSG_RESP_SIZE;
		} else if (bytes_to_rcv == BSL_UNKNOWN_MSG_RESP_SIZE) {
			bytes_to_rcv = BSL_SYNCED_RESP_SIZE;
		}
		retry_count = 0;
		mode_retry_cnt = 0;
	}
	vTaskDelay(pdMS_TO_TICKS(1000 * 1));
}

/**
  * @brief  Enables BSL if MSP is in SC mode.
  * @param  None
  * @retval None
  */

static void enable_sc_bsl(void)
{
	static u8 retry_count = 0;
	u8 en_bsl[SC_ENABLE_BSL_REQ_SIZE] = { 0x5C, 0x2, 0x1, 0x0, 0x0, 0x1, 0x0, 0x5C, 0x3 };

	VMC_LOG("CMD : SC ENABLE BSL ");
	if (do_uart_transaction(&en_bsl[0], SC_ENABLE_BSL_REQ_SIZE, TRUE, SC_ENABLE_BSL_RESP_SIZE)) {
		if ((rcv_bufr[0] == ESCAPE_CHAR) && (rcv_bufr[2] == MSP432_COMMS_MSG_GOOD)) {
			retry_count = 0;
			update_state = eBsl_Synced;
		} else {
			retry_count += 1;
			update_state = eSc_Enable_Bsl;
		}
	} else {
		retry_count += 1;
	}

	if (retry_count >= SC_UPDATE_MAX_RETRY_COUNT) {
		retry_count = 0;
		update_error = eSc_Update_Error_En_Bsl_Failed;
		update_status = eStatus_Failure;
		update_state = eSc_State_Idle;
		VMC_ERR("Update failure: Retry !! ");
	}
	vTaskDelay(pdMS_TO_TICKS(1000 * 1));
}

/**
  * @brief  Sync with BSL for further UART communication.
  * @param  None
  * @retval None
  */

static void sync_vmc_bsl(void)
{
	static u8 retry_count = 0;
	u8 syncbuf[BSL_SYNCED_REQ_SIZE] = { BSL_SYNC_REQ_CHAR };

	VMC_LOG("CMD : BSL SYNCED ");
	if (do_uart_transaction(&syncbuf[0], BSL_SYNCED_REQ_SIZE, TRUE, BSL_SYNCED_RESP_SIZE)) {
		/* Check if we received a successful/expected response. */
		if (rcv_bufr[0] == BSL_SYNC_SUCCESS) {
			retry_count = 0;
			update_progress = 1;
			update_state = eBsl_Unlock_Password;
		} else {
			retry_count += 1;
			update_state = eBsl_Synced;
			VMC_ERR("SYNC Failure.. Retrying !! ");
		}
	} else {
		retry_count += 1;
	}

	if (retry_count >= SC_UPDATE_MAX_RETRY_COUNT) {
		retry_count = 0;
		update_error = eSc_Update_Error_Vmc_Bsl_Sync_Failed;
		update_status = eStatus_Failure;
		update_state = eSc_State_Idle;
		VMC_ERR("Update failure: Retry !! ");
	}
	vTaskDelay(pdMS_TO_TICKS(1000 * 1));
}

/**
  * @brief  Unlocks the BSL.
  * @param  None
  * @retval None
  */

static void unlock_bsl(void)
{
	static u8 retry_count = 0;
	/* Please refer to the TI SLAU622I Data Sheet for packet descriptions. */
	u8 bsl_pwd[BSL_UNLOCK_PASSWORD_REQ_SIZE] = { BSL_MSG_HEADER, 0x39, 0x00,
			CMD_RX_PASSWORD, 0x58, 0x41, 0x50, 0x3A, 0x20, 0x58, 0x69, 0x6C,
			0x69, 0x6E, 0x78, 0x20, 0x41, 0x6C, 0x6C, 0x20, 0x50, 0x72, 0x6F,
			0x67, 0x72, 0x61, 0x6D, 0x6D, 0x61, 0x62, 0x6C, 0x65, 0x2C, 0x20,
			0x58, 0x42, 0x42, 0x3A, 0x20, 0x58, 0x69, 0x6C, 0x69, 0x6E, 0x78,
			0x20, 0x42, 0x72, 0x61, 0x6E, 0x64, 0x65, 0x64, 0x20, 0x42, 0x6F,
			0x61, 0x72, 0x64, 0x73, 0x82, 0xE5 };

	VMC_LOG("CMD : BSL UNLOCK PASSWORD ");
	if (do_uart_transaction(&bsl_pwd[0], BSL_UNLOCK_PASSWORD_REQ_SIZE, TRUE, BSL_UNLOCK_PASSWORD_RESP_SIZE)) {
		/* Validate BSL response */
		if (!vmc_validate_bsl_resp(&rcv_bufr[0], BSL_UNLOCK_PASSWORD_RESP_SIZE)) {
			/* Check if we received a successful/expected response. */
			if ((rcv_bufr[BSL_RESP_CMD_OFFSET] == BSL_RESP_CMD_CHAR)
					&& (rcv_bufr[BSL_RESP_MSG_OFFSET] == BSL_RESP_MSG_CHAR)) {
				retry_count = 0;
				update_state = eBsl_Mass_Erase;
			} else {
				retry_count += 1;
				update_state = eBsl_Unlock_Password;
			}
		} else {
			retry_count += 1;
			update_state = eBsl_Unlock_Password;
			VMC_ERR("Invalid Response from BSL.. Retrying.. !!");
		}
	} else {
		retry_count += 1;
	}

	if (retry_count >= SC_UPDATE_MAX_RETRY_COUNT) {
		retry_count = 0;
		update_error = eSc_Update_Error_Bsl_Unlock_Password_Failed;
		update_status = eStatus_Failure;
		update_state = eSc_State_Idle;
		VMC_ERR("Update failure: Retry !! ");
	}
	vTaskDelay(pdMS_TO_TICKS(200));
}

/**
  * @brief  Erase MSP flash.
  * @param  None
  * @retval None
  */

static void mass_erase_bsl(void)
{
	static u8 retry_count = 0;
	/* Please refer to the TI SLAU622I Data Sheet for packet descriptions. */
	u8 mass_erase[BSL_MASS_ERASE_REQ_SIZE] = { BSL_MSG_HEADER, 0x01, 0x00, CMD_MASS_ERASE, 0x64, 0xA3 };

	VMC_LOG("CMD : BSL MASS ERASE ");
	if (do_uart_transaction(&mass_erase[0], BSL_MASS_ERASE_REQ_SIZE, TRUE, BSL_MASS_ERASE_RESP_SIZE)) {
		/* Validate BSL response */
		if (!vmc_validate_bsl_resp(&rcv_bufr[0], BSL_MASS_ERASE_RESP_SIZE)) {
			/* Check if we received a successful/expected response. */
			if ((rcv_bufr[BSL_RESP_CMD_OFFSET] == BSL_RESP_CMD_CHAR)
					&& (rcv_bufr[BSL_RESP_MSG_OFFSET] == BSL_RESP_MSG_CHAR)) {
				retry_count = 0;
				update_state = eBsl_Data_Tx_32;
			} else {
				retry_count += 1;
				update_state = eBsl_Mass_Erase;
			}
		} else {
			retry_count += 1;
			update_state = eBsl_Mass_Erase;
			VMC_ERR("Invalid Response from BSL.. Retrying.. !! ");
		}
	} else {
		retry_count += 1;
	}

	if (retry_count >= SC_UPDATE_MAX_RETRY_COUNT) {
		retry_count = 0;
		update_error = eSc_Update_Error_Erase_Failed;
		update_status = eStatus_Failure;
		update_state = eSc_State_Idle;
		VMC_ERR("Update failure: Retry !! ");
	}
	vTaskDelay(pdMS_TO_TICKS(200));
}

/**
  * @brief  Sends an amount of data to BSL.
  * @param  None
  * @retval None
  */

static void send_data_to_bsl(void)
{
	u8 start_symbol = 0x00;
	data_ptr = 0x00;
	all_pkt_sent = false;
	max_data_slot = ((fpt_sc_loc.size / 100) + ((fpt_sc_loc.size % 100)));

	VMC_LOG("CMD : BSL DATA TX 32 ");
	start_symbol = IO_SYNC_READ8(fpt_sc_loc.start_address + SC_TOT_HEADER_SIZE);
	if (start_symbol != SECTION_START) {
		update_status = eStatus_Failure;
		update_state = eSc_State_Idle;
		update_error = eSc_Update_Error_Invalid_Sc_Symbol_Found;
		VMC_ERR("Invalid symbol... ");
		return;
	}

	addr_flag = true;
	while (all_pkt_sent != true) {
		Cl_SecureMemset(bsl_send_data_pkt, 0x00, sizeof(bsl_send_data_pkt));

		portENTER_CRITICAL();
		parse_fpt_sc((fpt_sc_loc.start_address + SC_TOT_HEADER_SIZE), &bsl_send_data_pkt[0], &total_length);
		portEXIT_CRITICAL();

		if (do_uart_transaction(&bsl_send_data_pkt[0], total_length, TRUE, BSL_DATA_TX_32_RESP_SIZE)) {
			/* Validate BSL response */
			if (!vmc_validate_bsl_resp(&rcv_bufr[0], BSL_DATA_TX_32_RESP_SIZE)) {
				/* Check if we received a successful/expected response. */
				if ((rcv_bufr[BSL_RESP_CMD_OFFSET] == BSL_RESP_CMD_CHAR)
						&& (rcv_bufr[BSL_RESP_MSG_OFFSET] == BSL_RESP_MSG_CHAR)) {
					/* Check if the data was written to the MSP flash successfully. */
					if (matchcrc_postwrite(parsed_sc_flash_addr) != eStatus_Success) {
						update_status = eStatus_Failure;
						update_error = eSc_Update_Error_Post_Crc_Failed;
						VMC_ERR("Post CRC failed... ");
						break;
					}
				} else {
					update_status = eStatus_Failure;
					update_error = eSc_Update_Error_Invalid_Bsl_Resp;
					VMC_ERR("BSL Resp failure. Retry.. ");
					break;
				}
			} else {
				update_status = eStatus_Failure;
				update_error = eSc_Update_Error_Invalid_Bsl_Resp;
				VMC_ERR("Invalid Response from BSL..");
				break;
			}
		}
		/* 10msec packet-to-packet */
		vTaskDelay(pdMS_TO_TICKS(10));
	}
	/* Clean up */
	addr_flag = false;
	data_ptr = 0x00;
	curr_prog = 0;
	max_data_slot = 0;

	/* Move the FSM ahead and reset the BSL state machine,
	 * as we failed in the middle of packet transfer. */
	if (update_status != eStatus_Failure) {
		update_state = eBsl_Load_Pc_32;
	} else {
		update_state = eBsl_Reboot_Reset;
		update_status = eStatus_In_Progress;
	}
	vTaskDelay(pdMS_TO_TICKS(10));
}

/**
  * @brief  Causes the BSL to begin execution at the given address.
  * @param  None
  * @retval None
  */

static void loadpc_bsl(void)
{
	/* Please refer to the TI SLAU622I Data Sheet for packet descriptions.*/
	u8 load_pc[BSL_LOAD_PC_REQ_SIZE] = { BSL_MSG_HEADER, 0x05, 0x00, CMD_LOAD_PC_32, 0x01, 0x02, 00, 00, 0xB8, 0x66 };

	VMC_LOG("CMD : BSL LOAD PC 32 ");
	if (do_uart_transaction(&load_pc[0], BSL_LOAD_PC_REQ_SIZE, FALSE, 0)) {
		update_progress += 1;
		if (update_progress == 100)
			VMC_LOG("Update Complete : %d%% ",update_progress);

		VMC_LOG("SC Application Loaded !! ");
		update_status = eStatus_Success;
		update_state = eSc_State_Idle;
	}
}

/**
  * @brief  This used to initiate a reboot-reset into the MSP MCU.
  * @param  None
  * @retval None
  */

static void reboot_bsl(void)
{
	/* NOTE: After a reboot reset, it takes approximately 10msec before it starts again. */
	/* Please refer to the TI SLAU622I Data Sheet for packet descriptions. */
	u8 reboot_rst[BSL_REBOOT_RESET_REQ_SIZE] = { BSL_MSG_HEADER, 0x01, 0x00, CMD_REBOOT_RESET, 0x37, 0x95 };

	VMC_LOG("CMD : BSL REBOOT RESET ");
	if (do_uart_transaction(&reboot_rst[0], BSL_REBOOT_RESET_REQ_SIZE, FALSE, 0)) {
		/* Reset the progress variables. */
		update_status = eStatus_Success;
		update_state = eSc_State_Idle;
		VMC_LOG("SuC BSL Rebooted !! ");
	}
	vTaskDelay(pdMS_TO_TICKS(100));
}

#ifdef VMC_DEBUG
/**
  * @brief  This used to get the BSL version.
  * @param  None
  * @retval None
  */

static void get_bsl_version(void)
{
	static u8 retry_count = 0;
	/* Please refer to the TI SLAU622I Data Sheet for packet descriptions. */
	u8 version[BSL_VERSION_REQ_SIZE] = { BSL_MSG_HEADER, 0x01, 0x00, CMD_TX_BSL_VERSION, 0xE8, 0x62 };

	VMC_LOG("CMD : TX BSL VERSION ");
	if (do_uart_transaction(&version[0], BSL_VERSION_REQ_SIZE, TRUE, BSL_VERSION_RESP_SIZE)) {
		/* Validate BSL response */
		if (!vmc_validate_bsl_resp(&rcv_bufr[0], BSL_VERSION_RESP_SIZE)) {
			/* Check if we received a successful/expected response. */
			if ((rcv_bufr[BSL_RESP_BUILD_ID_OFFSET_H] == BSL_VERSION_BYTE_H)
					&& (rcv_bufr[BSL_RESP_BUILD_ID_OFFSET_L] == BSL_VERSION_BYTE_L)) {
				retry_count = 0;
				update_status = eStatus_Success;
				update_state = eSc_State_Idle;
			} else {
				retry_count += 1;
				update_state = eTx_Bsl_Version;
			}
		} else {
			retry_count += 1;
			update_state = eTx_Bsl_Version;
			VMC_ERR("BSL Resp failure. Retry.. ");
		}
	} else {
		retry_count += 1;
	}

	if (retry_count >= SC_UPDATE_MAX_RETRY_COUNT) {
		retry_count = 0;
		update_status = eStatus_Failure;
		update_state = eSc_State_Idle;
		update_error = eSc_Update_Error_Failed_To_Get_Bsl_Version;
		VMC_ERR("Failed to get BSL version: Retry !! ");
	}
	/* Reset the progress variables. */
	update_progress = 0;
	curr_prog = 0;
	max_data_slot = 0;
	vTaskDelay(pdMS_TO_TICKS(1000 * 1));
}
#endif

/**
  * @brief	Starts sc firmware update on a trigger from XRT.
  *
  *		SC firmware update design state diagram.
  *		State Notation: state name (respective enum flag). For example, sync SC BSL (eSc_Bsl_Sync).
  * 
  * 		1. Starting from Sync SC BSL state, for each state we have 10 retries,
  *		   based on that we will decide whether we will proceed further or exit.
  *		2. NOTE: For MASS erase, internally BSL will perform a sector erase on the whole SC application flash area.
  *
  *    		Initial State --------> Start --------> Sync SC BSL
  * 			^			      	(eSc_Bsl_Sync)
  *	 	  	|                 		     |
  *    	 		|			    (Fail)   |
  *    	 	Exit clean up <----- Fail State <------------+------------+
  *	      		^	         ^		     |	          |
  *	     		|		 |   		(MSP in SC)       |
  *	      		|                |	        Enable SC BSL     |
  *			|		 |	      	(eSc_Enable_Bsl)  |
  *	      		|	         |	(Fail)	     |            |
  *	      		|	         +-------------------+	     (MSP in BSL)
  *	      		|	         ^	             | 	          |
  *	      		|	         |	        Sync VMC BSL      |
  *			|		 |	        (eBsl_Synced)     |
  *	      		|		 |	(Fail)	     |	          |
  *	      		|		 +-------------------+	          |
  *	      		|		 ^	             |            |
  *	      		|		 |     	         Unlock BSL <-----+
  * 			|		 |		(eBsl_Unlock_Password)
  *	      		|		 |	(Fail)	     |
  *	      		|		 +-------------------+
  *	      		|	         ^	     	     |
  *	      		|		 |		 MASS Erase 
  * 			|		 |		(eBsl_Mass_Erase)
  *	      		|		 |	(Fail)	     |
  *	      		|     +--------> +-------------------+
  *	      		|     |		 ^	  	     |
  *	      		|     |	         |		 Data Tx_32
  * 			|     |	         |		(eBsl_Data_Tx_32)
  *	      		|     |      Reboot BSL	      (Fail) |
  *	      		|     |  (eBsl_Reboot_Reset) <-------+
  *	      		|     |  	     		     |
  *	      		|     |        			 Load Pc_32
  * 			|     |	 			(eBsl_Load_Pc_32)
  *	      		|     |	 	(Fail)	     	     |
  *	      		|     +------------------------------+
  *	      		|				     |
  *	      		+------------------------------------+
  *
  *	@param	None
  *
  *	@retval	None
  */

u8 update_scfw(void)
{
	u8 ret_val = 0;

	if ((update_state == eSc_State_Idle)
			&& ((update_status == eStatus_Success)
					|| (update_status == eStatus_Failure))) {
		update_state = eSc_Bsl_Sync;
		update_status = eStatus_In_Progress;
		update_error = eSc_Update_No_Error;
	}

	while (update_status == eStatus_In_Progress) {
		switch (update_state) {
		case eSc_Bsl_Sync:
			sync_sc_bsl();
			break;
		case eSc_Enable_Bsl:
			enable_sc_bsl();
			break;
		case eBsl_Synced:
			sync_vmc_bsl();
			break;
		case eBsl_Unlock_Password:
			unlock_bsl();
			break;
		case eBsl_Mass_Erase:
			mass_erase_bsl();
			break;
		case eBsl_Data_Tx_32:
			send_data_to_bsl();
			break;
		case eBsl_Load_Pc_32:
			loadpc_bsl();
			break;
		case eBsl_Reboot_Reset:
			reboot_bsl();
			break;
#ifdef VMC_DEBUG
		case eTx_Bsl_Version:
			get_bsl_version();
			break;
#endif
		default:
			VMC_ERR("Invalid Command !! ");
			update_status = eStatus_Failure;
			update_state = eSc_State_Idle;
		}
	}
	ret_val = (u8)update_error;

	/* Waiting for 5sec so that XRT will get the updated progress of the SC update. */
	vTaskDelay(pdMS_TO_TICKS(1000 * 5));

	/* Reset progress variables */
	update_progress = 0;
	curr_prog = 0;
	max_data_slot = 0;
	update_error = eSc_Update_No_Error;

	Cl_SecureMemset(rcv_bufr, 0x00, sizeof(rcv_bufr));
	rcvd_byte_count = 0;

	/* Reset all VMC <-> SC flags to restart COMMs */
	vmc_set_sc_status(false);
	vmc_set_power_mode_status(false);
	vmc_set_snsr_resp_status(false);
	/* SC FW updated, Resend the Board Info */
	vmc_set_boardInfo_status(false);

	return ret_val;
}

static u8 start_scfw_update(void)
{
	u8 ret_val = 0;
  
	if (!fpt_sc_valid) {
		VMC_ERR("No Valid SC available !! ");
		return ((u8)eSc_Update_Error_No_Valid_Fpt_Sc_Found);
	}

	ret_val = get_sc_checksum();
	if (!ret_val) {
		VMC_LOG("SC Up-to-date !! ");
		return ((u8)eSc_Up_To_Date_No_Update_Req);
	}

	VMC_LOG("SC Needs Update !! ");
	ret_val = update_scfw();
	return ret_val;
}

int cl_vmc_scfw_program(cl_msg_t *msg)
{
	return ((int)start_scfw_update());
}

void cl_vmc_scfw_version(struct fpt_sc_version *version)
{
	if (version != NULL) {
		cl_msg_t msg = { 0 };
		(void) get_fpt_sc_version(&msg, version);
	}
}

int cl_vmc_scfw_init()
{
	struct fpt_sc_version version = { 0 };

	cl_vmc_scfw_version(&version);

	return 0;
}
