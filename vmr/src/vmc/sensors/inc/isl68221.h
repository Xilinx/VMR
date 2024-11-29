
/******************************************************************************
 * * Copyright (C) 2024 AMD, Inc.    All rights reserved.
 * * SPDX-License-Identifier: MIT
 * *******************************************************************************/

#ifndef INCLUDE_ISL68221_H
#define INCLUDE_ISL68221_H

#include "xil_types.h"

#define ISL68221_PAGE_REGISTER                    0x00

#define ISL68221_SELECT_PAGE_RAIL_0               0x00
#define ISL68221_SELECT_PAGE_RAIL_1               0x01
#define ISL68221_SELECT_PAGE_RAIL_2               0x01

#define ISL68221_OUTPUT_VOLTAGE_REGISTER          0x8B
#define ISL68221_OUTPUT_CURRENT_REGISTER          0x8C
#define ISL68221_READ_TEMPERATURE_0               0x8D
#define ISL68221_READ_TEMPERATURE_1               0x8E
#define ISL68221_READ_TEMPERATURE_2               0x8F

u8 ucISL68221WriteRegister(u8 ucI2cNum, u8 ucSlaveAddr, u8 ucRegisterAddress, u8 *pucRegisterContent);
u8 ucISL68221ReadRegister(u8 ucI2cNum, u8 ucSlaveAddr, u8 ucRegisterAddress, u8 *pucRegisterContent);

u8 ucISL68221ReadVoltage0(u8 ucBusNum, u8 ucSlaveAddr, float *pfVoltageInmV);
u8 ucISL68221ReadVoltage1(u8 ucBusNum, u8 ucSlaveAddr, float *pfVoltageInmV);
u8 ucISL68221ReadVoltage2(u8 ucBusNum, u8 ucSlaveAddr, float *pfVoltageInmV);
u8 ucISL68221ReadCurrent0(u8 ucBusNum, u8 ucSlaveAddr, float *pfCurrentInA);
u8 ucISL68221ReadCurrent1(u8 ucBusNum, u8 ucSlaveAddr, float *pfCurrentInA);
u8 ucISL68221ReadCurrent2(u8 ucBusNum, u8 ucSlaveAddr, float *pfCurrentInA);
u8 ucISL68221ReadTemperature0(u8 ucBusNum, u8 ucSlaveAddr, float *pfTemperature);
u8 ucISL68221ReadTemperature1(u8 ucBusNum, u8 ucSlaveAddr, float *pfTemperature);
u8 ucISL68221ReadTemperature2(u8 ucBusNum, u8 ucSlaveAddr, float *pfTemperature);










#endif
