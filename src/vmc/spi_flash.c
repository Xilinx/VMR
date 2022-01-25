/******************************************************************************
* Copyright (C) 2010 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/***************************** Include Files *********************************/

#include "MX25_CMD.h"

#ifdef PLATFORM_CS2200

#include "xparameters.h"	/* SDK generated parameters */
#include "xplatform_info.h"
#include "xspips.h"		/* SPI device driver */
#include "vmc_api.h"

/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define SPI_FLASH_DEVICE_ID		XPAR_XSPIPS_0_DEVICE_ID


/*
 * The following constants define the offsets within a FlashBuffer data
 * type for each kind of data.  Note that the read data offset is not the
 * same as the write data because the SPI driver is designed to allow full
 * duplex transfers such that the number of bytes received is the number
 * sent and received.
 */
#define COMMAND_OFFSET		0 /* Flash instruction */
#define ADDRESS_1_OFFSET	1 /* MSB byte of address to read or write */
#define ADDRESS_2_OFFSET	2 /* Middle byte of address to read or write */
#define ADDRESS_3_OFFSET	3 /* LSB byte of address to read or write */
#define DATA_OFFSET			4 /* Start of Data for Read/Write */
#define DUMMY_SIZE			1 /* Number of dummy bytes for fast read */
#define RD_ID_SIZE			4 /* Read ID command + 3 bytes ID response */

/*
 * The following constants specify the extra bytes which are sent to the
 * flash on the SPI interface, that are not data, but control information
 * which includes the command and address
 */
#define OVERHEAD_SIZE		4

#define UNIQUE_VALUE		0x05

/*
 * The following constants specify the max amount of data and the size of the
 * the buffer required to hold the data and overhead to transfer the data to
 * and from the flash.
 */
#define MAX_DATA		1024*1024

/*
 * The following constant defines the slave select signal that is used to
 * to select the flash device on the SPI bus, this signal is typically
 * connected to the chip select of the device
 */
#define FLASH_SPI_SELECT_1	0x01
#define FLASH_SPI_SELECT_0	0x00

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

s32 SPI_Flash_Config(void);

static void SPI_Flash_Erase(XSpiPs *SpiPtr);

static void SPI_Flash_Write(XSpiPs *SpiPtr, u32 Address, u32 ByteCount);

static void SPI_Flash_Read(XSpiPs *SpiPtr, u32 Address, u32 ByteCount);


static int SPI_Flash_Read_ID(XSpiPs *SpiInstance);

/************************** Variable Definitions *****************************/


XSpiPs SpiFlashInstance;


/*
 * Write Address Location in Serial Flash.
 */
static int TestAddress;

/*
 * The following variables are used to read and write to the eeprom and they
 * are global to avoid having large buffers on the stack
 */
u8 ReadBuff[MAX_DATA + DATA_OFFSET + DUMMY_SIZE];
u8 WriteBuffer[MAX_DATA + DATA_OFFSET];


/*****************************************************************************/
/**
*
* The purpose of this function is to illustrate how to use the XSpiPs
* device driver in polled mode. This function writes and reads data
* from a serial flash.
*
* @param	SpiInstancePtr is a pointer to the SPI driver instance to use.
*
* @param	SpiDeviceId is the Instance Id of SPI in the system.
*
* @return
*		- XST_SUCCESS if successful
*		- XST_FAILURE if not successful
*
* @note
*
* If the device slave select is not correct and the device is not responding
* on bus it will read a status of 0xFF for the status register as the bus
* is pulled up.
*
*****************************************************************************/
static int SPI_Flash_Init(XSpiPs *SpiInstancePtr,
			 u16 SpiDeviceId)
{
	int Status;
	u8 *BufferPtr;
	u8 UniqueValue;
	u32 Count;
	u32 MaxSize = MAX_DATA;
	u32 ChipSelect = FLASH_SPI_SELECT_1;
	XSpiPs_Config *SpiConfig;
	u32 Platform;

	Platform = XGetPlatform_Info();
	if ((Platform == XPLAT_ZYNQ_ULTRA_MP) || (Platform == XPLAT_VERSAL)) {
		MaxSize = 1024 * 10;
		ChipSelect = FLASH_SPI_SELECT_0;	/* Device is on CS 0 */
	}

	/*
	 * Initialize the SPI driver so that it's ready to use
	 */
	SpiConfig = XSpiPs_LookupConfig(SpiDeviceId);
	if (NULL == SpiConfig) {
		return XST_FAILURE;
	}

	Status = XSpiPs_CfgInitialize(SpiInstancePtr, SpiConfig,
					SpiConfig->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Perform a self-test to check hardware build
	 */
	Status = XSpiPs_SelfTest(SpiInstancePtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Set the SPI device as a master with manual start and manual
	 * chip select mode options
	 */
	XSpiPs_SetOptions(SpiInstancePtr, XSPIPS_MANUAL_START_OPTION | \
			XSPIPS_MASTER_OPTION | XSPIPS_FORCE_SSELECT_OPTION);


	/*
	 * Set the SPI device pre-scalar to divide by 8
	 */
	XSpiPs_SetClkPrescaler(SpiInstancePtr, XSPIPS_CLK_PRESCALE_8);

	/*
	 * Set the flash chip select
	 */
	XSpiPs_SetSlaveSelect(SpiInstancePtr, ChipSelect);

	/*
	 * Read the flash Id
	 */
	Status = SPI_Flash_Read_ID(SpiInstancePtr);
	if (Status != XST_SUCCESS) {
		VMC_ERR("SPI Flash Read ID Failed\r\n");
		return XST_FAILURE;
	}

	VMC_LOG("Flash ID Done!");

	return XST_SUCCESS;
}

/******************************************************************************
*
*
* This function writes to the desired address in serial flash connected to
* the SPI interface.
*
* @param	SpiPtr is a pointer to the SPI driver instance to use.
* @param	Address contains the address to write data to in the flash.
* @param	ByteCount contains the number of bytes to write.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void SPI_Flash_Write(XSpiPs *SpiPtr, u32 Address, u32 ByteCount)
{


}

/******************************************************************************
*
* This function reads from the  serial flash connected to the
* SPI interface.
*
* @param	SpiPtr is a pointer to the SPI driver instance to use.
* @param	Address contains the address to read data from in the flash.
* @param	ByteCount contains the number of bytes to read.
* @param	Command is the command used to read data from the flash. SPI
*		device supports one of the Read, Fast Read commands to read
*		data from the flash.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void SPI_Flash_Read(XSpiPs *SpiPtr, u32 Address, u32 ByteCount)
{
	/* TODO: The following is not tested.*/

//	/*
//	 * Setup the read command with the specified address and data for the
//	 * flash
//	 */
//	WriteBuffer[COMMAND_OFFSET]   = FLASH_CMD_READ;
//	WriteBuffer[ADDRESS_1_OFFSET] = (u8)((Address & 0xFF0000) >> 16);
//	WriteBuffer[ADDRESS_2_OFFSET] = (u8)((Address & 0xFF00) >> 8);
//	WriteBuffer[ADDRESS_3_OFFSET] = (u8)(Address & 0xFF);
//
//	XSpiPs_PolledTransfer(SpiPtr, WriteBuffer, ReadBuff,
//			  ByteCount + OVERHEAD_SIZE);

}


/******************************************************************************
*
* This function reads serial flash ID connected to the SPI interface.
*
* @param	None.
*
* @return
*		- XST_SUCCESS if successful
*		- XST_FAILURE if not successful
*
* @note		None.
*
******************************************************************************/
static int SPI_Flash_Read_ID(XSpiPs *SpiInstance)
{

	u8 Index;
	int Status;
	u8 ByteCount = 4;
	u8 SendBuffer[8];
	u8 RecvBuffer[4] = {0};

	SendBuffer[0] = FLASH_CMD_RDID;
	SendBuffer[1] = 0;
	SendBuffer[2] = 0;
	SendBuffer[3] = 0;

	for(Index=0; Index < ByteCount; Index++) {
		SendBuffer[4 + Index] = 0x00;
	}

	Status = XSpiPs_PolledTransfer(SpiInstance, SendBuffer, RecvBuffer,
			 (ByteCount));
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}


	VMC_LOG("\n\r");
	VMC_PRNT("SPI Flash ID:");
	for(Index = 1; Index < ByteCount; Index++) {
		VMC_PRNT(" 0x%0X ", RecvBuffer[Index]);
	}


	return XST_SUCCESS;
}

/******************************************************************************
*
*
* This function erases the sectors in the  serial flash connected to the
* SPI interface.
*
* @param	SpiPtr is a pointer to the SPI driver instance to use.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void SPI_Flash_Erase(XSpiPs *SpiPtr)
{

}


s32 SPI_Flash_Config(void)
{
	return SPI_Flash_Init(&SpiFlashInstance,SPI_FLASH_DEVICE_ID);
}

#endif
