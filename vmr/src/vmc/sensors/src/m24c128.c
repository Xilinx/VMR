/******************************************************************************
 * Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
 * SPDX-License-Identifier: MIT
 *******************************************************************************/

#include "cl_uart_rtos.h"
#include "cl_log.h"
#include "cl_i2c.h"
#include "../inc/m24c128.h"
#include "../../vmc_api.h"

u8 M24C128_ReadByte(u8 i2c_num, u8 slaveAddr, u16 *AddressOffset,u8 *RegisterValue)
{
	u8 status = 1;

    if( ( NULL != AddressOffset ) &
        ( NULL != RegisterValue ) )
    {
        status = i2c_send_rs_recv(i2c_num, slaveAddr, (u8 *)AddressOffset, 2, RegisterValue, 1);

        if (status == XST_FAILURE)
        {
            CL_LOG(APP_VMC, "Failed to read M24C128 \n\r");
            return status;
        }

    }

	return status;
}

u8 M24C128_ReadMultiBytes(u8 i2c_num, u8 slaveAddr, u16 *AddressOffset, u8 *RegisterValue, size_t BufSize)
{
	u8 status = 1;

    if( ( NULL != AddressOffset ) &
        ( NULL != RegisterValue ) )
    {
        status = i2c_send_rs_recv(i2c_num, slaveAddr, (u8 *)AddressOffset, EEPROM_ADDRESS_SIZE, RegisterValue, BufSize);

        if (status == XST_FAILURE)
        {
            VMC_LOG("Failed to read M24C128 \n\r");
            return status;
        }
    }

	return status;
}
