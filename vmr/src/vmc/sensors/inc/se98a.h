/******************************************************************************
* Copyright (C) 2024 AMD, Inc.    All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#ifndef INCLUDE_SE98A_H
#define INCLUDE_SE98A_H

#include "xil_types.h"

u8 SE98A_ReadTemperature(u8 i2c_num, u8 slaveAddr, s16 *temperatureValue);

#endif
