/******************************************************************************
* Copyright (C) 2024 Advanced Micro Devices, Inc.    All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/


#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <cmocka.h>

#include "cl_vmc.h"
#include "cl_mem.h"
#include "cl_uart_rtos.h"
#include "cl_i2c.h"
#include "cl_log.h"
#include "cl_msg.h"
#include "vmc_api.h"
#include "sensors/inc/se98a.h"
#include "sensors/inc/max6639.h"
#include "sensors/inc/qsfp.h"
#include "vmc_sensors.h"
#include "vmc_asdm.h"
#include "xsysmonpsv.h"
#include "vmc_sc_comms.h"
#include "vmc_update_sc.h"
#include "vmr_common.h"
#include "vmc_main.h"
#include "clock_throttling.h"

SemaphoreHandle_t vmc_sc_lock = NULL;

void __wrap_vV80GetSupportedSdrInfo( u32 *pulPlatformSupportedSensors, u32 *pulSdrCount )
{
    return;
}

s8 __wrap_scV80AsdmTemperatureReadBoard( snsrRead_t *pxSnsrData )
{
    return 0;
}

s8 __wrap_scV80AsdmTemperatureReadQSFP( snsrRead_t *pxSnsrData )
{
    return 0;
}

s8 __wrap_scV80AsdmTemperatureReadVccint( snsrRead_t *pxSnsrData )
{
    return 0;
}

s8 __wrap_scV80AsdmReadPower( snsrRead_t *pxSnsrData )
{
    return 0;
}

s8 __wrap_scV80AsdmGetVoltageNames( u8 ucIndex, char8* pucSnsrName, u8 *pucSensorId, sensorMonitorFunc *pxSensorHandler )
{
    return 0;
}

s8 __wrap_scV80AsdmGetCurrentNames( u8 ucIndex, char8* pucSnsrName, u8 *pucSensorId, sensorMonitorFunc *pxSensorHandler )
{
    return 0;
}

s8 __wrap_scV80AsdmGetTemperatureNames( u8 ucIndex, char8* pucSnsrName, u8 *pucSensorId, sensorMonitorFunc *pxSensorHandler )
{
    return 0;
}

s8 __wrap_scV80AsdmGetQSFPName( u8 ucIndex, char8* pucSnsrName, u8 *pucSensorId, sensorMonitorFunc *pxSensorHandler )
{
    return 0;
}

