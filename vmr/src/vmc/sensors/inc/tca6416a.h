/******************************************************************************
* Copyright (C) 2023 AMD, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#ifndef INCLUDE_TCA6416A_H
#define INCLUDE_TCA6416A_H

#include "xil_types.h"

#define TCA6416AR_ADDRESS           ( 0x20 )
#define TCA6416AR_CONFIGURATION_0   ( 0x06 )
#define TCA6416AR_OUTPUT_PORT_0     ( 0x02 )
#define TCA6416AR_BIT_6             ( 1 << 6 )

u8 ucTca6416aRegisterRead( u8 ucI2cNum, u8 ucSlaveAddr, u8 ucRegister, u8 *pucRegisterValue );

u8 ucTca6416aRegisterWrite( u8 ucI2cNum, u8 ucSlaveAddr, u8 ucRegister, u8 ucRegisterValue );

u8 ucEnableDDRDIMM( void );

#endif
