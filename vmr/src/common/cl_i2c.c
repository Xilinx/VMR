/******************************************************************************
* Copyright (C) 2024 Advanced Micro Devices, Inc.    All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "xiicps.h"
#include "cl_i2c.h"
#include "cl_log.h"

XIicPs_Config engines[3] =  {
		{0, 0xf1000000, 33333333 },			// PMC_I2C
		{1, 0xff020000, 33333333 },			// LPD_I2C0
		{2, 0xff030000, 33333333 }			// LPD_I2C1

};


XIicPs IicInstance[3] = {
		{{0, 0xf1000000, 33333333 },			// Config
		TRUE,				//u32 IsReady;		/**< Device is initialized and ready */
		0,					//u32 Options;		/**< Options set in the device */
		(u8 *)0,				//u8 *SendBufferPtr;	/**< Pointer to send buffer */
		(u8 *)0,				//u8 *RecvBufferPtr;	/**< Pointer to recv buffer */
		0,					//s32 SendByteCount;	/**< Number of bytes still expected to send */
		0,					//s32 RecvByteCount;	/**< Number of bytes still expected to receive */
		0,					//s32 CurrByteCount;	/**< No. of bytes expected in current transfer */

		0,					//s32 UpdateTxSize;	/**< If tx size register has to be updated */
		0,					//s32 IsSend;		/**< Whether master is sending or receiving */
		0,					//s32 IsRepeatedStart;	/**< Indicates if user set repeated start */
		0,					//s32 Is10BitAddr;	/**< Indicates if user set 10 bit address */

		NULL,				//XIicPs_IntrHandler StatusHandler;  /**< Event handler function */
	#if defined  (XCLOCKING)
		0,					//u32 IsClkEnabled;	/**< Input clock enabled */
	#endif
		(void *)NULL				//void *CallBackRef;	/**< Callback reference for event handler */
		}

};
/*-------------------------------------------------------------------------------------------------

-------------------------------------------------------------------------------------------------*/

u8 cl_I2CInit(void)
{
    XIicPs_CfgInitialize(&IicInstance[0], &engines[0],engines[0].BaseAddress);
	if (XIicPs_SetSClk(&IicInstance[0], IIC_SCLK_RATE) == XST_SUCCESS)
		VMR_LOG("I2C0:DONE");
	else
		VMR_LOG("I2C0:FAIL");

    XIicPs_CfgInitialize(&IicInstance[1], &engines[1],engines[1].BaseAddress);
	if (XIicPs_SetSClk(&IicInstance[1], IIC_SCLK_RATE) == XST_SUCCESS)
		VMR_LOG("I2C1:DONE");
	else
		VMR_LOG("I2C1:FAIL");

    XIicPs_CfgInitialize(&IicInstance[2], &engines[2],engines[2].BaseAddress);
	if (XIicPs_SetSClk(&IicInstance[2], IIC_SCLK_RATE) == XST_SUCCESS)
		VMR_LOG("I2C2:DONE");
	else
		VMR_LOG("I2C2:FAIL");

	return TRUE;
}


/*****************************************************************************/
/**
* This function reads data from the IIC device into a specified buffer.
*
* @param	BufferPtr contains the address of the data buffer to be filled.
* @param	ByteCount contains the number of bytes in the buffer to be read.
*
* @return	XST_SUCCESS if successful else XST_FAILURE.
*
* @note		None.
*
******************************************************************************/


u8 i2c_send(u8 i2c_num, unsigned char i2c_addr, unsigned char * i2c_data, long int data_length)
{

//	XIicPs_MasterSend(&IicInstance[i2c_num], i2c_data, data_length, i2c_addr);

	int Status = XIicPs_MasterSendPolled(&IicInstance[i2c_num], i2c_data,
			data_length, i2c_addr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Wait until bus is idle to start another transfer.
	 */
	while (XIicPs_BusIsBusy(&IicInstance[i2c_num]));

	return XST_SUCCESS;
}

u8 i2c_recv(u8 i2c_num, unsigned char i2c_addr, unsigned char * i2c_read_buff, long int i2c_read_len)
{
	/*
	 * Receive Data.
	 */
	int Status = XIicPs_MasterRecvPolled(&IicInstance[i2c_num], i2c_read_buff,i2c_read_len, i2c_addr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Wait until bus is idle to start another transfer.
	 */
	while (XIicPs_BusIsBusy(&IicInstance[i2c_num]));

	return XST_SUCCESS;
}

u8  i2c_send_rs_recv(u8 i2c_num, unsigned char i2c_addr, unsigned char * i2c_write_buff, long int write_length, unsigned char * i2c_read_buff, long int read_length)
{

	/*
	 * Enable repeated start option.
	 * This call will give an indication to the driver.
	 * The hold bit is actually set before beginning the following transfer
	 */
	XIicPs_SetOptions(&IicInstance[i2c_num], XIICPS_REP_START_OPTION);
	/*
	 * Send the Data.
	 */
	int Status = XIicPs_MasterSendPolled(&IicInstance[i2c_num], i2c_write_buff,
			write_length, i2c_addr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	/*
	 * Disable repeated start option.
	 * This call will give an indication to the driver.
	 * The hold bit is actually reset when the following transfer ends.
	 */
	XIicPs_ClearOptions(&IicInstance[i2c_num], XIICPS_REP_START_OPTION);

	/*
	 * Receive the Data.
	 */
	Status = XIicPs_MasterRecvPolled(&IicInstance[i2c_num], i2c_read_buff,
			read_length, i2c_addr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Wait until bus is idle to start another transfer.
	 */
	while (XIicPs_BusIsBusy(&IicInstance[i2c_num]));

	return XST_SUCCESS;



}
