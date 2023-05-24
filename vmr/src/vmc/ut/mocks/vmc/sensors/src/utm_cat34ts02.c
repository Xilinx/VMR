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
#include "cat34ts02.h"

u8 __wrap_ucCAT34TS02ReadTemperature( u8 ucI2cNum, u8 ucSlaveAddr, s16 *pssTemperatureValue )
{
    u8 status = 0;

    *pssTemperatureValue = 37;
     return status;
}
