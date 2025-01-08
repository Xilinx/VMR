
/******************************************************************************
* Copyright (C) 2024 Advanced Micro Devices, Inc.    All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <cmocka.h>

#include "cl_i2c.h"
#include "lm75.h"

/*****************************Mock functions *******************************/
u8 __wrap_LM75_ReadTemperature(u8 i2c_num, u8 slaveAddr, s16 *temperatureValue)
{
	  u8 status = 0;
	  *temperatureValue = 70;

	  return mock_type(u8);
}

