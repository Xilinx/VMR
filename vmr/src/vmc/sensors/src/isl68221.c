/******************************************************************************
 *  * * Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
 *   * * SPDX-License-Identifier: MIT
 *    * *******************************************************************************/

#include "cl_i2c.h"
#include "../inc/isl68221.h"

#define STATUS_FAILURE      ( 1 )

u8 ucISL68221WriteRegister( u8 ucI2cNum, u8 ucSlaveAddr, u8 ucRegisterAddress, u8 *pucRegisterContent )
{
    u8 ucWriteData[2]   = {0};
    u8 ucStatus         = STATUS_FAILURE;
    
    if( NULL != pucRegisterContent )
    {
        ucWriteData[0]      = ucRegisterAddress;
        ucWriteData[1]      = *pucRegisterContent;
        
        ucStatus = i2c_send(ucI2cNum, ucSlaveAddr, ucWriteData, 2 );
    }

    return( ucStatus );
}

u8 ucISL68221ReadRegister( u8 ucI2cNum, u8 ucSlaveAddr, u8 ucRegisterAddress, u8 *pucRegisterContent )
{
    u8 ucStatus = STATUS_FAILURE;
    
    if( NULL != pucRegisterContent )
    {
        ucStatus = i2c_send_rs_recv( ucI2cNum, ucSlaveAddr, &ucRegisterAddress,1, pucRegisterContent, 2 );
    }
    
    return( ucStatus );
}

u8 ucISL68221ReadVoltage0( u8 ucBusNum, u8 ucSlaveAddr, float *pfVoltageInmV )
{
    u8 ucWriteBuf[8]    = {0};
    u8 ucReadBuf[8]     = {0};
    u8 ucStatus         = 1;
    u16 usReadData      = 0;

    if( NULL != pfVoltageInmV )
    {
        ucWriteBuf[0] = ISL68221_SELECT_PAGE_RAIL_0;

        ucStatus = ucISL68221WriteRegister( ucBusNum, ucSlaveAddr, ISL68221_PAGE_REGISTER,( u8 * )ucWriteBuf );
        if( STATUS_FAILURE == ucStatus )
        {
            return ucStatus;
        }

        ucStatus = ucISL68221ReadRegister( ucBusNum, ucSlaveAddr, ISL68221_OUTPUT_VOLTAGE_REGISTER,( u8 * )&ucReadBuf[0] );
        if( STATUS_FAILURE == ucStatus ) 
        {
            return ucStatus;
        }
        usReadData = ( ucReadBuf[1] << 8 ) | ucReadBuf[0];
        *pfVoltageInmV = ( ( float ) usReadData );
    }

    return ucStatus;
}

u8 ucISL68221ReadVoltage1(u8 ucBusNum, u8 ucSlaveAddr, float *pfVoltageInmV)
{
    u8 ucWriteBuf[8]    = {0};
    u8 ucReadBuf[8]     = {0};
    u8 ucStatus         = 1;
    u16 usReadData      = 0;

    if( NULL != pfVoltageInmV )
    {
        ucWriteBuf[0] = ISL68221_SELECT_PAGE_RAIL_1;

        ucStatus = ucISL68221WriteRegister( ucBusNum, ucSlaveAddr, ISL68221_PAGE_REGISTER,( u8 * )ucWriteBuf );
        if( STATUS_FAILURE == ucStatus )
        {
            return ucStatus;
        }

        ucStatus = ucISL68221ReadRegister( ucBusNum, ucSlaveAddr, ISL68221_OUTPUT_VOLTAGE_REGISTER,( u8 * )&ucReadBuf[0] );
        if( STATUS_FAILURE == ucStatus )
        {
            return ucStatus;
        }
        usReadData = ( ucReadBuf[1] << 8 ) | ucReadBuf[0];
        *pfVoltageInmV = ( ( float ) usReadData );
    }
    
    return ucStatus;
}

u8 ucISL68221ReadVoltage2( u8 ucBusNum, u8 ucSlaveAddr, float *pfVoltageInmV )
{
    u8 ucWriteBuf[8]    = {0};
    u8 ucReadBuf[8]     = {0};
    u8 ucStatus         = 1;
    u16 usReadData      = 0;

    if( NULL != pfVoltageInmV )
    {
        ucWriteBuf[0] = ISL68221_SELECT_PAGE_RAIL_2;

        ucStatus = ucISL68221WriteRegister( ucBusNum, ucSlaveAddr, ISL68221_PAGE_REGISTER,( u8 * )ucWriteBuf );
        if( STATUS_FAILURE == ucStatus )
        {
            return ucStatus;
        }

        ucStatus = ucISL68221ReadRegister( ucBusNum, ucSlaveAddr, ISL68221_OUTPUT_VOLTAGE_REGISTER,( u8 * )&ucReadBuf[0] );
        if( STATUS_FAILURE == ucStatus )
        {
            return ucStatus;
        }
        usReadData = ( ucReadBuf[1] << 8 ) | ucReadBuf[0];
        *pfVoltageInmV = ( ( float ) usReadData );
    }
    
    return ucStatus;
}

u8 ucISL68221ReadCurrent0( u8 ucBusNum, u8 ucSlaveAddr, float *pfCurrentInA )
{
    u8 ucWriteBuf[8]    = {0};
    u8 ucReadBuf[8]     = {0};
    u8 ucStatus         = 1;
    u16 usReadData      = 0;

    if( NULL != pfCurrentInA )
    {
        ucWriteBuf[0] = ISL68221_SELECT_PAGE_RAIL_0;

        ucStatus = ucISL68221WriteRegister( ucBusNum, ucSlaveAddr, ISL68221_PAGE_REGISTER, ( u8 * )ucWriteBuf );
        if( STATUS_FAILURE == ucStatus )
        {
            return ucStatus;
        }

        ucStatus = ucISL68221ReadRegister( ucBusNum, ucSlaveAddr, ISL68221_OUTPUT_CURRENT_REGISTER, ( u8 * )&ucReadBuf[0] );
        if( STATUS_FAILURE == ucStatus )
        {
            return ucStatus;
        }
        usReadData = ( ucReadBuf[1] << 8 ) | ucReadBuf[0];
        *pfCurrentInA = ( ( float ) usReadData )/10;
    }
    
    return ucStatus;

}

u8 ucISL68221ReadCurrent1( u8 ucBusNum, u8 ucSlaveAddr, float *pfCurrentInA )
{
    u8 ucWriteBuf[8]    = {0};
    u8 ucReadBuf[8]     = {0};
    u8 ucStatus         = 1;
    u16 usReadData      = 0;

    if( NULL != pfCurrentInA )
    {
        ucWriteBuf[0] = ISL68221_SELECT_PAGE_RAIL_1;

        ucStatus = ucISL68221WriteRegister( ucBusNum, ucSlaveAddr, ISL68221_PAGE_REGISTER, ( u8 * )ucWriteBuf );
        if( STATUS_FAILURE == ucStatus )
        {
            return ucStatus;
        }

        ucStatus = ucISL68221ReadRegister( ucBusNum, ucSlaveAddr, ISL68221_OUTPUT_CURRENT_REGISTER,( u8 * )&ucReadBuf[0] );
        if( STATUS_FAILURE == ucStatus )
        {
            return ucStatus;
        }
        usReadData = ( ucReadBuf[1] << 8 ) | ucReadBuf[0];
        *pfCurrentInA = ( ( float ) usReadData )/10;
    }
    
    return ucStatus;
}

u8 ucISL68221ReadCurrent2( u8 ucBusNum, u8 ucSlaveAddr, float *pfCurrentInA )
{
    u8 ucWriteBuf[8]    = {0};
    u8 ucReadBuf[8]     = {0};
    u8 ucStatus         = 1;
    u16 usReadData      = 0;

    if( NULL != pfCurrentInA )
    {
        ucWriteBuf[0] = ISL68221_SELECT_PAGE_RAIL_2;

        ucStatus = ucISL68221WriteRegister( ucBusNum, ucSlaveAddr, ISL68221_PAGE_REGISTER,( u8 * )ucWriteBuf );
        if( STATUS_FAILURE == ucStatus )
        {
            return ucStatus;
        }

        ucStatus = ucISL68221ReadRegister( ucBusNum, ucSlaveAddr, ISL68221_OUTPUT_CURRENT_REGISTER,( u8 * )&ucReadBuf[0] );
        if( STATUS_FAILURE == ucStatus )
        {
            return ucStatus;
        }
        usReadData = ( ucReadBuf[1] << 8 ) | ucReadBuf[0];
        *pfCurrentInA = ( ( float ) usReadData )/10;
    }
    
    return ucStatus;

}

u8 ucISL68221ReadTemperature0( u8 ucBusNum, u8 ucSlaveAddr, float *pfTemperature )
{
    u8 ucWriteBuf[8]    = {0};
    u8 ucReadBuf[8]     = {0};
    u8 ucStatus         = 1;
    u16 usReadData      = 0;

    if( NULL != pfTemperature )
    {
        ucWriteBuf[0] = ISL68221_SELECT_PAGE_RAIL_0;

        ucStatus = ucISL68221WriteRegister( ucBusNum, ucSlaveAddr, ISL68221_PAGE_REGISTER, ( u8 * )ucWriteBuf );
        if( STATUS_FAILURE == ucStatus )
        {
            return ucStatus;
        }

        ucStatus = ucISL68221ReadRegister(ucBusNum, ucSlaveAddr, ISL68221_READ_TEMPERATURE_0, ( u8 * )&ucReadBuf[0] );
        if( STATUS_FAILURE == ucStatus )
        {
            return ucStatus;
        }
        usReadData = ( ucReadBuf[1] << 8 ) | ucReadBuf[0];
        *pfTemperature = ( ( float ) usReadData );
    }
    
    return ucStatus;
}

u8 ucISL68221ReadTemperature1( u8 ucBusNum, u8 ucSlaveAddr, float *pfTemperature )
{
    u8 ucWriteBuf[8]    = {0};
    u8 ucReadBuf[8]     = {0};
    u8 ucStatus         = 1;
    u16 usReadData      = 0;

    if( NULL != pfTemperature )
    {
        ucWriteBuf[0] = ISL68221_SELECT_PAGE_RAIL_0;

        ucStatus = ucISL68221WriteRegister( ucBusNum, ucSlaveAddr, ISL68221_PAGE_REGISTER, ( u8 * )ucWriteBuf );
        if( STATUS_FAILURE == ucStatus )
        {
            return ucStatus;
        }

        ucStatus = ucISL68221ReadRegister( ucBusNum, ucSlaveAddr, ISL68221_READ_TEMPERATURE_1, ( u8 * )&ucReadBuf[0] );
        if( STATUS_FAILURE == ucStatus )
        {
            return ucStatus;
        }
        usReadData = (ucReadBuf[1] << 8) | ucReadBuf[0];
        *pfTemperature = ( ( float ) usReadData) ;
    }
        
    return ucStatus;
}

u8 ucISL68221ReadTemperature2(u8 ucBusNum, u8 ucSlaveAddr, float *pfTemperature)
{
    u8 ucWriteBuf[8]    = {0};
    u8 ucReadBuf[8]     = {0};
    u8 ucStatus         = 1;
    u16 usReadData      = 0;

    if( NULL != pfTemperature )
    {
        ucWriteBuf[0] = ISL68221_SELECT_PAGE_RAIL_0;

        ucStatus = ucISL68221WriteRegister( ucBusNum, ucSlaveAddr, ISL68221_PAGE_REGISTER, ( u8 * )ucWriteBuf );
        if( STATUS_FAILURE == ucStatus )
        {
            return ucStatus;
        }

        ucStatus = ucISL68221ReadRegister( ucBusNum, ucSlaveAddr, ISL68221_READ_TEMPERATURE_2, ( u8 * )&ucReadBuf[0] );
        if( STATUS_FAILURE == ucStatus )
        {
            return ucStatus;
        }
        usReadData = ( ucReadBuf[1] << 8 ) | ucReadBuf[0];
        *pfTemperature = ( ( float ) usReadData );
    }
    
    return ucStatus;
}
