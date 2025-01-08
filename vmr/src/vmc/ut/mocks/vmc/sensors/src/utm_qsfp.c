/******************************************************************************
* Copyright (C) 2023 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <cmocka.h>
#include "xstatus.h"

static float fQSFPTemp = 80;

void vQSFPSetNewValue(float fNewValue)
{
	fQSFPTemp = fNewValue;
}

u8 __wrap_ucQSFPI2CMuxReadTemperature( float *pfTemperatureValue, u8 ucSensorInstance )
{
    u8 ucStatus         = XST_SUCCESS;

    *pfTemperatureValue = ( 120 < fQSFPTemp  ) ? fQSFPTemp :32+ucSensorInstance;

    return mock_type(u8);
}
