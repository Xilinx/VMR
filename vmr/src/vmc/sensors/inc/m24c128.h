/******************************************************************************
 * Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
 * SPDX-License-Identifier: MIT
 *******************************************************************************/

#include "xil_types.h"

#define SLAVE_ADDRESS_M24C128 0x52

#define EEPROM_ADDRESS_SIZE 2

u8 M24C128_ReadByte(u8 i2c_num, u8 slaveAddr, u16 *AddressOffset,u8 *RegisterValue);
u8 M24C128_ReadMultiBytes(u8 i2c_num, u8 slaveAddr, u16 *AddressOffset, u8 *RegisterValue, size_t BufSize);
