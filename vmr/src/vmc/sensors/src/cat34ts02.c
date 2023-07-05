/******************************************************************************usTempHexVal
 * Copyright (C) 2023 Xilinx, Inc.  All rights reserved.
 * SPDX-License-Identifier: MIT
 *******************************************************************************/

//#include "cl_uart_rtos.h"
#include "cl_log.h"
#include "cl_i2c.h"
#include "../inc/cat34ts02.h"

u8 ucCAT34TS02ReadTemperature( u8 ucI2cNum, u8 ucSlaveAddr, s16 *pssTemperatureValue )
{
    u8 ucStatus             = 1;
    u16 usTempHexVal        = 0;
    u8 ucI2cReadBuff[2]     = {0};
    u8 ucI2cReadLen         = 2;
    u8 ucI2cWriteBuff[1]    = {0};
    ucI2cWriteBuff[0]       = CAT34TS02_TEMPERATURE_REGISTER;

    if( NULL != pssTemperatureValue )
    {
        ucStatus = i2c_send_rs_recv( ucI2cNum, ucSlaveAddr, &ucI2cWriteBuff[0], 1, &ucI2cReadBuff[0], ucI2cReadLen );
        if( ucStatus )
        {
            VMR_LOG("Failed to read CAT34TS02 temperature\n\r");
        }
        else
        {
            /* Arrange the bytes in  the correct order */
            usTempHexVal = ucI2cReadBuff[1] | ( ucI2cReadBuff[0] << 8 );

            /* Bit 12 (MSB) determines if the number is positive or negative */
            const int negative = (usTempHexVal & ( 1 << CAT34TS02_TEMPERATURE_SIGN_BIT ) ) != 0;

            usTempHexVal &= ( 0xFFF );
            s16 nativeInt = 0;

            /* If the value was negative we need to correct the MSB to reflect that
            and finally divide by 16 to change the temperature to a whole number */
            if( negative )
            {
                nativeInt = usTempHexVal | ~((1 << CAT34TS02_TEMPERATURE_SIGN_BIT) - 1);
                *pssTemperatureValue = ( s16 )(nativeInt / ( 1 << CAT34TS02_TEMP_NON_WHOLE_BITS ) );
            }
            else
            {
                nativeInt = usTempHexVal;
                *pssTemperatureValue = ( s16 )(nativeInt / ( 1 << CAT34TS02_TEMP_NON_WHOLE_BITS ) );
            }
        }
    }
    return ucStatus;
}

u8 ucCAT34TS02ReadByte( u8 ucI2cNum, u8 ucSlaveAddr, u16 *pusAddressOffset,u8 *pucRegisterValue )
{
    u8 ucStatus = 1;

    if( ( NULL != pusAddressOffset ) &
        ( NULL != pucRegisterValue ) )
    {
        ucStatus = i2c_send_rs_recv( ucI2cNum, ucSlaveAddr, (u8 *)pusAddressOffset, 2, pucRegisterValue, 1 );

        if( ucStatus )
        {
            VMR_LOG("Failed to read CAT34TS02 \n\r");
        }
    }

    return ucStatus;
}

u8 ucCAT34TS02ReadMultiBytes(u8 ucI2cNum, u8 ucSlaveAddr, u16 *pusAddressOffset, u8 *pucRegisterValue, size_t xBufSize)
{
    u8 ucStatus = 1;

    if( ( NULL != pusAddressOffset ) &
        ( NULL != pucRegisterValue ) )
    {
        ucStatus = i2c_send_rs_recv(ucI2cNum, ucSlaveAddr, (u8 *)pusAddressOffset, CAT34TS02_EEPROM_ADDRESS_SIZE, pucRegisterValue, xBufSize);

        if( ucStatus )
        {
            VMR_LOG("Failed to read CAT34TS02 \n\r");
        }
    }

    return ucStatus;
}
