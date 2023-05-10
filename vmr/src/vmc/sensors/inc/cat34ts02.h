/******************************************************************************
 * Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
 * SPDX-License-Identifier: MIT
 *******************************************************************************/

#include "xil_types.h"

#define CAT34TS02_SLAVE_ADDRESS             0x52
#define CAT34TS02_SLAVE_ADDRESS_TEMPERATURE 0x1A
#define CAT34TS02_TEMPERATURE_REGISTER      0x05
#define EEPROM_ADDRESS_SIZE 2

u8 ucCAT34TS02ReadTemperature( u8 ucI2cNum, u8 ucSlaveAddr, s16 *pssTemperatureValue );
u8 ucCAT34TS02ReadByte( u8 ucI2cNum, u8 ucSlaveAddr, u16 *pusAddressOffset,u8 *pucRegisterValue );
u8 ucCAT34TS02ReadMultiBytes(u8 ucI2cNum, u8 ucSlaveAddr, u16 *pusAddressOffset, u8 *pucRegisterValue, size_t xBufSize);
