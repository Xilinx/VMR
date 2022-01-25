/******************************************************************************
 * * Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
 * * SPDX-License-Identifier: MIT
 * *******************************************************************************/

#include "cl_i2c.h"
#include "../inc/lm75.h"

u8 LM75_ReadTemperature(u8 i2c_num, u8 slaveAddr, s32 *temperatureValue)
{
    u8 status = XST_FAILURE;
    s32 TempHexVal = 0;
    u8 i2c_read_buff[2] = {0};
    u8 i2c_read_len = 2;
    u8 i2c_write_buff[2];
    
    i2c_write_buff[0] = 0x00;
    
    status = i2c_send_rs_recv(i2c_num, slaveAddr, &i2c_write_buff[0], 1, &i2c_read_buff[0], i2c_read_len);
    if(XST_SUCCESS == status)
    {
		/* only 11 MSB should be used , 5 LSB bits are zero and ignored  */
		TempHexVal = i2c_read_buff[1] | (i2c_read_buff[0] << 8);
		TempHexVal = TempHexVal >> 5;

		/* The 11th bit is the sign bit ,
		 *
		 * For +ve temperature:
		 * TempHexVal * 0.125 = Actual Temperature
		 *
		 * For -ve Temperature:
		 * -(two's compliment of TempHexVal) * 0.125 = Actual Temperature
		 */

		if(TempHexVal < 0x3FF)
		{
			/* Positive Temperature */
			*temperatureValue = (int)TempHexVal * 0.125;
		}
		else
		{
			/* Negative Temperature */
			TempHexVal = TempHexVal & 0x7FF;
			*temperatureValue = (int)(0x800 - TempHexVal ) * 0.125 * (-1);
	}
    }

    return status;
}
