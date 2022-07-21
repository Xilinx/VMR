/******************************************************************************
 * * Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
 * * SPDX-License-Identifier: MIT
 * *******************************************************************************/

#include "cl_i2c.h"
#include "../inc/lm75.h"

u8 LM75_ReadTemperature(u8 i2c_num, u8 slaveAddr, s32 *temperatureValue)
{
    u8 status = 0;
    u16 TempHexVal = 0;
    u8 i2c_read_buff[2] = {0};
    u8 i2c_read_len = 2;
    u8 i2c_write_buff[2];
    
    i2c_write_buff[0] = 0x00;
    
    status = i2c_send_rs_recv(i2c_num, slaveAddr, &i2c_write_buff[0], 1, &i2c_read_buff[0], i2c_read_len);
    if(0 == status)
    	{
		TempHexVal = i2c_read_buff[1] | (i2c_read_buff[0] << 8);
		TempHexVal = TempHexVal >> 7;

		const int negative = (TempHexVal & (1 << 8)) != 0;
		TempHexVal &= (0xFF);
		s16 nativeInt = 0;

		if (negative)
		{
  			nativeInt = TempHexVal | ~((1 << 8) - 1);
			*temperatureValue = (s16)(nativeInt/2);
		}
		else
		{
			nativeInt = TempHexVal;
			*temperatureValue = (s16)(nativeInt/2);
    	}
		}

    return status;
}
