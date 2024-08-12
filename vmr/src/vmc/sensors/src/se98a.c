/******************************************************************************
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#include "cl_i2c.h"
#include "se98a.h"

u8 SE98A_ReadTemperature(u8 i2c_num, u8 slaveAddr, s16 *temperatureValue)
{
    u8 status = 0;
    s16 TempHexVal = 0;
    unsigned char i2c_read_buff[2] = {0};
    long int i2c_read_len = 2;
    unsigned char i2c_write_buff[2];
    
    i2c_write_buff[0] = 0x05;
    
    status = i2c_send_rs_recv(i2c_num, slaveAddr, &i2c_write_buff[0], 1, &i2c_read_buff[0], i2c_read_len);
    if(0 == status)
    {
	/* only high 13 bits of the returned 16 bits are used */
	TempHexVal = i2c_read_buff[1] | (i2c_read_buff[0] << 8);
	TempHexVal = TempHexVal & 0x1FFF;

	/* The 13th bit is the sign bit ,
	 *
	 * For +ve temperature:
	 * TempHexVal * 0.0625 = Actual Temperature
	 *
	 * For -ve Temperature:
	 * (1000 - TempHexVal) * 0.625 = Actual Temperature
	 */

	if(TempHexVal < 0xFFF) 
	{
	    /* Positive Temperature */
	    *temperatureValue = (int)TempHexVal * 0.0625;
	}
	else 
	{
	    /* Negative Temperature */
	    TempHexVal = TempHexVal & 0xFFF;
	    *temperatureValue = (int)(0x1000 - TempHexVal ) * 0.0625 * (-1);
	}
    }

    return status;
}

