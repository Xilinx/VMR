/******************************************************************************
 * Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
 * SPDX-License-Identifier: MIT
 *******************************************************************************/

#include "FreeRTOS.h"
#include "timers.h"
#include "task.h"

#include "vmc_api.h"
#include "cl_msg.h"
#include "cl_uart_rtos.h"
#include "vmc_update_sc.h"
#include "vmc_sc_comms.h"


upgrade_status_t upgradeStatus = STATUS_SUCCESS;
upgrade_state_t upgradeState = SC_STATE_IDLE;
scUpateError_t upgradeError = SC_UPDATE_NO_ERROR;

TimerHandle_t xSCUpdateTimeOutTimer = NULL;
TaskHandle_t xSCUpdateTaskHandle = NULL;
extern uart_rtos_handle_t uart_vmcsc_log;

/* Global structures */
SC_VMC_Data sc_version;
cl_msg_t fpt_sc_offsets;
efpt_sc_t fpt_sc_loc;

/* Variables for SC update data packets */
u8 bsl_send_data_pkt[BSL_MAX_DATA_SIZE] = {0x00};
u8 addr_flag = 0x00;
u32 addr = 0x00;
u16 total_length = 0x00;
u32 data_ptr = 0;
u32 Prev_length = 0x00;
u16 CRC_Val = 0x00;
u16 dataCRC = 0x00;
u8 msg[1] = {0x00};
u8 fpt_sc_version[3] = {0x00};
bool all_pkt_sent = false;
bool fptSCvalid = false;

/* Variable to keep track whether SC update is going on or not */
u8 sc_update_flag = 0x00;

/* Variables will keep track of SC update progress */
int32_t update_progress = 0;
u32 curr_prog = 0;
u32 max_data_slot = 0;

u8 receive_bufr[32] = {0x00};
u32 receivedByteCount = 0x00;

u8 bslPasswd[BSL_UNLOCK_PASSWORD_REQ] = { 0x80, 0x39, 0x00, 0x21, 0x58, 0x41,
		0x50, 0x3A, 0x20, 0x58, 0x69, 0x6C, 0x69, 0x6E, 0x78, 0x20, 0x41, 0x6C,
		0x6C, 0x20, 0x50, 0x72, 0x6F, 0x67, 0x72, 0x61, 0x6D, 0x6D, 0x61, 0x62,
		0x6C, 0x65, 0x2C, 0x20, 0x58, 0x42, 0x42, 0x3A, 0x20, 0x58, 0x69, 0x6C,
		0x69, 0x6E, 0x78, 0x20, 0x42, 0x72, 0x61, 0x6E, 0x64, 0x65, 0x64, 0x20,
		0x42, 0x6F, 0x61, 0x72, 0x64, 0x73, 0x82, 0xE5 };


extern void rmgmt_extension_fpt_query(struct cl_msg *msg);


int32_t VMC_SCFW_Program_Progress(void)
{
	return update_progress;
}


static inline void VMC_Read_Data32(u32 *addr, u32 *data, size_t sz)
{
	u32 i;

	for (i = 0; i < sz; ++i)
	{
		*(data + i) = IO_SYNC_READ32((UINTPTR)(addr + i));
	}
}

unsigned short crc16( unsigned char * pMsg, unsigned int size )
{
	unsigned short crc = 0xFFFF;
    unsigned short i = 0;

    for(i = 0 ; i < size ; i ++)
    {
    	unsigned short int x;

        x = ( ( crc >> 8 ) ^ pMsg[ i ] ) & 0xff;
        x ^= x >> 4;
        crc = ( crc << 8 ) ^ ( x << 12 ) ^ ( x <<5 ) ^ x;
    }

    return crc;
}


char Char2HexAdd( char C)
{

    if (C >= '0' && C <= '9')
	{
		C = C - '0';
	}
    else if (C >= 'a' && C <='f')
	{
		C = C - 'a' + 10;
	}

    return (C & 0x0F);
}


char Char2Hex (char C)
{

    if (C >= '0' && C <= '9')
	{
		C = C - '0';
	}
    else if (C >= 'A' && C <='F')
	{
		C = C - 'A' + 10;
	}

    return (C & 0x0F);
}


int VMC_Validate_BSL_resp(unsigned char *buffer, unsigned short length)
{
	int status = -1;
	u16 dataLen = 0;
	u16 recvdCRC = 0;
	u16 calcCRC = 0;

	if(buffer[0] == 0x00 &&	buffer[1] == BSL_MSG_HEADER)
	{
		dataLen = ((buffer[3] << 8) | buffer[2]);

		recvdCRC = ( (buffer[length - 1] << 8) | (buffer[length - 2]));
		calcCRC = crc16(&buffer[4],dataLen);

		if(recvdCRC == calcCRC)
		{
			status = 0;
//			VMC_LOG("CRC Check SUCCESSFUL... \r\n");
		}
		else
		{
			VMC_ERR("CRC Check Failed... \r\n");
		}
	}

	return status;
}


u8 Check_Received_SC_Header(void *ptr1, void *ptr2, u8 len)
{
	u8 *buf1 = ptr1;
	u8 *buf2 = ptr2;

    while (len > 0)
    {
        if (*buf1 != *buf2)
        {
            return (*buf1 - *buf2);
        }

        len--;
        buf1++;
        buf2++;
    }
    return 0;
}

u8 Get_SC_Checksum(void)
{
	/* Get the SC version and compare with the backup one and return accordingly */
	u8 retVal = UPDATE_REQUIRED;

	/* if (fpt_sc_version[MAJOR] > sc_version.scVersion[MAJOR]) {
		retVal = UPDATE_REQUIRED;
	} else if (fpt_sc_version[MAJOR] == sc_version.scVersion[MAJOR]) {
		if (fpt_sc_version[MINOR] > sc_version.scVersion[MINOR]) {
			retVal = UPDATE_REQUIRED;
		} else if (fpt_sc_version[MINOR] == sc_version.scVersion[MINOR]) {
			if (fpt_sc_version[REVISION] > sc_version.scVersion[REVISION]) {
				retVal = UPDATE_REQUIRED;
			} else if (fpt_sc_version[REVISION] == sc_version.scVersion[REVISION]) {
				retVal = NO_UPDATE_REQUIRED;
			} else {
				retVal = UPDATE_REQUIRED;
			}
		} else {
			retVal = UPDATE_REQUIRED;
		}
	} else {
		retVal = UPDATE_REQUIRED;
	} */

	return retVal;
}

void xSCUpdateTimeOutTimerCallback(TimerHandle_t xTimer)
{
	/* Clean Up */
	upgradeError = SC_UPDATE_ERROR_OPEARTION_TIMEDOUT;
	update_progress = upgradeError;
	upgradeStatus = STATUS_FAILURE;
	upgradeState = SC_STATE_IDLE;

	memset(receive_bufr,0x00,sizeof(receive_bufr));
	receivedByteCount = 0;

	/* Timer reset */
	if(xTimerReset(xTimer, pdMS_TO_TICKS(100)) != pdPASS)
	{
		VMC_ERR("\n\rFailed to reset xSCUpdateTimeOutTimer \r\n");
	}

	/* Release done flag */
	sc_update_flag = 0x00;

	VMC_LOG("\n\rSC Update Timeout.. \n\rERROR in SC Update. Retry... \r\n");
}

void VMC_Get_Fpt_SC_Version(cl_msg_t *msg)
{
	u8 read_buffer[SC_HEADER_SIZE] = {0};
	u8 header[SC_HEADER_SIZE] = SC_HEADER_MSG;
	u8 retVal = 0x01;
	data_ptr = 0x00;


	rmgmt_extension_fpt_query(msg);

	fpt_sc_loc.start_address = msg->multiboot_payload.scfw_offset;
	fpt_sc_loc.size = msg->multiboot_payload.scfw_size;

	VMC_LOG("\n\rFpt SC Base Addr : 0x%x\r\n",fpt_sc_loc.start_address);
	VMC_LOG("\n\rFpt SC size : 0x%x\r\n",fpt_sc_loc.size);

	VMC_Read_Data32((u32 *) fpt_sc_loc.start_address, (u32 *)read_buffer, (SC_HEADER_SIZE+1)/4);

	retVal = Check_Received_SC_Header(read_buffer,header,sizeof(read_buffer));
	if(retVal == 0x00)
	{
		fptSCvalid = true;
		VMC_LOG("\n\rSC Identification Passed !!\r\n");

		portENTER_CRITICAL();
		VMC_Parse_Fpt_SC_Version(((fpt_sc_loc.start_address + SC_TOT_HEADER_SIZE + fpt_sc_loc.size) - 0x33), &fpt_sc_version[0]);
		portEXIT_CRITICAL();

		VMC_LOG("\n\rFpt SC version : v%d.%d.%d\r\n",fpt_sc_version[0], fpt_sc_version[1], fpt_sc_version[2]);

		xSCUpdateTimeOutTimer = xTimerCreate("SC Update timeout timer", pdMS_TO_TICKS(1000 * 60), pdFALSE, NULL, xSCUpdateTimeOutTimerCallback);
		if((xSCUpdateTimeOutTimer == NULL))
		{
			VMC_LOG("\n\rSC Update Timer creation failed.. !!\r\n");
		}
	}
	else
	{
		fptSCvalid = false;
		VMC_ERR("\n\rSC Identification failed !!\r\n");
	}
}

int32_t VMC_Start_SC_Update(void)
{
	int32_t retVal = 0;

	retVal = Get_SC_Checksum();
	if(retVal && fptSCvalid)
	{
		VMC_LOG("\n\rSC Needs Update !!\r\n");

		if(xTimerStart(xSCUpdateTimeOutTimer, pdMS_TO_TICKS(100)) != pdPASS)
		{
			VMC_ERR("\n\rFailed to start xSCUpdateTimeOutTimer \r\n");
		}

		if (xTaskNotify(xSCUpdateTaskHandle, 0, eNoAction) != pdPASS)
		{
			retVal = -1;
			xTimerStop(xSCUpdateTimeOutTimer, pdMS_TO_TICKS(100));
			VMC_ERR("Notification failed !!\r\n");
		}
	}
	else
	{
		if(!fptSCvalid)
		{
			VMC_LOG("\n\rNo Valid SC available !!\r\n");
			retVal = -1;
		}
		else if(!retVal)
		{
			VMC_LOG("\n\rSC Up-to-date !!\r\n");
			retVal = -1;
		}
	}

	return retVal;
}


void VMC_Parse_Fpt_SC_Version(u32 addr_location, u8 *versionbuff)
{
	u8 state = PARSE_ADDRESS;
	u8 Complete_flag = 0;
	u8 version_ptr = 0x00;
	bool version_flag = false;

	while (Complete_flag == 0)
	{
		switch (state)
		{

		case PARSE_ADDRESS:

			msg[0] = IO_SYNC_READ8(addr_location + data_ptr);
			data_ptr += BYTES_TO_READ;

			if ((addr_flag == 1) && (msg[0] != SECTION_START) && (msg[0] != FILE_TERMINATION_CHAR))
			{
				addr_flag = 0;
				addr = 0;
				while (msg[0] != NEW_LINE)
				{
					msg[0] = Char2HexAdd(msg[0]);
					addr = (addr << (4)) | (msg[0] & 0x0F);
					msg[0] = IO_SYNC_READ8(addr_location + data_ptr);
					data_ptr += BYTES_TO_READ;
				}

				VMC_LOG("\n\radd : %x\r\n", addr);
				if((addr == 0x7e000) || (addr == 0x7e002) || (addr == 0x7e004))
				{
					version_flag = true;
				}

				state = PARSE_DATA;
			}
			else
			{
				if (msg[0] == SECTION_START)
				{
					addr_flag = 1;
				}
				else if (msg[0] == FILE_TERMINATION_CHAR)
				{
					Complete_flag = 1;
				}
				else
				{
					state = PARSE_DATA;
				}
			}

			break;

		case PARSE_DATA:

			msg[0] = IO_SYNC_READ8(addr_location + data_ptr);
			data_ptr += BYTES_TO_READ;

			while ((msg[0] != SECTION_START) && (msg[0] != FILE_TERMINATION_CHAR))
			{
				if (msg[0] != NEW_LINE && msg[0] != SPACE)
				{
					msg[0] = Char2Hex(msg[0]);
					if(version_flag)
						versionbuff[version_ptr] = msg[0] & 0x0F;

					if ((msg[0] == NEW_LINE) || (msg[0] == SPACE) || (msg[0] == SECTION_START))
					{
						addr_flag = 1;
						version_flag = false;
						state = PARSE_ADDRESS;
						break;
					}

					msg[0] = IO_SYNC_READ8(addr_location + data_ptr);
					data_ptr += BYTES_TO_READ;
					msg[0] = Char2Hex(msg[0]);
					if(version_flag)
					{
						versionbuff[version_ptr] = (versionbuff[version_ptr] << 4) | (msg[0] & 0x0F);
						version_ptr++;
					}
				}

				msg[0] = IO_SYNC_READ8(addr_location + data_ptr);
				data_ptr += BYTES_TO_READ;
			}
			if (msg[0] == SECTION_START)
			{
				addr_flag = 1;
				state = PARSE_ADDRESS;
				version_flag = false;
			}
			else if (msg[0] == FILE_TERMINATION_CHAR)
			{
				Complete_flag = 1;
			}

			break;

		default:
			break;

		}
	}
}


void VMC_Parse_Fpt_SC(u32 addr_location, u8 *bsl_send_data_pkt, u16 *pkt_length)
{
	u8 state = PARSE_ADDRESS;
	u8 Complete_flag = 0;
	u32 length = 0;

	while (Complete_flag == 0)
	{
		switch (state)
		{

		case PARSE_ADDRESS:

			msg[0] = IO_SYNC_READ8(addr_location + data_ptr);
			data_ptr += BYTES_TO_READ;
			curr_prog +=1;
			if(curr_prog >= max_data_slot)
			{
				update_progress +=1;
				curr_prog = 0;
			}

			if ((addr_flag == 1) && (msg[0] != SECTION_START) && (msg[0] != FILE_TERMINATION_CHAR))
			{
				addr_flag = 0;
				addr = 0;
				while (msg[0] != NEW_LINE)
				{
					msg[0] = Char2HexAdd(msg[0]);
					addr = (addr << (4)) | (msg[0] & 0x0F);
					msg[0] = IO_SYNC_READ8(addr_location + data_ptr);
					data_ptr += BYTES_TO_READ;
					curr_prog +=1;
					if(curr_prog >= max_data_slot)
					{
						update_progress +=1;
						curr_prog = 0;
					}
				}
				VMC_LOG("\n\radd : %x\r\n", addr);
				bsl_send_data_pkt[4] = addr & 0xFF;
				bsl_send_data_pkt[5] = addr >> 8 & 0xFF;
				bsl_send_data_pkt[6] = addr >> 16 & 0xFF;
				bsl_send_data_pkt[7] = addr >> 24 & 0xFF;
				length += 8;

				state = PARSE_DATA;
			}
			else
			{
				if (msg[0] == SECTION_START)
				{
					addr_flag = 1;
				}
				else if (msg[0] == FILE_TERMINATION_CHAR)
				{
					Complete_flag = 1;
					all_pkt_sent = true;
					update_progress = 99;
				}
				else
				{
					addr = addr + Prev_length;
					bsl_send_data_pkt[4] = addr & 0xFF;
					bsl_send_data_pkt[5] = addr >> 8 & 0xFF;
					bsl_send_data_pkt[6] = addr >> 16 & 0xFF;
					bsl_send_data_pkt[7] = addr >> 24 & 0xFF;
					length += 8;
					state = PARSE_DATA;
				}
			}

			break;

		case PARSE_DATA:

			msg[0] = IO_SYNC_READ8(addr_location + data_ptr);
			data_ptr += BYTES_TO_READ;
			curr_prog +=1;
			if(curr_prog >= max_data_slot)
			{
				update_progress +=1;
				curr_prog = 0;
			}

			while ((msg[0] != SECTION_START) && (length < (BSL_MAX_DATA_SIZE - BSL_DATA_PACKET_CRC_SIZE)) && (msg[0] != FILE_TERMINATION_CHAR))
			{
				if (msg[0] != NEW_LINE && msg[0] != SPACE)
				{
					msg[0] = Char2Hex(msg[0]);
					bsl_send_data_pkt[length] = msg[0] & 0x0F;

					msg[0] = IO_SYNC_READ8(addr_location + data_ptr);
					data_ptr += BYTES_TO_READ;
					curr_prog +=1;
					if(curr_prog >= max_data_slot)
					{
						update_progress +=1;
						curr_prog = 0;
					}
					msg[0] = Char2Hex(msg[0]);
					bsl_send_data_pkt[length] = (bsl_send_data_pkt[length] << 4) | (msg[0] & 0x0F);
					length++;
				}

				msg[0] = IO_SYNC_READ8(addr_location + data_ptr);
				data_ptr += BYTES_TO_READ;
				curr_prog +=1;
				if(curr_prog >= max_data_slot)
				{
					update_progress +=1;
					curr_prog = 0;
				}
			}
			if (msg[0] == SECTION_START)
			{
				addr_flag = 1;
			}
			else if (msg[0] == FILE_TERMINATION_CHAR)
			{
				all_pkt_sent = true;
				/* VMC_LOG("Progress before q : %d",update_progress); */
				update_progress = 99;
			}

			Complete_flag = 1;
			*pkt_length = length + 2;
			length -= 3;
			bsl_send_data_pkt[0] = BSL_MSG_HEADER;
			bsl_send_data_pkt[1] = length & 0xFF;
			bsl_send_data_pkt[2] = length >> 8 & 0xFF;
			bsl_send_data_pkt[3] = 0x20;

			Prev_length = length - 5;

			CRC_Val = crc16(&bsl_send_data_pkt[3], length);
			length += 3;
			bsl_send_data_pkt[length] = CRC_Val & 0xFF;
			bsl_send_data_pkt[length + 1] = (CRC_Val >> 8) & 0xFF;
			dataCRC = crc16(&bsl_send_data_pkt[8], Prev_length);

			break;

		default:
			break;

		}
	}
}



upgrade_status_t matchCRC_postWrite(unsigned int writeAdd)
{
	upgrade_status_t status = STATUS_SUCCESS;

//	VMC_LOG("\n\rSend CMD : BSL_CRC_RX_32 \n\r");

	u8 crcPkt[12] = {0};

	crcPkt[0] = 0x80;

	/* Pkt len */
	crcPkt[1] = 0x7;
	crcPkt[2] = 0;

	/* CMD Id */
	crcPkt[3] = 0x26;

	/* Add the Address byte */
	crcPkt[4] = writeAdd & 0xFF;
	crcPkt[5] = (writeAdd >> 8)  & 0xFF;
	crcPkt[6] = (writeAdd >> 16) & 0xFF;
	crcPkt[7] = (writeAdd >> 24) & 0xFF;

	/* Length to calculate the Data */
	crcPkt[8] = Prev_length & 0xFF;
	crcPkt[9] = (Prev_length >> 8) & 0xFF;

	/* Append the CRC */
	u16 checksum = crc16(&crcPkt[3],7);
	crcPkt[10] = checksum & 0xFF;
	crcPkt[11] = (checksum >> 8) & 0xFF;


	if( UART_RTOS_Send(&uart_vmcsc_log, crcPkt, BSL_CRC_RESQ) != UART_SUCCESS )
	{
		VMC_ERR("Uart Send Failure \r\n");
	}
	else
	{
		if( UART_RTOS_Receive(&uart_vmcsc_log, &receive_bufr[0], BSL_CRC_RESP, &receivedByteCount,RCV_TIMEOUT_MS(200)) != UART_SUCCESS )
		{
			VMC_ERR("Uart Receive Failure \r\n");
		}
		else
		{

			if(!VMC_Validate_BSL_resp(&receive_bufr[0], BSL_CRC_RESP))
			{
				if(receive_bufr[4] == BSL_CRC_SUCCESS_RESP)
				{
					u16 readCRC = ((receive_bufr[6] << 8) | (receive_bufr[5]));
					if(dataCRC != readCRC)
					{
						status = STATUS_FAILURE;
						VMC_ERR("\n\r Mismatch CRC  expected: %x Revd : %x", dataCRC, readCRC);
					}
				}
				else
				{
					status = STATUS_FAILURE;
				}
			}
		}
	}

	memset(receive_bufr,0x00,sizeof(receive_bufr));
	receivedByteCount = 0;

	return status;
}


void SCUpdateTask(void * arg)
{
	VMC_LOG(" SC Update Task Created !!!\n\r");

	/* After Bootup, this function validates fpt SC image and the version */
	VMC_Get_Fpt_SC_Version(&fpt_sc_offsets);


    while(1)
    {
    	/* Wait for notification */
        xTaskNotifyWait(ULONG_MAX, ULONG_MAX, NULL, portMAX_DELAY);

        sc_update_flag = 0x01;

		if ((upgradeState == SC_STATE_IDLE)
				&& (upgradeStatus == STATUS_SUCCESS
					|| upgradeStatus == STATUS_FAILURE))
		{
			upgradeState = SC_BSL_SYNC;
			upgradeStatus = STATUS_IN_PROGRESS;
			upgradeError = SC_UPDATE_NO_ERROR;
		}

		while(upgradeStatus == STATUS_IN_PROGRESS)
		{
			switch(upgradeState)
			{
				case SC_BSL_SYNC:
				{
					u8 sc_bsl_sync[SC_BSL_SYNCED_REQ] = {0xFF};
					memset(receive_bufr,0xFF,sizeof(receive_bufr));

					VMC_LOG("\n\rSend CMD : SC_BSL_SYNC \n\r");

					if( UART_RTOS_Send(&uart_vmcsc_log, &sc_bsl_sync[0], SC_BSL_SYNCED_REQ) != UART_SUCCESS )
					{
						VMC_ERR("Uart Send Failure \r\n");
					}
					else
					{
						if( UART_RTOS_Receive(&uart_vmcsc_log, &receive_bufr[0], SC_BSL_SYNCED_RESP, &receivedByteCount, RCV_TIMEOUT_MS(200)) != UART_SUCCESS )
						{
							VMC_ERR("Uart Receive Failure.. Retrying !! \r\n");
						}
						else
						{
							/* Check if MSP is in SC/BSL mode */
							if ((receive_bufr[0] == ESCAPE_CHAR) && (receive_bufr[1] == STX))
							{
								upgradeState = SC_ENABLE_BSL;
							}
							else if(receive_bufr[0] == BSL_SYNC_SUCCESS)
							{
								upgradeState = BSL_UNLOCK_PASSWORD;
							}
							else
							{
								VMC_LOG("SC BSL SYNC Failure.. Retrying !! \r\n");
								upgradeState = SC_BSL_SYNC;
							}
						}
					}

					memset(receive_bufr,0xFF,sizeof(receive_bufr));
					receivedByteCount = 0;
					vTaskDelay(DELAY_MS(1000));

					break;
				}

				case SC_ENABLE_BSL:
				{
					u8 enbsl[SC_ENABLE_BSL_REQ] = { 0x5C,0x2,0x1,0x0,0x0,0x1,0x0,0x5C,0x3 };

					VMC_LOG("\n\rSend CMD : SC_ENABLE_BSL \n\r");

					if( UART_RTOS_Send(&uart_vmcsc_log, &enbsl[0], SC_ENABLE_BSL_REQ) != UART_SUCCESS )
					{
						VMC_ERR("Uart Send Failure \r\n");
					}
					else
					{
						if( UART_RTOS_Receive(&uart_vmcsc_log, &receive_bufr[0], SC_ENABLE_BSL_RESP, &receivedByteCount, RCV_TIMEOUT_MS(200)) != UART_SUCCESS )
						{
							VMC_ERR("Uart Receive Failure.. Retrying !! \r\n");
						}
						else
						{
							if(receive_bufr[2] == MSP432_COMMS_MSG_GOOD)
							{
								upgradeState = BSL_SYNCED;
							}
							else
							{
								upgradeState = SC_ENABLE_BSL;
							}
						}
					}

					memset(receive_bufr,0xFF,sizeof(receive_bufr));
					receivedByteCount = 0;
					vTaskDelay(DELAY_MS(1000));

					break;
				}

				case BSL_SYNCED:
				{
					u8 syncbuf[BSL_SYNCED_REQ] = {0xFF};

					VMC_LOG("\n\rSend CMD : BSL_SYNCED \n\r");

					if( UART_RTOS_Send(&uart_vmcsc_log, &syncbuf[0], BSL_SYNCED_REQ) != UART_SUCCESS )
					{
						VMC_ERR("Uart Send Failure \r\n");
					}
					else
					{
						if( UART_RTOS_Receive(&uart_vmcsc_log, &receive_bufr[0], BSL_SYNCED_RESP, &receivedByteCount,RCV_TIMEOUT_MS(100)) != UART_SUCCESS )
						{
							VMC_ERR("Uart Receive Failure.. Retrying !! \r\n");
						}
						else
						{
							/* Will not wait for the Rx or Tx to complete,
							 * as we need send SYNC char till the time we didn't get ACK from BSL */
							if(receive_bufr[0] == BSL_SYNC_SUCCESS)
							{
								upgradeState = BSL_UNLOCK_PASSWORD;
								update_progress = 1;
							}
							else
							{
								VMC_LOG("SYNC Failure.. Retrying !! \r\n");
								upgradeState = BSL_SYNCED;
							}
						}
					}

					memset(receive_bufr,0x00,sizeof(receive_bufr));
					receivedByteCount = 0;
					vTaskDelay(DELAY_MS(1000));

				    break;
				}

				case BSL_UNLOCK_PASSWORD:
				{
					static u8 retryCount = 0;
					VMC_LOG("\n\rSend CMD : BSL_UNLOCK_PASSWORD \n\r");

					if( UART_RTOS_Send(&uart_vmcsc_log, &bslPasswd[0], BSL_UNLOCK_PASSWORD_REQ) != UART_SUCCESS )
					{
						VMC_ERR("Uart Send Failure \r\n");
					}
					else
					{
						if( UART_RTOS_Receive(&uart_vmcsc_log, &receive_bufr[0], BSL_UNLOCK_PASSWORD_RESP, &receivedByteCount,RCV_TIMEOUT_MS(200)) != UART_SUCCESS )
						{
							VMC_ERR("Uart Receive Failure.. Retrying !! \r\n");
						}
						else
						{
							if(!VMC_Validate_BSL_resp(&receive_bufr[0], BSL_UNLOCK_PASSWORD_RESP))
							{
								if(receive_bufr[4] == BSL_DATA_WRITE_RESP_FIRST_CHAR && receive_bufr[5] == BSL_DATA_WRITE_RESP_SEC_CHAR)
								{
									upgradeState = BSL_MASS_ERASE;
									retryCount = 0;
								}
								else
								{
									upgradeState = BSL_UNLOCK_PASSWORD;
								}
							}
							else
							{
								retryCount +=1;
								upgradeState = BSL_UNLOCK_PASSWORD;
							}
						}
					}

					if(retryCount == SC_UPDATE_MAX_RETRY_COUNT)
					{
						retryCount = 0;
						upgradeError = SC_UPDATE_ERROR_BSL_UNLOCK_PASSWORD_FAILED;
						update_progress = upgradeError;
						upgradeStatus = STATUS_FAILURE;
						upgradeState = SC_STATE_IDLE;

						/* Stop timer as update failed */
						if(xTimerStop(xSCUpdateTimeOutTimer, 0) != pdPASS)
						{
							VMC_ERR("\n\rFailed to stop xSCUpdateTimeOutTimer \r\n");
						}

						VMC_LOG("\n\rUpdate failure. Retry.. \n\r");
					}

					memset(receive_bufr,0x00,sizeof(receive_bufr));
					receivedByteCount = 0;
					vTaskDelay(DELAY_MS(1000));

					break;
				}

				case BSL_MASS_ERASE:
				{
					u8 massErase[BSL_MASS_ERASE_REQ] = { 0x80,0x01,0x00,0x15,0x64,0xA3 };

					VMC_LOG("\n\rSend CMD : BSL_MASS_ERASE \n\r");

					if( UART_RTOS_Send(&uart_vmcsc_log, &massErase[0], BSL_MASS_ERASE_REQ) != UART_SUCCESS )
					{
						VMC_ERR("Uart Send Failure \r\n");
					}
					else
					{
						if( UART_RTOS_Receive(&uart_vmcsc_log, &receive_bufr[0], BSL_MASS_ERASE_RESP, &receivedByteCount,RCV_TIMEOUT_MS(200)) != UART_SUCCESS )
						{
							VMC_ERR("Uart Receive Failure \r\n");
						}
						else
						{
							if(!VMC_Validate_BSL_resp(&receive_bufr[0], BSL_MASS_ERASE_RESP))
							{
								if(receive_bufr[4] == BSL_DATA_WRITE_RESP_FIRST_CHAR && receive_bufr[5] == BSL_DATA_WRITE_RESP_SEC_CHAR)
								{
									upgradeState = BSL_DATA_TX_32;
								}
								else
								{
									upgradeState = BSL_MASS_ERASE;
								}
							}
							else
							{
								upgradeState = BSL_MASS_ERASE;
							}
						}
					}

					memset(receive_bufr,0x00,sizeof(receive_bufr));
					receivedByteCount = 0;
					vTaskDelay(DELAY_MS(20));

					break;
				}

				case BSL_SECTOR_ERASE:
				{
					VMC_LOG("\n\rSend CMD : BSL_SECTOR_ERASE \n\r");

					break;
				}

				case BSL_DATA_TX_32:
				{
					u8 start_symbol = 0x00;
					bool sc_available_for_parsing = false;
					data_ptr = 0x00;
					all_pkt_sent = false;
					max_data_slot = ((fpt_sc_loc.size/100) + ((fpt_sc_loc.size%100)));

					VMC_LOG("\n\rSend CMD : BSL_DATA_TX_32 \n\r");

					start_symbol = IO_SYNC_READ8(fpt_sc_loc.start_address + SC_TOT_HEADER_SIZE);
					if(start_symbol == SECTION_START)
					{
						addr_flag = 0x01;
						VMC_LOG("symbol matched !!\r\n");
						sc_available_for_parsing = true;
					}
					else
					{
						VMC_ERR("Invalid symbol... \r\n");
						upgradeError = SC_UPDATE_ERROR_INVALID_SC_SYMBOL_FOUND;
						update_progress = upgradeError;
						upgradeStatus = STATUS_FAILURE;
						upgradeState = SC_STATE_IDLE;

						/* Stop timer as update failed */
						if(xTimerStop(xSCUpdateTimeOutTimer, 0) != pdPASS)
						{
							VMC_ERR("\n\rFailed to stop xSCUpdateTimeOutTimer \r\n");
						}

						break;
					}


					if(sc_available_for_parsing)
					{
						sc_available_for_parsing = false;

						while(all_pkt_sent != true)
						{
							memset(bsl_send_data_pkt, 0x00, BSL_MAX_DATA_SIZE);
							memset(receive_bufr, 0x00, sizeof(receive_bufr));
							receivedByteCount = 0x00;

							portENTER_CRITICAL();
							VMC_Parse_Fpt_SC((fpt_sc_loc.start_address + SC_TOT_HEADER_SIZE), &bsl_send_data_pkt[0], &total_length);
							portEXIT_CRITICAL();

							if( UART_RTOS_Send(&uart_vmcsc_log, &bsl_send_data_pkt[0], total_length) != UART_SUCCESS )
							{
								VMC_ERR("Uart Send Failure \r\n");
							}
							else
							{
								if( UART_RTOS_Receive(&uart_vmcsc_log, &receive_bufr[0], BSL_DATA_TX_32_RESP, &receivedByteCount,RCV_TIMEOUT_MS(200)) != UART_SUCCESS )
								{
									VMC_ERR("Uart Receive Failure \r\n");
								}
								else
								{
									if(!VMC_Validate_BSL_resp(&receive_bufr[0], BSL_DATA_TX_32_RESP))
									{
										if(receive_bufr[4] == BSL_DATA_WRITE_RESP_FIRST_CHAR && receive_bufr[5] == BSL_DATA_WRITE_RESP_SEC_CHAR)
										{
											if( matchCRC_postWrite(addr) != STATUS_SUCCESS )
											{
												upgradeStatus = STATUS_FAILURE;
												upgradeState = SC_STATE_IDLE;
												upgradeError = SC_UPDATE_ERROR_POST_CRC_FAILED;
												update_progress = upgradeError;
												data_ptr = 0x00;
												VMC_ERR("\r\nPost CRC failed... \r\n");
												break;
											}
										}
										else
										{
											upgradeError = SC_UPDATE_ERROR_INVALID_BSL_RESP;
											update_progress = upgradeError;
											VMC_LOG("\r\nBSL Resp failure. Retry.. \r\n");
										}
									}
									else
									{
										upgradeStatus = STATUS_FAILURE;
										upgradeState = SC_STATE_IDLE;
										upgradeError = SC_UPDATE_ERROR_INVALID_BSL_RESP;
										update_progress = upgradeError;
										data_ptr = 0x00;
										VMC_LOG("\r\nReceived undesirable response from BSL... \r\n");
										break;
									}
								}
							}
							/* VMC_LOG("\n\rProgress : %d \n\r",update_progress); */
							/* Packet to packet 10mSec */
							vTaskDelay(DELAY_MS(10));
						}
					}
					else
					{
						data_ptr = 0x00;
						upgradeError = SC_UPDATE_ERROR_NO_VALID_FPT_SC_FOUND;
						update_progress = upgradeError;
						upgradeStatus = STATUS_FAILURE;
						upgradeState = SC_STATE_IDLE;
						VMC_LOG("\r\nNo Valid SC to parse... \r\n");
						break;
					}

					/* Clean up */
					msg[0] = 0;
					data_ptr = 0x00;
					curr_prog = 0;
					max_data_slot = 0;
					receivedByteCount = 0;
					memset(receive_bufr, 0x00, sizeof(receive_bufr));
					vTaskDelay(DELAY_MS(20));

					upgradeState = BSL_LOAD_PC_32;

					break;
				}

				case BSL_CRC_RX_32:
				{
					VMC_LOG("\n\rSend CMD : BSL_CRC_RX_32 \n\r");

					break;
				}

				case BSL_LOAD_PC_32:
				{
					u8 load_pc[BSL_LOAD_PC_REQ] = { 0x80,0x05,0x00,0x27,0x01,0x02,00,00,0xB8,0x66 };

					VMC_LOG("\n\rSend CMD : BSL_LOAD_PC_32 \n\r");

					if( UART_RTOS_Send(&uart_vmcsc_log, &load_pc[0], BSL_LOAD_PC_REQ) != UART_SUCCESS )
					{
						VMC_ERR("Uart Send Failure \r\n");
					}
					else
					{
						curr_prog = 0;
						max_data_slot = 0;
						update_progress +=1;
						if(update_progress == 100)
							VMC_LOG("\n\rUpdate Complete : %d%% \n\r",update_progress);

						VMC_LOG("SC Application Loaded !!\n\r");

						upgradeStatus = STATUS_SUCCESS;
						upgradeState = SC_STATE_IDLE;
					}

					/* Stop timer as update completed */
					if(xTimerStop(xSCUpdateTimeOutTimer, 0) != pdPASS)
					{
						VMC_ERR("\n\rFailed to stop xSCUpdateTimeOutTimer \r\n");
					}

					memset(receive_bufr,0x00,sizeof(receive_bufr));
					receivedByteCount = 0;

					break;
				}

				case MSP_GET_STATUS:
				{
					VMC_LOG("\n\rSend CMD : MSP_GET_STATUS \n\r");

					break;
				}

				case BSL_REBOOT_RESET:
				{
					/* NOTE: After the reboot reset it takes approximately 10mSec before the BSL is started again. */
					u8 rebootreset[BSL_REBOOT_RESET_REQ] = { 0x80,0x01,0x00,0x25,0x37,0x95 };

					VMC_LOG("\n\rSend CMD : BSL_REBOOT_RESET \n\r");

					if( UART_RTOS_Send(&uart_vmcsc_log, &rebootreset[0], BSL_REBOOT_RESET_REQ) != UART_SUCCESS )
					{
						VMC_ERR("Uart Send Failure \r\n");
					}
					else
					{
						/* Reset the progress variables */
						update_progress = 0;
						curr_prog = 0;
						max_data_slot = 0;

						upgradeStatus = STATUS_SUCCESS;
						upgradeState = SC_STATE_IDLE;
						upgradeError = SC_UPDATE_NO_ERROR;

						VMC_LOG("SuC BSL Rebooted !!\n\r");
					}

					memset(receive_bufr,0x00,sizeof(receive_bufr));
					receivedByteCount = 0;
					vTaskDelay(DELAY_MS(1000));

					break;
				}

				case TX_BSL_VERSION:
				{
					u8 version[BSL_VERSION_REQ] = { 0x80,0x01,0x00,0x19,0xE8,0x62 };

					VMC_LOG("\n\rSend CMD : TX_BSL_VERSION \n\r");

					if( UART_RTOS_Send(&uart_vmcsc_log, &version[0], BSL_VERSION_REQ) != UART_SUCCESS )
					{
						VMC_ERR("Uart Send Failure \r\n");
					}
					else
					{
						if( UART_RTOS_Receive(&uart_vmcsc_log, &receive_bufr[0], BSL_VERSION_RESP, &receivedByteCount,RCV_TIMEOUT_MS(200)) != UART_SUCCESS )
						{
							VMC_ERR("Uart Receive Failure \r\n");
						}
						else
						{
							if(!VMC_Validate_BSL_resp(&receive_bufr[0], BSL_VERSION_RESP))
							{
								if(receive_bufr[13] == 0x00 && receive_bufr[14] == 0x0D)
								{
									upgradeStatus = STATUS_SUCCESS;
									upgradeState = SC_STATE_IDLE;
								}
								else
								{
									upgradeState = TX_BSL_VERSION;
								}
							}
							else
							{
								upgradeState = TX_BSL_VERSION;
							}
						}
					}

					memset(receive_bufr,0x00,sizeof(receive_bufr));
					receivedByteCount = 0;

					/* Reset the progress variables */
					update_progress = 0;
					curr_prog = 0;
					max_data_slot = 0;

					vTaskDelay(DELAY_MS(1000));

					break;
				}

				default:
					VMC_LOG("\n\rInvalid State return \r\n");
					upgradeStatus = STATUS_FAILURE;
					upgradeState = SC_STATE_IDLE;
			}
		}
		/* Waiting for 5Sec so that XRT will get the updated progress of SC update */
		vTaskDelay(DELAY_MS(1000 * 5));

		/* Reset progress variables */
		update_progress = 0;
		curr_prog = 0;
		max_data_slot = 0;
		upgradeError = SC_UPDATE_NO_ERROR;

		/* Release done flag */
		sc_update_flag = 0x00;
    }

    vTaskSuspend(NULL);
}

void SC_Update_Task_Create(void)
{
    /* Create SC update task */
    if (xTaskCreate(SCUpdateTask,
    		        (const char *) "SC_Update_Task",
					TASK_STACK_DEPTH,
					NULL,
					tskIDLE_PRIORITY + 1,
					&xSCUpdateTaskHandle) != pdPASS)
    {
    	CL_LOG(APP_VMC,"Failed to Create SC Update Task \n\r");
    	return ;
	}
}
