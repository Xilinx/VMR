
#include "FreeRTOS.h"
#include <semphr.h>
#include "task.h"
#include "vmc_api.h"
#include "cl_msg.h"
#include "cl_uart_rtos.h"
#include "vmc_update_sc.h"
#include "vmc_sc_comms.h"


upgrade_status upgradeStatus = STATUS_SUCCESS;
upgrade_state upgradeState = SC_STATE_IDLE;

TaskHandle_t xSCUpdateTaskHandle = NULL;
extern uart_rtos_handle_t uart_vmcsc_log;
extern TaskHandle_t xVMCSCTask;

#define SC_UPDATE_RECV_TIMEOUT      (10)
/* Global structures */
SC_VMC_Data sc_version;
cl_msg_t fpt_sc_offsets;
efpt_sc_t fpt_sc_loc;

/* Variables for fw update data packets*/
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

u8 sc_update_required = 0x00;

/* Variable to keep track whether SC update is going on or not */
u8 sc_update_flag = 0x00;

/* Variables will keep track of SC update progress */
int32_t update_progress = 0;
u32 curr_prog = 0;
u32 max_data_slot = 0;

u8 receive_bufr[32] = {0xFF};
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


void VMC_Get_Fpt_SC_Version(cl_msg_t *msg)
{
	u8 read_buffer[SC_HEADER_SIZE] = {0};
	u8 header[SC_HEADER_SIZE] = SC_HEADER_MSG;
	u8 retVal = 0x01;
	data_ptr = 0x00;


	rmgmt_extension_fpt_query(msg);

	fpt_sc_loc.start_address = msg->multiboot_payload.scfw_offset;
	fpt_sc_loc.size = msg->multiboot_payload.scfw_size;

	VMC_LOG("\n\rSC Base Addr : 0x%x\r\n",fpt_sc_loc.start_address);
	VMC_LOG("\n\rSC size : 0x%x\r\n",fpt_sc_loc.size);

	VMC_Read_Data32((u32 *) fpt_sc_loc.start_address, (u32 *)read_buffer, (SC_HEADER_SIZE+1)/4);

	retVal = Check_Received_SC_Header(read_buffer,header,sizeof(read_buffer));
	if(retVal == 0x00)
	{

		VMC_LOG("\n\rSC Identification Passed !!\r\n");

		portENTER_CRITICAL();
		VMC_Parse_Fpt_SC_Version(((fpt_sc_loc.start_address + SC_TOT_HEADER_SIZE + fpt_sc_loc.size) - 0x33), &fpt_sc_version[0]);
		portEXIT_CRITICAL();

		VMC_LOG("\n\rFpt SC version : v%d.%d.%d\r\n",fpt_sc_version[0], fpt_sc_version[1], fpt_sc_version[2]);

	}
	else
	{
		VMC_ERR("\n\rSC Identification failed !!\r\n");
	}

}

int32_t VMC_Start_SC_Update(void)
{
	int32_t retVal = 0;

	retVal = Get_SC_Checksum();
	if(retVal)
	{
		VMC_LOG("\n\rSC Needs Update !!\r\n");

		if (xTaskNotify(xSCUpdateTaskHandle, 0, eNoAction) != pdPASS)
		{
			VMC_ERR("Notification failed !!\r\n");
			retVal = -1;
		}
	}
	else
	{
		VMC_LOG("\n\rSC Up-to-date !!\r\n");
		retVal = -1;
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



upgrade_status matchCRC_postWrite(unsigned int writeAdd)
{
	upgrade_status status = STATUS_SUCCESS;

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
		if( UART_RTOS_Receive(&uart_vmcsc_log, &receive_bufr[0], BSL_CRC_RESP, &receivedByteCount,SC_UPDATE_RECV_TIMEOUT) != UART_SUCCESS )
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
        vTaskSuspend(xVMCSCTask);

		if(upgradeState == SC_STATE_IDLE && upgradeStatus == STATUS_SUCCESS)
		{
			upgradeState = SC_ENABLE_BSL;
			upgradeStatus = STATUS_IN_PROGRESS;
		}

		while(upgradeStatus == STATUS_IN_PROGRESS)
		{
			switch(upgradeState)
			{
				case SC_ENABLE_BSL:
				{
					VMC_LOG("\n\rSend CMD : SC_ENABLE_BSL \n\r");

					VMC_send_packet(MSP432_COMMS_EN_BSL,MSP432_COMMS_NO_FLAG,0x00,0x00);

				    upgradeState = BSL_SYNCED;

				    /* TODO: Replace sleep with vTaskDelay*/
					sleep(2);

					break;
				}

				case BSL_SYNCED:
				{
					u8 syncbuf[BSL_SYNCED_REQ] = {0xFF};
					static u8 flag = 0;

					VMC_LOG("\n\rSend CMD : BSL_SYNCED \n\r");

					if(!flag){
						UART_VMC_SC_Enable(&uart_vmcsc_log);
						flag = 1;
					}

					if( UART_RTOS_Send(&uart_vmcsc_log, &syncbuf[0], BSL_SYNCED_REQ) != UART_SUCCESS )
					{
						VMC_ERR("Uart Send Failure \r\n");
					}
					else
					{
						if( UART_RTOS_Receive(&uart_vmcsc_log, &receive_bufr[0], BSL_SYNCED_RESP, &receivedByteCount,SC_UPDATE_RECV_TIMEOUT) != UART_SUCCESS )
						{
							VMC_ERR("Uart Receive Failure \r\n");
						}
						else
						{
							/* Will not wait for the Rx or Tx to complete,
							 * as we need send SYNC char till the time we didn't get ACK from BSL */
							if(receive_bufr[0] == BSL_SYNC_SUCCESS)
							{
								upgradeState = BSL_UNLOCK_PASSWORD;
								update_progress = 1;
								flag = 0;
							}
							else
							{
								upgradeState = BSL_SYNCED;
							}
						}
					}

					memset(receive_bufr,0x00,sizeof(receive_bufr));

					sleep(1);

				    break;
				}

				case BSL_UNLOCK_PASSWORD:
				{

					VMC_LOG("\n\rSend CMD : BSL_UNLOCK_PASSWORD \n\r");

					if( UART_RTOS_Send(&uart_vmcsc_log, &bslPasswd[0], BSL_UNLOCK_PASSWORD_REQ) != UART_SUCCESS )
					{
						VMC_ERR("Uart Send Failure \r\n");
					}
					else
					{
						if( UART_RTOS_Receive(&uart_vmcsc_log, &receive_bufr[0], BSL_UNLOCK_PASSWORD_RESP, &receivedByteCount,SC_UPDATE_RECV_TIMEOUT) != UART_SUCCESS )
						{
							VMC_ERR("Uart Receive Failure \r\n");
						}
						else
						{
							if(!VMC_Validate_BSL_resp(&receive_bufr[0], BSL_UNLOCK_PASSWORD_RESP))
							{
								if(receive_bufr[4] == BSL_DATA_WRITE_RESP_FIRST_CHAR && receive_bufr[5] == BSL_DATA_WRITE_RESP_SEC_CHAR)
								{
									upgradeState = BSL_MASS_ERASE;
								}
								else
								{
									upgradeState = BSL_UNLOCK_PASSWORD;
								}
							}
							else
							{
								upgradeState = BSL_UNLOCK_PASSWORD;
							}
						}
					}

					memset(receive_bufr,0x00,sizeof(receive_bufr));

					usleep(20000);

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
						if( UART_RTOS_Receive(&uart_vmcsc_log, &receive_bufr[0], BSL_MASS_ERASE_RESP, &receivedByteCount,SC_UPDATE_RECV_TIMEOUT) != UART_SUCCESS )
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
									upgradeState = BSL_UNLOCK_PASSWORD;
								}
							}
							else
							{
								upgradeState = BSL_UNLOCK_PASSWORD;
							}
						}
					}

					memset(receive_bufr,0x00,sizeof(receive_bufr));

					usleep(20000);

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
						VMC_ERR("STATUS FALIURE \r\n");
						upgradeStatus = STATUS_FAILURE;
						upgradeState = SC_STATE_IDLE;
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
								if( UART_RTOS_Receive(&uart_vmcsc_log, &receive_bufr[0], BSL_DATA_TX_32_RESP, &receivedByteCount,SC_UPDATE_RECV_TIMEOUT) != UART_SUCCESS )
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
												update_progress = 0xA5;
												data_ptr = 0x00;
												VMC_ERR("\r\nPost CRC failed... \r\n");
												break;
											}
										}
										else
										{
											VMC_ERR("STATUS FALIURE \r\n");
											update_progress = 0xA5;
										}
									}
									else
									{
										upgradeStatus = STATUS_FAILURE;
										upgradeState = SC_STATE_IDLE;
										update_progress = 0xA5;
										data_ptr = 0x00;
										break;
									}
								}
							}
//							xil_printf("\n\rProgress : %d \n\r",update_progress);
							/* Packet to packet 5mSec */
							usleep(5000);
						}
					}
					else
					{
						VMC_ERR("STATUS FALIURE \r\n");
						data_ptr = 0x00;
						upgradeStatus = STATUS_FAILURE;
						upgradeState = SC_STATE_IDLE;
						break;
					}

					msg[0] = 0;
					data_ptr = 0x00;
					memset(receive_bufr, 0x00, sizeof(receive_bufr));

					usleep(20000);

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
						upgradeStatus = STATUS_SUCCESS;
						upgradeState = SC_STATE_IDLE;
						update_progress +=1;
						if(update_progress == 100)
							VMC_LOG("\n\rUpdate Complete : %d%% \n\r",update_progress);

						/* Reset the progress variables */
						update_progress = 0;
						curr_prog = 0;
						max_data_slot = 0;

						VMC_LOG("SC Application Loaded !!\n\r");
					}

					memset(receive_bufr,0x00,sizeof(receive_bufr));

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

						VMC_LOG("SuC BSL Rebooted !!\n\r");
					}

					memset(receive_bufr,0x00,sizeof(receive_bufr));

					sleep(1);

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
						if( UART_RTOS_Receive(&uart_vmcsc_log, &receive_bufr[0], BSL_VERSION_RESP, &receivedByteCount,SC_UPDATE_RECV_TIMEOUT) != UART_SUCCESS )
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
									upgradeState = BSL_UNLOCK_PASSWORD;
								}
							}
							else
							{
								upgradeState = BSL_UNLOCK_PASSWORD;
							}
						}
					}

					memset(receive_bufr,0x00,sizeof(receive_bufr));

					/* Reset the progress variables */
					update_progress = 0;
					curr_prog = 0;
					max_data_slot = 0;

					break;
				}

				default:
					VMC_LOG("\n\rInvalid State return \r\n");
					upgradeStatus = STATUS_FAILURE;
					upgradeState = SC_STATE_IDLE;
			}
		}
		sc_update_flag = 0x00;
		vTaskResume(xVMCSCTask);
    }

    vTaskSuspend(NULL);
}


void SC_Update_Task_Create(void)
{
    /* Create the SC update task */
    if (xTaskCreate(SCUpdateTask,
    		        (const char *) "SC_Update_Task",
					1024,
					NULL,
					tskIDLE_PRIORITY + 2,
					&xSCUpdateTaskHandle) != pdPASS)
    {
    	CL_LOG(APP_VMC,"Failed to Create SC Update Task \n\r");
    	return ;
	}

}
