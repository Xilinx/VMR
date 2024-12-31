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
#include "ina3221.h"

#define SLAVE_ADDRESS_INA3221_1     ( 0x41 )

float fINA3221ChangedValue = 999;

void vINA3221SetNewValue(float fNewValue)
{
	fINA3221ChangedValue = fNewValue;
}

/*****************************Mock functions *******************************/
u8 __wrap_INA3221_ReadVoltage( u8 ucBusNum, u8 ucSlaveAddr, u8 ucChannelNum, float *pfVoltageInmV )
{
	u8 ucStatus = 0;

    if( SLAVE_ADDRESS_INA3221_1 == ucSlaveAddr )
    {
        if( 0 == ucChannelNum )
        {
            *pfVoltageInmV = ( fINA3221ChangedValue ? 3300 : 0);
        }
        else if( 1 == ucChannelNum )
        {
            *pfVoltageInmV = ( fINA3221ChangedValue ? 12073 : 0);
        }
        else if( 2 == ucChannelNum )
        {
            *pfVoltageInmV = ( fINA3221ChangedValue ? 12074 : 0);
        }
    }
    else
    {
        if( 0 == ucChannelNum )
        {
            *pfVoltageInmV = ( fINA3221ChangedValue ? 12000 : 0);
        }
        else if( 1 == ucChannelNum )
        {
            *pfVoltageInmV = ( fINA3221ChangedValue ? 3000 : 0);
        }
        else if( 2 == ucChannelNum )
        {
            *pfVoltageInmV = ( fINA3221ChangedValue ? 1561 : 0);
        }
    }
        return mock_type(u8);
}

u8 __wrap_INA3221_ReadCurrent( u8 ucBusNum, u8 ucSlaveAddr, u8 ucChannelNum, float *pfCurrentInmA )
{
	u8 ucStatus = 0;
    if( SLAVE_ADDRESS_INA3221_1 == ucSlaveAddr )
    {
        if( 0 == ucChannelNum )
        {
            *pfCurrentInmA = ( fINA3221ChangedValue ? ( ( 6000 == fINA3221ChangedValue ) ? 6000 : 720 ) : 0 );
        }
        else if( 1 == ucChannelNum )
        {
            *pfCurrentInmA = ( fINA3221ChangedValue ? ( ( 6000 == fINA3221ChangedValue ) ? 6000 : 730 ) : 0 );
        }
        else if( 2 == ucChannelNum )
        {
            *pfCurrentInmA = ( fINA3221ChangedValue ? ( ( 6000 == fINA3221ChangedValue ) ? 6000 : 740 ) : 0 );
        }
    }
    else
    {
        if( 0 == ucChannelNum)
        {
            *pfCurrentInmA = ( fINA3221ChangedValue ? ( ( 6000 == fINA3221ChangedValue ) ? 6000 : 5000 ) : 0 );
        }
        else if( 1 == ucChannelNum )
        {
            *pfCurrentInmA = ( fINA3221ChangedValue ? 4000 : 0);
        }
        else if( 2 == ucChannelNum )
        {
            *pfCurrentInmA = ( fINA3221ChangedValue ? 710 : 0);
        }
    }
        return mock_type(u8);
}

u8 __wrap_INA3221_ReadPower( u8 ucBusNum, u8 ucSlaveAddr, u8 ucChannelNum, float *pfPowerInmW )
{
        u8 ucStatus = 0;
        *pfPowerInmW = 75;

        return mock_type(u8);
}

