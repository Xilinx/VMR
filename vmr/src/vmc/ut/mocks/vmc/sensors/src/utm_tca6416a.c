/******************************************************************************
* Copyright (C) 2023 AMD, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <cmocka.h>

#include "cl_i2c.h"
#include "tca6416a.h"

u8 __wrap_ucTca6416aRegisterRead( u8 ucI2cNum, u8 ucSlaveAddr, u8 ucRegister,u8 *pucRegisterValue )
{
    u8 status = 0;

    *pucRegisterValue = 0xFF;
     return status;
}

u8 __wrap_ucTca6416aRegisterWrite( u8 ucI2cNum, u8 ucSlaveAddr, u8 ucRegister, u8 ucRegisterValue )
{
    u8 status = 0;

     return status;
}

u8 __wrap_ucEnableDDRDIMM( void )
{
    u8 status = 0;

     return status;
}
