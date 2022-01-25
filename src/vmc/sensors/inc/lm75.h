/******************************************************************************
 *  * * Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
 *   * * SPDX-License-Identifier: MIT
 *    * *******************************************************************************/

#ifndef LM75_H_
#define LM75_H_

#define TEMP_LM75_0x48	(0x48)
#define TEMP_LM75_0x4A	(0x4A)

u8 LM75_ReadTemperature(u8 i2c_num, u8 slaveAddr, s32 *temperatureValue);

#endif
