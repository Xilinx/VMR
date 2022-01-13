/******************************************************************************
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#include "xstatus.h"
#include "cl_i2c.h"
#include "cl_log.h"
#include "../inc/qsfp.h"
#include "../../vmc_api.h"

u8 i2c_num_pmc = 0;

u8 QSFP_ReadTemperature(float *TemperatureValue, u8 sensorInstance)
{
	u8 status = XST_FAILURE;

	unsigned char lsb_temp_reg = QSFP_LSB_TEMPERATURE_REG;
	unsigned char msb_temp_reg = QSFP_MSB_TEMPERATURE_REG;

	unsigned char temperature_buff[2];
	u16 TemperatureHexValue = 0;
	long int i2c_read_len = 1;

	u32 MIO ,QSFP_bits;

	volatile u32  rd_Address = QSFP_LOW_SPEED_IO_READ_OFFSET;

	volatile u32  wr_Address = QSFP_LOW_SPPED_IO_WRITE_OFFSET;

	MIO = Xil_In32(rd_Address);

	/*	IO pins : QSFP 0 is bits 14 - 18 , QSFP 1 is bits 19-23
		bit 14 MODSEL_L QSFP0
		bit 15 RESET_L  QSFP0
		bit 16 MODPRS_L QSFP0
		bit 17 INT_L    QSFP0
		bit 18 LPMODE   QSFP0

		bit 19 MODSEL_L QSFP1
		bit 20 RESET_L  QSFP1
		bit 21 MODPRS_L QSFP1
		bit 22 INT_L    QSFP1
		bit 23 LPMODE   QSFP1  */

	QSFP_bits = (MIO >> (14 + ((sensorInstance-1) * 5))) & 0x1F;

	/* check for QSFP module presence */

	if (!((QSFP_bits & 0x4 ) >> 2))
	{
		//CL_LOG (APP_VMC ,"QSFP_%d module present",i);

		MIO = MIO & ~(1 << (14 + ((sensorInstance-1) * 5))); // clear MODSEL to select QSFP 0 or 1 based on QSFP index

		Xil_Out32(wr_Address,MIO);

		usleep(1000); // Required to update MODSEL status on MIO

		status = i2c_send_rs_recv(i2c_num_pmc, QSFP_SLAVE_ADDRESS, &lsb_temp_reg, 1, &temperature_buff[0], i2c_read_len);
		if (status == XST_FAILURE)
		{
			CL_LOG (APP_VMC ,"failed to read QSFP LSB temperature register");
			return status;
		}

		status = i2c_send_rs_recv(i2c_num_pmc, QSFP_SLAVE_ADDRESS, &msb_temp_reg, 1, &temperature_buff[1], i2c_read_len);
		if (status == XST_FAILURE)
		{
			CL_LOG (APP_VMC ,"failed to read QSFP MSB temperature register");
			return status;
		}

		/* Store MS byte f temperature. */
		TemperatureHexValue = (temperature_buff[1] << 8) | temperature_buff[0];

		/* Temperature reading is a signed 16 bit value with a resolution of 1/256 C.
		 * The total range is from -128 to +128
		 * MSB bits have the following weights:
		 *  Bit 15: Sign bit
		 *  Bit 14: 64 C (2^6)
		 *  Bit 13: 32 C (2^5)
		 *  Bit 12: 16 C (2^4)
		 *  Bit 11: 8 C (2^3)
		 *  Bit 10: 4 C (2^2)
		 *  Bit 9:  2 C (2^1)
		 *  Bit 8:  1 C (2^0)
		 *
		 * LSB bits have the following weights:
		 *  Bit 7:  0.5 C (2^-1)
		 *  Bit 6:  0.25 C (2^-2)
		 *  Bit 5:  0.125 C (2^-3)
		 *  Bit 4:  0.625 C (2^-4)
		 *  Bit 3:  0.03125 C (2^-5)
		 *  Bit 2:  0.015625 C (2^-6)
		 *  Bit 1:  0.0078125 C (2^-7)
		 *  Bit 0:  0.00390625 C (2^-8)
		 * */


		/* From the above calculations, 0x7FFF is the largest temperature value that
		 * can be read regardless of the signed bit.
		 *
		 * Since the 15th bit is the sign bit , if we did read a negative temperature,
		 * then the value will be greater than 0x7FFF.
		 *
		 * For +ve temperature:
		 * TempHexVal * (2^-8) = Actual Temperature
		 *
		 * For -ve Temperature:
		 * Instead of taking the 2s complement, another approach to get the negative
		 * temperature would be to subtract the read temperature from 2^15 and
		 * then multiply with the resolution(00390625 C in out case) and add a -ve sign.
		 * (0x8000 - TempHexVal) * (2^-8) * (-1) = Actual Temperature
		 *
		 */
		if(TemperatureHexValue <= QSFP_MAX_POSITIVE_TEMP) /* +ve Temperature */
		{
			*TemperatureValue = TemperatureHexValue * QSFP_TEMPERATURE_RESOLUTION;
		}
		else /* -ve Temperature */
		{
			/* Ignore the signed bit here, since we have already determined that it is a
			 * negative temperature */
			TemperatureHexValue = TemperatureHexValue & QSFP_TEMP_BIT_MASK;
			*TemperatureValue = (QSFP_MAX_NEGETIVE_TEMP - TemperatureHexValue ) * QSFP_TEMPERATURE_RESOLUTION * (-1);
		}

		MIO = MIO | (1 << (14 + ((sensorInstance-1) * 5))); //set MODSEL to de-select QSFP 0 or 1 based on QSFP index

		Xil_Out32(wr_Address,MIO);
	}
	else
	{
		status = XST_DEVICE_NOT_FOUND;
	}
	return status;
}
