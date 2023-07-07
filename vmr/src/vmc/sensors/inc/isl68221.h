
/******************************************************************************
 * * Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
 * * SPDX-License-Identifier: MIT
 * *******************************************************************************/

#ifndef INCLUDE_ISL68221_H
#define INCLUDE_ISL68221_H

#include "xil_types.h"

#define ISL68221_PAGE_REGISTER                    0x00

#define ISL68221_SELECT_PAGE_VCCINT               0x00
#define ISL68221_SELECT_PAGE_1v2_VCCO             0x01

#define ISL68221_OUTPUT_VOLTAGE_REGISTER          0x8B
#define ISL68221_OUTPUT_CURRENT_REGISTER          0x8C
#define ISL68221_READ_POWERSTAGE_TEMPERATURE      0x8D

u8 ISL68221_write_register(u8 i2c_num, u8 SlaveAddr, u8 register_address, u8 *register_content);
u8 ISL68221_read_register(u8 i2c_num, u8 SlaveAddr, u8 register_address, u8 *register_content);

u8 ISL68221_ReadVCCINT_Voltage(u8 busnum, u8 slaveAddr, float *voltageInmV);
u8 ISL68221_ReadVCCINT_Current(u8 busnum, u8 slaveAddr, float *currentInA);
u8 ISL68221_ReadVCCINT_Temperature(u8 busnum, u8 slaveAddr, float *temperature);

#endif
