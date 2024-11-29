
/******************************************************************************
 * * Copyright (C) 2024 AMD, Inc.    All rights reserved.
 * * SPDX-License-Identifier: MIT
 * *******************************************************************************/

#ifndef INCLUDE_INA3221_H
#define INCLUDE_INA3221_H

#include "xil_types.h"

#define INA3221_CH1_SHUNT_VOLTAGE               0x01 
#define INA3221_CH1_BUS_VOLTAGE                 0x02 

#define INA3221_CH2_SHUNT_VOLTAGE               0x03
#define INA3221_CH2_BUS_VOLTAGE                 0x04

#define INA3221_CH3_SHUNT_VOLTAGE               0x05
#define INA3221_CH3_BUS_VOLTAGE                 0x06


u8 INA3221_ReadVoltage(u8 busnum, u8 slaveAddr, u8 channelNum, float *voltageInmV);
u8 INA3221_ReadCurrent(u8 busnum, u8 slaveAddr, u8 channelNum, float *currentInmA);
u8 INA3221_ReadPower(u8 busnum, u8 slaveAddr, u8 channelNum, float *powerInmW);

#endif
