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
#include "ina3221.h"

#define SLAVE_ADDRESS_INA3221_1     ( 0x41 )

/*****************************Mock functions *******************************/
u8 __wrap_INA3221_ReadVoltage( u8 ucBusNum, u8 ucSlaveAddr, u8 ucChannelNum, float *pfVoltageInmV )
{
	u8 ucStatus = 0;

    if( SLAVE_ADDRESS_INA3221_1 == ucSlaveAddr )
    {
        if( 0 == ucChannelNum )
        {
            *pfVoltageInmV = 3300;
        }
        else if( 1 == ucChannelNum )
        {
            *pfVoltageInmV = 12073;
        }
        else if( 2 == ucChannelNum )
        {
            *pfVoltageInmV = 12074;
        }
    }
    else
    {
        if( 0 == ucChannelNum )
        {
            *pfVoltageInmV = 12000;
        }
        else if( 1 == ucChannelNum )
        {
            *pfVoltageInmV = 3000;
        }
        else if( 2 == ucChannelNum )
        {
            *pfVoltageInmV = 1561;
        }
    }
        return ucStatus;
}

u8 __wrap_INA3221_ReadCurrent( u8 ucBusNum, u8 ucSlaveAddr, u8 ucChannelNum, float *pfCurrentInmA )
{
	u8 ucStatus = 0;
    if( SLAVE_ADDRESS_INA3221_1 == ucSlaveAddr )
    {
        if( 0 == ucChannelNum )
        {
            *pfCurrentInmA = 720;
        }
        else if( 1 == ucChannelNum )
        {
            *pfCurrentInmA = 730;
        }
        else if( 2 == ucChannelNum )
        {
            *pfCurrentInmA = 740;
        }
    }
    else
    {
        if( 0 == ucChannelNum)
        {
            *pfCurrentInmA = 5000;
        }
        else if( 1 == ucChannelNum )
        {
            *pfCurrentInmA = 4000;
        }
        else if( 2 == ucChannelNum )
        {
            *pfCurrentInmA = 710;
        }
}
        return ucStatus;
}

u8 __wrap_INA3221_ReadPower( u8 ucBusNum, u8 ucSlaveAddr, u8 ucChannelNum, float *pfPowerInmW )
{
        u8 ucStatus = 0;
        *pfPowerInmW = 75;

        return ucStatus;
}

