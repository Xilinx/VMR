/******************************************************************************
* Copyright (C) 2023 Advanced Micro Devices, Inc.  All rights reserved.
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

float fChangedValue = 999;

void vSetNewValue(float fNewValue)
{
	fChangedValue = fNewValue;
}

/*****************************Mock functions *******************************/
u8 __wrap_ucISL68221ReadVoltage0( u8 ucBusNum, u8 ucSlaveAddr, float *pfVoltageInmV )
{
    u8 ucStatus = 0;

    if( SLAVE_ADDRESS_ISL68221_1 == ucSlaveAddr )
    {
        *pfVoltageInmV = ( fChangedValue ? 650 : 0);
    }
    else
    {
        *pfVoltageInmV = ( fChangedValue ? 12000 : 0);
    }

    if( 0 == fChangedValue )
    {
        pfVoltageInmV = 0;
    }

    return mock_type(u8);
}

u8 __wrap_ucISL68221ReadVoltage1( u8 ucBusNum, u8 ucSlaveAddr, float *pfVoltageInmV )
{
    u8 ucStatus = 0;
    if( SLAVE_ADDRESS_ISL68221_1 == ucSlaveAddr )
    {
        *pfVoltageInmV = ( fChangedValue ? 1266 : 0);
    }
    else
    {
        *pfVoltageInmV = ( fChangedValue ? 12 : 0);
    }

    return mock_type(u8);
}

u8 __wrap_ucISL68221ReadVoltage2( u8 ucBusNum, u8 ucSlaveAddr, float *pfVoltageInmV )
{
    u8 ucStatus = 0;
    if( SLAVE_ADDRESS_ISL68221_1 == ucSlaveAddr )
    {
        *pfVoltageInmV = ( fChangedValue ? 1267 : 0);
    }
    else
    {
        *pfVoltageInmV = ( fChangedValue ? 12 : 0);
    }

    return mock_type(u8);
}

u8 __wrap_ucISL68221ReadCurrent0( u8 ucBusNum, u8 ucSlaveAddr, float *pfCurrentInA )
{
    u8 ucucStatus = 0;
    if( SLAVE_ADDRESS_ISL68221_1 == ucSlaveAddr )
    {
        *pfCurrentInA = ( fChangedValue ? 0.75 : 0);
    }
    else
    {
        *pfCurrentInA = ( fChangedValue ? 4 : 0);
    }

    return mock_type(u8);
}

u8 __wrap_ucISL68221ReadCurrent1( u8 ucBusNum, u8 ucSlaveAddr, float *pfCurrentInA )
{
    u8 ucucStatus = 0;
    if( SLAVE_ADDRESS_ISL68221_1 == ucSlaveAddr )
    {
        *pfCurrentInA = ( fChangedValue ? 0.76 : 0);
    }
    else
    {
        *pfCurrentInA = ( fChangedValue ? 4 : 0);
    }

    return mock_type(u8);
}

u8 __wrap_ucISL68221ReadCurrent2( u8 ucBusNum, u8 ucSlaveAddr, float *pfCurrentInA )
{
    u8 ucStatus = 0;
    if( SLAVE_ADDRESS_ISL68221_1 == ucSlaveAddr )
    {
        *pfCurrentInA = ( fChangedValue ? 0.77 : 0);
    }
    else
    {
        *pfCurrentInA = ( fChangedValue ? 4 : 0);
    }

    return mock_type(u8);
}

u8 __wrap_ucISL68221ReadTemperature0( u8 ucBusNum, u8 ucSlaveAddr, float *pfTemperature )
{
    u8 ucStatus = 0;
    *pfTemperature = ( fChangedValue ? 60 : 110);

    return mock_type(u8);
}

u8 __wrap_ucISL68221ReadTemperature1( u8 ucBusNum, u8 ucSlaveAddr, float *pfTemperature )
{
    u8 ucStatus = 0;
    *pfTemperature = ( fChangedValue ? 60 : 110);

    return mock_type(u8);
}

u8 __wrap_ucISL68221ReadTemperature2( u8 ucBusNum, u8 ucSlaveAddr, float *pfTemperature )
{
    u8 ucStatus = 0;
    *pfTemperature = ( fChangedValue ? 60 : 110);

    return mock_type(u8);
}
