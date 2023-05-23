/******************************************************************************
* Copyright (C) 2023 AMD, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#include "xstatus.h"
#include "cl_i2c.h"
#include "cl_log.h"
#include "../inc/qsfp.h"
#include "../../vmc_api.h"

/* FreeRTOS includes */
#include <FreeRTOS.h>
#include <task.h>


u8 __wrap_ucQSFPI2CMuxReadTemperature( float *pfTemperatureValue, u8 ucSensorInstance )
{
    u8 ucStatus                         = XST_SUCCESS;

    *pfTemperatureValue = 32+ucSensorInstance;

    return ucStatus;
}
