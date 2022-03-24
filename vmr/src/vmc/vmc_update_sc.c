/******************************************************************************
 * Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
 * SPDX-License-Identifier: MIT
 *******************************************************************************/

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

#include "vmc_api.h"
#include "cl_msg.h"
#include "cl_uart_rtos.h"
#include "vmc_update_sc.h"
#include "vmc_sc_comms.h"
#include "../rmgmt/rmgmt_fpt.h"


static TaskHandle_t xSCUpdateTaskHandle = NULL;

extern SemaphoreHandle_t vmc_sc_comms_lock;
extern SemaphoreHandle_t vmc_sensor_monitoring_lock;
extern uart_rtos_handle_t uart_vmcsc_log;
extern SC_VMC_Data sc_vmc_data;
extern volatile bool isVMCActive;

/* Global structures */
efpt_sc_t FptScLoc;

/* Variables for SC update data packets */
u8 fpt_sc_version[MAX_SC_VERSION_SIZE] = {0x00};
static u32 Addr = 0x00;
static u32 DataPtr = 0x00;
static u32 PrevLength = 0x00;
static bool AddrFlag = false;
static bool AllPktSent = false;
static bool fptSCvalid = false;

/* Variables to keep track of SC update progress */
int32_t UpdateProgress = 0;
static u32 CurrProg = 0;
static u32 MaxDataSlot = 0;

u8 receive_bufr[BSL_MAX_RCV_DATA_SIZE] = {0x00};
static u32 ReceivedByteCount = 0x00;

static upgrade_status_t upgradeStatus = STATUS_SUCCESS;
static upgrade_state_t upgradeState = SC_STATE_IDLE;
static upgrade_error_t upgradeError = SC_UPDATE_NO_ERROR;



inline int32_t VMC_SCFW_Program_Progress(void) {
	return UpdateProgress;
}

static inline void VmcReadData32(u32 *addr, u32 *data, size_t sz) {
	u32 i = 0;

	for (i = 0; i < sz; ++i) {
		*(data + i) = IO_SYNC_READ32((UINTPTR )(addr + i));
	}
}

static inline u16 Crc16(u8 *pMsg, u32 size) {
	u16 crc = 0xFFFF;
	u16 i = 0;

	for (i = 0; i < size; i++) {
		u16 x;

		x = ((crc >> 8) ^ pMsg[i]) & 0xff;
		x ^= x >> 4;
		crc = (crc << 8) ^ (x << 12) ^ (x << 5) ^ x;
	}

	return crc;
}

static inline u8 Char2HexAdd(u8 C) {
	if (C >= '0' && C <= '9') {
		C = C - '0';
	} else if (C >= 'a' && C <= 'f') {
		C = C - 'a' + 10;
	}

	return (C & 0x0F);
}

static inline u8 Char2Hex(u8 C) {
	if (C >= '0' && C <= '9') {
		C = C - '0';
	} else if (C >= 'A' && C <= 'F') {
		C = C - 'A' + 10;
	}

	return (C & 0x0F);
}

static inline void UpdateFlags(upgrade_status_t status, upgrade_state_t state,
		upgrade_error_t error) {
	upgradeError = error;
	UpdateProgress = upgradeError;
	upgradeStatus = status;
	upgradeState = state;
}

static inline int32_t VmcValidateBslResp(u8 *buffer, u16 length) {
	int32_t Status = -1;
	u16 DataLen = 0;
	u16 RecvdCRC = 0;
	u16 CalcCRC = 0;

	if (buffer[0] == 0x00 && buffer[1] == BSL_MSG_HEADER) {
		DataLen = ((buffer[BSL_RESP_LEN_OFFSET_H] << 8)
				| buffer[BSL_RESP_LEN_OFFSET_L]);

		RecvdCRC = ((buffer[length - 1] << 8) | (buffer[length - 2]));
		CalcCRC = Crc16(&buffer[BSL_RESP_CMD_OFFSET], DataLen);

		if (RecvdCRC == CalcCRC) {
			Status = 0;
		}
	}

	return Status;
}

static inline u8 CheckReceivedSCHeader(void *ptr1, void *ptr2, u8 len) {
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

static inline s32 GetSCChecksum(void) {
	/* Get the SC version and compare it with the backup one, then return accordingly. */
	s32 RetVal = UPDATE_REQUIRED;

	return RetVal;
}

int32_t VMC_Start_SC_Update(void) {
	int32_t RetVal = 0;

	RetVal = GetSCChecksum();
	if ((RetVal) && (fptSCvalid)) {
		VMC_LOG("SC requires an update !!");

		if (xTaskNotify(xSCUpdateTaskHandle, 0, eNoAction) != pdPASS) {
			RetVal = -1;
			VMC_ERR("Notification failed !!");
		}
	} else {
		if (!fptSCvalid) {
			VMC_ERR("No valid SC available !!");
			RetVal = -1;
		} else if (!RetVal) {
			VMC_LOG("SC Up-to-date !!");
			RetVal = -1;
		}
	}

	return RetVal;
}

void VmcParseFptSCVersion(u32 addr_location, u8 *versionbuff) {
	u8 State = PARSE_ADDRESS;
	u8 CompleteFlag = 0;
	u8 VersionPtr = 0x00;
	u8 Msg = 0x00;
	bool VersionFlag = false;

	while (CompleteFlag == 0) {
		switch (State) {
		case PARSE_ADDRESS:

			Msg = IO_SYNC_READ8(addr_location + DataPtr);
			DataPtr += BYTES_TO_READ;

			if ((AddrFlag == true) && (Msg != SECTION_START)
					&& (Msg != FILE_TERMINATION_CHAR)) {
				AddrFlag = false;
				Addr = 0;
				while (Msg != NEW_LINE) {
					Msg = Char2HexAdd(Msg);
					Addr = ((Addr << (4)) | (Msg & 0x0F));
					Msg = IO_SYNC_READ8(addr_location + DataPtr);
					DataPtr += BYTES_TO_READ;
				}

				if ((Addr == SC_VER_MAJ_START_ADDR)
						|| (Addr == SC_VER_MIN_START_ADDR)
						|| (Addr == SC_VER_REV_START_ADDR)) {
					VersionFlag = true;
				}

				State = PARSE_DATA;
			} else {
				if (Msg == SECTION_START) {
					AddrFlag = true;
				} else if (Msg == FILE_TERMINATION_CHAR) {
					CompleteFlag = 1;
				} else {
					State = PARSE_DATA;
				}
			}
			break;

		case PARSE_DATA:

			Msg = IO_SYNC_READ8(addr_location + DataPtr);
			DataPtr += BYTES_TO_READ;

			while ((Msg != SECTION_START) && (Msg != FILE_TERMINATION_CHAR)) {
				if ((Msg != NEW_LINE) && (Msg != SPACE)) {
					Msg = Char2Hex(Msg);
					if (VersionFlag)
						versionbuff[VersionPtr] = (Msg & 0x0F);

					if ((Msg == NEW_LINE) || (Msg == SPACE)
							|| (Msg == SECTION_START)) {
						AddrFlag = true;
						VersionFlag = false;
						State = PARSE_ADDRESS;
						break;
					}

					Msg = IO_SYNC_READ8(addr_location + DataPtr);
					DataPtr += BYTES_TO_READ;
					Msg = Char2Hex(Msg);
					if (VersionFlag) {
						versionbuff[VersionPtr] =
								((versionbuff[VersionPtr] << 4) | (Msg & 0x0F));
						VersionPtr++;
					}
				}

				Msg = IO_SYNC_READ8(addr_location + DataPtr);
				DataPtr += BYTES_TO_READ;
			}
			if (Msg == SECTION_START) {
				AddrFlag = true;
				State = PARSE_ADDRESS;
				VersionFlag = false;
			} else if (Msg == FILE_TERMINATION_CHAR) {
				CompleteFlag = 1;
			}
			break;

		default:
			break;
		}
	}
}

void VmcParseFptSC(u32 addr_location, u8 *bsl_send_data_pkt, u16 *pkt_length,
		u16 *dataCRC) {
	u8 State = PARSE_ADDRESS;
	u8 CompleteFlag = 0;
	u32 Length = 0;
	u8 Msg = 0x00;
	u16 CRCVal = 0x00;

	while (CompleteFlag == 0) {
		switch (State) {
		case PARSE_ADDRESS:

			Msg = IO_SYNC_READ8(addr_location + DataPtr);
			DataPtr += BYTES_TO_READ;
			CurrProg += 1;
			if (CurrProg >= MaxDataSlot) {
				UpdateProgress += 1;
				CurrProg = 0;
			}

			if ((AddrFlag == true) && (Msg != SECTION_START)
					&& (Msg != FILE_TERMINATION_CHAR)) {
				AddrFlag = false;
				Addr = 0;
				while (Msg != NEW_LINE) {
					Msg = Char2HexAdd(Msg);
					Addr = ((Addr << (4)) | (Msg & 0x0F));
					Msg = IO_SYNC_READ8(addr_location + DataPtr);
					DataPtr += BYTES_TO_READ;
					CurrProg += 1;
					if (CurrProg >= MaxDataSlot) {
						UpdateProgress += 1;
						CurrProg = 0;
					}
				}
				VMC_LOG("Addr : %x", Addr);
				bsl_send_data_pkt[4] = Addr & 0xFF;
				bsl_send_data_pkt[5] = Addr >> 8 & 0xFF;
				bsl_send_data_pkt[6] = Addr >> 16 & 0xFF;
				bsl_send_data_pkt[7] = Addr >> 24 & 0xFF;
				Length += 8;

				State = PARSE_DATA;
			} else {
				if (Msg == SECTION_START) {
					AddrFlag = true;
				} else if (Msg == FILE_TERMINATION_CHAR) {
					CompleteFlag = 1;
					AllPktSent = true;
					UpdateProgress = 99;
				} else {
					Addr = Addr + PrevLength;
					bsl_send_data_pkt[4] = Addr & 0xFF;
					bsl_send_data_pkt[5] = Addr >> 8 & 0xFF;
					bsl_send_data_pkt[6] = Addr >> 16 & 0xFF;
					bsl_send_data_pkt[7] = Addr >> 24 & 0xFF;
					Length += 8;
					State = PARSE_DATA;
				}
			}
			break;

		case PARSE_DATA:

			Msg = IO_SYNC_READ8(addr_location + DataPtr);
			DataPtr += BYTES_TO_READ;
			CurrProg += 1;
			if (CurrProg >= MaxDataSlot) {
				UpdateProgress += 1;
				CurrProg = 0;
			}

			while ((Msg != SECTION_START)
					&& (Length < (BSL_MAX_DATA_SIZE - BSL_DATA_PACKET_CRC_SIZE))
					&& (Msg != FILE_TERMINATION_CHAR)) {
				if ((Msg != NEW_LINE) && (Msg != SPACE)) {
					Msg = Char2Hex(Msg);
					bsl_send_data_pkt[Length] = (Msg & 0x0F);

					Msg = IO_SYNC_READ8(addr_location + DataPtr);
					DataPtr += BYTES_TO_READ;
					CurrProg += 1;
					if (CurrProg >= MaxDataSlot) {
						UpdateProgress += 1;
						CurrProg = 0;
					}
					Msg = Char2Hex(Msg);
					bsl_send_data_pkt[Length] =
							((bsl_send_data_pkt[Length] << 4) | (Msg & 0x0F));
					Length++;
				}

				Msg = IO_SYNC_READ8(addr_location + DataPtr);
				DataPtr += BYTES_TO_READ;
				CurrProg += 1;
				if (CurrProg >= MaxDataSlot) {
					UpdateProgress += 1;
					CurrProg = 0;
				}
			}
			if (Msg == SECTION_START) {
				AddrFlag = true;
			} else if (Msg == FILE_TERMINATION_CHAR) {
				AllPktSent = true;
				UpdateProgress = 99;
			}

			CompleteFlag = 1;
			*pkt_length = Length + 2;
			Length -= 3;
			bsl_send_data_pkt[0] = BSL_MSG_HEADER;
			bsl_send_data_pkt[1] = (Length & 0xFF);
			bsl_send_data_pkt[2] = ((Length >> 8) & 0xFF);
			bsl_send_data_pkt[3] = CMD_RX_DATA_BLOCK_32;

			PrevLength = Length - 5;

			CRCVal = Crc16(&bsl_send_data_pkt[3], Length);
			Length += 3;
			bsl_send_data_pkt[Length] = CRCVal & 0xFF;
			bsl_send_data_pkt[Length + 1] = ((CRCVal >> 8) & 0xFF);
			*dataCRC = Crc16(&bsl_send_data_pkt[8], PrevLength);

			break;

		default:
			break;
		}
	}
}

void VmcGetFptSCVersion(void) {
	u8 ReadBuffer[SC_HEADER_SIZE + 1] = { 0 };
	u8 Header[] = SC_HEADER_MSG;
	u8 FptScStatus = INVALID_SC;
	DataPtr = 0x00;

	cl_msg_t Msg = { 0 };

	/* Fetch the fpt SC start address and size. */
	rmgmt_extension_fpt_query(&Msg);

	FptScLoc.start_address = Msg.multiboot_payload.scfw_offset;
	FptScLoc.size = Msg.multiboot_payload.scfw_size;

	VMC_LOG("Fpt SC base addr : 0x%x ", FptScLoc.start_address);
	VMC_LOG("Fpt SC size : 0x%x ", FptScLoc.size);

	VmcReadData32((u32 *) FptScLoc.start_address, (u32 *) ReadBuffer,
			(SC_HEADER_SIZE + 1) / 4);

	/* Check the fpt SC header against the expected header. */
	FptScStatus = CheckReceivedSCHeader(&ReadBuffer[0], &Header[0],
			(sizeof(ReadBuffer) - 1));
	if (VALID_SC == FptScStatus) {
		fptSCvalid = true;
		VMC_LOG("SC Identification : Pass !!");

		portENTER_CRITICAL();
		VmcParseFptSCVersion(
				((FptScLoc.start_address + SC_TOT_HEADER_SIZE + FptScLoc.size)
						- SC_VER_START_ADDR_WO_CHKSUM_BIN), &fpt_sc_version[0]);
		portEXIT_CRITICAL();

		VMC_LOG("Fpt SC version : v%d.%d.%d ",fpt_sc_version[MAJOR],
				fpt_sc_version[MINOR], fpt_sc_version[REVISION]);
	} else {
		fptSCvalid = false;
		VMC_ERR("SC Identification : Failed !!");
	}
}

static bool DoUartTrans(u8 *sendBuf, u16 bytesToSend, bool recvResp,
		u8 bytesToRecv) {
	bool RetVal = false;

	if (UART_SUCCESS
			== UART_RTOS_Send(&uart_vmcsc_log, &sendBuf[0], bytesToSend)) {
		RetVal = true;
	} else {
		RetVal = false;
		VMC_ERR("Uart Send Error. Retrying !! \r\n");
	}
	if ((recvResp) && (RetVal)) {
		/* Clean up the receiver buffer and count before every receive. */
		memset(receive_bufr, 0xFF, sizeof(receive_bufr));
		ReceivedByteCount = 0x00;

		if (UART_SUCCESS
				== UART_RTOS_Receive(&uart_vmcsc_log, &receive_bufr[0],
						bytesToRecv, &ReceivedByteCount, RCV_TIMEOUT_MS(200))) {
			(bytesToRecv == ReceivedByteCount) ? (RetVal = true) : (RetVal =
			false);
		} else {
			VMC_ERR("Uart Receive Error. Retrying !! \r\n");
			RetVal = false;
		}
	}
	return RetVal;
}

upgrade_status_t MatchCrcPostWrite(u32 writeAdd, u16 dataCRC) {
	upgrade_status_t status = STATUS_SUCCESS;

	u16 readCRC = 0;
	u8 crcPkt[12] = { 0 };

	crcPkt[0] = BSL_MSG_HEADER;

	/* Pkt len */
	crcPkt[1] = 0x7;
	crcPkt[2] = 0;

	/* CMD Id */
	crcPkt[3] = CMD_CRC_CHECK_32;

	/* Add the Address byte */
	crcPkt[4] = writeAdd & 0xFF;
	crcPkt[5] = (writeAdd >> 8) & 0xFF;
	crcPkt[6] = (writeAdd >> 16) & 0xFF;
	crcPkt[7] = (writeAdd >> 24) & 0xFF;

	/* Length to calculate the Data */
	crcPkt[8] = (PrevLength & 0xFF);
	crcPkt[9] = ((PrevLength >> 8) & 0xFF);

	/* Append the CRC */
	u16 Checksum = Crc16(&crcPkt[3], 7);
	crcPkt[10] = Checksum & 0xFF;
	crcPkt[11] = (Checksum >> 8) & 0xFF;

	if (DoUartTrans(&crcPkt[0], BSL_CRC_RESQ, TRUE, BSL_CRC_RESP)) {
		/* Validate BSL response */
		if (!VmcValidateBslResp(&receive_bufr[0], BSL_CRC_RESP)) {
			/* Check if we received a successful/expected response. */
			if (receive_bufr[4] == BSL_CRC_SUCCESS_RESP) {
				readCRC = ((receive_bufr[BSL_RESP_CAL_CHKSUM_H] << 8)
						| (receive_bufr[BSL_RESP_CAL_CHKSUM_L]));
				if (dataCRC != readCRC) {
					status = STATUS_FAILURE;
					VMC_ERR("CRC Mismatch !! Expected: %x Received : %x",
							dataCRC, readCRC);
				}
			} else {
				status = STATUS_FAILURE;
			}
		}
	}

	return status;
}

static void VmcScBslSync(void) {
	static u8 ModeRetryCnt = 0;
	static u8 RetryCount = 0;
	u8 sc_bsl_sync[SC_BSL_SYNCED_REQ] = { BSL_SYNC_REQ_CHAR };

	VMC_LOG("CMD : SC BSL SYNC ");
	if (DoUartTrans(&sc_bsl_sync[0], SC_BSL_SYNCED_REQ, TRUE,
	SC_BSL_SYNCED_RESP)) {
		/* Check if MSP is in SC/BSL mode. */
		if ((receive_bufr[0] == ESCAPE_CHAR) && (receive_bufr[1] == STX)) {
			ModeRetryCnt = 0;
			RetryCount = 0;
			upgradeState = SC_ENABLE_BSL;
		} else if ((receive_bufr[0] == BSL_SYNC_SUCCESS)
				&& (ModeRetryCnt <= MODE_VER_MAX_RETRY_COUNT)) {
			ModeRetryCnt += 1;
			/* Make sure MSP is in BSL mode. */
			if (ModeRetryCnt >= MODE_VER_MAX_RETRY_COUNT) {
				ModeRetryCnt = 0;
				RetryCount = 0;
				upgradeState = BSL_UNLOCK_PASSWORD;
			}
		} else {
			RetryCount += 1;
			upgradeState = SC_BSL_SYNC;
			VMC_ERR("SC BSL Sync Failure.. Retrying !! ");
		}
	} else {
		RetryCount += 1;
	}

	if (RetryCount >= SC_UPDATE_MAX_RETRY_COUNT) {
		RetryCount = 0;
		UpdateFlags(STATUS_FAILURE, SC_STATE_IDLE,
				SC_UPDATE_ERROR_SC_BSL_SYNC_FAILED);
		VMC_ERR("Update failure: Retry !! ");
	}
	vTaskDelay(pdMS_TO_TICKS(1000 * 1));
}

static void VmcScEnableBsl(void) {
	static u8 RetryCount = 0;
	u8 enbsl[SC_ENABLE_BSL_REQ] = { 0x5C, 0x2, 0x1, 0x0, 0x0, 0x1, 0x0, 0x5C,
			0x3 };

	VMC_LOG("CMD : SC ENABLE BSL ");
	if (DoUartTrans(&enbsl[0], SC_ENABLE_BSL_REQ, TRUE, SC_ENABLE_BSL_RESP)) {
		if ((receive_bufr[0] == ESCAPE_CHAR)
				&& (receive_bufr[2] == MSP432_COMMS_MSG_GOOD)) {
			RetryCount = 0;
			upgradeState = BSL_SYNCED;
		} else {
			RetryCount += 1;
			upgradeState = SC_ENABLE_BSL;
		}
	} else {
		RetryCount += 1;
	}

	if (RetryCount >= SC_UPDATE_MAX_RETRY_COUNT) {
		RetryCount = 0;
		UpdateFlags(STATUS_FAILURE, SC_STATE_IDLE,
				SC_UPDATE_ERROR_EN_BSL_FAILED);
		VMC_ERR("Update failure: Retry !! ");
	}
	vTaskDelay(pdMS_TO_TICKS(1000 * 1));
}

static void VmcBslSync(void) {
	static u8 RetryCount = 0;
	u8 syncbuf[BSL_SYNCED_REQ] = { BSL_SYNC_REQ_CHAR };

	VMC_LOG("CMD : BSL SYNCED ");
	if (DoUartTrans(&syncbuf[0], BSL_SYNCED_REQ, TRUE, BSL_SYNCED_RESP)) {
		/* We will not wait for the Rx or Tx to complete,
		 * as we need to send SYNC char till the time we don't get an ACK from BSL. */
		if (receive_bufr[0] == BSL_SYNC_SUCCESS) {
			RetryCount = 0;
			UpdateProgress = 1;
			upgradeState = BSL_UNLOCK_PASSWORD;
		} else {
			RetryCount += 1;
			upgradeState = BSL_SYNCED;
			VMC_ERR("SYNC Failure.. Retrying !! ");
		}
	} else {
		RetryCount += 1;
	}

	if (RetryCount >= SC_UPDATE_MAX_RETRY_COUNT) {
		RetryCount = 0;
		UpdateFlags(STATUS_FAILURE, SC_STATE_IDLE,
				SC_UPDATE_ERROR_VMC_BSL_SYNC_FAILED);
		VMC_ERR("Update failure: Retry !! ");
	}
	vTaskDelay(pdMS_TO_TICKS(1000 * 1));
}

static void VmcBslUnlock(void) {
	static u8 RetryCount = 0;
	/* Please refer to the TI SLAU622I Data Sheet for packet descriptions. */
	u8 bslPasswd[BSL_UNLOCK_PASSWORD_REQ] = { 0x80, 0x39, 0x00, 0x21, 0x58,
			0x41, 0x50, 0x3A, 0x20, 0x58, 0x69, 0x6C, 0x69, 0x6E, 0x78, 0x20,
			0x41, 0x6C, 0x6C, 0x20, 0x50, 0x72, 0x6F, 0x67, 0x72, 0x61, 0x6D,
			0x6D, 0x61, 0x62, 0x6C, 0x65, 0x2C, 0x20, 0x58, 0x42, 0x42, 0x3A,
			0x20, 0x58, 0x69, 0x6C, 0x69, 0x6E, 0x78, 0x20, 0x42, 0x72, 0x61,
			0x6E, 0x64, 0x65, 0x64, 0x20, 0x42, 0x6F, 0x61, 0x72, 0x64, 0x73,
			0x82, 0xE5 };

	VMC_LOG("CMD : BSL UNLOCK PASSWORD ");
	if (DoUartTrans(&bslPasswd[0], BSL_UNLOCK_PASSWORD_REQ, TRUE,
	BSL_UNLOCK_PASSWORD_RESP)) {
		/* Validate BSL response */
		if (!VmcValidateBslResp(&receive_bufr[0], BSL_UNLOCK_PASSWORD_RESP)) {
			/* Check if we received a successful/expected response. */
			if ((receive_bufr[BSL_RESP_CMD_OFFSET] == BSL_RESP_CMD_CHAR)
					&& (receive_bufr[BSL_RESP_MSG_OFFSET] == BSL_RESP_MSG_CHAR)) {
				RetryCount = 0;
				upgradeState = BSL_MASS_ERASE;
			} else {
				RetryCount += 1;
				upgradeState = BSL_UNLOCK_PASSWORD;
			}
		} else {
			RetryCount += 1;
			upgradeState = BSL_UNLOCK_PASSWORD;
			VMC_ERR("Invalid Response from BSL.. Retrying.. !!");
		}
	} else {
		RetryCount += 1;
	}

	if (RetryCount >= SC_UPDATE_MAX_RETRY_COUNT) {
		RetryCount = 0;
		UpdateFlags(STATUS_FAILURE, SC_STATE_IDLE,
				SC_UPDATE_ERROR_BSL_UNLOCK_PASSWORD_FAILED);
		VMC_ERR("Update failure: Retry !! ");
	}
	vTaskDelay(pdMS_TO_TICKS(1000 * 1));
}

static void VmcBslMassErase(void) {
	static u8 RetryCount = 0;
	/* Please refer to the TI SLAU622I Data Sheet for packet descriptions. */
	u8 massErase[BSL_MASS_ERASE_REQ] = { 0x80, 0x01, 0x00, 0x15, 0x64, 0xA3 };

	VMC_LOG("CMD : BSL MASS ERASE ");
	if (DoUartTrans(&massErase[0], BSL_MASS_ERASE_REQ, TRUE,
	BSL_MASS_ERASE_RESP)) {
		/* Validate BSL response */
		if (!VmcValidateBslResp(&receive_bufr[0], BSL_MASS_ERASE_RESP)) {
			/* Check if we received a successful/expected response. */
			if ((receive_bufr[BSL_RESP_CMD_OFFSET] == BSL_RESP_CMD_CHAR)
					&& (receive_bufr[BSL_RESP_MSG_OFFSET] == BSL_RESP_MSG_CHAR)) {
				RetryCount = 0;
				upgradeState = BSL_DATA_TX_32;
			} else {
				RetryCount += 1;
				upgradeState = BSL_MASS_ERASE;
			}

		} else {
			RetryCount += 1;
			upgradeState = BSL_MASS_ERASE;
			VMC_ERR("Invalid Response from BSL.. Retrying.. !! ");
		}
	} else {
		RetryCount += 1;
	}

	if (RetryCount >= SC_UPDATE_MAX_RETRY_COUNT) {
		RetryCount = 0;
		UpdateFlags(STATUS_FAILURE, SC_STATE_IDLE,
				SC_UPDATE_ERROR_ERASE_FAILED);
		VMC_ERR("Update failure: Retry !! ");
	}
	vTaskDelay(pdMS_TO_TICKS(200));
}

static void VmcBslSendDataPkts(void) {
	u8 bsl_send_data_pkt[BSL_MAX_DATA_SIZE] = { 0x00 };
	u16 TotalLength = 0x00;
	u8 StartSymbol = 0x00;
	u16 DataCRC = 0x00;
	bool ScAvailableForParsing = false;
	DataPtr = 0x00;
	AllPktSent = false;
	MaxDataSlot = ((FptScLoc.size / 100) + ((FptScLoc.size % 100)));

	VMC_LOG("CMD : BSL DATA TX 32 ");
	StartSymbol = IO_SYNC_READ8(FptScLoc.start_address + SC_TOT_HEADER_SIZE);
	if (StartSymbol == SECTION_START) {
		AddrFlag = true;
		ScAvailableForParsing = true;
		VMC_LOG("Symbol Matched !!");
	} else {
		UpdateFlags(STATUS_FAILURE, SC_STATE_IDLE,
				SC_UPDATE_ERROR_INVALID_SC_SYMBOL_FOUND);
		VMC_ERR("Invalid symbol... ");
	}

	if (ScAvailableForParsing) {
		ScAvailableForParsing = false;
		while (AllPktSent != true) {
			memset(bsl_send_data_pkt, 0x00, BSL_MAX_DATA_SIZE);

			portENTER_CRITICAL();
			VmcParseFptSC((FptScLoc.start_address + SC_TOT_HEADER_SIZE),
					&bsl_send_data_pkt[0], &TotalLength, &DataCRC);
			portEXIT_CRITICAL();

			if (DoUartTrans(&bsl_send_data_pkt[0], TotalLength, TRUE,
			BSL_DATA_TX_32_RESP)) {
				/* Validate BSL response */
				if (!VmcValidateBslResp(&receive_bufr[0],
				BSL_DATA_TX_32_RESP)) {
					/* Check if we received a successful/expected response. */
					if ((receive_bufr[BSL_RESP_CMD_OFFSET] == BSL_RESP_CMD_CHAR)
							&& (receive_bufr[BSL_RESP_MSG_OFFSET]
									== BSL_RESP_MSG_CHAR)) {
						/* Check if the data was written to the MSP flash successfully. */
						if (MatchCrcPostWrite(Addr, DataCRC)
								!= STATUS_SUCCESS) {
							UpdateFlags(STATUS_FAILURE, SC_STATE_IDLE,
									SC_UPDATE_ERROR_POST_CRC_FAILED);
							DataPtr = 0x00;
							VMC_ERR("Post CRC failed... ");
							break;
						}
					} else {
						UpdateFlags(STATUS_FAILURE, SC_STATE_IDLE,
								SC_UPDATE_ERROR_INVALID_BSL_RESP);
						DataPtr = 0x00;
						VMC_ERR("\r\nBSL Resp failure. Retry.. \r\n");
					}
				} else {
					UpdateFlags(STATUS_FAILURE, SC_STATE_IDLE,
							SC_UPDATE_ERROR_INVALID_BSL_RESP);
					DataPtr = 0x00;
					VMC_ERR("Invalid Response from BSL..");
					break;
				}
			}
			/* 10msec packet-to-packet */
			vTaskDelay(pdMS_TO_TICKS(10));
		}
	} else {
		UpdateFlags(STATUS_FAILURE, SC_STATE_IDLE,
				SC_UPDATE_ERROR_NO_VALID_FPT_SC_FOUND);
		DataPtr = 0x00;
		VMC_ERR("No Valid SC to parse... ");
	}

	/* Clean up */
	AddrFlag = false;
	DataPtr = 0x00;
	CurrProg = 0;
	MaxDataSlot = 0;
	upgradeState = BSL_LOAD_PC_32;
	vTaskDelay(pdMS_TO_TICKS(100));
}

static void VmcBslLoadPc(void) {
	/* Please refer to the TI SLAU622I Data Sheet for packet descriptions.*/
	u8 load_pc[BSL_LOAD_PC_REQ] = { 0x80, 0x05, 0x00, 0x27, 0x01, 0x02, 00, 00,
			0xB8, 0x66 };

	VMC_LOG("CMD : BSL LOAD PC 32 ");
	if (DoUartTrans(&load_pc[0], BSL_LOAD_PC_REQ, FALSE, 0)) {
		UpdateProgress += 1;
		if (UpdateProgress == 100)
			VMC_LOG("Update Complete : %d%% ",UpdateProgress);

		VMC_LOG("SC Application Loaded !! ");
		upgradeStatus = STATUS_SUCCESS;
		upgradeState = SC_STATE_IDLE;
	}
	vTaskDelay(pdMS_TO_TICKS(100));
}

static void VmcBslReboot(void) {
	/* NOTE: After a reboot reset, it takes approximately 10msec before it starts again. */
	/* Please refer to the TI SLAU622I Data Sheet for packet descriptions. */
	u8 rebootrst[BSL_REBOOT_RESET_REQ] = { 0x80, 0x01, 0x00, 0x25, 0x37, 0x95 };

	VMC_LOG("CMD : BSL REBOOT RESET ");
	if (DoUartTrans(&rebootrst[0], BSL_REBOOT_RESET_REQ, FALSE, 0)) {
		/* Reset the progress variables. */
		upgradeStatus = STATUS_SUCCESS;
		upgradeState = SC_STATE_IDLE;
		VMC_LOG("SuC BSL Rebooted !! ");
	}
	vTaskDelay(pdMS_TO_TICKS(1000 * 1));
}

static void VmcBslVersion(void) {
	static u8 RetryCount = 0;
	/* Please refer to the TI SLAU622I Data Sheet for packet descriptions. */
	u8 version[BSL_VERSION_REQ] = { 0x80, 0x01, 0x00, 0x19, 0xE8, 0x62 };

	VMC_LOG("CMD : TX BSL VERSION ");
	if (DoUartTrans(&version[0], BSL_VERSION_REQ, TRUE, BSL_VERSION_RESP)) {
		/* Validate BSL response */
		if (!VmcValidateBslResp(&receive_bufr[0], BSL_VERSION_RESP)) {
			/* Check if we received a successful/expected response. */
			if ((receive_bufr[BSL_RESP_BUILD_ID_OFFSET_H] == BSL_VERSION_BYTE_H)
					&& (receive_bufr[BSL_RESP_BUILD_ID_OFFSET_L]
							== BSL_VERSION_BYTE_L)) {
				RetryCount = 0;
				upgradeStatus = STATUS_SUCCESS;
				upgradeState = SC_STATE_IDLE;
			} else {
				RetryCount += 1;
				upgradeState = TX_BSL_VERSION;
			}
		} else {
			RetryCount += 1;
			upgradeState = TX_BSL_VERSION;
			VMC_ERR("BSL Resp failure. Retry.. ");
		}
	} else {
		RetryCount += 1;
	}

	if (RetryCount >= SC_UPDATE_MAX_RETRY_COUNT) {
		RetryCount = 0;
		UpdateFlags(STATUS_FAILURE, SC_STATE_IDLE,
				SC_UPDATE_ERROR_FAILED_TO_GET_BSL_VERSION);
		VMC_ERR("Failed to get BSL version: Retry !! ");
	}
	/* Reset the progress variables. */
	UpdateProgress = 0;
	CurrProg = 0;
	MaxDataSlot = 0;
	vTaskDelay(pdMS_TO_TICKS(1000 * 1));
}

void SCUpdateTask(void * arg) {
	VMC_LOG("SC Update Task Created !!! ");

	/* After boot-up, this function validates the fpt SC image and the version. */
	VmcGetFptSCVersion();

	while (1) {
		/* Waits for notification */
		xTaskNotifyWait(ULONG_MAX, ULONG_MAX, NULL, portMAX_DELAY);

		if (xSemaphoreTake(vmc_sc_comms_lock, portMAX_DELAY) == pdFALSE) {
			VMC_ERR("Failed to take vmc_sc_comms_lock !! ");
			continue;
		}

		if (xSemaphoreTake(vmc_sensor_monitoring_lock, portMAX_DELAY) == pdFALSE) {
			xSemaphoreGive(vmc_sc_comms_lock);
			VMC_ERR("Failed to take vmc_sensor_monitoring_lock !! ");
			continue;
		}

		if ((upgradeState == SC_STATE_IDLE)	&&
				((upgradeStatus == STATUS_SUCCESS) ||
						(upgradeStatus == STATUS_FAILURE))) {
			upgradeState = SC_BSL_SYNC;
			upgradeStatus = STATUS_IN_PROGRESS;
			upgradeError = SC_UPDATE_NO_ERROR;
		}

		while (upgradeStatus == STATUS_IN_PROGRESS) {
			switch (upgradeState) {
			case SC_BSL_SYNC:
				VmcScBslSync();
				break;
			case SC_ENABLE_BSL:
				VmcScEnableBsl();
				break;
			case BSL_SYNCED:
				VmcBslSync();
				break;
			case BSL_UNLOCK_PASSWORD:
				VmcBslUnlock();
				break;
			case BSL_MASS_ERASE:
				VmcBslMassErase();
				break;
			case BSL_DATA_TX_32:
				VmcBslSendDataPkts();
				break;
			case BSL_LOAD_PC_32:
				VmcBslLoadPc();
				break;
			case BSL_REBOOT_RESET:
				VmcBslReboot();
				break;
			case TX_BSL_VERSION:
				VmcBslVersion();
				break;
			default:
				VMC_ERR("Invalid Command !! ");
				upgradeStatus = STATUS_FAILURE;
				upgradeState = SC_STATE_IDLE;
			}
		}

		/* Waiting for 5sec so that XRT will get the updated progress of the SC update. */
		vTaskDelay(pdMS_TO_TICKS(1000 * 5));

		/* Reset progress variables */
		UpdateProgress = 0;
		CurrProg = 0;
		MaxDataSlot = 0;
		upgradeError = SC_UPDATE_NO_ERROR;

		memset(receive_bufr, 0x00, sizeof(receive_bufr));
		ReceivedByteCount = 0;

		/* Re-send the VMC active flag to SC */
		isVMCActive = false;

		/* Resume SC communication and monitoring */
		xSemaphoreGive(vmc_sensor_monitoring_lock);
		xSemaphoreGive(vmc_sc_comms_lock);
	}

	vTaskSuspend(NULL);
}

void SC_Update_Task_Create(void) {
	if (xTaskCreate(SCUpdateTask, (const char *) "SC Update Task",
	TASK_STACK_DEPTH,
	NULL,
	tskIDLE_PRIORITY + 1, &xSCUpdateTaskHandle) != pdPASS) {
		CL_LOG(APP_VMC, "Failed to Create SC Update Task \n\r");
		return;
	}
}
