/******************************************************************************
* Copyright (C) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/


/******************************************************************************/
/**
*
* @file xospipsv_flash_polled_example.c
*
*
* This file contains a design example using the OSPIPSV driver (xospipsv)
* The example writes to flash in IO mode and reads it back in DMA mode.
* It runs in polled mode.
* The hardware which this example runs on, must have an octal serial Flash
* (Micron) for it to run.
*
* This example has been tested with the Micron Octal Serial Flash (mt35xu01gbba).
*
* @note
*
* None.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who Date     Changes
* ----- --- -------- -----------------------------------------------
* 1.0   nsk  02/19/17 First release
*       sk   01/09/19 Updated flash configuration table with
*                     supported commands.
*       sk   02/04/19 Add support for SDR+PHY and DDR+PHY modes.
* 1.0   akm 03/29/19 Fixed data alignment issues on IAR compiler.
* 1.1   sk  07/23/19 Based on RX Tuning, updated the dummy cycles.
*       sk  08/08/19 Issue device reset to bring back to default state.
* 1.3   sk  05/27/20 Added Stacked mode support.
*
*</pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "stdlib.h"
#include "stdio.h"
#include "FreeRTOS.h"
#include "task.h"

#include "cl_log.h"
#include "cl_main.h"
#include "cl_flash.h"
#include "cl_mem.h"
#include "vmr_common.h"

#include "xospipsv_flash_config.h"

#define OSPI_ERR(fmt, arg...) \
	CL_ERR(APP_MAIN, fmt, ##arg)
#define OSPI_WARN(fmt, arg...) \
	CL_ERR(APP_MAIN, fmt, ##arg)
#define OSPI_LOG(fmt, arg...) \
	CL_LOG(APP_MAIN, fmt, ##arg)
#define OSPI_DBG(fmt, arg...) \
	CL_DBG(APP_MAIN, fmt, ##arg)

/* default ospi device */
#define OSPIPSV_DEVICE_ID		XPAR_XOSPIPSV_0_DEVICE_ID /* from xparameters.h */
/* pdi start location offset */
#define RPU_PDI_ADDRESS		0x0
#define APU_PDI_ADDRESS		(RPU_PDI_ADDRESS + 0x1000000) /* RPU + 16 M */

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

int OspiPsvPolledFlashExample(XOspiPsv *OspiPsvInstancePtr, u16 OspiPsvDeviceId);

static int FlashReadID(XOspiPsv *OspiPsvPtr);
static int FlashErase(XOspiPsv *OspiPsvPtr, u32 Address, u32 ByteCount, u8 *WriteBfrPtr);
static int FlashIoWrite(XOspiPsv *OspiPsvPtr, u32 Address, u32 ByteCount,
				u8 *WriteBfrPtr);
static int FlashLinearWrite(XOspiPsv *OspiPsvPtr, u32 Address, u32 ByteCount,
				u8 *WriteBfrPtr);

static int FlashRead(XOspiPsv *OspiPsvPtr, u32 Address, u32 ByteCount,
				u8 *WriteBfrPtr, u8 *ReadBfrPtr);
/*
static int FlashRead_async(XOspiPsv *OspiPsvPtr, u32 Address, u32 ByteCount,
				u8 *WriteBfrPtr, u8 *ReadBfrPtr);
*/
static u32 GetRealAddr(XOspiPsv *OspiPsvPtr, u32 Address);
static int BulkErase(XOspiPsv *OspiPsvPtr, u8 *WriteBfrPtr);
static int DieErase(XOspiPsv *OspiPsvPtr, u8 *WriteBfrPtr);
int FlashRegisterRead(XOspiPsv *OspiPsvPtr, u32 ByteCount, u8 Command, u8 *ReadBfrPtr);
int FlashRegisterWrite(XOspiPsv *OspiPsvPtr, u32 ByteCount, u8 Command,
					u8 *WriteBfrPtr, u8 WrEn);
s32 InitCmd(XOspiPsv *OspiPsvInstancePtr);
static int FlashEnterExit4BAddMode(XOspiPsv *OspiPsvPtr, int Enable);
static int FlashSetSDRDDRMode(XOspiPsv *OspiPsvPtr, int Mode);

/************************** Variable Definitions *****************************/
//static u8 TxBfrPtr;
#define OSPI_READ_BUFFER_SIZE 8
#define OSPI_DATA_ALIGNMENT 8

#ifdef __ICCARM__
#pragma data_alignment = OSPI_DATA_ALIGNMENT
static u8 ReadBfrPtr[OSPI_READ_BUFFER_SIZE];
#else
static u8 ReadBfrPtr[OSPI_READ_BUFFER_SIZE]__attribute__ ((aligned(OSPI_DATA_ALIGNMENT)));
#endif

static u32 FlashMake;
static u32 FCTIndex;	/* Flash configuration table index */

/*
 * The instances to support the device drivers are global such that they
 * are initialized to zero each time the program runs. They could be local
 * but should at least be static so they are zeroed.
 */
static XOspiPsv OspiPsvInstance;
static XOspiPsv_Msg FlashMsg;

static u8 CmdBfr[8];

static int ospi_flash_percentage = 0;
static u32 OspiSectorSize = 0;

/*****************************************************************************/
/**
*
* The purpose of this function is to determine the number of lines used
* for command, address and data
*
* @param	OspiPsvPtr is a pointer to the OSPIPSV driver component to use.
* @param	Read is to tell whether a read or write
*
* @return	returns value to program the lines for command, address and data.
*
* @note		None.
*
*****************************************************************************/
static u32 XOspiPsv_Get_Proto(XOspiPsv *OspiPsvInstancePtr, int Read)
{
	u32 Val;

	if(Read) {
		Val = XOSPIPSV_READ_1_8_8;
	} else {
		if (OspiPsvInstancePtr->OpMode == XOSPIPSV_IDAC_MODE) {
			Val = XOSPIPSV_WRITE_1_1_1;
		} else {
			Val = XOSPIPSV_WRITE_1_1_8;
		}
	}
	return Val;
}

/*
 * try 10 times until exhausted
 */
static int pollTransfer(XOspiPsv *OspiPsvPtr, XOspiPsv_Msg *flashMsg) {
	const TickType_t x1second = pdMS_TO_TICKS( 100*1 );
	int Status, i = 0;

	Status = XOspiPsv_PollTransfer(OspiPsvPtr, flashMsg);
	while (Status != XST_SUCCESS && i++ < 10) {
		xil_printf("retry...%d\r\n", i);
		vTaskDelay( x1second );
		Status = XOspiPsv_PollTransfer(OspiPsvPtr, flashMsg);
	}

	if (Status != 0)
		xil_printf("retry failed\r\n");
	return Status;
}

#if 0
/*****************************************************************************/
/**
*
* This function performs read. DMA is the default setting.
*
* @param	OspiPsvPtr is a pointer to the OSPIPSV driver component to use.
* @param	Address contains the address of the first sector which needs to
*			be erased.
* @param	ByteCount contains the total size to be erased.
* @param	Pointer to the write buffer which contains data to be transmitted
* @param	Pointer to the read buffer to which valid received data should be
* 			written
*
* @return	XST_SUCCESS if successful, else XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
int FlashRead_async(XOspiPsv *OspiPsvPtr, u32 Address, u32 ByteCount,
				u8 *WriteBfrPtr, u8 *ReadBfrPtr)
{
	u8 Status;
	u32 RealAddr;
	u32 BytesToRead;
	u32 ByteCnt = ByteCount;

	xil_printf("ReadCmd 0x%x\r\n", Flash_Config_Table[FCTIndex].ReadCmd);

	if ((Address < Flash_Config_Table[FCTIndex].FlashDeviceSize) &&
		((Address + ByteCount) >= Flash_Config_Table[FCTIndex].FlashDeviceSize) &&
		(OspiPsvPtr->Config.ConnectionMode == XOSPIPSV_CONNECTION_MODE_STACKED)) {
		BytesToRead = (Flash_Config_Table[FCTIndex].FlashDeviceSize - Address);
	} else {
		BytesToRead = ByteCount;
	}
	while (ByteCount != 0) {
		/*
		 * Translate address based on type of connection
		 * If stacked assert the slave select based on address
		 */
		RealAddr = GetRealAddr(OspiPsvPtr, Address);

		FlashMsg.Opcode = (u8)Flash_Config_Table[FCTIndex].ReadCmd;
		FlashMsg.Addrsize = 4;
		FlashMsg.Addrvalid = 1;
		FlashMsg.TxBfrPtr = NULL;
		FlashMsg.RxBfrPtr = ReadBfrPtr;
		FlashMsg.ByteCount = BytesToRead;
		FlashMsg.Flags = XOSPIPSV_MSG_FLAG_RX;
		FlashMsg.Addr = RealAddr;
		FlashMsg.Proto = XOspiPsv_Get_Proto(OspiPsvPtr, 1);
		FlashMsg.Dummy = Flash_Config_Table[FCTIndex].DummyCycles +
				OspiPsvPtr->Extra_DummyCycle;
		FlashMsg.IsDDROpCode = 0;
		if (OspiPsvPtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
			FlashMsg.Proto = XOSPIPSV_READ_8_8_8;
			FlashMsg.Dummy = 16 + OspiPsvPtr->Extra_DummyCycle;
		}

		Status = XOspiPsv_StartDmaTransfer(OspiPsvPtr, &FlashMsg);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
		Status = XOspiPsv_CheckDmaDone(OspiPsvPtr);
		while(Status != XST_SUCCESS) {
			Status = XOspiPsv_CheckDmaDone(OspiPsvPtr);
		}

		ByteCount -= BytesToRead;
		Address += BytesToRead;
		ReadBfrPtr += BytesToRead;
		BytesToRead = ByteCnt - BytesToRead;
	}

	return 0;
}
#endif

/*****************************************************************************/
/**
*
* Reads the flash ID and identifies the flash in FCT table.
*
* @param	None.
*
* @return	XST_SUCCESS if successful, else XST_FAILURE.
*
* @note		None.
*
*****************************************************************************/
int FlashReadID(XOspiPsv *OspiPsvPtr)
{
	int Status;
	int ReadIdBytes = 8;
	u32 ReadId = 0;

	/*
	 * Read ID
	 */
	FlashMsg.Opcode = READ_ID;
	FlashMsg.Addrsize = 0;
	FlashMsg.Addrvalid = 0;
	FlashMsg.TxBfrPtr = NULL;
	FlashMsg.RxBfrPtr = ReadBfrPtr;
	FlashMsg.ByteCount = ReadIdBytes;
	FlashMsg.Flags = XOSPIPSV_MSG_FLAG_RX;
	FlashMsg.Dummy = OspiPsvPtr->Extra_DummyCycle;
	FlashMsg.IsDDROpCode = 0;
	FlashMsg.Proto = 0;
	if (OspiPsvPtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
		FlashMsg.Dummy += 8;
		FlashMsg.Proto = XOSPIPSV_READ_8_0_8;
	}
	Status = pollTransfer(OspiPsvPtr, &FlashMsg);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	xil_printf("FlashID = ");
	while(ReadIdBytes >= 0 ) {
		xil_printf("0x%x ", ReadBfrPtr[FlashMsg.ByteCount - ReadIdBytes]);
		ReadIdBytes--;
	}
	xil_printf("\n\r");

	OspiPsvPtr->DeviceIdData = ((ReadBfrPtr[3] << 24) | (ReadBfrPtr[2] << 16) |
		(ReadBfrPtr[1] << 8) | ReadBfrPtr[0]);
	ReadId = ((ReadBfrPtr[0] << 16) | (ReadBfrPtr[1] << 8) | ReadBfrPtr[2]);

	FlashMake = ReadBfrPtr[0];

	Status = CalculateFCTIndex(ReadId, &FCTIndex);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function writes to the  serial Flash connected to the OSPIPSV interface.
* All the data put into the buffer must be in the same page of the device with
* page boundaries being on 256 byte boundaries. This can be used when controller
* is in Linear mode.
*
* @param	OspiPsvPtr is a pointer to the OSPIPSV driver component to use.
* @param	Address contains the address to write data to in the Flash.
* @param	ByteCount contains the number of bytes to write.
* @param	Pointer to the write buffer (which is to be transmitted)
*
* @return	XST_SUCCESS if successful, else XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
int FlashLinearWrite(XOspiPsv *OspiPsvPtr, u32 Address, u32 ByteCount,
				u8 *WriteBfrPtr)
{
	u8 Status;

	FlashMsg.Opcode = WRITE_ENABLE_CMD;
	FlashMsg.Addrsize = 0;
	FlashMsg.Addrvalid = 0;
	FlashMsg.TxBfrPtr = NULL;
	FlashMsg.RxBfrPtr = NULL;
	FlashMsg.ByteCount = 0;
	FlashMsg.Flags = XOSPIPSV_MSG_FLAG_TX;
	FlashMsg.IsDDROpCode = 0;
	FlashMsg.Proto = 0;
	FlashMsg.Dummy = 0;
	if (OspiPsvPtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
		FlashMsg.Proto = XOSPIPSV_WRITE_8_0_0;
	}

	Status = pollTransfer(OspiPsvPtr, &FlashMsg);
	if (Status != XST_SUCCESS)
		return XST_FAILURE;

	FlashMsg.Opcode = (u8)(Flash_Config_Table[FCTIndex].WriteCmd >> 8);
	FlashMsg.Addrvalid = 1;
	FlashMsg.TxBfrPtr = WriteBfrPtr;
	FlashMsg.RxBfrPtr = NULL;
	FlashMsg.ByteCount = ByteCount;
	FlashMsg.Flags = XOSPIPSV_MSG_FLAG_TX;
	FlashMsg.Addrsize = 4;
	FlashMsg.Addr = Address;
	FlashMsg.Proto = XOspiPsv_Get_Proto(OspiPsvPtr, 0);
	FlashMsg.Dummy = 0;
	if (OspiPsvPtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
		FlashMsg.Proto = XOSPIPSV_WRITE_8_8_8;
	}
	FlashMsg.IsDDROpCode = 0;
	Status = pollTransfer(OspiPsvPtr, &FlashMsg);
	if (Status != XST_SUCCESS)
		return XST_FAILURE;

	return 0;
}

/*****************************************************************************/
/**
*
* This function writes to the  serial Flash connected to the OSPIPSV interface.
* All the data put into the buffer must be in the same page of the device with
* page boundaries being on 256 byte boundaries. This can be used in IO or DMA
* mode.
*
* @param	OspiPsvPtr is a pointer to the OSPIPSV driver component to use.
* @param	Address contains the address to write data to in the Flash.
* @param	ByteCount contains the number of bytes to write.
* @param	Pointer to the write buffer (which is to be transmitted)
*
* @return	XST_SUCCESS if successful, else XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
int FlashIoWrite(XOspiPsv *OspiPsvPtr, u32 Address, u32 ByteCount,
				u8 *WriteBfrPtr)
{
#ifdef __ICCARM__
#pragma data_alignment = 4
	u8 FlashStatus[2];
#else
	u8 FlashStatus[2] __attribute__ ((aligned(4)));
#endif
	u8 Status;
	u32 Bytestowrite;
	u32 RealAddr;

	while(ByteCount != 0) {
		/*
		 * Translate address based on type of connection
		 * If stacked assert the slave select based on address
		 */
		RealAddr = GetRealAddr(OspiPsvPtr, Address);

		FlashMsg.Opcode = WRITE_ENABLE_CMD;
		FlashMsg.Addrsize = 0;
		FlashMsg.Addrvalid = 0;
		FlashMsg.TxBfrPtr = NULL;
		FlashMsg.RxBfrPtr = NULL;
		FlashMsg.ByteCount = 0;
		FlashMsg.Flags = XOSPIPSV_MSG_FLAG_TX;
		FlashMsg.IsDDROpCode = 0;
		FlashMsg.Proto = 0;
		FlashMsg.Dummy = 0;
		if (OspiPsvPtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
			FlashMsg.Proto = XOSPIPSV_WRITE_8_0_0;
		}

		Status = XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
		if (Status != XST_SUCCESS) {
			xil_printf("poll transfer0 failed\r\n");
			return XST_FAILURE;
		}

		if(ByteCount <= 8) {
			Bytestowrite = ByteCount;
			ByteCount = 0;
		} else {
			Bytestowrite = 8;
			ByteCount -= 8;
		}

		FlashMsg.Opcode = (u8)Flash_Config_Table[FCTIndex].WriteCmd;
		FlashMsg.Addrvalid = 1;
		FlashMsg.TxBfrPtr = WriteBfrPtr;
		FlashMsg.RxBfrPtr = NULL;
		FlashMsg.ByteCount = Bytestowrite;
		FlashMsg.Flags = XOSPIPSV_MSG_FLAG_TX;
		FlashMsg.Proto = XOspiPsv_Get_Proto(OspiPsvPtr, 0);
		FlashMsg.Dummy = 0;
		FlashMsg.Addrsize = 4;
		FlashMsg.IsDDROpCode = 0;
		FlashMsg.Addr = RealAddr;
		if (OspiPsvPtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
			FlashMsg.Proto = XOSPIPSV_WRITE_8_8_8;
		}
		Status = XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
		if (Status != XST_SUCCESS) {
			xil_printf("poll transfer1 failed\r\n");
			return XST_FAILURE;
		}

		WriteBfrPtr += 8;
		Address += 8;

		while (1) {
			FlashMsg.Opcode = Flash_Config_Table[FCTIndex].StatusCmd;
			FlashMsg.Addrsize = 0;
			FlashMsg.Addrvalid = 0;
			FlashMsg.TxBfrPtr = NULL;
			FlashMsg.RxBfrPtr = FlashStatus;
			FlashMsg.ByteCount = 1;
			FlashMsg.Dummy = OspiPsvPtr->Extra_DummyCycle;
			FlashMsg.Flags = XOSPIPSV_MSG_FLAG_RX;
			FlashMsg.IsDDROpCode = 0;
			FlashMsg.Proto = 0;
			if (OspiPsvPtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
				FlashMsg.Proto = XOSPIPSV_READ_8_0_8;
				FlashMsg.ByteCount = 2;
				FlashMsg.Dummy += 8;
			}

			Status = XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
			if (Status != XST_SUCCESS) {
				xil_printf("poll transfer n failed\r\n");
				return XST_FAILURE;
			}

			if ((FlashStatus[0] & 0x80) != 0)
				break;
		}
	}
	return 0;
}

/*****************************************************************************/
/**
*
* This function erases the sectors in the  serial Flash connected to the
* OSPIPSV interface.
*
* @param	OspiPsvPtr is a pointer to the OSPIPSV driver component to use.
* @param	Address contains the address of the first sector which needs to
*		be erased.
* @param	ByteCount contains the total size to be erased.
* @param	Pointer to the write buffer (which is to be transmitted)
*
* @return	XST_SUCCESS if successful, else XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
int FlashErase(XOspiPsv *OspiPsvPtr, u32 Address, u32 ByteCount,
		u8 *WriteBfrPtr)
{
	int Sector;
	u32 NumSect;
#ifdef __ICCARM__
#pragma data_alignment = 4
	u8 FlashStatus[2];
#else
	u8 FlashStatus[2] __attribute__ ((aligned(4)));
#endif
	u8 Status;
	u32 RealAddr;

	/*
	 * If erase size is same as the total size of the flash, use bulk erase
	 * command or die erase command multiple times as required
	 */
	if (ByteCount == ((Flash_Config_Table[FCTIndex]).NumSect *
		(Flash_Config_Table[FCTIndex]).SectSize) ) {

		if (OspiPsvPtr->Config.ConnectionMode == XOSPIPSV_CONNECTION_MODE_STACKED) {
			Status = XOspiPsv_SelectFlash(OspiPsvPtr, XOSPIPSV_SELECT_FLASH_CS0);
			if (Status != XST_SUCCESS)
				return XST_FAILURE;
		}
		if(Flash_Config_Table[FCTIndex].NumDie == 1) {
			xil_printf("EraseCmd 0x%x\n\r", (u8)(Flash_Config_Table[FCTIndex].EraseCmd >> 8));
			/*
			 * Call Bulk erase
			 */
			BulkErase(OspiPsvPtr, WriteBfrPtr);
		}

		if(Flash_Config_Table[FCTIndex].NumDie > 1) {
			xil_printf("EraseCmd 0x%x\n\r", (u8)(Flash_Config_Table[FCTIndex].EraseCmd >> 16));
			/*
			 * Call Die erase
			 */
			DieErase(OspiPsvPtr, WriteBfrPtr);
		}

		if (OspiPsvPtr->Config.ConnectionMode == XOSPIPSV_CONNECTION_MODE_STACKED) {
			Status = XOspiPsv_SelectFlash(OspiPsvPtr, XOSPIPSV_SELECT_FLASH_CS1);
			if (Status != XST_SUCCESS)
				return XST_FAILURE;

			if(Flash_Config_Table[FCTIndex].NumDie == 1) {
				xil_printf("EraseCmd 0x%x\n\r", (u8)(Flash_Config_Table[FCTIndex].EraseCmd >> 8));
				/*
				 * Call Bulk erase
				 */
				BulkErase(OspiPsvPtr, WriteBfrPtr);
			}

			if(Flash_Config_Table[FCTIndex].NumDie > 1) {
				xil_printf("EraseCmd 0x%x\n\r", (u8)(Flash_Config_Table[FCTIndex].EraseCmd >> 16));
				/*
				 * Call Die erase
				 */
				DieErase(OspiPsvPtr, WriteBfrPtr);
			}
		}
		return 0;
	}

	xil_printf("EraseCmd 0x%x\n\r", (u8)Flash_Config_Table[FCTIndex].EraseCmd);
	/*
	 * If the erase size is less than the total size of the flash, use
	 * sector erase command
	 */

	/*
	 * Calculate no. of sectors to erase based on byte count
	 */
	NumSect = ByteCount/(Flash_Config_Table[FCTIndex].SectSize) + 1;

	/*
	 * If ByteCount to k sectors,
	 * but the address range spans from N to N+k+1 sectors, then
	 * increment no. of sectors to be erased
	 */

	if( ((Address + ByteCount) & Flash_Config_Table[FCTIndex].SectMask) ==
		((Address + (NumSect * Flash_Config_Table[FCTIndex].SectSize)) &
		Flash_Config_Table[FCTIndex].SectMask) ) {
		NumSect++;
	}

	for (Sector = 0; Sector < NumSect; Sector++) {

		/*
		 * Translate address based on type of connection
		 * If stacked assert the slave select based on address
		 */
		RealAddr = GetRealAddr(OspiPsvPtr, Address);

		/*
		 * Send the write enable command to the Flash so that it can be
		 * written to, this needs to be sent as a separate transfer before
		 * the write
		 */

		FlashMsg.Opcode = WRITE_ENABLE_CMD;
		FlashMsg.Addrsize = 0;
		FlashMsg.Addrvalid = 0;
		FlashMsg.TxBfrPtr = NULL;
		FlashMsg.RxBfrPtr = NULL;
		FlashMsg.ByteCount = 0;
		FlashMsg.Flags = XOSPIPSV_MSG_FLAG_TX;
		FlashMsg.IsDDROpCode = 0;
		FlashMsg.Proto = 0;
		FlashMsg.Dummy = 0;
		if (OspiPsvPtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
			FlashMsg.Proto = XOSPIPSV_WRITE_8_0_0;
		}

		Status = pollTransfer(OspiPsvPtr, &FlashMsg);
		if (Status != XST_SUCCESS)
			return XST_FAILURE;

		FlashMsg.Opcode = (u8)Flash_Config_Table[FCTIndex].EraseCmd;
		FlashMsg.Addrsize = 4;
		FlashMsg.Addrvalid = 1;
		FlashMsg.TxBfrPtr = NULL;
		FlashMsg.RxBfrPtr = NULL;
		FlashMsg.ByteCount = 0;
		FlashMsg.Flags = XOSPIPSV_MSG_FLAG_TX;
		FlashMsg.Addr = RealAddr;
		FlashMsg.IsDDROpCode = 0;
		FlashMsg.Proto = 0;
		FlashMsg.Dummy = 0;
		if (OspiPsvPtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
			FlashMsg.Proto = XOSPIPSV_WRITE_8_8_0;
		}
		Status = pollTransfer(OspiPsvPtr, &FlashMsg);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
		while (1) {
			FlashMsg.Opcode = Flash_Config_Table[FCTIndex].StatusCmd;
			FlashMsg.Addrsize = 0;
			FlashMsg.Addrvalid = 0;
			FlashMsg.TxBfrPtr = NULL;
			FlashMsg.RxBfrPtr = FlashStatus;
			FlashMsg.ByteCount = 1;
			FlashMsg.Flags = XOSPIPSV_MSG_FLAG_RX;
			FlashMsg.Dummy = OspiPsvPtr->Extra_DummyCycle;
			FlashMsg.IsDDROpCode = 0;
			FlashMsg.Proto = 0;
			if (OspiPsvPtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
				FlashMsg.Proto = XOSPIPSV_READ_8_0_8;
				FlashMsg.ByteCount = 2;
				FlashMsg.Dummy += 8;
			}

			Status = pollTransfer(OspiPsvPtr, &FlashMsg);
			if (Status != XST_SUCCESS) {
				return XST_FAILURE;
			}

			if ((FlashStatus[0] & 0x80) != 0) {
				break;
			}
		}
		Address += Flash_Config_Table[FCTIndex].SectSize;
	}
	return 0;
}

/*****************************************************************************/
/**
*
* This function performs read. DMA is the default setting.
*
* @param	OspiPsvPtr is a pointer to the OSPIPSV driver component to use.
* @param	Address contains the address of the first sector which needs to
*			be erased.
* @param	ByteCount contains the total size to be erased.
* @param	Pointer to the write buffer which contains data to be transmitted
* @param	Pointer to the read buffer to which valid received data should be
* 			written
*
* @return	XST_SUCCESS if successful, else XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
int FlashRead(XOspiPsv *OspiPsvPtr, u32 Address, u32 ByteCount,
				u8 *WriteBfrPtr, u8 *ReadBfrPtr)
{
	u8 Status;
	u32 RealAddr;
	u32 BytesToRead;
	u32 ByteCnt = ByteCount;

	//xil_printf("ReadCmd 0x%x\r\n", Flash_Config_Table[FCTIndex].ReadCmd);

	if ((Address < Flash_Config_Table[FCTIndex].FlashDeviceSize) &&
		((Address + ByteCount) >= Flash_Config_Table[FCTIndex].FlashDeviceSize) &&
		(OspiPsvPtr->Config.ConnectionMode == XOSPIPSV_CONNECTION_MODE_STACKED)) {
		BytesToRead = (Flash_Config_Table[FCTIndex].FlashDeviceSize - Address);
	} else {
		BytesToRead = ByteCount;
	}
	while (ByteCount != 0) {
		/*
		 * Translate address based on type of connection
		 * If stacked assert the slave select based on address
		 */
		RealAddr = GetRealAddr(OspiPsvPtr, Address);

		FlashMsg.Opcode = (u8)Flash_Config_Table[FCTIndex].ReadCmd;
		FlashMsg.Addrsize = 4;
		FlashMsg.Addrvalid = 1;
		FlashMsg.TxBfrPtr = NULL;
		FlashMsg.RxBfrPtr = ReadBfrPtr;
		FlashMsg.ByteCount = BytesToRead;
		FlashMsg.Flags = XOSPIPSV_MSG_FLAG_RX;
		FlashMsg.Addr = RealAddr;
		FlashMsg.Proto = XOspiPsv_Get_Proto(OspiPsvPtr, 1);
		FlashMsg.Dummy = Flash_Config_Table[FCTIndex].DummyCycles +
				OspiPsvPtr->Extra_DummyCycle;
		FlashMsg.IsDDROpCode = 0;
		if (OspiPsvPtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
			FlashMsg.Proto = XOSPIPSV_READ_8_8_8;
			FlashMsg.Dummy = 16 + OspiPsvPtr->Extra_DummyCycle;
		}

		Status = XOspiPsv_PollTransfer(OspiPsvPtr, &FlashMsg);
		if (Status != XST_SUCCESS) {
			xil_printf("FlashRead fail: %d\r\n", Status);
			return XST_FAILURE;
		}

		ByteCount -= BytesToRead;
		Address += BytesToRead;
		ReadBfrPtr += BytesToRead;
		BytesToRead = ByteCnt - BytesToRead;
	}

	return 0;
}

/*****************************************************************************/
/**
*
* This functions performs a bulk erase operation when the
* flash device has a single die. Works for both Spansion and Micron
*
* @param	OspiPsvPtr is a pointer to the OSPIPSV driver component to use.
* @param	WritBfrPtr is the pointer to command+address to be sent
*
* @return	XST_SUCCESS if successful, else XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
int BulkErase(XOspiPsv *OspiPsvPtr, u8 *WriteBfrPtr)
{
#ifdef __ICCARM__
#pragma data_alignment = 4
	u8 FlashStatus[2];
#else
	u8 FlashStatus[2] __attribute__ ((aligned(4)));
#endif
	int Status;


	FlashMsg.Opcode = WRITE_ENABLE_CMD;
	FlashMsg.Addrsize = 0;
	FlashMsg.Addrvalid = 0;
	FlashMsg.TxBfrPtr = NULL;
	FlashMsg.RxBfrPtr = NULL;
	FlashMsg.ByteCount = 0;
	FlashMsg.Flags = XOSPIPSV_MSG_FLAG_TX;
	FlashMsg.IsDDROpCode = 0;
	FlashMsg.Proto = 0;
	FlashMsg.Dummy = 0;
	if (OspiPsvPtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
		FlashMsg.Proto = XOSPIPSV_WRITE_8_0_0;
	}

	Status = pollTransfer(OspiPsvPtr, &FlashMsg);
	if (Status != XST_SUCCESS) {
			return XST_FAILURE;
	}

	/*
	 * Send the write enable command to the Flash so that it can be
	 * written to, this needs to be sent as a separate transfer before
	 * the write
	 */
	FlashMsg.Opcode = (u8)(Flash_Config_Table[FCTIndex].EraseCmd >> 8);
	FlashMsg.Addrsize = 0;
	FlashMsg.Addrvalid = 0;
	FlashMsg.TxBfrPtr = NULL;
	FlashMsg.RxBfrPtr = NULL;
	FlashMsg.ByteCount = 0;
	FlashMsg.Flags = XOSPIPSV_MSG_FLAG_TX;
	FlashMsg.IsDDROpCode = 0;
	FlashMsg.Proto = 0;
	FlashMsg.Dummy = 0;
	if (OspiPsvPtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
		FlashMsg.Proto = XOSPIPSV_WRITE_8_0_0;
	}

	Status = pollTransfer(OspiPsvPtr, &FlashMsg);
	if (Status != XST_SUCCESS) {
			return XST_FAILURE;
	}
	while (1) {
		FlashMsg.Opcode = Flash_Config_Table[FCTIndex].StatusCmd;
		FlashMsg.Addrsize = 0;
		FlashMsg.Addrvalid = 0;
		FlashMsg.TxBfrPtr = NULL;
		FlashMsg.RxBfrPtr = FlashStatus;
		FlashMsg.ByteCount = 1;
		FlashMsg.Flags = XOSPIPSV_MSG_FLAG_RX;
		FlashMsg.Dummy = OspiPsvPtr->Extra_DummyCycle;
		FlashMsg.IsDDROpCode = 0;
		FlashMsg.Proto = 0;
		if (OspiPsvPtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
			FlashMsg.Proto = XOSPIPSV_READ_8_0_8;
			FlashMsg.ByteCount = 2;
			FlashMsg.Dummy += 8;
		}

		Status = pollTransfer(OspiPsvPtr, &FlashMsg);
		if (Status != XST_SUCCESS)
			return XST_FAILURE;

		if ((FlashStatus[0] & 0x80) != 0)
			break;
	}
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This functions performs a die erase operation on all the die in
* the flash device. This function uses the die erase command for
* Micron 512Mbit and 1Gbit
*
* @param	OspiPsvPtr is a pointer to the OSPIPSV driver component to use.
* @param	WritBfrPtr is the pointer to command+address to be sent
*
* @return	XST_SUCCESS if successful, else XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
int DieErase(XOspiPsv *OspiPsvPtr, u8 *WriteBfrPtr)
{
	u8 DieCnt;
#ifdef __ICCARM__
#pragma data_alignment = 4
	u8 FlashStatus[2];
#else
	u8 FlashStatus[2] __attribute__ ((aligned(4)));
#endif
	int Status;

	for(DieCnt = 0; DieCnt < Flash_Config_Table[FCTIndex].NumDie; DieCnt++) {
		/*
		 * Send the write enable command to the Flash so that it can be
		 * written to, this needs to be sent as a separate transfer before
		 * the write
		 */
		FlashMsg.Opcode = WRITE_ENABLE_CMD;
		FlashMsg.Addrsize = 0;
		FlashMsg.Addrvalid = 0;
		FlashMsg.TxBfrPtr = NULL;
		FlashMsg.RxBfrPtr = NULL;
		FlashMsg.ByteCount = 0;
		FlashMsg.Flags = XOSPIPSV_MSG_FLAG_TX;
		FlashMsg.IsDDROpCode = 0;
		FlashMsg.Proto = 0;
		FlashMsg.Dummy = 0;
		if (OspiPsvPtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
			FlashMsg.Proto = XOSPIPSV_WRITE_8_0_0;
		}

		Status = pollTransfer(OspiPsvPtr, &FlashMsg);
		if (Status != XST_SUCCESS)
			return XST_FAILURE;

		FlashMsg.Opcode = (u8)(Flash_Config_Table[FCTIndex].EraseCmd >> 16);
		FlashMsg.Addrsize = 0;
		FlashMsg.Addrvalid = 0;
		FlashMsg.TxBfrPtr = NULL;
		FlashMsg.RxBfrPtr = NULL;
		FlashMsg.ByteCount = 0;
		FlashMsg.Flags = XOSPIPSV_MSG_FLAG_TX;
		FlashMsg.IsDDROpCode = 0;
		FlashMsg.Proto = 0;
		FlashMsg.Dummy = 0;
		if (OspiPsvPtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
			FlashMsg.Proto = XOSPIPSV_WRITE_8_0_0;
		}

		Status = pollTransfer(OspiPsvPtr, &FlashMsg);
		if (Status != XST_SUCCESS)
			return XST_FAILURE;
		while (1) {
			FlashMsg.Opcode = Flash_Config_Table[FCTIndex].StatusCmd;
			FlashMsg.Addrsize = 0;
			FlashMsg.Addrvalid = 0;
			FlashMsg.TxBfrPtr = NULL;
			FlashMsg.RxBfrPtr = FlashStatus;
			FlashMsg.ByteCount = 1;
			FlashMsg.Flags = XOSPIPSV_MSG_FLAG_RX;
			FlashMsg.Dummy = OspiPsvPtr->Extra_DummyCycle;
			FlashMsg.IsDDROpCode = 0;
			FlashMsg.Proto = 0;
			if (OspiPsvPtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
				FlashMsg.Proto = XOSPIPSV_READ_8_0_8;
				FlashMsg.ByteCount = 2;
				FlashMsg.Dummy += 8;
			}

			Status = pollTransfer(OspiPsvPtr, &FlashMsg);
			if (Status != XST_SUCCESS)
				return XST_FAILURE;
			if ((FlashStatus[0] & 0x80) != 0)
				break;
		}
	}
	return 0;
}

/*****************************************************************************/
/**
* This API enters the flash device into 4 bytes addressing mode.
* As per the Micron spec, before issuing the command to enter into 4 byte addr
* mode, a write enable command is issued.
*
* @param	OspiPtr is a pointer to the OSPIPSV driver component to use.
* @param	Enable is a either 1 or 0 if 1 then enters 4 byte if 0 exits.
*
* @return	 - XST_SUCCESS if successful.
* 		 - XST_FAILURE if it fails.
*
*
******************************************************************************/
int FlashEnterExit4BAddMode(XOspiPsv *OspiPsvPtr, int Enable)
{
	int Status;
	u8 Command;
#ifdef __ICCARM__
#pragma data_alignment = 4
	u8 FlashStatus[2];
#else
	u8 FlashStatus[2] __attribute__ ((aligned(4)));
#endif

	if(Enable)
		Command = ENTER_4B_ADDR_MODE;
	else
		Command = EXIT_4B_ADDR_MODE;

	FlashMsg.Opcode = WRITE_ENABLE_CMD;
	FlashMsg.Addrsize = 0;
	FlashMsg.Addrvalid = 0;
	FlashMsg.TxBfrPtr = NULL;
	FlashMsg.RxBfrPtr = NULL;
	FlashMsg.ByteCount = 0;
	FlashMsg.Flags = XOSPIPSV_MSG_FLAG_TX;
	FlashMsg.IsDDROpCode = 0;
	FlashMsg.Proto = 0;
	if (OspiPsvPtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
		FlashMsg.Proto = XOSPIPSV_WRITE_8_0_0;
	}
	Status = pollTransfer(OspiPsvPtr, &FlashMsg);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	FlashMsg.Opcode = Command;
	FlashMsg.Addrvalid = 0;
	FlashMsg.Addrsize = 0;
	FlashMsg.TxBfrPtr = NULL;
	FlashMsg.RxBfrPtr = NULL;
	FlashMsg.ByteCount = 0;
	FlashMsg.Flags = XOSPIPSV_MSG_FLAG_TX;
	FlashMsg.Addrsize = 3;
	FlashMsg.IsDDROpCode = 0;
	FlashMsg.Proto = 0;
	if (OspiPsvPtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
		FlashMsg.Proto = XOSPIPSV_WRITE_8_0_0;
	}

	Status = pollTransfer(OspiPsvPtr, &FlashMsg);
	if (Status != XST_SUCCESS)
		return XST_FAILURE;

	while (1) {
		FlashMsg.Opcode = Flash_Config_Table[FCTIndex].StatusCmd;
		FlashMsg.Addrsize = 0;
		FlashMsg.Addrvalid = 0;
		FlashMsg.TxBfrPtr = NULL;
		FlashMsg.RxBfrPtr = FlashStatus;
		FlashMsg.ByteCount = 1;
		FlashMsg.Flags = XOSPIPSV_MSG_FLAG_RX;
		FlashMsg.Dummy = OspiPsvPtr->Extra_DummyCycle;
		FlashMsg.IsDDROpCode = 0;
		FlashMsg.Proto = 0;
		if (OspiPsvPtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
			FlashMsg.Proto = XOSPIPSV_READ_8_0_8;
			FlashMsg.ByteCount = 2;
			FlashMsg.Dummy += 8;
		}

		Status = pollTransfer(OspiPsvPtr, &FlashMsg);
		if (Status != XST_SUCCESS)
			return XST_FAILURE;

		if ((FlashStatus[0] & 0x80) != 0)
			break;
	}

	switch (FlashMake) {
	case MICRON_OCTAL_ID_BYTE0:
		FlashMsg.Opcode = WRITE_DISABLE_CMD;
		FlashMsg.Addrsize = 0;
		FlashMsg.Addrvalid = 0;
		FlashMsg.TxBfrPtr = NULL;
		FlashMsg.RxBfrPtr = NULL;
		FlashMsg.ByteCount = 0;
		FlashMsg.Flags = XOSPIPSV_MSG_FLAG_TX;
		FlashMsg.IsDDROpCode = 0;
		FlashMsg.Proto = 0;
		if (OspiPsvPtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
			FlashMsg.Proto = XOSPIPSV_WRITE_8_0_0;
		}

		Status = pollTransfer(OspiPsvPtr, &FlashMsg);
		if (Status != XST_SUCCESS)
			return XST_FAILURE;
		break;

		default:
			break;
	}

	return Status;
}

/*****************************************************************************/
/**
* This API enters the flash device into Octal DDR mode or exit from octal DDR
* mode (switches to Extended SPI mode).
*
* @param	OspiPtr is a pointer to the OSPIPSV driver component to use.
* @param	Enable is either 1 or 0 if 1 then enter octal DDR mode if 0 exits.
*
* @return	 - XST_SUCCESS if successful.
* 		 - XST_FAILURE if it fails.
*
*
******************************************************************************/
int FlashSetSDRDDRMode(XOspiPsv *OspiPsvPtr, int Mode)
{
	int Status;
#ifdef __ICCARM__
#pragma data_alignment = 4
	u8 ConfigReg[2];
#pragma data_alignment = 4
	u8 Data[2];
#else
	u8 ConfigReg[2] __attribute__ ((aligned(4)));
	u8 Data[2] __attribute__ ((aligned(4)));
#endif

	if (Mode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
		Data[0] = 0xE7;
		Data[1] = 0xE7;
	} else {
		Data[0] = 0xFF;
		Data[1] = 0xFF;
	}


	FlashMsg.Opcode = WRITE_ENABLE_CMD;
	FlashMsg.Addrsize = 0;
	FlashMsg.Addrvalid = 0;
	FlashMsg.TxBfrPtr = NULL;
	FlashMsg.RxBfrPtr = NULL;
	FlashMsg.ByteCount = 0;
	FlashMsg.Flags = XOSPIPSV_MSG_FLAG_TX;
	FlashMsg.IsDDROpCode = 0;
	FlashMsg.Proto = 0;
	if (OspiPsvPtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
		FlashMsg.Proto = XOSPIPSV_WRITE_8_0_0;
	}

	Status = pollTransfer(OspiPsvPtr, &FlashMsg);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	if (OspiPsvPtr->OpMode == XOSPIPSV_DAC_EN_OPTION)
		XOspiPsv_ConfigureAutoPolling(OspiPsvPtr, Mode);

	FlashMsg.Opcode = WRITE_CONFIG_REG;
	FlashMsg.Addrvalid = 1;
	FlashMsg.Addrsize = 3;
	FlashMsg.Addr = 0x0;
	FlashMsg.TxBfrPtr = Data;
	FlashMsg.RxBfrPtr = NULL;
	FlashMsg.ByteCount = 1;
	FlashMsg.Flags = XOSPIPSV_MSG_FLAG_TX;
	FlashMsg.IsDDROpCode = 0;
	FlashMsg.Proto = 0;
	if (OspiPsvPtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
		FlashMsg.Proto = XOSPIPSV_WRITE_8_8_8;
		FlashMsg.Addrsize = 4;
		FlashMsg.ByteCount = 2;
	}

	Status = pollTransfer(OspiPsvPtr, &FlashMsg);
	if (Status != XST_SUCCESS)
		return XST_FAILURE;

	Status = XOspiPsv_SetSdrDdrMode(OspiPsvPtr, Mode);
	if (Status != XST_SUCCESS)
		return XST_FAILURE;

	/* Read Configuration register */
	FlashMsg.Opcode = READ_CONFIG_REG;
	FlashMsg.Addrsize = 3;
	FlashMsg.Addr = 0x0;
	FlashMsg.Addrvalid = 1;
	FlashMsg.TxBfrPtr = NULL;
	FlashMsg.RxBfrPtr = ConfigReg;
	FlashMsg.ByteCount = 1;
	FlashMsg.Flags = XOSPIPSV_MSG_FLAG_RX;
	FlashMsg.Dummy = 8 + OspiPsvPtr->Extra_DummyCycle;
	FlashMsg.IsDDROpCode = 0;
	FlashMsg.Proto = 0;
	if (OspiPsvPtr->SdrDdrMode == XOSPIPSV_EDGE_MODE_DDR_PHY) {
		/* Read Configuration register */
		FlashMsg.ByteCount = 2;
		FlashMsg.Proto = XOSPIPSV_READ_8_8_8;
		FlashMsg.Addrsize = 4;
	}
	Status = pollTransfer(OspiPsvPtr, &FlashMsg);
	if (Status != XST_SUCCESS)
		return XST_FAILURE;

	if (ConfigReg[0] != Data[0])
		return XST_FAILURE;

	return Status;
}

/*****************************************************************************/
/**
 *
 * This functions translates the address based on the type of interconnection.
 * In case of stacked, this function asserts the corresponding slave select.
 *
 * @param	OspiPsvPtr is a pointer to the OSPIPSV driver component to use.
 * @param	Address which is to be accessed (for erase, write or read)
 *
 * @return	RealAddr is the translated address - for single it is unchanged;
 *			for stacked, the lower flash size is subtracted;
 *
 * @note	In addition to get the actual address to work on flash this
 *			function also selects the CS based on the configuration detected.
 *
 ******************************************************************************/
u32 GetRealAddr(XOspiPsv *OspiPsvPtr, u32 Address)
{
	u32 RealAddr = Address;
	u8 Chip_Sel = XOSPIPSV_SELECT_FLASH_CS0;

	if ((OspiPsvPtr->Config.ConnectionMode == XOSPIPSV_CONNECTION_MODE_STACKED) &&
			(Address & Flash_Config_Table[FCTIndex].FlashDeviceSize)) {
		Chip_Sel = XOSPIPSV_SELECT_FLASH_CS1;
		RealAddr = Address & (~Flash_Config_Table[FCTIndex].FlashDeviceSize);
	}

	(void)XOspiPsv_SelectFlash(OspiPsvPtr, Chip_Sel);

	return RealAddr;
}

int ospi_flash_init()
{
	int Status;
	u32 PAGE_SIZE = 0;

	XOspiPsv *OspiPsvInstancePtr = &OspiPsvInstance;
	XOspiPsv_Config *OspiPsvConfig;

	Status = XOspiPsv_DeviceReset(XOSPIPSV_HWPIN_RESET);
	if (Status != XST_SUCCESS)
		return XST_FAILURE;

	/*
	 * Initialize the OSPIPSV driver so that it's ready to use
	 */
	OspiPsvConfig = XOspiPsv_LookupConfig(OSPIPSV_DEVICE_ID);
	if (NULL == OspiPsvConfig) {
		return XST_FAILURE;
	}

	Status = XOspiPsv_CfgInitialize(OspiPsvInstancePtr, OspiPsvConfig);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Enable IDAC controller in OSPI
	 */
	XOspiPsv_SetOptions(OspiPsvInstancePtr, XOSPIPSV_IDAC_EN_OPTION);
	/*
	 * Set the prescaler for OSPIPSV clock
	 */
	XOspiPsv_SetClkPrescaler(OspiPsvInstancePtr, XOSPIPSV_CLK_PRESCALE_12);

	Status = XOspiPsv_SelectFlash(OspiPsvInstancePtr, XOSPIPSV_SELECT_FLASH_CS0);
	if (Status != XST_SUCCESS)
		return XST_FAILURE;

	/*
	 * Read flash ID and obtain all flash related information
	 * It is important to call the read id function before
	 * performing proceeding to any operation, including
	 * preparing the WriteBuffer
	 */
	Status = FlashReadID(OspiPsvInstancePtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Set Flash device and Controller modes */
	Status = FlashSetSDRDDRMode(OspiPsvInstancePtr, XOSPIPSV_EDGE_MODE_SDR_NON_PHY);
	if (Status != XST_SUCCESS)
		return XST_FAILURE;

	if(Flash_Config_Table[FCTIndex].FlashDeviceSize > SIXTEENMB) {
		Status = FlashEnterExit4BAddMode(OspiPsvInstancePtr, 1);
		if(Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
	}

	if (OspiPsvInstancePtr->Config.ConnectionMode == XOSPIPSV_CONNECTION_MODE_STACKED) {
		Flash_Config_Table[FCTIndex].NumPage *= 2;
		Flash_Config_Table[FCTIndex].NumSect *= 2;

		/* Reset the controller mode to NON-PHY */
		Status = XOspiPsv_SetSdrDdrMode(OspiPsvInstancePtr, XOSPIPSV_EDGE_MODE_SDR_NON_PHY);
		if (Status != XST_SUCCESS)
			return XST_FAILURE;

		Status = XOspiPsv_SelectFlash(OspiPsvInstancePtr, XOSPIPSV_SELECT_FLASH_CS1);
		if (Status != XST_SUCCESS)
			return XST_FAILURE;

		/* Set Flash device and Controller modes */
		Status = FlashSetSDRDDRMode(OspiPsvInstancePtr, XOSPIPSV_EDGE_MODE_SDR_NON_PHY);
		if (Status != XST_SUCCESS)
			return XST_FAILURE;

		if(Flash_Config_Table[FCTIndex].FlashDeviceSize > SIXTEENMB) {
			Status = FlashEnterExit4BAddMode(OspiPsvInstancePtr, 1);
			if(Status != XST_SUCCESS) {
				return XST_FAILURE;
			}
		}
	}

	PAGE_SIZE = (Flash_Config_Table[FCTIndex].PageSize);
	if (PAGE_SIZE != OSPI_VERSAL_PAGESIZE) {
		OSPI_ERR("ERR: page size is: %d, expected: %d\r\n",
			PAGE_SIZE, OSPI_VERSAL_PAGESIZE);
		return XST_FAILURE;
	}

	OspiSectorSize =  (Flash_Config_Table[FCTIndex]).SectSize;
	OSPI_LOG("DONE: FCTINdex %d, OSPI page size %d, sector size %d",
		FCTIndex, OSPI_VERSAL_PAGESIZE, OspiSectorSize);

	return XST_SUCCESS;
}

static inline u32 getBaseAddress(flash_area_t area) {
	switch (area) {
	case (CL_FLASH_BOOT) :
		return RPU_PDI_ADDRESS;
	case (CL_FLASH_APU) :
		return APU_PDI_ADDRESS;
	default:
		OSPI_LOG("unknown flash area %d, fail.", area);
		break;
	}

	return (-1);
}

int ospi_flash_read(flash_area_t area, u8 *buffer, u32 offset, u32 len)
{
	int Status;
	u32 baseAddress = getBaseAddress(area);

	baseAddress += offset;
	XOspiPsv *OspiPsvInstancePtr = &OspiPsvInstance;

	OSPI_DBG("0x%x len %d", baseAddress, len);

	bzero(buffer, len);
	Status = FlashRead(OspiPsvInstancePtr, baseAddress, len, CmdBfr, buffer);
	if (Status != XST_SUCCESS) {
		OSPI_ERR("ERR: Read failed:%d\r\n", Status);
		return XST_FAILURE;
	}

	OSPI_DBG("done");
	return 0;
}

int ospi_flash_write(flash_area_t area, u8 *WriteBuffer, u32 offset, u32 len)
{
	int Status;
	int Count;
	int Page = 0;
	u32 PAGE_COUNT = 0;
	u32 PAGE_SIZE = OSPI_VERSAL_PAGESIZE;
	u32 baseAddress = getBaseAddress(area);
	u8 ReadBuffer[OSPI_VERSAL_PAGESIZE] __attribute__ ((aligned(64)));
	
	ospi_flash_percentage = 0;

	baseAddress += offset;
	XOspiPsv *OspiPsvInstancePtr = &OspiPsvInstance;

	OSPI_DBG("0x%x, len %d", baseAddress, len);

	PAGE_COUNT = len / PAGE_SIZE;
	if (len % PAGE_SIZE) {
		PAGE_COUNT++;
		OSPI_WARN("len %d is not page %d aligned", len, PAGE_SIZE);

	}
	if (offset % PAGE_SIZE) {
		OSPI_WARN("offset %d is not page %d aligned", offset, PAGE_SIZE);
	}

	OSPI_DBG("Flashing... Page Count: %d, PageSize %d", PAGE_COUNT, PAGE_SIZE);

	/* Write first, then read back and verify */
	if (XOspiPsv_GetOptions(OspiPsvInstancePtr) == XOSPIPSV_DAC_EN_OPTION) {
		OSPI_DBG("WriteCmd: 0x%x \n\r", (u8)(Flash_Config_Table[FCTIndex].WriteCmd >> 8));
		Status = FlashLinearWrite(OspiPsvInstancePtr, baseAddress,
		(Flash_Config_Table[FCTIndex].PageSize * PAGE_COUNT), WriteBuffer);
		if (Status != XST_SUCCESS)
			return XST_FAILURE;
	} else {
		OSPI_DBG("WriteCmd: 0x%x \n\r", (u8)Flash_Config_Table[FCTIndex].WriteCmd);
		for (Page = 0; Page < PAGE_COUNT; Page++) {
			u32 write_offset = (Page * Flash_Config_Table[FCTIndex].PageSize);

			ospi_flash_percentage = (Page*100/PAGE_COUNT);

			xil_printf("\r%d", ospi_flash_percentage);
			fflush(stdout);

			Status = FlashIoWrite(OspiPsvInstancePtr, baseAddress + write_offset,
				((Flash_Config_Table[FCTIndex].PageSize)), WriteBuffer + write_offset);
			if (Status != XST_SUCCESS) {
				OSPI_ERR("ERR: write failed: %d\r\n", Status);
				return XST_FAILURE;
			}
		}
	}

	/* read back: check some pages numbers */
	OSPI_DBG("write done. read back to verify.");
	bzero(ReadBuffer, sizeof (ReadBuffer));
	for (Count = 0; Count < len; Count += PAGE_SIZE ) {
		if (Count != 0 && (Count % (PAGE_COUNT / 10)))
			continue;

		Status = FlashRead(OspiPsvInstancePtr, baseAddress + Count, PAGE_SIZE,
			CmdBfr, ReadBuffer);
		if (Status != XST_SUCCESS) {
			OSPI_ERR("ERR: Read failed:%d", Status);
			return XST_FAILURE;
		}
		
		for (int i = 0; i < PAGE_SIZE; i++) {
			if (*((u32 *)ReadBuffer) != -1 ||
			    (Count + i) >= len ||
			    ReadBuffer[i] == WriteBuffer[Count+i])
				continue;

			for (int idx = 0; idx < 16; idx++) {
				OSPI_LOG("%02x ", ReadBuffer[idx]);
			}
			OSPI_ERR(" <= data in ospi");
			for (int idx = 0; idx < 16; idx++) {
				OSPI_LOG("%02x ", WriteBuffer[Count+idx]);
			}
			OSPI_ERR(" <= data from pdi");

			OSPI_ERR("mis-match offset: %d, read 0x%x: pdi 0x%x",
				Count+i, ReadBuffer[i], WriteBuffer[Count+i]);
			return XST_FAILURE;
		}
	}

	ospi_flash_percentage = 100;
	OSPI_DBG("done.");
	return 0;
}

int ospi_flash_erase(flash_area_t area, u32 offset, u32 len)
{
	int Status;
	u32 baseAddress = getBaseAddress(area);

	baseAddress += offset;
	XOspiPsv *OspiPsvInstancePtr = &OspiPsvInstance;


	if (baseAddress & OSPI_VERSAL_PAGESIZE) {
		OSPI_WARN("base address is not %d aligned", OSPI_VERSAL_PAGESIZE); 
	}

	Status = FlashErase(OspiPsvInstancePtr, baseAddress, len, CmdBfr);

	OSPI_LOG("0x%x, len: %d, ret: %d", baseAddress, len, Status);
	return Status;
}

int ospi_flash_copy(flash_area_t area, u32 src, u32 tgt, u32 len)
{
	u8 ReadBuffer[OSPI_VERSAL_PAGESIZE] __attribute__ ((aligned(64)));
	u32 page_size = OSPI_VERSAL_PAGESIZE;
	u32 idx = 0;
	int ret = 0;

	OSPI_DBG("src 0x%x, tgt 0x%x. start", src, tgt);

	ospi_flash_percentage = 0;
	/* copy enough (len + 1) page size from srouce to target */
	for (idx = 0; idx <= len; idx += page_size) {
		ospi_flash_percentage = (idx * 100 / len);

		ret = ospi_flash_read(area, ReadBuffer, src + idx, page_size);
		if (ret)
			return ret;
		
		ret = ospi_flash_write(area, ReadBuffer, tgt + idx, page_size);
		if (ret)
			return ret;
	}

	ospi_flash_percentage = 100;
	OSPI_DBG("src 0x%x, tgt 0x%x. done %d", src, tgt, ret);
	return ret;
}

/*
 * ospi_erase is based on sector size
 * ospi_write is based on page size
 * write only takes effect after an erase
 * since sector size > page size
 * a reliabe write should read sector back, change data,
 * erase then write whole sector back.
 */
int ospi_flash_safe_write(flash_area_t area, u8 *WriteBuffer, u32 offset, u32 len)
{
	int ret = 0;
	u8 *SectorBuffer = NULL;
	u32 last_sector_off = 0;
	u32 last_sector_addr = 0;

	if (area != CL_FLASH_BOOT) {
		OSPI_ERR("unsupported flash area type %d", area);
		return -EINVAL;
	}

	/*
	 * Check offset is page aligned
	 */
	if (offset & OSPI_VERSAL_PAGESIZE) {
		OSPI_ERR("cannot write on not page %s size aligned address 0x%x",
			OSPI_VERSAL_PAGESIZE, offset);
		return -EINVAL;
	}

	/*
	 * Always perform erase on sector size and write on page size
	 * Note: for versal, the page size is 256, sector size is 131072 (128k)
	 * so for last erase, we read whole sector back, update data, erase and rewrite
	 */
	if (len % OspiSectorSize) {
		OSPI_WARN("len %d is not OSPI Sector %d aligned", len, OspiSectorSize);

		SectorBuffer = pvPortMalloc(OspiSectorSize);
		if (SectorBuffer == NULL) {
			OSPI_ERR("no enable memory from pvPortMalloc");
			return -ENOMEM;
		}
		
		last_sector_off = len - len % OspiSectorSize;
		last_sector_addr = offset + last_sector_off;
		ret = ospi_flash_read(CL_FLASH_BOOT, SectorBuffer, last_sector_addr, OspiSectorSize);	
		if (ret)
			goto done;

		Cl_SecureMemcpy(SectorBuffer, OspiSectorSize,
				WriteBuffer + last_sector_off, len % OspiSectorSize);

		OSPI_WARN("cached one sector %d data start from offset 0x%x",
			OspiSectorSize, last_sector_addr);
	}

	/* Note: this will erase the last sector as well, but we catched it in the SectorBuffer */
	ret = ospi_flash_erase(CL_FLASH_BOOT, offset, len);
	if (ret)
		goto done;

	ret = ospi_flash_write(CL_FLASH_BOOT, WriteBuffer, offset, len);
	if (ret)
		goto done;

	if (SectorBuffer) {
		OSPI_WARN("rewrite cached one sector data");
		ret = ospi_flash_write(CL_FLASH_BOOT, SectorBuffer, last_sector_addr, OspiSectorSize);
		if (ret)
			goto done;
	}
done:
	if (SectorBuffer) {
		vPortFree(SectorBuffer);
		SectorBuffer = NULL;
	}

	OSPI_LOG("done");
	return ret;
}

int ospi_flash_progress()
{
	return ospi_flash_percentage;
}
