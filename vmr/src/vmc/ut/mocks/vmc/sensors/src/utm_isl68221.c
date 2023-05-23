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
#include "isl68221.h"
#define SLAVE_ADDRESS_ISL68221_1     ( 0x61 )
/*****************************Mock functions *******************************/
u8 __wrap_ucISL68221ReadVoltage0( u8 ucBusNum, u8 ucSlaveAddr, float *pfVoltageInmV )
{
    u8 ucStatus = 0;

    if( SLAVE_ADDRESS_ISL68221_1 == ucSlaveAddr )
    {
        *pfVoltageInmV = 650;
    }
    else
    {
        *pfVoltageInmV = 12000;
    }

    return ucStatus;
}

u8 __wrap_ucISL68221ReadVoltage1( u8 ucBusNum, u8 ucSlaveAddr, float *pfVoltageInmV )
{
    u8 ucStatus = 0;
    if( SLAVE_ADDRESS_ISL68221_1 == ucSlaveAddr )
    {
        *pfVoltageInmV = 1266;
    }
    else
    {
        *pfVoltageInmV = 12;
    }

    return ucStatus;
}

u8 __wrap_ucISL68221ReadVoltage2( u8 ucBusNum, u8 ucSlaveAddr, float *pfVoltageInmV )
{
    u8 ucStatus = 0;
    if( SLAVE_ADDRESS_ISL68221_1 == ucSlaveAddr )
    {
        *pfVoltageInmV = 1267;
    }
    else
    {
        *pfVoltageInmV = 12;
    }

    return ucStatus;
}

u8 __wrap_ucISL68221ReadCurrent0( u8 ucBusNum, u8 ucSlaveAddr, float *pfCurrentInA )
{
    u8 ucucStatus = 0;
    if( SLAVE_ADDRESS_ISL68221_1 == ucSlaveAddr )
    {
        *pfCurrentInA = 0.75;
    }
    else
    {
        *pfCurrentInA = 4;
    }

    return ucucStatus;
}

u8 __wrap_ucISL68221ReadCurrent1( u8 ucBusNum, u8 ucSlaveAddr, float *pfCurrentInA )
{
    u8 ucucStatus = 0;
    if( SLAVE_ADDRESS_ISL68221_1 == ucSlaveAddr )
    {
        *pfCurrentInA = 0.76;
    }
    else
    {
        *pfCurrentInA = 4;
    }

    return ucucStatus;
}

u8 __wrap_ucISL68221ReadCurrent2( u8 ucBusNum, u8 ucSlaveAddr, float *pfCurrentInA )
{
    u8 ucStatus = 0;
    if( SLAVE_ADDRESS_ISL68221_1 == ucSlaveAddr )
    {
        *pfCurrentInA = 0.77;
    }
    else
    {
        *pfCurrentInA = 4;
    }

    return ucStatus;
}

u8 __wrap_ucISL68221ReadTemperature0( u8 ucBusNum, u8 ucSlaveAddr, float *pfTemperature )
{
    u8 ucStatus = 0;
    *pfTemperature = 60;

    return ucStatus;
}

u8 __wrap_ucISL68221ReadTemperature1( u8 ucBusNum, u8 ucSlaveAddr, float *pfTemperature )
{
    u8 ucStatus = 0;
    *pfTemperature = 60;

    return ucStatus;
}

u8 __wrap_ucISL68221ReadTemperature2( u8 ucBusNum, u8 ucSlaveAddr, float *pfTemperature )
{
    u8 ucStatus = 0;
    *pfTemperature = 60;

    return ucStatus;
}
