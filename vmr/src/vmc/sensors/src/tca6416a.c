/******************************************************************************
* Copyright (C) 2023 AMD, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#include "cl_i2c.h"
#include "cl_log.h"
#include "xstatus.h"
#include "tca6416a.h"

u8 ucTca6416aRegisterRead( u8 ucI2cNum, u8 ucSlaveAddr, u8 ucRegister, u8 *pucRegisterValue )
{
    u8 ucStatus         = 1;
    u8 pucSendData[1]   = { 0 };

    if( NULL != pucRegisterValue )
    {
        pucSendData[0] = ucRegister;
        
        ucStatus = i2c_send_rs_recv( ucI2cNum, ucSlaveAddr, pucSendData, 1, pucRegisterValue, 1 );
    }

    return ucStatus;
}

u8 ucTca6416aRegisterWrite( u8 ucI2cNum, u8 ucSlaveAddr, u8 ucRegister, u8 ucRegisterValue )
{
    u8 ucStatus         = 1;
    u8 pucSendData[2]   = { 0 };

    pucSendData[0] = ucRegister;
    pucSendData[1] = ucRegisterValue;
    
    ucStatus = i2c_send( ucI2cNum, ucSlaveAddr, pucSendData, 2 );

    return ucStatus;
}

u8 ucEnableDDRDIMM( void )
{
    u8 ucStatus         = 1;
    u8 ucI2cNum         = 1;
    u8 ucRegisterValue  = 0;

    /* Read Configuration 0 Register - Command Byte 06 */
    ucStatus = ucTca6416aRegisterRead( ucI2cNum, TCA6416AR_ADDRESS, TCA6416AR_CONFIGURATION_0, &ucRegisterValue );

    if( XST_SUCCESS == ucStatus )
    {
        /* Write Configuration 0 Register setting bit 6 to 0 - Command Byte 06 */
        ucRegisterValue = ( ucRegisterValue & ~( TCA6416AR_BIT_6 ) );
        ucStatus = ucTca6416aRegisterWrite( ucI2cNum, TCA6416AR_ADDRESS, TCA6416AR_CONFIGURATION_0, ucRegisterValue );
        
        if( XST_SUCCESS == ucStatus )
        {
            /* Read Output 0 Register - Command Byte 06 */
            ucStatus = ucTca6416aRegisterRead( ucI2cNum, TCA6416AR_ADDRESS, TCA6416AR_OUTPUT_PORT_0, &ucRegisterValue );
            
            if( XST_SUCCESS == ucStatus )
            {
                /* Write Output 0 Register setting bit 6 to 1 - Command Byte 02 */
                ucRegisterValue = ( ucRegisterValue | ( TCA6416AR_BIT_6 ) );
                ucStatus = ucTca6416aRegisterWrite( ucI2cNum, TCA6416AR_ADDRESS, TCA6416AR_OUTPUT_PORT_0, ucRegisterValue );
            }
        }
    }

    if( XST_SUCCESS != ucStatus )
    {
        CL_LOG(APP_VMC, "Failed to enable DDR DIMM\n\r" );
    }

    return ucStatus;
}
