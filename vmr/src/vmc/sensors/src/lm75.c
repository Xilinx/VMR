/******************************************************************************
 * * Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
 * * SPDX-License-Identifier: MIT
 * *******************************************************************************/

#include "cl_i2c.h"
#include "lm75.h"

u8 LM75_ReadTemperature( u8 i2c_num, u8 slaveAddr, s16 *temperatureValue )
{
    u8 status = 1;
    u16 TempHexVal = 0;
    u8 i2c_read_buff[2] = {0};
    u8 i2c_read_len = 2;
    u8 i2c_write_buff[2];
    
    i2c_write_buff[0] = 0x00;
    
    if( NULL != temperatureValue )
    {
        status = i2c_send_rs_recv( i2c_num, slaveAddr, &i2c_write_buff[0], 1, &i2c_read_buff[0], i2c_read_len );
        if( 0 == status )
        {
            /* Arrange the bytes in  the correct order */
            TempHexVal = i2c_read_buff[1] | ( i2c_read_buff[0] << 8 );

            /* Lowest 7 bits are undefined so shift right */
            TempHexVal = TempHexVal >> 7;

            /* Bit 8 (MSB) determines if the number is positive or negative */
            const int negative = ( TempHexVal & ( 1 << 8 ) ) != 0;  
            TempHexVal &= ( 0xFF );
            s16 nativeInt = 0;

            /* If the value was negative we need to correct the MSB to reflect that 
            and finally divide by 2 to change the temperature to a whole number */
            if( negative )
            {
                nativeInt = TempHexVal | ~( ( 1 << 8 ) - 1 );
                *temperatureValue = ( s16 )( nativeInt/2 );
            }
            else
            {
                nativeInt = TempHexVal;
                *temperatureValue = ( s16 )( nativeInt/2 );
            }
        }
    }
    return status;
}
