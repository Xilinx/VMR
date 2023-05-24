/******************************************************************************
* Copyright (C) 2023 AMD, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#include "xstatus.h"

u8 __wrap_ucQSFPI2CMuxReadTemperature( float *pfTemperatureValue, u8 ucSensorInstance )
{
    u8 ucStatus         = XST_SUCCESS;

    *pfTemperatureValue = 32+ucSensorInstance;

    return ucStatus;
}
