/******************************************************************************
* Copyright (C) 2024 Advanced Micro Devices, Inc.    All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <cmocka.h>
#include "xstatus.h"
#include "cl_i2c.h"
#include "cl_log.h"
#include "v80.h"
#include "vmc_api.h"
#include "vmc_main.h"

/* FreeRTOS includes */
#include <FreeRTOS.h>
#include <task.h>

extern void vQSFPSetNewValue(float fNewValue);
extern void vINA3221SetNewValue(float fNewValue);
extern Vmc_Sensors_Gl_t sensor_glvr;

static void test_scV80AsdmGetQSFPName( void **state ) 
{
    ( void ) state; /* unused */
    u8 ucStatus                         = 0;
    u8 ucIndex                          = 0;
    char8 pucSnsrName[30]               = { 0 };
    u8 ucSensorId                       = 0;
    sensorMonitorFunc xSensorHandler    = NULL;

    /* Test 1 ucSensorId 0 parameter */
    ucIndex = 0;
    ucStatus = scV80AsdmGetQSFPName(  ucIndex, pucSnsrName, &ucSensorId, &xSensorHandler );
    assert_true(  ucStatus == XST_SUCCESS );

    /* Test 2 ucSensorId 1 parameter */ 
    ucIndex = 1;
    ucStatus = scV80AsdmGetQSFPName(  ucIndex, pucSnsrName, &ucSensorId, &xSensorHandler );
    assert_true(  ucStatus == XST_SUCCESS );

    /* Test 3 ucSensorId 2 parameter */ 
    ucIndex = 2;
    ucStatus = scV80AsdmGetQSFPName(  ucIndex, pucSnsrName, &ucSensorId, &xSensorHandler );
    assert_true(  ucStatus == XST_SUCCESS );

    /* Test 4 ucSensorId 3 parameter */ 
    ucIndex = 3;
    ucStatus = scV80AsdmGetQSFPName(  ucIndex, pucSnsrName, &ucSensorId, &xSensorHandler );
    assert_true(  ucStatus == XST_SUCCESS );
}

static void test_scV80AsdmGetQSFPNameFail( void **state ) 
{
    ( void ) state; /* unused */
    u8 ucStatus                         = 0;
    u8 ucIndex                          = 0;
    char8 pucSnsrName[30]               = { 0 };
    u8 ucSensorId                       = 0;
    sensorMonitorFunc xSensorHandler    = NULL;

    /* Test 1 NULL parameter */
    ucStatus = scV80AsdmGetQSFPName(  ucIndex, NULL, &ucSensorId, &xSensorHandler );
    assert_true(  ucStatus == XST_FAILURE );

    /* Test 2 NULL parameter */
    ucStatus = scV80AsdmGetQSFPName(  ucIndex, pucSnsrName, NULL, &xSensorHandler );
    assert_true(  ucStatus == XST_FAILURE );

    /* Test 3 NULL parameter */
    ucStatus = scV80AsdmGetQSFPName(  ucIndex, pucSnsrName, &ucSensorId, NULL );
    assert_true(  ucStatus == XST_FAILURE );

    /* Test 4 ucIndex to large */ 
    ucIndex = 4;
    ucStatus = scV80AsdmGetQSFPName(  ucIndex, pucSnsrName, &ucSensorId, &xSensorHandler );
    assert_true(  ucStatus == XST_FAILURE );
}

static void test_vV80GetSupportedSdrInfo( void **state ) 
{
    ( void ) state; /* unused */
    u32 pulPlatformSupportedSensors[30] = { 0 };
    u32 ulSdrCount                      = 0;

    /* Test 1  */
    vV80GetSupportedSdrInfo( pulPlatformSupportedSensors, &ulSdrCount );
    assert_true(  ulSdrCount == 18 );

}

static void test_vV80GetSupportedSdrInfoFail( void **state ) 
{
    ( void ) state; /* unused */
    u32 pulPlatformSupportedSensors[30] = { 0 };
    u32 ulSdrCount                      = 0;

    /* Test 1 NULL parameter */
    vV80GetSupportedSdrInfo(  NULL, &ulSdrCount );
    assert_true(  ulSdrCount == 0 );

    /* Test 2 NULL parameter */
    vV80GetSupportedSdrInfo(  pulPlatformSupportedSensors, NULL );
    assert_true(  ulSdrCount == 0 );
}

static void test_vV80AsdmUpdateRecordCount( void **state ) 
{
    ( void ) state; /* unused */
    Asdm_Header_t xHeaderInfo[5] = { 0 };

    /* Test 1  */
    xHeaderInfo[0].repository_type = 0xC0;
    xHeaderInfo[1].repository_type = 0xC1;
    xHeaderInfo[2].repository_type = 0xC2;
    xHeaderInfo[3].repository_type = 0xC3;
    xHeaderInfo[4].repository_type = 0xC4;

    vV80AsdmUpdateRecordCount( &xHeaderInfo );

    assert_true(  xHeaderInfo[0].no_of_records == 11 );
    assert_true(  xHeaderInfo[1].no_of_records == 9 );
    assert_true(  xHeaderInfo[2].no_of_records == 10 );
    assert_true(  xHeaderInfo[3].no_of_records == 10 );
    assert_true(  xHeaderInfo[4].no_of_records == 1 );
}

static void test_vV80AsdmUpdateRecordCountFail( void **state ) 
{
    ( void ) state; /* unused */
    Asdm_Header_t xHeaderInfo[5] = { 0 };

    xHeaderInfo[0].repository_type = 0xC0;
    xHeaderInfo[1].repository_type = 0xC1;
    xHeaderInfo[2].repository_type = 0xC2;
    xHeaderInfo[3].repository_type = 0xC3;
    xHeaderInfo[4].repository_type = 0xC4;

    /* Test 1 NULL parameter */
    vV80AsdmUpdateRecordCount(  NULL );


    assert_true(  xHeaderInfo[0].no_of_records == 0 );
    assert_true(  xHeaderInfo[1].no_of_records == 0 );
    assert_true(  xHeaderInfo[2].no_of_records == 0 );
    assert_true(  xHeaderInfo[3].no_of_records == 0 );
    assert_true(  xHeaderInfo[4].no_of_records == 0 );
    
}

static void test_scV80AsdmTemperatureReadInlet( void **state ) 
{
    ( void ) state; /* unused */
    u8 ucStatus             = 0;
    snsrRead_t xSnsrData    = {0};

    will_return( __wrap_LM75_ReadTemperature, XST_SUCCESS );

    ucStatus = scV80AsdmTemperatureReadInlet( &xSnsrData );
    
    assert_true(  ucStatus == XST_SUCCESS );
    assert_true(  xSnsrData.sensorInstance  == 0 );
    assert_true(  xSnsrData.sensorValueSize == 2 );
    assert_true(  xSnsrData.snsrSatus       == 1 );
    assert_true(  xSnsrData.mspSensorIndex  == 0 );
    assert_true(  xSnsrData.snsrValue[0]    == 70 );
    assert_true(  xSnsrData.snsrValue[1]    == 0 );
    assert_true(  xSnsrData.snsrValue[2]    == 0 );
    assert_true(  xSnsrData.snsrValue[3]    == 0 );

}

static void test_scV80AsdmTemperatureReadInletFail( void **state ) 
{
    ( void ) state; /* unused */
    u8 ucStatus             = 0;
    snsrRead_t xSnsrData   = {0};
    
    /* Test 1 */
    ucStatus = scV80AsdmTemperatureReadInlet( NULL );

    assert_true(  ucStatus == XST_FAILURE );
    assert_true(  xSnsrData.sensorInstance  == 0 );
    assert_true(  xSnsrData.sensorValueSize == 0 );
    assert_true(  xSnsrData.snsrSatus       == 0 );
    assert_true(  xSnsrData.mspSensorIndex  == 0 );
    assert_true(  xSnsrData.snsrValue[0]    == 0 );
    assert_true(  xSnsrData.snsrValue[1]    == 0 );
    assert_true(  xSnsrData.snsrValue[2]    == 0 );
    assert_true(  xSnsrData.snsrValue[3]    == 0 );

    /* Test 2 */
    will_return( __wrap_LM75_ReadTemperature, XST_FAILURE );
    
    ucStatus = scV80AsdmTemperatureReadInlet( &xSnsrData );
     
    assert_true(  ucStatus == XST_FAILURE );
    assert_true(  xSnsrData.sensorInstance  == 0 );
    assert_true(  xSnsrData.sensorValueSize == 0 );
    assert_true(  xSnsrData.snsrSatus       == Vmc_Snsr_State_Comms_failure );
    assert_true(  xSnsrData.mspSensorIndex  == 0 );
    assert_true(  xSnsrData.snsrValue[0]    == 0 );
    assert_true(  xSnsrData.snsrValue[1]    == 0 );
    assert_true(  xSnsrData.snsrValue[2]    == 0 );
    assert_true(  xSnsrData.snsrValue[3]    == 0 );
}

static void test_scV80AsdmTemperatureReadOutlet( void **state ) 
{
    ( void ) state; /* unused */
    u8 ucStatus             = 0;
    snsrRead_t xSnsrData    = {0};

    will_return( __wrap_ucCAT34TS02ReadTemperature, XST_SUCCESS );
    ucStatus = scV80AsdmTemperatureReadOutlet( &xSnsrData );

    assert_true(  ucStatus == XST_SUCCESS );
    assert_true(  xSnsrData.sensorInstance  == 0 );
    assert_true(  xSnsrData.sensorValueSize == 2 );
    assert_true(  xSnsrData.snsrSatus       == 1 );
    assert_true(  xSnsrData.mspSensorIndex  == 0 );
    assert_true(  xSnsrData.snsrValue[0]    == 37 );
    assert_true(  xSnsrData.snsrValue[1]    == 0 );
    assert_true(  xSnsrData.snsrValue[2]    == 0 );
    assert_true(  xSnsrData.snsrValue[3]    == 0 );

}

static void test_scV80AsdmTemperatureReadOutletFail( void **state ) 
{
    ( void ) state; /* unused */
    u8 ucStatus             = 0;
    snsrRead_t xSnsrData   = {0};
    
    /* Test 1 */
    ucStatus = scV80AsdmTemperatureReadOutlet( NULL );

    assert_true(  ucStatus == XST_FAILURE );
    assert_true(  xSnsrData.sensorInstance  == 0 );
    assert_true(  xSnsrData.sensorValueSize == 0 );
    assert_true(  xSnsrData.snsrSatus       == 0 );
    assert_true(  xSnsrData.mspSensorIndex  == 0 );
    assert_true(  xSnsrData.snsrValue[0]    == 0 );
    assert_true(  xSnsrData.snsrValue[1]    == 0 );
    assert_true(  xSnsrData.snsrValue[2]    == 0 );
    assert_true(  xSnsrData.snsrValue[3]    == 0 );

    /* Test 2 */
    will_return( __wrap_ucCAT34TS02ReadTemperature, XST_FAILURE );
    ucStatus = scV80AsdmTemperatureReadOutlet( &xSnsrData );

    assert_true(  ucStatus == XST_FAILURE );
    assert_true(  xSnsrData.sensorInstance  == 0 );
    assert_true(  xSnsrData.sensorValueSize == 0 );
    assert_true(  xSnsrData.snsrSatus       == Vmc_Snsr_State_Comms_failure );
    assert_true(  xSnsrData.mspSensorIndex  == 0 );
    assert_true(  xSnsrData.snsrValue[0]    == 0 );
    assert_true(  xSnsrData.snsrValue[1]    == 0 );
    assert_true(  xSnsrData.snsrValue[2]    == 0 );
    assert_true(  xSnsrData.snsrValue[3]    == 0 );
}

static void test_scV80AsdmTemperatureReadBoard( void **state ) 
{
    ( void ) state; /* unused */
    u8 ucStatus             = 0;
    snsrRead_t xSnsrData    = {0};

    will_return( __wrap_ucCAT34TS02ReadTemperature, XST_SUCCESS );
    will_return( __wrap_LM75_ReadTemperature, XST_SUCCESS );
    will_return( __wrap_ucISL68221ReadTemperature0, XST_SUCCESS );
    will_return( __wrap_ucISL68221ReadTemperature0, XST_SUCCESS );
    will_return( __wrap_ucISL68221ReadTemperature1, XST_SUCCESS );
    will_return( __wrap_ucISL68221ReadTemperature2, XST_SUCCESS );
    vV80TemperatureMonitor();
    ucStatus = scV80AsdmTemperatureReadBoard( &xSnsrData );

    assert_true(  ucStatus == XST_SUCCESS );
    assert_true(  xSnsrData.sensorInstance  == 0 );
    assert_true(  xSnsrData.sensorValueSize == 2 );
    assert_true(  xSnsrData.snsrSatus       == 1 );
    assert_true(  xSnsrData.mspSensorIndex  == 0 );
    assert_true(  xSnsrData.snsrValue[0]    == 70 );
    assert_true(  xSnsrData.snsrValue[1]    == 0 );
    assert_true(  xSnsrData.snsrValue[2]    == 0 );
    assert_true(  xSnsrData.snsrValue[3]    == 0 );

}

static void test_scV80AsdmTemperatureReadBoardFail( void **state ) 
{
    ( void ) state; /* unused */
    u8 ucStatus             = 0;
    snsrRead_t xSnsrData    = {0};

    will_return( __wrap_ucCAT34TS02ReadTemperature, XST_SUCCESS );
    will_return( __wrap_LM75_ReadTemperature, XST_SUCCESS );
    will_return( __wrap_ucISL68221ReadTemperature0, XST_SUCCESS );
    will_return( __wrap_ucISL68221ReadTemperature0, XST_SUCCESS );
    will_return( __wrap_ucISL68221ReadTemperature1, XST_SUCCESS );
    will_return( __wrap_ucISL68221ReadTemperature2, XST_SUCCESS );
    vV80TemperatureMonitor();
    ucStatus = scV80AsdmTemperatureReadBoard( NULL );
    
    assert_true(  ucStatus == XST_FAILURE );
    assert_true(  xSnsrData.sensorInstance  == 0 );
    assert_true(  xSnsrData.sensorValueSize == 0 );
    assert_true(  xSnsrData.snsrSatus       == 0 );
    assert_true(  xSnsrData.mspSensorIndex  == 0 );
    assert_true(  xSnsrData.snsrValue[0]    == 0 );
    assert_true(  xSnsrData.snsrValue[1]    == 0 );
    assert_true(  xSnsrData.snsrValue[2]    == 0 );
    assert_true(  xSnsrData.snsrValue[3]    == 0 );
}

static void test_scV80TemperatureReadVCCHBM( void **state ) 
{
    ( void ) state; /* unused */
    u8 ucStatus             = 0;
    snsrRead_t xSnsrData    = {0};

    will_return( __wrap_ucISL68221ReadTemperature0, XST_SUCCESS );
    ucStatus = scV80TemperatureReadVCCHBM( &xSnsrData );

    assert_true(  ucStatus == XST_SUCCESS );
    assert_true(  xSnsrData.sensorInstance  == 0 );
    assert_true(  xSnsrData.sensorValueSize == 2 );
    assert_true(  xSnsrData.snsrSatus       == 1 );
    assert_true(  xSnsrData.mspSensorIndex  == 0 );
    assert_true(  xSnsrData.snsrValue[0]    == 60 );
    assert_true(  xSnsrData.snsrValue[1]    == 0 );
    assert_true(  xSnsrData.snsrValue[2]    == 0 );
    assert_true(  xSnsrData.snsrValue[3]    == 0 );

}

static void test_scV80TemperatureReadVCCHBMFail( void **state ) 
{
    ( void ) state; /* unused */
    u8 ucStatus             = 0;
    snsrRead_t xSnsrData   = {0};

    /* Test 1 NULL parameter */
    ucStatus = scV80TemperatureReadVCCHBM( NULL );
        
    assert_true(  ucStatus == XST_FAILURE );
    assert_true(  xSnsrData.sensorInstance  == 0 );
    assert_true(  xSnsrData.sensorValueSize == 0 );
    assert_true(  xSnsrData.snsrSatus       == 0 );
    assert_true(  xSnsrData.mspSensorIndex  == 0 );
    assert_true(  xSnsrData.snsrValue[0]    == 0 );
    assert_true(  xSnsrData.snsrValue[1]    == 0 );
    assert_true(  xSnsrData.snsrValue[2]    == 0 );
    assert_true(  xSnsrData.snsrValue[3]    == 0 );

    /* Test 2 failed read */
    will_return( __wrap_ucISL68221ReadTemperature0, XST_FAILURE );
    ucStatus = scV80TemperatureReadVCCHBM( &xSnsrData );
    
    assert_true(  ucStatus == XST_FAILURE );
    assert_true(  xSnsrData.sensorInstance  == 0 );
    assert_true(  xSnsrData.sensorValueSize == 0 );
    assert_true(  xSnsrData.snsrSatus       == Vmc_Snsr_State_Comms_failure );
    assert_true(  xSnsrData.mspSensorIndex  == 0 );
    assert_true(  xSnsrData.snsrValue[0]    == 0 );
    assert_true(  xSnsrData.snsrValue[1]    == 0 );
    assert_true(  xSnsrData.snsrValue[2]    == 0 );
    assert_true(  xSnsrData.snsrValue[3]    == 0 );
}

static void test_scV80TemperatureReadVCCODIMM( void **state ) 
{
    ( void ) state; /* unused */
    u8 ucStatus             = 0;
    snsrRead_t xSnsrData    = {0};

    will_return( __wrap_ucISL68221ReadTemperature1, XST_SUCCESS );
    ucStatus = scV80TemperatureReadVCCODIMM( &xSnsrData );

    assert_true(  ucStatus == XST_SUCCESS );
    assert_true(  xSnsrData.sensorInstance  == 0 );
    assert_true(  xSnsrData.sensorValueSize == 2 );
    assert_true(  xSnsrData.snsrSatus       == 1 );
    assert_true(  xSnsrData.mspSensorIndex  == 0 );
    assert_true(  xSnsrData.snsrValue[0]    == 60 );
    assert_true(  xSnsrData.snsrValue[1]    == 0 );
    assert_true(  xSnsrData.snsrValue[2]    == 0 );
    assert_true(  xSnsrData.snsrValue[3]    == 0 );

}

static void test_scV80TemperatureReadVCCODIMMFail( void **state ) 
{
    ( void ) state; /* unused */
    u8 ucStatus             = 0;
    snsrRead_t xSnsrData   = {0};

    /* Test 1 NULL parameter */
    ucStatus = scV80TemperatureReadVCCODIMM( NULL );
    
    assert_true(  ucStatus == XST_FAILURE );
    assert_true(  xSnsrData.sensorInstance  == 0 );
    assert_true(  xSnsrData.sensorValueSize == 0 );
    assert_true(  xSnsrData.snsrSatus       == 0 );
    assert_true(  xSnsrData.mspSensorIndex  == 0 );
    assert_true(  xSnsrData.snsrValue[0]    == 0 );
    assert_true(  xSnsrData.snsrValue[1]    == 0 );
    assert_true(  xSnsrData.snsrValue[2]    == 0 );
    assert_true(  xSnsrData.snsrValue[3]    == 0 );

    /* Test 2 failed read */
    will_return( __wrap_ucISL68221ReadTemperature1, XST_FAILURE );
    ucStatus = scV80TemperatureReadVCCODIMM( &xSnsrData );
    
    assert_true(  ucStatus == XST_FAILURE );
    assert_true(  xSnsrData.sensorInstance  == 0 );
    assert_true(  xSnsrData.sensorValueSize == 0 );
    assert_true(  xSnsrData.snsrSatus       == Vmc_Snsr_State_Comms_failure );
    assert_true(  xSnsrData.mspSensorIndex  == 0 );
    assert_true(  xSnsrData.snsrValue[0]    == 0 );
    assert_true(  xSnsrData.snsrValue[1]    == 0 );
    assert_true(  xSnsrData.snsrValue[2]    == 0 );
    assert_true(  xSnsrData.snsrValue[3]    == 0 );
}

static void test_scV80TemperatureReadGTXAVTT( void **state ) 
{
    ( void ) state; /* unused */
    u8 ucStatus             = 0;
    snsrRead_t xSnsrData    = {0};

    will_return( __wrap_ucISL68221ReadTemperature2, XST_SUCCESS );
    ucStatus = scV80TemperatureReadGTXAVTT( &xSnsrData );

    assert_true(  ucStatus == XST_SUCCESS );
    assert_true(  xSnsrData.sensorInstance  == 0 );
    assert_true(  xSnsrData.sensorValueSize == 2 );
    assert_true(  xSnsrData.snsrSatus       == 1 );
    assert_true(  xSnsrData.mspSensorIndex  == 0 );
    assert_true(  xSnsrData.snsrValue[0]    == 60 );
    assert_true(  xSnsrData.snsrValue[1]    == 0 );
    assert_true(  xSnsrData.snsrValue[2]    == 0 );
    assert_true(  xSnsrData.snsrValue[3]    == 0 );
}

static void test_scV80TemperatureReadGTXAVTTFail( void **state ) 
{
    ( void ) state; /* unused */
    u8 ucStatus             = 0;
    snsrRead_t xSnsrData   = {0};

    /* Test 1 NULL parameter */
    ucStatus = scV80TemperatureReadGTXAVTT( NULL );
    
    assert_true(  ucStatus == XST_FAILURE );
    assert_true(  xSnsrData.sensorInstance  == 0 );
    assert_true(  xSnsrData.sensorValueSize == 0 );
    assert_true(  xSnsrData.snsrSatus       == 0 );
    assert_true(  xSnsrData.mspSensorIndex  == 0 );
    assert_true(  xSnsrData.snsrValue[0]    == 0 );
    assert_true(  xSnsrData.snsrValue[1]    == 0 );
    assert_true(  xSnsrData.snsrValue[2]    == 0 );
    assert_true(  xSnsrData.snsrValue[3]    == 0 );

    /* Test 2 failed read */
    will_return( __wrap_ucISL68221ReadTemperature2, XST_FAILURE );
    ucStatus = scV80TemperatureReadGTXAVTT( &xSnsrData );
    
    assert_true(  ucStatus == XST_FAILURE );
    assert_true(  xSnsrData.sensorInstance  == 0 );
    assert_true(  xSnsrData.sensorValueSize == 0 );
    assert_true(  xSnsrData.snsrSatus       == Vmc_Snsr_State_Comms_failure );
    assert_true(  xSnsrData.mspSensorIndex  == 0 );
    assert_true(  xSnsrData.snsrValue[0]    == 0 );
    assert_true(  xSnsrData.snsrValue[1]    == 0 );
    assert_true(  xSnsrData.snsrValue[2]    == 0 );
    assert_true(  xSnsrData.snsrValue[3]    == 0 );
}

static void test_scV80AsdmTemperatureReadQSFP( void **state ) 
{
    ( void ) state; /* unused */
    u8 ucStatus                 = 0;
    snsrRead_t xSnsrData        = {0};
    xSnsrData.sensorInstance    = 0;

    will_return( __wrap_ucQSFPI2CMuxReadTemperature, XST_SUCCESS );
    ucStatus = scV80AsdmTemperatureReadQSFP( &xSnsrData );

    assert_true(  ucStatus == XST_SUCCESS );
    assert_true(  xSnsrData.sensorInstance  == 0 );
    assert_true(  xSnsrData.sensorValueSize == 2 );
    assert_true(  xSnsrData.snsrSatus       == 1 );
    assert_true(  xSnsrData.mspSensorIndex  == 0 );
    assert_true(  xSnsrData.snsrValue[0]    == 32 );
    assert_true(  xSnsrData.snsrValue[1]    == 0 );
    assert_true(  xSnsrData.snsrValue[2]    == 0 );
    assert_true(  xSnsrData.snsrValue[3]    == 0 );
}

static void test_scV80AsdmTemperatureReadQSFPFail( void **state ) 
{
    ( void ) state; /* unused */
    u8 ucStatus             = 0;
    snsrRead_t xSnsrData   = {0};

    /* Test 1 NULL parameter */
    ucStatus = scV80AsdmTemperatureReadQSFP( NULL );
    
    assert_true(  ucStatus == XST_FAILURE );
    assert_true(  xSnsrData.sensorInstance  == 0 );
    assert_true(  xSnsrData.sensorValueSize == 0 );
    assert_true(  xSnsrData.snsrSatus       == 0 );
    assert_true(  xSnsrData.mspSensorIndex  == 0 );
    assert_true(  xSnsrData.snsrValue[0]    == 0 );
    assert_true(  xSnsrData.snsrValue[1]    == 0 );
    assert_true(  xSnsrData.snsrValue[2]    == 0 );
    assert_true(  xSnsrData.snsrValue[3]    == 0 );

    /* Test 2 failed read */
    will_return( __wrap_ucQSFPI2CMuxReadTemperature, XST_FAILURE );
    ucStatus = scV80AsdmTemperatureReadQSFP( &xSnsrData );
    
    assert_true(  ucStatus == XST_FAILURE );
    assert_true(  xSnsrData.sensorInstance  == 0 );
    assert_true(  xSnsrData.sensorValueSize == 0 );
    assert_true(  xSnsrData.snsrSatus       == Vmc_Snsr_State_Comms_failure );
    assert_true(  xSnsrData.mspSensorIndex  == 0 );
    assert_true(  xSnsrData.snsrValue[0]    == 0 );
    assert_true(  xSnsrData.snsrValue[1]    == 0 );
    assert_true(  xSnsrData.snsrValue[2]    == 0 );
    assert_true(  xSnsrData.snsrValue[3]    == 0 );

    /* Test 3 No QSFP present */
    will_return( __wrap_ucQSFPI2CMuxReadTemperature, XST_DEVICE_NOT_FOUND );
    ucStatus = scV80AsdmTemperatureReadQSFP( &xSnsrData );
    
    assert_true(  ucStatus == XST_DEVICE_NOT_FOUND );
    assert_true(  xSnsrData.sensorInstance  == 0 );
    assert_true(  xSnsrData.sensorValueSize == 0 );
    assert_true(  xSnsrData.snsrSatus       == Vmc_Snsr_State_Unavailable );
    assert_true(  xSnsrData.mspSensorIndex  == 0 );
    assert_true(  xSnsrData.snsrValue[0]    == 0 );
    assert_true(  xSnsrData.snsrValue[1]    == 0 );
    assert_true(  xSnsrData.snsrValue[2]    == 0 );
    assert_true(  xSnsrData.snsrValue[3]    == 0 );
}

static void test_scV80AsdmTemperatureReadQSFPThreshold( void **state ) 
{
    ( void ) state; /* unused */
    u8 ucStatus             = 0;
    snsrRead_t xSnsrData   = {0};

    /* Test 1 Threshold Cross */
    vQSFPSetNewValue(130);
    will_return( __wrap_ucQSFPI2CMuxReadTemperature, XST_SUCCESS );
    ucStatus = scV80AsdmTemperatureReadQSFP( &xSnsrData );
    
    assert_true(  ucStatus == XST_SUCCESS );
    assert_true(  xSnsrData.sensorInstance  == 0 );
    assert_true(  xSnsrData.sensorValueSize == 2 );
    assert_true(  xSnsrData.snsrSatus       == 1 );
    assert_true(  xSnsrData.mspSensorIndex  == 0 );
    assert_true(  xSnsrData.snsrValue[0]    == 130 );
    assert_true(  xSnsrData.snsrValue[1]    == 0 );
    assert_true(  xSnsrData.snsrValue[2]    == 0 );
    assert_true(  xSnsrData.snsrValue[3]    == 0 );  
}

static void test_vV80TemperatureMonitor( void **state ) 
{
    ( void ) state; /* unused */

    will_return( __wrap_ucCAT34TS02ReadTemperature, XST_SUCCESS );
    will_return( __wrap_LM75_ReadTemperature, XST_SUCCESS );
    will_return( __wrap_ucISL68221ReadTemperature0, XST_SUCCESS );
    will_return( __wrap_ucISL68221ReadTemperature0, XST_SUCCESS );
    will_return( __wrap_ucISL68221ReadTemperature1, XST_SUCCESS );
    will_return( __wrap_ucISL68221ReadTemperature2, XST_SUCCESS );
    vV80TemperatureMonitor( );

    will_return( __wrap_ucCAT34TS02ReadTemperature, XST_FAILURE );
    will_return( __wrap_LM75_ReadTemperature, XST_SUCCESS );
    will_return( __wrap_ucISL68221ReadTemperature0, XST_SUCCESS );
    will_return( __wrap_ucISL68221ReadTemperature0, XST_SUCCESS );
    will_return( __wrap_ucISL68221ReadTemperature1, XST_SUCCESS );
    will_return( __wrap_ucISL68221ReadTemperature2, XST_SUCCESS );
    vV80TemperatureMonitor( );

    will_return( __wrap_ucCAT34TS02ReadTemperature, XST_SUCCESS );
    will_return( __wrap_LM75_ReadTemperature, XST_FAILURE );
    will_return( __wrap_ucISL68221ReadTemperature0, XST_SUCCESS );
    will_return( __wrap_ucISL68221ReadTemperature0, XST_SUCCESS );
    will_return( __wrap_ucISL68221ReadTemperature1, XST_SUCCESS );
    will_return( __wrap_ucISL68221ReadTemperature2, XST_SUCCESS );
    vV80TemperatureMonitor( );

    will_return( __wrap_ucCAT34TS02ReadTemperature, XST_SUCCESS );
    will_return( __wrap_LM75_ReadTemperature, XST_SUCCESS );
    will_return( __wrap_ucISL68221ReadTemperature0, XST_FAILURE );
    will_return( __wrap_ucISL68221ReadTemperature0, XST_SUCCESS );
    will_return( __wrap_ucISL68221ReadTemperature1, XST_SUCCESS );
    will_return( __wrap_ucISL68221ReadTemperature2, XST_SUCCESS );
    vV80TemperatureMonitor( );

    will_return( __wrap_ucCAT34TS02ReadTemperature, XST_SUCCESS );
    will_return( __wrap_LM75_ReadTemperature, XST_SUCCESS );
    will_return( __wrap_ucISL68221ReadTemperature0, XST_SUCCESS );
    will_return( __wrap_ucISL68221ReadTemperature0, XST_FAILURE );
    will_return( __wrap_ucISL68221ReadTemperature1, XST_SUCCESS );
    will_return( __wrap_ucISL68221ReadTemperature2, XST_SUCCESS );
    vV80TemperatureMonitor( );

    will_return( __wrap_ucCAT34TS02ReadTemperature, XST_SUCCESS );
    will_return( __wrap_LM75_ReadTemperature, XST_SUCCESS );
    will_return( __wrap_ucISL68221ReadTemperature0, XST_SUCCESS );
    will_return( __wrap_ucISL68221ReadTemperature0, XST_SUCCESS );
    will_return( __wrap_ucISL68221ReadTemperature1, XST_FAILURE );
    will_return( __wrap_ucISL68221ReadTemperature2, XST_SUCCESS );
    vV80TemperatureMonitor( );

    will_return( __wrap_ucCAT34TS02ReadTemperature, XST_SUCCESS );
    will_return( __wrap_LM75_ReadTemperature, XST_SUCCESS );
    will_return( __wrap_ucISL68221ReadTemperature0, XST_SUCCESS );
    will_return( __wrap_ucISL68221ReadTemperature0, XST_SUCCESS );
    will_return( __wrap_ucISL68221ReadTemperature1, XST_SUCCESS );
    will_return( __wrap_ucISL68221ReadTemperature2, XST_FAILURE );
    vV80TemperatureMonitor( );
}

static void test_vV80QSFPMonitor( void **state ) 
{
    ( void ) state; /* unused */

    will_return( __wrap_ucQSFPI2CMuxReadTemperature, XST_SUCCESS );
    will_return( __wrap_ucQSFPI2CMuxReadTemperature, XST_SUCCESS );
    will_return( __wrap_ucQSFPI2CMuxReadTemperature, XST_SUCCESS );
    will_return( __wrap_ucQSFPI2CMuxReadTemperature, XST_SUCCESS );

    vV80QSFPMonitor( );

    will_return( __wrap_ucQSFPI2CMuxReadTemperature, XST_FAILURE );
    will_return( __wrap_ucQSFPI2CMuxReadTemperature, XST_SUCCESS );
    will_return( __wrap_ucQSFPI2CMuxReadTemperature, XST_SUCCESS );
    will_return( __wrap_ucQSFPI2CMuxReadTemperature, XST_SUCCESS );

    vV80QSFPMonitor( );

    will_return( __wrap_ucQSFPI2CMuxReadTemperature, XST_DEVICE_NOT_FOUND );
    will_return( __wrap_ucQSFPI2CMuxReadTemperature, XST_SUCCESS );
    will_return( __wrap_ucQSFPI2CMuxReadTemperature, XST_SUCCESS );
    will_return( __wrap_ucQSFPI2CMuxReadTemperature, XST_SUCCESS );

    vV80QSFPMonitor( );
}

static void test_slV80VMCFetchBoardInfo( void **state ) 
{
    ( void ) state; /* unused */
    s32 slDataSize      = -1;
    u8 board_snsr_data[255]  = {0};

    slDataSize = slV80VMCFetchBoardInfo( board_snsr_data );

    assert_true(  slDataSize == 161 );
}

static void test_slV80VMCFetchBoardInfoFail( void **state ) 
{
    ( void ) state; /* unused */
    s32 slDataSize      = 0;

    slDataSize = slV80VMCFetchBoardInfo( NULL );

    assert_true(  slDataSize == -1 );
}


static void test_vV80VoltageMonitor12VPEX( void **state ) 
{
    ( void ) state; /* unused */
    sensor_glvr.sensor_readings.voltage[e12V_PEX] = 0.0;
    will_return( __wrap_INA3221_ReadVoltage, XST_SUCCESS );
    vV80VoltageMonitor12VPEX( );
    assert_true(  sensor_glvr.sensor_readings.voltage[e12V_PEX] == 12000 );
}

static void test_vV80VoltageMonitor12VPEXFail( void **state ) 
{
    ( void ) state; /* unused */

    /* Test 1 failed read */
    sensor_glvr.sensor_readings.voltage[e12V_PEX] = 0.0;
    will_return( __wrap_INA3221_ReadVoltage, XST_FAILURE );
    vV80VoltageMonitor12VPEX( );
    assert_true(  sensor_glvr.sensor_readings.voltage[e12V_PEX] == 0 );
}

static void test_vV80CurrentMonitor12VPEX( void **state ) 
{
    ( void ) state; /* unused */

    sensor_glvr.sensor_readings.current[e12V_PEX] = 0.0;
    will_return( __wrap_INA3221_ReadCurrent, XST_SUCCESS );
    vV80CurrentMonitor12VPEX( );
    assert_true(  sensor_glvr.sensor_readings.current[e12V_PEX] == 5000 );
}

static void test_vV80CurrentMonitor12VPEXFail( void **state ) 
{
    ( void ) state; /* unused */

    sensor_glvr.sensor_readings.current[e12V_PEX] = 0.0;
    /* Test 1 failed read */
    will_return( __wrap_INA3221_ReadCurrent, XST_FAILURE );
    vV80CurrentMonitor12VPEX( );
    assert_true(  sensor_glvr.sensor_readings.current[e12V_PEX] == 0 );    
}

static void test_vV80VoltageMonitor3v3PEX( void **state ) 
{
    ( void ) state; /* unused */

    sensor_glvr.sensor_readings.voltage[e3V3_PEX] = 0.0;
    will_return( __wrap_INA3221_ReadVoltage, XST_SUCCESS );
    vV80VoltageMonitor3v3PEX( );
    assert_true(  sensor_glvr.sensor_readings.voltage[e3V3_PEX] == 3000 );
}

static void test_vV80VoltageMonitor3v3PEXFail( void **state ) 
{
    ( void ) state; /* unused */

    /* Test 1 failed read */
    sensor_glvr.sensor_readings.voltage[e3V3_PEX] = 0.0;
    will_return( __wrap_INA3221_ReadVoltage, XST_FAILURE );
    vV80VoltageMonitor3v3PEX( );
    assert_true(  sensor_glvr.sensor_readings.voltage[e3V3_PEX] == 0 );
}

static void test_vV80CurrentMonitor3v3PEX( void **state ) 
{
    ( void ) state; /* unused */

    sensor_glvr.sensor_readings.current[e3V3_PEX] = 0.0;
    will_return( __wrap_INA3221_ReadCurrent, XST_SUCCESS );
    vV80CurrentMonitor3v3PEX( );
    assert_true(  sensor_glvr.sensor_readings.current[e3V3_PEX] == 4000 );
}

static void test_vV80CurrentMonitor3v3PEXFail( void **state ) 
{
    ( void ) state; /* unused */

    /* Test 1 failed read */
    sensor_glvr.sensor_readings.current[e3V3_PEX] = 0.0;
    will_return( __wrap_INA3221_ReadCurrent, XST_FAILURE );
    vV80CurrentMonitor3v3PEX( );
    assert_true(  sensor_glvr.sensor_readings.current[e3V3_PEX] == 0 );    
}

static void test_vV80VoltageMonitor1V5VCCAUX( void **state ) 
{
    ( void ) state; /* unused */

    sensor_glvr.sensor_readings.voltage[e1V5_VCC_AUX] = 0.0;
    will_return( __wrap_INA3221_ReadVoltage, XST_SUCCESS );
    vV80VoltageMonitor1V5VCCAUX( );
    assert_true(  sensor_glvr.sensor_readings.voltage[e1V5_VCC_AUX] == 1561 );
}

static void test_vV80VoltageMonitor1V5VCCAUXFail( void **state ) 
{
    ( void ) state; /* unused */

    /* Test 1 failed read */
    sensor_glvr.sensor_readings.voltage[e1V5_VCC_AUX] = 0.0;
    will_return( __wrap_INA3221_ReadVoltage, XST_FAILURE );
    vV80VoltageMonitor1V5VCCAUX( );
    assert_true(  sensor_glvr.sensor_readings.voltage[e1V5_VCC_AUX] == 0 );
}

static void test_vV80CurrentMonitor1V5VCCAUX( void **state ) 
{
    ( void ) state; /* unused */

    sensor_glvr.sensor_readings.current[e1V5_VCC_AUX] = 0.0;
    will_return( __wrap_INA3221_ReadCurrent, XST_SUCCESS );
    vV80CurrentMonitor1V5VCCAUX( );
    assert_true(  sensor_glvr.sensor_readings.current[e1V5_VCC_AUX] == 710 );
}

static void test_vV80CurrentMonitor1V5VCCAUXFail( void **state ) 
{
    ( void ) state; /* unused */

    /* Test 1 failed read */
    sensor_glvr.sensor_readings.current[e1V5_VCC_AUX] = 0.0;
    will_return( __wrap_INA3221_ReadCurrent, XST_FAILURE );
    vV80CurrentMonitor1V5VCCAUX( );
    assert_true(  sensor_glvr.sensor_readings.current[e1V5_VCC_AUX] == 0 );    
}

static void test_vV80VoltageMonitor3V3QSFP( void **state ) 
{
    ( void ) state; /* unused */

    sensor_glvr.sensor_readings.voltage[e3V3_QSFP] = 0.0;
    will_return( __wrap_INA3221_ReadVoltage, XST_SUCCESS );
    vV80VoltageMonitor3V3QSFP( );
    assert_true(  sensor_glvr.sensor_readings.voltage[e3V3_QSFP] == 3300 );
}

static void test_vV80VoltageMonitor3V3QSFPFail( void **state ) 
{
    ( void ) state; /* unused */

    /* Test 1 failed read */
    sensor_glvr.sensor_readings.voltage[e3V3_QSFP] = 0.0;
    will_return( __wrap_INA3221_ReadVoltage, XST_FAILURE );
    vV80VoltageMonitor3V3QSFP( );
    assert_true(  sensor_glvr.sensor_readings.voltage[e3V3_QSFP] == 0 );
}

static void test_vV80CurrentMonitor3V3QSFP( void **state ) 
{
    ( void ) state; /* unused */

    sensor_glvr.sensor_readings.current[e3V3_QSFP] = 0.0;
    will_return( __wrap_INA3221_ReadCurrent, XST_SUCCESS );
    vV80CurrentMonitor3V3QSFP( );
    assert_true(  sensor_glvr.sensor_readings.current[e3V3_QSFP] == 720 );
}

static void test_vV80CurrentMonitor3V3QSFPFail( void **state ) 
{
    ( void ) state; /* unused */

    /* Test 1 failed read */
    sensor_glvr.sensor_readings.current[e3V3_QSFP] = 0.0;
    will_return( __wrap_INA3221_ReadCurrent, XST_FAILURE );
    vV80CurrentMonitor3V3QSFP( );
    assert_true(  sensor_glvr.sensor_readings.current[e3V3_QSFP] == 0 );    
}

static void test_vV80VoltageMonitor12VAUX0( void **state ) 
{
    ( void ) state; /* unused */

    sensor_glvr.sensor_readings.voltage[e12V_AUX0] = 0.0;
    will_return( __wrap_INA3221_ReadVoltage, XST_SUCCESS );
    vV80VoltageMonitor12VAUX0( );
    assert_true(  sensor_glvr.sensor_readings.voltage[e12V_AUX0] == 12073 );
}

static void test_vV80VoltageMonitor12VAUX0Fail( void **state ) 
{
    ( void ) state; /* unused */

    /* Test 1 failed read */
    sensor_glvr.sensor_readings.voltage[e12V_AUX0] = 0.0;
    will_return( __wrap_INA3221_ReadVoltage, XST_FAILURE );
    vV80VoltageMonitor12VAUX0( );
    assert_true(  sensor_glvr.sensor_readings.voltage[e12V_AUX0] == 0 );
}

static void test_vV80CurrentMonitor12VAUX0( void **state ) 
{
    ( void ) state; /* unused */

    sensor_glvr.sensor_readings.current[e12V_AUX0] = 0.0;
    will_return( __wrap_INA3221_ReadCurrent, XST_SUCCESS );
    vV80CurrentMonitor12VAUX0( );
    assert_true(  sensor_glvr.sensor_readings.current[e12V_AUX0] == 730 );
}

static void test_vV80CurrentMonitor12VAUX0Fail( void **state ) 
{
    ( void ) state; /* unused */

    /* Test 1 failed read */
    sensor_glvr.sensor_readings.current[e12V_AUX0] = 0.0;
    will_return( __wrap_INA3221_ReadCurrent, XST_FAILURE );
    vV80CurrentMonitor12VAUX0( );
    assert_true(  sensor_glvr.sensor_readings.current[e12V_AUX0] == 0 );    
}

static void test_vV80VoltageMonitor12VAUX1( void **state ) 
{
    ( void ) state; /* unused */

    sensor_glvr.sensor_readings.voltage[e12V_AUX1] = 0.0;
    will_return( __wrap_INA3221_ReadVoltage, XST_SUCCESS );
    vV80VoltageMonitor12VAUX1( );
    assert_true(  sensor_glvr.sensor_readings.voltage[e12V_AUX1] == 12074 );
}

static void test_vV80VoltageMonitor12VAUX1Fail( void **state ) 
{
    ( void ) state; /* unused */

    /* Test 1 failed read */
    sensor_glvr.sensor_readings.voltage[e12V_AUX1] = 0.0;
    will_return( __wrap_INA3221_ReadVoltage, XST_FAILURE );
    vV80VoltageMonitor12VAUX1( );
    assert_true(  sensor_glvr.sensor_readings.voltage[e12V_AUX1] == 0 );
}

static void test_vV80CurrentMonitor12VAUX1( void **state ) 
{
    ( void ) state; /* unused */

    sensor_glvr.sensor_readings.current[e12V_AUX1] = 0.0;
    will_return( __wrap_INA3221_ReadCurrent, XST_SUCCESS );
    vV80CurrentMonitor12VAUX1( );
    assert_true(  sensor_glvr.sensor_readings.current[e12V_AUX1] == 740 );
}

static void test_vV80CurrentMonitor12VAUX1Fail( void **state ) 
{
    ( void ) state; /* unused */

    /* Test 1 failed read */
    sensor_glvr.sensor_readings.current[e12V_AUX1] = 0.0;
    will_return( __wrap_INA3221_ReadCurrent, XST_FAILURE );
    vV80CurrentMonitor12VAUX1( );
    assert_true(  sensor_glvr.sensor_readings.current[e12V_AUX1] == 0 );    
}

static void test_vV80VoltageMonitorVccHBM( void **state ) 
{
    ( void ) state; /* unused */

    sensor_glvr.sensor_readings.voltage[eVCC_HBM] = 0.0;
    will_return( __wrap_ucISL68221ReadVoltage0, XST_SUCCESS );
    vV80VoltageMonitorVccHBM( );
    assert_true(  sensor_glvr.sensor_readings.voltage[eVCC_HBM] == 650 );
}

static void test_vV80VoltageMonitorVccHBMFail( void **state ) 
{
    ( void ) state; /* unused */

    /* Test 1 failed read */
    sensor_glvr.sensor_readings.voltage[eVCC_HBM] = 0.0;
    will_return( __wrap_ucISL68221ReadVoltage0, XST_FAILURE );
    vV80VoltageMonitorVccHBM( );
    assert_true(  sensor_glvr.sensor_readings.voltage[eVCC_HBM] == 0 );
}

static void test_vV80CurrentMonitorVccHBM( void **state ) 
{
    ( void ) state; /* unused */

    sensor_glvr.sensor_readings.current[eVCC_HBM] = 0.0;
    will_return( __wrap_ucISL68221ReadCurrent0, XST_SUCCESS );
    vV80CurrentMonitorVccHBM( );
    assert_true(  sensor_glvr.sensor_readings.current[eVCC_HBM] == 0.75 );
}

static void test_vV80CurrentMonitorVccHBMFail( void **state ) 
{
    ( void ) state; /* unused */

    /* Test 1 failed read */
    sensor_glvr.sensor_readings.current[eVCC_HBM] = 0.0;
    will_return( __wrap_ucISL68221ReadCurrent0, XST_FAILURE );
    vV80CurrentMonitorVccHBM( );
     assert_true(  sensor_glvr.sensor_readings.current[eVCC_HBM] == 0 );   
}

static void test_vV80VoltageMonitor1V2VccoDimm( void **state ) 
{
    ( void ) state; /* unused */

    sensor_glvr.sensor_readings.voltage[e1V2_VCCO_DIMM] = 0.0;
    will_return( __wrap_ucISL68221ReadVoltage1, XST_SUCCESS );
    vV80VoltageMonitor1V2VccoDimm( );
    assert_true(  sensor_glvr.sensor_readings.voltage[e1V2_VCCO_DIMM] == 1266 );
}

static void test_vV80VoltageMonitor1V2VccoDimmFail( void **state ) 
{
    ( void ) state; /* unused */

    /* Test 1 failed read */
    sensor_glvr.sensor_readings.voltage[e1V2_VCCO_DIMM] = 0.0;
    will_return( __wrap_ucISL68221ReadVoltage1, XST_FAILURE );
    vV80VoltageMonitor1V2VccoDimm( );
    assert_true(  sensor_glvr.sensor_readings.voltage[e1V2_VCCO_DIMM] == 0 );
}

static void test_vV80CurrentMonitor1V2VccoDimm( void **state ) 
{
    ( void ) state; /* unused */

    sensor_glvr.sensor_readings.current[e1V2_VCCO_DIMM] = 0.0;
    will_return( __wrap_ucISL68221ReadCurrent1, XST_SUCCESS );
    vV80CurrentMonitor1V2VccoDimm( );
    assert_true(  sensor_glvr.sensor_readings.current[e1V2_VCCO_DIMM] == ( float )0.76 );
}

static void test_vV80CurrentMonitor1V2VccoDimmFail( void **state ) 
{
    ( void ) state; /* unused */

    /* Test 1 failed read */
    sensor_glvr.sensor_readings.current[e1V2_VCCO_DIMM] = 0.0;
    will_return( __wrap_ucISL68221ReadCurrent1, XST_FAILURE );
    vV80CurrentMonitor1V2VccoDimm( );
    assert_true(  sensor_glvr.sensor_readings.current[e1V2_VCCO_DIMM] == 0 );    
}

static void test_vV80VoltageMonitor1V2GTXAVTT( void **state ) 
{
    ( void ) state; /* unused */

    sensor_glvr.sensor_readings.voltage[e1V2_GTXAVTT] = 0.0;
    will_return( __wrap_ucISL68221ReadVoltage2, XST_SUCCESS );
    vV80VoltageMonitor1V2GTXAVTT( );
    assert_true(  sensor_glvr.sensor_readings.voltage[e1V2_GTXAVTT] == 1267 );
}

static void test_vV80VoltageMonitor1V2GTXAVTTFail( void **state ) 
{
    ( void ) state; /* unused */

    /* Test 1 failed read */
    sensor_glvr.sensor_readings.voltage[e1V2_GTXAVTT] = 0.0;
    will_return( __wrap_ucISL68221ReadVoltage2, XST_FAILURE );
    vV80VoltageMonitor1V2GTXAVTT( );
    assert_true(  sensor_glvr.sensor_readings.voltage[e1V2_GTXAVTT] == 0 );
}

static void test_vV80CurrentMonitor1V2GTXAVTT( void **state ) 
{
    ( void ) state; /* unused */

    sensor_glvr.sensor_readings.current[e1V2_GTXAVTT] = 0.0;
    will_return( __wrap_ucISL68221ReadCurrent2, XST_SUCCESS );
    vV80CurrentMonitor1V2GTXAVTT( );
    assert_true(  sensor_glvr.sensor_readings.current[e1V2_GTXAVTT] == ( float )0.77 );
}

static void test_vV80CurrentMonitor1V2GTXAVTTFail( void **state ) 
{
    ( void ) state; /* unused */

    /* Test 1 failed read */
    sensor_glvr.sensor_readings.current[e1V2_GTXAVTT] = 0.0;
    will_return( __wrap_ucISL68221ReadCurrent2, XST_FAILURE );
    vV80CurrentMonitor1V2GTXAVTT( );
    assert_true(  sensor_glvr.sensor_readings.current[e1V2_GTXAVTT] == 0 );
}

static void test_vV80VoltageMonitorVccint( void **state ) 
{
    ( void ) state; /* unused */

    sensor_glvr.sensor_readings.voltage[eVCCINT] = 0.0;
    will_return( __wrap_ucISL68221ReadVoltage0, XST_SUCCESS );
    vV80VoltageMonitorVccint( );
    assert_true(  sensor_glvr.sensor_readings.voltage[eVCCINT] == 12000 );
}

static void test_vV80VoltageMonitorVccintFail( void **state ) 
{
    ( void ) state; /* unused */

    /* Test 1 failed read */
    sensor_glvr.sensor_readings.voltage[eVCCINT] = 0.0;
    will_return( __wrap_ucISL68221ReadVoltage0, XST_FAILURE );
    vV80VoltageMonitorVccint( );
    assert_true(  sensor_glvr.sensor_readings.voltage[eVCCINT] == 0 );
}

static void test_vV80CurrentMonitorVccint( void **state ) 
{
    ( void ) state; /* unused */
    sensor_glvr.sensor_readings.current[eVCCINT] = 0.0;
    will_return( __wrap_ucISL68221ReadCurrent0, XST_SUCCESS );
    vV80CurrentMonitorVccint( );
    assert_true(  sensor_glvr.sensor_readings.current[eVCCINT] == 4 );
}

static void test_vV80CurrentMonitorVccintFail( void **state ) 
{
    ( void ) state; /* unused */

    /* Test 1 failed read */
    sensor_glvr.sensor_readings.current[eVCCINT] = 0.0;
    will_return( __wrap_ucISL68221ReadCurrent0, XST_FAILURE );
    vV80CurrentMonitorVccint( );
    assert_true(  sensor_glvr.sensor_readings.current[eVCCINT] == 0 ); 
}

static void test_scV80AsdmReadPower( void **state ) 
{
    ( void ) state; /* unused */
    s8 scStatus             = 0;
    snsrRead_t xSnsrData   = {0};

    scStatus = scV80AsdmReadPower( &xSnsrData );

    assert_true(  scStatus == XST_SUCCESS );
}

static void test_scV80AsdmReadPowerFail( void **state ) 
{
    ( void ) state; /* unused */
    s8 scStatus             = 0;
    snsrRead_t xSnsrData   = {0};

    /* Test 1 failed read */
    scStatus = scV80AsdmReadPower( NULL );

    assert_true(  scStatus == XST_FAILURE );
}

static void test_scV80AsdmReadVoltage12v( void **state ) 
{
    ( void ) state; /* unused */
    s8 scStatus             = 0;
    snsrRead_t xSnsrData   = {0};

    scStatus = scV80AsdmReadVoltage12v( &xSnsrData );

    assert_true(  scStatus == XST_SUCCESS );
}

static void test_scV80AsdmReadVoltage12vZero( void **state ) 
{
    ( void ) state; /* unused */
    s8 scStatus             = 0;
    snsrRead_t xSnsrData   = {0};

    /* Test 1 Zero Value */
    vINA3221SetNewValue( 0 );
    will_return( __wrap_INA3221_ReadVoltage, XST_SUCCESS );
    vV80VoltageMonitor12VPEX( );
    scStatus = scV80AsdmReadVoltage12v( &xSnsrData );

    assert_true(  scStatus == XST_SUCCESS );
}

static void test_scV80AsdmReadVoltage12vFail( void **state ) 
{
    ( void ) state; /* unused */
    s8 scStatus             = 0;
    snsrRead_t xSnsrData   = {0};

    /* Test 1 failed read */
    scStatus = scV80AsdmReadVoltage12v( NULL );

    assert_true(  scStatus == XST_FAILURE );
}

static void test_scV80AsdmReadVoltage3v3( void **state ) 
{
    ( void ) state; /* unused */
    s8 scStatus             = 0;
    snsrRead_t xSnsrData   = {0};

    scStatus = scV80AsdmReadVoltage3v3( &xSnsrData );

    assert_true(  scStatus == XST_SUCCESS );
}

static void test_scV80AsdmReadVoltage3v3Zero( void **state ) 
{
    ( void ) state; /* unused */
    s8 scStatus             = 0;
    snsrRead_t xSnsrData   = {0};

    /* Test 1 Zero Value */
    vINA3221SetNewValue( 0 );
    will_return( __wrap_INA3221_ReadVoltage, XST_SUCCESS );
    vV80VoltageMonitor3v3PEX( );
    scStatus = scV80AsdmReadVoltage3v3( &xSnsrData );

    assert_true(  scStatus == XST_SUCCESS );
}

static void test_scV80AsdmReadVoltage3v3Fail( void **state ) 
{
    ( void ) state; /* unused */
    s8 scStatus             = 0;
    snsrRead_t xSnsrData   = {0};

    /* Test 1 failed read */
    scStatus = scV80AsdmReadVoltage3v3( NULL );

    assert_true(  scStatus == XST_FAILURE );
}

static void test_scV80AsdmReadVoltageVccint( void **state ) 
{
    ( void ) state; /* unused */
    s8 scStatus             = 0;
    snsrRead_t xSnsrData   = {0};

    scStatus = scV80AsdmReadVoltageVccint( &xSnsrData );

    assert_true(  scStatus == XST_SUCCESS );
}

static void test_scV80AsdmReadVoltageVccintZero( void **state ) 
{
    ( void ) state; /* unused */
    s8 scStatus             = 0;
    snsrRead_t xSnsrData   = {0};

    /* Test 1 Zero Value */
    vSetNewValue( 0 );
    will_return( __wrap_ucISL68221ReadVoltage0, XST_SUCCESS );
    vV80VoltageMonitorVccint( );
    scStatus = scV80AsdmReadVoltageVccint( &xSnsrData );

    assert_true(  scStatus == XST_SUCCESS );
}

static void test_scV80AsdmReadVoltageVccintFail( void **state ) 
{
    ( void ) state; /* unused */
    s8 scStatus             = 0;
    snsrRead_t xSnsrData   = {0};

    /* Test 1 failed read */
    scStatus = scV80AsdmReadVoltageVccint( NULL );

    assert_true(  scStatus == XST_FAILURE );
}

static void test_scV80AsdmReadVoltage12VAUX0( void **state ) 
{
    ( void ) state; /* unused */
    s8 scStatus             = 0;
    snsrRead_t xSnsrData   = {0};

    scStatus = scV80AsdmReadVoltage12VAUX0( &xSnsrData );

    assert_true(  scStatus == XST_SUCCESS );
}

static void test_scV80AsdmReadVoltage12VAUX0Zero( void **state ) 
{
    ( void ) state; /* unused */
    s8 scStatus             = 0;
    snsrRead_t xSnsrData   = {0};

    /* Test 1 Zero Value */
    vINA3221SetNewValue( 0 );
    will_return( __wrap_INA3221_ReadVoltage, XST_SUCCESS );
    vV80VoltageMonitor12VAUX0( );
    scStatus = scV80AsdmReadVoltage12VAUX0( &xSnsrData );

    assert_true(  scStatus == XST_SUCCESS );
}

static void test_scV80AsdmReadVoltage12VAUX0Fail( void **state ) 
{
    ( void ) state; /* unused */
    s8 scStatus             = 0;
    snsrRead_t xSnsrData   = {0};

    /* Test 1 failed read */
    scStatus = scV80AsdmReadVoltage12VAUX0( NULL );

    assert_true(  scStatus == XST_FAILURE );
}

static void test_scV80AsdmReadVoltage12VAUX1( void **state ) 
{
    ( void ) state; /* unused */
    s8 scStatus             = 0;
    snsrRead_t xSnsrData   = {0};

    scStatus = scV80AsdmReadVoltage12VAUX1( &xSnsrData );

    assert_true(  scStatus == XST_SUCCESS );
}

static void test_scV80AsdmReadVoltage12VAUX1Zero( void **state ) 
{
    ( void ) state; /* unused */
    s8 scStatus             = 0;
    snsrRead_t xSnsrData   = {0};

    /* Test 1 Zero Value */
    vINA3221SetNewValue( 0 );
    will_return( __wrap_INA3221_ReadVoltage, XST_SUCCESS );
    vV80VoltageMonitor12VAUX1( );
    scStatus = scV80AsdmReadVoltage12VAUX1( &xSnsrData );

    assert_true(  scStatus == XST_SUCCESS );
}

static void test_scV80AsdmReadVoltage12VAUX1Fail( void **state ) 
{
    ( void ) state; /* unused */
    s8 scStatus             = 0;
    snsrRead_t xSnsrData   = {0};

    /* Test 1 failed read */
    scStatus = scV80AsdmReadVoltage12VAUX1( NULL );

    assert_true(  scStatus == XST_FAILURE );
}

static void test_scV80AsdmReadVoltage1V5VCCAUX( void **state ) 
{
    ( void ) state; /* unused */
    s8 scStatus             = 0;
    snsrRead_t xSnsrData   = {0};

    scStatus = scV80AsdmReadVoltage1V5VCCAUX( &xSnsrData );

    assert_true(  scStatus == XST_SUCCESS );
}

static void test_scV80AsdmReadVoltage1V5VCCAUXZero( void **state ) 
{
    ( void ) state; /* unused */
    s8 scStatus             = 0;
    snsrRead_t xSnsrData   = {0};

    /* Test 1 Zero Value */
    vINA3221SetNewValue( 0 );
    will_return( __wrap_INA3221_ReadVoltage, XST_SUCCESS );
    vV80VoltageMonitor1V5VCCAUX( );
    scStatus = scV80AsdmReadVoltage1V5VCCAUX( &xSnsrData );

    assert_true(  scStatus == XST_SUCCESS );
}

static void test_scV80AsdmReadVoltage1V5VCCAUXFail( void **state ) 
{
    ( void ) state; /* unused */
    s8 scStatus             = 0;
    snsrRead_t xSnsrData   = {0};

    /* Test 1 failed read */
    scStatus = scV80AsdmReadVoltage1V5VCCAUX( NULL );

    assert_true(  scStatus == XST_FAILURE );
}

static void test_scV80AsdmReadVoltage3V3QSFP( void **state ) 
{
    ( void ) state; /* unused */
    s8 scStatus             = 0;
    snsrRead_t xSnsrData   = {0};

    scStatus = scV80AsdmReadVoltage3V3QSFP( &xSnsrData );

    assert_true(  scStatus == XST_SUCCESS );
}

static void test_scV80AsdmReadVoltage3V3QSFPZero( void **state ) 
{
    ( void ) state; /* unused */
    s8 scStatus             = 0;
    snsrRead_t xSnsrData   = {0};

    /* Test 1 Zero Value */
    vINA3221SetNewValue( 0 );
    will_return( __wrap_INA3221_ReadVoltage, XST_SUCCESS );
    vV80VoltageMonitor3V3QSFP( );
    scStatus = scV80AsdmReadVoltage3V3QSFP( &xSnsrData );

    assert_true(  scStatus == XST_SUCCESS );
}

static void test_scV80AsdmReadVoltage3V3QSFPFail( void **state ) 
{
    ( void ) state; /* unused */
    s8 scStatus             = 0;
    snsrRead_t xSnsrData   = {0};

    /* Test 1 failed read */
    scStatus = scV80AsdmReadVoltage3V3QSFP( NULL );

    assert_true(  scStatus == XST_FAILURE );
}

static void test_scV80AsdmReadVoltageVccHBM( void **state ) 
{
    ( void ) state; /* unused */
    s8 scStatus             = 0;
    snsrRead_t xSnsrData   = {0};

    scStatus = scV80AsdmReadVoltageVccHBM( &xSnsrData );

    assert_true(  scStatus == XST_SUCCESS );
}

static void test_scV80AsdmReadVoltageVccHBMZero( void **state ) 
{
    ( void ) state; /* unused */
    s8 scStatus             = 0;
    snsrRead_t xSnsrData   = {0};

    /* Test 1 Zero Value */
    vSetNewValue( 0 );
    will_return( __wrap_ucISL68221ReadVoltage0, XST_SUCCESS );
    vV80VoltageMonitorVccHBM( );
    scStatus = scV80AsdmReadVoltageVccHBM( &xSnsrData );

    assert_true(  scStatus == XST_SUCCESS );
}

static void test_scV80AsdmReadVoltageVccHBMFail( void **state ) 
{
    ( void ) state; /* unused */
    s8 scStatus             = 0;
    snsrRead_t xSnsrData   = {0};

    /* Test 1 failed read */
    scStatus = scV80AsdmReadVoltageVccHBM( NULL );

    assert_true(  scStatus == XST_FAILURE );
}

static void test_scV80AsdmReadVoltage1V2VccoDimm( void **state ) 
{
    ( void ) state; /* unused */
    s8 scStatus             = 0;
    snsrRead_t xSnsrData   = {0};

    scStatus = scV80AsdmReadVoltage1V2VccoDimm( &xSnsrData );

    assert_true(  scStatus == XST_SUCCESS );
}

static void test_scV80AsdmReadVoltage1V2VccoDimmZero( void **state ) 
{
    ( void ) state; /* unused */
    s8 scStatus             = 0;
    snsrRead_t xSnsrData   = {0};

    /* Test 1 Zero Value */
    vSetNewValue( 0 );
    will_return( __wrap_ucISL68221ReadVoltage1, XST_SUCCESS );
    vV80VoltageMonitor1V2VccoDimm( );
    scStatus = scV80AsdmReadVoltage1V2VccoDimm( &xSnsrData );

    assert_true(  scStatus == XST_SUCCESS );
}

static void test_scV80AsdmReadVoltage1V2VccoDimmFail( void **state ) 
{
    ( void ) state; /* unused */
    s8 scStatus             = 0;
    snsrRead_t xSnsrData   = {0};

    /* Test 1 failed read */
    scStatus = scV80AsdmReadVoltage1V2VccoDimm( NULL );

    assert_true(  scStatus == XST_FAILURE );
}

static void test_scV80AsdmReadVoltage1V2GTXAVT( void **state ) 
{
    ( void ) state; /* unused */
    s8 scStatus             = 0;
    snsrRead_t xSnsrData   = {0};

    scStatus = scV80AsdmReadVoltage1V2GTXAVT( &xSnsrData );

    assert_true(  scStatus == XST_SUCCESS );
}

static void test_scV80AsdmReadVoltage1V2GTXAVTZero( void **state ) 
{
    ( void ) state; /* unused */
    s8 scStatus             = 0;
    snsrRead_t xSnsrData   = {0};

    /* Test 1 Zero Value */
    vSetNewValue( 0 );
    will_return( __wrap_ucISL68221ReadVoltage2, XST_SUCCESS );
    vV80VoltageMonitor1V2GTXAVTT( );
    scStatus = scV80AsdmReadVoltage1V2GTXAVT( &xSnsrData );

    assert_true(  scStatus == XST_SUCCESS );
}

static void test_scV80AsdmReadVoltage1V2GTXAVTFail( void **state ) 
{
    ( void ) state; /* unused */
    s8 scStatus             = 0;
    snsrRead_t xSnsrData   = {0};

    /* Test 1 failed read */
    scStatus = scV80AsdmReadVoltage1V2GTXAVT( NULL );

    assert_true(  scStatus == XST_FAILURE );
}

static void test_scV80AsdmReadCurrent12v( void **state ) 
{
    ( void ) state; /* unused */
    s8 scStatus             = 0;
    snsrRead_t xSnsrData   = {0};

    scStatus = scV80AsdmReadCurrent12v( &xSnsrData );

    assert_true(  scStatus == XST_SUCCESS );
}

static void test_scV80AsdmReadCurrent12vZero( void **state ) 
{
    ( void ) state; /* unused */
    s8 scStatus             = 0;
    snsrRead_t xSnsrData   = {0};

   /* Test 1 Zero Value */
    vINA3221SetNewValue( 0 );
    will_return( __wrap_INA3221_ReadCurrent, XST_SUCCESS );
    vV80CurrentMonitor12VPEX( );
    scStatus = scV80AsdmReadCurrent12v( &xSnsrData );

    assert_true(  scStatus == XST_SUCCESS );
}

static void test_scV80AsdmReadCurrent12vFail( void **state ) 
{
    ( void ) state; /* unused */
    s8 scStatus             = 0;
    snsrRead_t xSnsrData   = {0};

    /* Test 1 failed read */
    scStatus = scV80AsdmReadCurrent12v( NULL );

    assert_true(  scStatus == XST_FAILURE );
}

static void test_scV80AsdmReadCurrent3v3( void **state ) 
{
    ( void ) state; /* unused */
    s8 scStatus             = 0;
    snsrRead_t xSnsrData   = {0};

    scStatus = scV80AsdmReadCurrent3v3( &xSnsrData );

    assert_true(  scStatus == XST_SUCCESS );
}

static void test_scV80AsdmReadCurrent3v3Zero( void **state ) 
{
    ( void ) state; /* unused */
    s8 scStatus             = 0;
    snsrRead_t xSnsrData   = {0};

    /* Test 1 Zero Value */
    vINA3221SetNewValue( 0 );
    will_return( __wrap_INA3221_ReadCurrent, XST_SUCCESS );
    vV80CurrentMonitor3v3PEX( );
    scStatus = scV80AsdmReadCurrent3v3( &xSnsrData );

    assert_true(  scStatus == XST_SUCCESS );
}

static void test_scV80AsdmReadCurrent3v3Fail( void **state ) 
{
    ( void ) state; /* unused */
    s8 scStatus             = 0;
    snsrRead_t xSnsrData   = {0};

    /* Test 1 failed read */
    scStatus = scV80AsdmReadCurrent3v3( NULL );

    assert_true(  scStatus == XST_FAILURE );
}

static void test_scV80AsdmReadCurrentVccint( void **state ) 
{
    ( void ) state; /* unused */
    s8 scStatus             = 0;
    snsrRead_t xSnsrData   = {0};

    scStatus = scV80AsdmReadCurrentVccint( &xSnsrData );

    assert_true(  scStatus == XST_SUCCESS );
}

static void test_scV80AsdmReadCurrentVccintZero( void **state ) 
{
    ( void ) state; /* unused */
    s8 scStatus             = 0;
    snsrRead_t xSnsrData   = {0};

    /* Test 1 Zero Value */
    vSetNewValue( 0 );
    will_return( __wrap_ucISL68221ReadCurrent0, XST_SUCCESS );
    vV80CurrentMonitorVccint( );
    scStatus = scV80AsdmReadCurrentVccint( &xSnsrData );

    assert_true(  scStatus == XST_SUCCESS );
}

static void test_scV80AsdmReadCurrentVccintFail( void **state ) 
{
    ( void ) state; /* unused */
    s8 scStatus             = 0;
    snsrRead_t xSnsrData   = {0};

    /* Test 1 failed read */
    scStatus = scV80AsdmReadCurrentVccint( NULL );

    assert_true(  scStatus == XST_FAILURE );
}

static void test_scV80AsdmReadCurrent12VAUX0( void **state ) 
{
    ( void ) state; /* unused */
    s8 scStatus             = 0;
    snsrRead_t xSnsrData   = {0};

    scStatus = scV80AsdmReadCurrent12VAUX0( &xSnsrData );

    assert_true(  scStatus == XST_SUCCESS );
}

static void test_scV80AsdmReadCurrent12VAUX0Zero( void **state ) 
{
    ( void ) state; /* unused */
    s8 scStatus             = 0;
    snsrRead_t xSnsrData   = {0};

    /* Test 1 Zero Value */
    vINA3221SetNewValue( 0 );
    will_return( __wrap_INA3221_ReadCurrent, XST_SUCCESS );
    vV80CurrentMonitor12VAUX0( );
    scStatus = scV80AsdmReadCurrent12VAUX0( &xSnsrData );

    assert_true(  scStatus == XST_SUCCESS );
}

static void test_scV80AsdmReadCurrent12VAUX0Fail( void **state ) 
{
    ( void ) state; /* unused */
    s8 scStatus             = 0;
    snsrRead_t xSnsrData   = {0};

    /* Test 1 failed read */
    scStatus = scV80AsdmReadCurrent12VAUX0( NULL );

    assert_true(  scStatus == XST_FAILURE );
}

static void test_scV80AsdmReadCurrent12VAUX1( void **state ) 
{
    ( void ) state; /* unused */
    s8 scStatus             = 0;
    snsrRead_t xSnsrData   = {0};

    scStatus = scV80AsdmReadCurrent12VAUX1( &xSnsrData );

    assert_true(  scStatus == XST_SUCCESS );
}

static void test_scV80AsdmReadCurrent12VAUX1Zero( void **state ) 
{
    ( void ) state; /* unused */
    s8 scStatus             = 0;
    snsrRead_t xSnsrData   = {0};

    /* Test 1 Zero Value */
    vINA3221SetNewValue( 0 );
    will_return( __wrap_INA3221_ReadCurrent, XST_SUCCESS );
    vV80CurrentMonitor12VAUX1( );
    scStatus = scV80AsdmReadCurrent12VAUX1( &xSnsrData );

    assert_true(  scStatus == XST_SUCCESS );
}

static void test_scV80AsdmReadCurrent12VAUX1Fail( void **state ) 
{
    ( void ) state; /* unused */
    s8 scStatus             = 0;
    snsrRead_t xSnsrData   = {0};

    /* Test 1 failed read */
    scStatus = scV80AsdmReadCurrent12VAUX1( NULL );

    assert_true(  scStatus == XST_FAILURE );
}

static void test_scV80AsdmReadCurrent1V5VCCAUX( void **state ) 
{
    ( void ) state; /* unused */
    s8 scStatus             = 0;
    snsrRead_t xSnsrData   = {0};

    scStatus = scV80AsdmReadCurrent1V5VCCAUX( &xSnsrData );

    assert_true(  scStatus == XST_SUCCESS );
}

static void test_scV80AsdmReadCurrent1V5VCCAUXZero( void **state ) 
{
    ( void ) state; /* unused */
    s8 scStatus             = 0;
    snsrRead_t xSnsrData   = {0};

    /* Test 1 Zero Value */
    vINA3221SetNewValue( 0 );
    will_return( __wrap_INA3221_ReadCurrent, XST_SUCCESS );
    vV80CurrentMonitor1V5VCCAUX( );
    scStatus = scV80AsdmReadCurrent1V5VCCAUX( &xSnsrData );

    assert_true(  scStatus == XST_SUCCESS );
}

static void test_scV80AsdmReadCurrent1V5VCCAUXFail( void **state ) 
{
    ( void ) state; /* unused */
    s8 scStatus             = 0;
    snsrRead_t xSnsrData   = {0};

    /* Test 1 failed read */
    scStatus = scV80AsdmReadCurrent1V5VCCAUX( NULL );

    assert_true(  scStatus == XST_FAILURE );
}

static void test_scV80AsdmReadCurrent3V3QSFP( void **state ) 
{
    ( void ) state; /* unused */
    s8 scStatus             = 0;
    snsrRead_t xSnsrData   = {0};

    scStatus = scV80AsdmReadCurrent3V3QSFP( &xSnsrData );

    assert_true(  scStatus == XST_SUCCESS );
}

static void test_scV80AsdmReadCurrent3V3QSFPZero( void **state ) 
{
    ( void ) state; /* unused */
    s8 scStatus             = 0;
    snsrRead_t xSnsrData   = {0};

    /* Test 1 Zero Value */
    vINA3221SetNewValue( 0 );
    will_return( __wrap_INA3221_ReadCurrent, XST_SUCCESS );
    vV80CurrentMonitor3V3QSFP( );
    scStatus = scV80AsdmReadCurrent3V3QSFP( &xSnsrData );

    assert_true(  scStatus == XST_SUCCESS );
}

static void test_scV80AsdmReadCurrent3V3QSFPFail( void **state ) 
{
    ( void ) state; /* unused */
    s8 scStatus             = 0;
    snsrRead_t xSnsrData   = {0};

    /* Test 1 failed read */
    scStatus = scV80AsdmReadCurrent3V3QSFP( NULL );

    assert_true(  scStatus == XST_FAILURE );
}

static void test_scV80AsdmReadCurrentVccHBM( void **state ) 
{
    ( void ) state; /* unused */
    s8 scStatus             = 0;
    snsrRead_t xSnsrData   = {0};

    scStatus = scV80AsdmReadCurrentVccHBM( &xSnsrData );

    assert_true(  scStatus == XST_SUCCESS );
}

static void test_scV80AsdmReadCurrentVccHBMZero( void **state ) 
{
    ( void ) state; /* unused */
    s8 scStatus             = 0;
    snsrRead_t xSnsrData   = {0};

    /* Test 1 Zero Value */
    vSetNewValue( 0 );
    will_return( __wrap_ucISL68221ReadCurrent0, XST_SUCCESS );
    vV80CurrentMonitorVccHBM( );
    scStatus = scV80AsdmReadCurrentVccHBM( &xSnsrData );

    assert_true(  scStatus == XST_SUCCESS );
}

static void test_scV80AsdmReadCurrentVccHBMFail( void **state ) 
{
    ( void ) state; /* unused */
    s8 scStatus             = 0;
    snsrRead_t xSnsrData   = {0};

    /* Test 1 failed read */
    scStatus = scV80AsdmReadCurrentVccHBM( NULL );

    assert_true(  scStatus == XST_FAILURE );
}

static void test_scV80AsdmReadCurrent1V2VccoDimm( void **state ) 
{
    ( void ) state; /* unused */
    s8 scStatus             = 0;
    snsrRead_t xSnsrData   = {0};

    scStatus = scV80AsdmReadCurrent1V2VccoDimm( &xSnsrData );

    assert_true(  scStatus == XST_SUCCESS );
}

static void test_scV80AsdmReadCurrent1V2VccoDimmZero( void **state ) 
{
    ( void ) state; /* unused */
    s8 scStatus             = 0;
    snsrRead_t xSnsrData   = {0};

    /* Test 1 Zero Value */
    vSetNewValue( 0 );
    will_return( __wrap_ucISL68221ReadCurrent1, XST_SUCCESS );
    vV80CurrentMonitor1V2VccoDimm( );
    scStatus = scV80AsdmReadCurrent1V2VccoDimm( &xSnsrData );

    assert_true(  scStatus == XST_SUCCESS );
}

static void test_scV80AsdmReadCurrent1V2VccoDimmFail( void **state ) 
{
    ( void ) state; /* unused */
    s8 scStatus             = 0;
    snsrRead_t xSnsrData   = {0};

    /* Test 1 failed read */
    scStatus = scV80AsdmReadCurrent1V2VccoDimm( NULL );

    assert_true(  scStatus == XST_FAILURE );
}

static void test_scV80AsdmReadCurrent1V2GTXAVT( void **state ) 
{
    ( void ) state; /* unused */
    s8 scStatus             = 0;
    snsrRead_t xSnsrData   = {0};

    scStatus = scV80AsdmReadCurrent1V2GTXAVT( &xSnsrData );

    assert_true(  scStatus == XST_SUCCESS );
}

static void test_scV80AsdmReadCurrent1V2GTXAVTZero( void **state ) 
{
    ( void ) state; /* unused */
    s8 scStatus             = 0;
    snsrRead_t xSnsrData   = {0};

    /* Test 1 Zero Value */
    vSetNewValue( 0 );
    will_return( __wrap_ucISL68221ReadCurrent2, XST_SUCCESS );
    vV80CurrentMonitor1V2GTXAVTT( );
    scStatus = scV80AsdmReadCurrent1V2GTXAVT( &xSnsrData );

    assert_true(  scStatus == XST_SUCCESS );
}

static void test_scV80AsdmReadCurrent1V2GTXAVTFail( void **state ) 
{
    ( void ) state; /* unused */
    s8 scStatus             = 0;
    snsrRead_t xSnsrData   = {0};

    /* Test 1 failed read */
    scStatus = scV80AsdmReadCurrent1V2GTXAVT( NULL );

    assert_true(  scStatus == XST_FAILURE );
}

static void test_vV80PowerMonitor( void **state ) 
{
    ( void ) state; /* unused */

    /* Test 1 Zero Value */
    will_return( __wrap_INA3221_ReadVoltage, XST_SUCCESS );
    vV80VoltageMonitor12VPEX( );
    will_return( __wrap_INA3221_ReadVoltage, XST_SUCCESS );
    vV80VoltageMonitor3v3PEX( );
    will_return( __wrap_INA3221_ReadCurrent, XST_SUCCESS );
    vV80CurrentMonitor12VPEX( );
    will_return( __wrap_INA3221_ReadCurrent, XST_SUCCESS );
    vV80CurrentMonitor3v3PEX( );
    vV80PowerMonitor( );

}
static void test_vV80PowerMonitorZero( void **state ) 
{
    ( void ) state; /* unused */

    /* Test 1 Zero Value */
    vINA3221SetNewValue( 0 );
    will_return( __wrap_INA3221_ReadVoltage, XST_SUCCESS );
    vV80VoltageMonitor12VPEX( );
    will_return( __wrap_INA3221_ReadVoltage, XST_SUCCESS );
    vV80VoltageMonitor3v3PEX( );
    will_return( __wrap_INA3221_ReadCurrent, XST_SUCCESS );
    vV80CurrentMonitor12VPEX( );
    will_return( __wrap_INA3221_ReadCurrent, XST_SUCCESS );
    vV80CurrentMonitor3v3PEX( );
    vV80PowerMonitor( );

}

static void test_vV80PowerMonitorThresholdCross( void **state ) 
{
    ( void ) state; /* unused */

    /* Test 1 Threshold Cross */
    vINA3221SetNewValue( 6000 );
    
    will_return( __wrap_INA3221_ReadCurrent, XST_SUCCESS );
    vV80CurrentMonitor12VPEX( );
    will_return( __wrap_INA3221_ReadVoltage, XST_SUCCESS );
    vV80VoltageMonitor12VPEX( );
    will_return( __wrap_INA3221_ReadVoltage, XST_SUCCESS );
    vV80VoltageMonitor3v3PEX( );
    will_return( __wrap_INA3221_ReadCurrent, XST_SUCCESS );
    vV80CurrentMonitor12VPEX( );
    will_return( __wrap_INA3221_ReadCurrent, XST_SUCCESS );
    vV80CurrentMonitor3v3PEX( );

    vV80PowerMonitor( );

    will_return( __wrap_INA3221_ReadCurrent, XST_SUCCESS );
    vV80CurrentMonitor12VPEX( );
    will_return( __wrap_INA3221_ReadVoltage, XST_SUCCESS );
    vV80VoltageMonitor12VPEX( );
    will_return( __wrap_INA3221_ReadVoltage, XST_SUCCESS );
    vV80VoltageMonitor3v3PEX( );
    will_return( __wrap_INA3221_ReadCurrent, XST_SUCCESS );
    vV80CurrentMonitor12VPEX( );
    will_return( __wrap_INA3221_ReadCurrent, XST_SUCCESS );
    vV80CurrentMonitor3v3PEX( );

    vV80PowerMonitor( );

    will_return( __wrap_INA3221_ReadCurrent, XST_SUCCESS );
    vV80CurrentMonitor12VPEX( );
    will_return( __wrap_INA3221_ReadVoltage, XST_SUCCESS );
    vV80VoltageMonitor12VPEX( );
    will_return( __wrap_INA3221_ReadVoltage, XST_SUCCESS );
    vV80VoltageMonitor3v3PEX( );
    will_return( __wrap_INA3221_ReadCurrent, XST_SUCCESS );
    vV80CurrentMonitor12VPEX( );
    will_return( __wrap_INA3221_ReadCurrent, XST_SUCCESS );
    vV80CurrentMonitor3v3PEX( );

    vV80PowerMonitor( );

    will_return( __wrap_INA3221_ReadCurrent, XST_SUCCESS );
    vV80CurrentMonitor12VPEX( );
    will_return( __wrap_INA3221_ReadVoltage, XST_SUCCESS );
    vV80VoltageMonitor12VPEX( );
    will_return( __wrap_INA3221_ReadVoltage, XST_SUCCESS );
    vV80VoltageMonitor3v3PEX( );
    will_return( __wrap_INA3221_ReadCurrent, XST_SUCCESS );
    vV80CurrentMonitor12VPEX( );
    will_return( __wrap_INA3221_ReadCurrent, XST_SUCCESS );
    vV80CurrentMonitor3v3PEX( );

    vV80PowerMonitor( );

    will_return( __wrap_INA3221_ReadCurrent, XST_SUCCESS );
    vV80CurrentMonitor12VPEX( );
    will_return( __wrap_INA3221_ReadVoltage, XST_SUCCESS );
    vV80VoltageMonitor12VPEX( );
    will_return( __wrap_INA3221_ReadVoltage, XST_SUCCESS );
    vV80VoltageMonitor3v3PEX( );
    will_return( __wrap_INA3221_ReadCurrent, XST_SUCCESS );
    vV80CurrentMonitor12VPEX( );
    will_return( __wrap_INA3221_ReadCurrent, XST_SUCCESS );
    vV80CurrentMonitor3v3PEX( );

    vV80PowerMonitor( );

    will_return( __wrap_INA3221_ReadCurrent, XST_SUCCESS );
    vV80CurrentMonitor12VPEX( );
    will_return( __wrap_INA3221_ReadVoltage, XST_SUCCESS );
    vV80VoltageMonitor12VPEX( );
    will_return( __wrap_INA3221_ReadVoltage, XST_SUCCESS );
    vV80VoltageMonitor3v3PEX( );
    will_return( __wrap_INA3221_ReadCurrent, XST_SUCCESS );
    vV80CurrentMonitor12VPEX( );
    will_return( __wrap_INA3221_ReadCurrent, XST_SUCCESS );
    vV80CurrentMonitor3v3PEX( );

    vV80PowerMonitor( );

    will_return( __wrap_INA3221_ReadCurrent, XST_SUCCESS );
    vV80CurrentMonitor12VPEX( );
    will_return( __wrap_INA3221_ReadVoltage, XST_SUCCESS );
    vV80VoltageMonitor12VPEX( );
    will_return( __wrap_INA3221_ReadVoltage, XST_SUCCESS );
    vV80VoltageMonitor3v3PEX( );
    will_return( __wrap_INA3221_ReadCurrent, XST_SUCCESS );
    vV80CurrentMonitor12VPEX( );
    will_return( __wrap_INA3221_ReadCurrent, XST_SUCCESS );
    vV80CurrentMonitor3v3PEX( );

    vV80PowerMonitor( );

    will_return( __wrap_INA3221_ReadCurrent, XST_SUCCESS );
    vV80CurrentMonitor12VPEX( );
    will_return( __wrap_INA3221_ReadVoltage, XST_SUCCESS );
    vV80VoltageMonitor12VPEX( );
    will_return( __wrap_INA3221_ReadVoltage, XST_SUCCESS );
    vV80VoltageMonitor3v3PEX( );
    will_return( __wrap_INA3221_ReadCurrent, XST_SUCCESS );
    vV80CurrentMonitor12VPEX( );
    will_return( __wrap_INA3221_ReadCurrent, XST_SUCCESS );
    vV80CurrentMonitor3v3PEX( );

    vV80PowerMonitor( );

    will_return( __wrap_INA3221_ReadCurrent, XST_SUCCESS );
    vV80CurrentMonitor12VPEX( );
    will_return( __wrap_INA3221_ReadVoltage, XST_SUCCESS );
    vV80VoltageMonitor12VPEX( );
    will_return( __wrap_INA3221_ReadVoltage, XST_SUCCESS );
    vV80VoltageMonitor3v3PEX( );
    will_return( __wrap_INA3221_ReadCurrent, XST_SUCCESS );
    vV80CurrentMonitor12VPEX( );
    will_return( __wrap_INA3221_ReadCurrent, XST_SUCCESS );
    vV80CurrentMonitor3v3PEX( );

    vV80PowerMonitor( );

    will_return( __wrap_INA3221_ReadCurrent, XST_SUCCESS );
    vV80CurrentMonitor12VPEX( );
    will_return( __wrap_INA3221_ReadVoltage, XST_SUCCESS );
    vV80VoltageMonitor12VPEX( );
    will_return( __wrap_INA3221_ReadVoltage, XST_SUCCESS );
    vV80VoltageMonitor3v3PEX( );
    will_return( __wrap_INA3221_ReadCurrent, XST_SUCCESS );
    vV80CurrentMonitor12VPEX( );
    will_return( __wrap_INA3221_ReadCurrent, XST_SUCCESS );
    vV80CurrentMonitor3v3PEX( );

    vV80PowerMonitor( );

    will_return( __wrap_INA3221_ReadCurrent, XST_SUCCESS );
    vV80CurrentMonitor12VPEX( );
    will_return( __wrap_INA3221_ReadVoltage, XST_SUCCESS );
    vV80VoltageMonitor12VPEX( );
    will_return( __wrap_INA3221_ReadVoltage, XST_SUCCESS );
    vV80VoltageMonitor3v3PEX( );
    will_return( __wrap_INA3221_ReadCurrent, XST_SUCCESS );
    vV80CurrentMonitor12VPEX( );
    will_return( __wrap_INA3221_ReadCurrent, XST_SUCCESS );
    vV80CurrentMonitor3v3PEX( );

    vV80PowerMonitor( );
}


static void test_scV80AsdmTemperatureReadVccint( void **state ) 
{
    ( void ) state; /* unused */
    s8 scStatus             = 0;
    snsrRead_t xSnsrData   = {0};

    scStatus = scV80AsdmTemperatureReadVccint( &xSnsrData );

    assert_true(  scStatus == XST_SUCCESS );
}

static void test_scV80AsdmTemperatureReadVccintThresholdCross( void **state ) 
{
    ( void ) state; /* unused */
    s8 scStatus             = 0;
    snsrRead_t xSnsrData   = {0};

    vSetNewValue( 0 );
    will_return( __wrap_LM75_ReadTemperature, XST_SUCCESS );
    will_return( __wrap_ucCAT34TS02ReadTemperature, XST_SUCCESS );
    will_return( __wrap_ucISL68221ReadTemperature0, XST_SUCCESS );
    will_return( __wrap_ucISL68221ReadTemperature0, XST_SUCCESS );
    will_return( __wrap_ucISL68221ReadTemperature1, XST_SUCCESS );
    will_return( __wrap_ucISL68221ReadTemperature2, XST_SUCCESS );
    vV80TemperatureMonitor();
    scStatus = scV80AsdmTemperatureReadVccint( &xSnsrData );

    assert_true(  scStatus == XST_SUCCESS );
}

static void test_scV80AsdmTemperatureReadVccintFail( void **state ) 
{
    ( void ) state; /* unused */
    s8 scStatus             = 0;
    snsrRead_t xSnsrData   = {0};

    /* Test 1 failed read */
    scStatus = scV80AsdmTemperatureReadVccint( NULL );

    assert_true(  scStatus == XST_FAILURE );
}

static void test_scV80AsdmTemperatureReadVccHbm( void **state ) 
{
    ( void ) state; /* unused */
    s8 scStatus             = 0;
    snsrRead_t xSnsrData   = {0};

    scStatus = scV80AsdmTemperatureReadVccHbm( &xSnsrData );

    assert_true(  scStatus == XST_SUCCESS );
}

static void test_scV80AsdmTemperatureReadVccHbmThresholdCross( void **state ) 
{
    ( void ) state; /* unused */
    s8 scStatus             = 0;
    snsrRead_t xSnsrData   = {0};

    scStatus = scV80AsdmTemperatureReadVccHbm( &xSnsrData );

    assert_true(  scStatus == XST_SUCCESS );
}

static void test_scV80AsdmTemperatureReadVccHbmFail( void **state ) 
{
    ( void ) state; /* unused */
    s8 scStatus             = 0;
    snsrRead_t xSnsrData   = {0};

    /* Test 1 failed read */
    scStatus = scV80AsdmTemperatureReadVccHbm( NULL );

    assert_true(  scStatus == XST_FAILURE );
}

static void test_scV80AsdmTemperatureRead1V2VccoDimm( void **state ) 
{
    ( void ) state; /* unused */
    s8 scStatus             = 0;
    snsrRead_t xSnsrData   = {0};

    scStatus = scV80AsdmTemperatureRead1V2VccoDimm( &xSnsrData );

    assert_true(  scStatus == XST_SUCCESS );
}


static void test_scV80AsdmTemperatureRead1V2VccoDimmFail( void **state ) 
{
    ( void ) state; /* unused */
    s8 scStatus             = 0;
    snsrRead_t xSnsrData   = {0};

    /* Test 1 failed read */
    scStatus = scV80AsdmTemperatureRead1V2VccoDimm( NULL );

    assert_true(  scStatus == XST_FAILURE );
}

static void test_scV80AsdmTemperatureRead1V2GTXAVTT( void **state ) 
{
    ( void ) state; /* unused */
    s8 scStatus             = 0;
    snsrRead_t xSnsrData   = {0};

    scStatus = scV80AsdmTemperatureRead1V2GTXAVTT( &xSnsrData );

    assert_true(  scStatus == XST_SUCCESS );
}


static void test_scV80AsdmTemperatureRead1V2GTXAVTTFail( void **state ) 
{
    ( void ) state; /* unused */
    s8 scStatus             = 0;
    snsrRead_t xSnsrData   = {0};

    /* Test 1 failed read */
    scStatus = scV80AsdmTemperatureRead1V2GTXAVTT( NULL );

    assert_true(  scStatus == XST_FAILURE );
}

static void test_scV80AsdmGetTemperatureNamesFail( void **state ) 
{
    ( void ) state; /* unused */
    s8 scStatus             = 0;
    u8 ucIndex              = 0;
    char8 ucSnsrName[30]    = {0};
    u8 ucSensorId           = 0;
    sensorMonitorFunc xSensorHandler = {0};

    /* Test 1 failed read */
    scStatus = scV80AsdmGetTemperatureNames( ucIndex, NULL, &ucSensorId, &xSensorHandler );

    assert_true(  scStatus == XST_FAILURE );
}

static void test_scV80AsdmGetCurrentNamesFail( void **state ) 
{
    ( void ) state; /* unused */
    s8 scStatus             = 0;
    u8 ucIndex              = 0;
    char8 ucSnsrName[30]    = {0};
    u8 ucSensorId           = 0;
    sensorMonitorFunc xSensorHandler = {0};

    /* Test 1 failed read */
    scStatus = scV80AsdmGetCurrentNames( ucIndex, NULL, &ucSensorId, &xSensorHandler );

    assert_true(  scStatus == XST_FAILURE );
}

static void test_scV80AsdmGetVoltageNamesFail( void **state ) 
{
    ( void ) state; /* unused */
    s8 scStatus             = 0;
    u8 ucIndex              = 0;
    char8 ucSnsrName[30]    = {0};
    u8 ucSensorId           = 0;
    sensorMonitorFunc xSensorHandler = {0};

    /* Test 1 failed read */
    scStatus = scV80AsdmGetVoltageNames( ucIndex, NULL, &ucSensorId, &xSensorHandler );

    assert_true(  scStatus == XST_FAILURE );
}

int main( void ) 
{
    srand( time( 0 ) );
    const struct CMUnitTest tests[] = 
    {
        cmocka_unit_test( test_scV80AsdmGetQSFPName ),
        cmocka_unit_test( test_scV80AsdmGetQSFPNameFail ),
        cmocka_unit_test( test_vV80GetSupportedSdrInfo ),
        cmocka_unit_test( test_vV80GetSupportedSdrInfoFail ),
        cmocka_unit_test( test_vV80AsdmUpdateRecordCount ),
        cmocka_unit_test( test_vV80AsdmUpdateRecordCountFail ),
        cmocka_unit_test( test_scV80AsdmTemperatureReadInlet ),
        cmocka_unit_test( test_scV80AsdmTemperatureReadInletFail ),
        cmocka_unit_test( test_scV80AsdmTemperatureReadOutlet ),
        cmocka_unit_test( test_scV80AsdmTemperatureReadOutletFail ),
        cmocka_unit_test( test_scV80AsdmTemperatureReadBoard ),
        cmocka_unit_test( test_scV80AsdmTemperatureReadBoardFail ),
        cmocka_unit_test( test_scV80TemperatureReadVCCHBM ),
        cmocka_unit_test( test_scV80TemperatureReadVCCHBMFail ),
        cmocka_unit_test( test_scV80TemperatureReadVCCODIMM ),
        cmocka_unit_test( test_scV80TemperatureReadVCCODIMMFail ),
        cmocka_unit_test( test_scV80TemperatureReadGTXAVTT ),
        cmocka_unit_test( test_scV80TemperatureReadGTXAVTTFail ),
        cmocka_unit_test( test_vV80TemperatureMonitor ),
        cmocka_unit_test( test_scV80AsdmTemperatureReadQSFP ),
        cmocka_unit_test( test_scV80AsdmTemperatureReadQSFPFail ),
        cmocka_unit_test( test_scV80AsdmTemperatureReadQSFPThreshold ),
        cmocka_unit_test( test_vV80QSFPMonitor ),
        cmocka_unit_test( test_slV80VMCFetchBoardInfo ),
        cmocka_unit_test( test_slV80VMCFetchBoardInfoFail ),
        cmocka_unit_test( test_vV80VoltageMonitor12VPEX ),
        cmocka_unit_test( test_vV80VoltageMonitor12VPEXFail ),
        cmocka_unit_test( test_vV80CurrentMonitor12VPEX ),
        cmocka_unit_test( test_vV80CurrentMonitor12VPEXFail ),
        cmocka_unit_test( test_vV80VoltageMonitor3v3PEX ),
        cmocka_unit_test( test_vV80VoltageMonitor3v3PEXFail ),
        cmocka_unit_test( test_vV80CurrentMonitor3v3PEX ),
        cmocka_unit_test( test_vV80CurrentMonitor3v3PEXFail ),
        cmocka_unit_test( test_vV80VoltageMonitor1V5VCCAUX ),
        cmocka_unit_test( test_vV80VoltageMonitor1V5VCCAUXFail ),
        cmocka_unit_test( test_vV80CurrentMonitor1V5VCCAUX ),
        cmocka_unit_test( test_vV80CurrentMonitor1V5VCCAUXFail ),
        cmocka_unit_test( test_vV80VoltageMonitor3V3QSFP ),
        cmocka_unit_test( test_vV80VoltageMonitor3V3QSFPFail ),
        cmocka_unit_test( test_vV80CurrentMonitor3V3QSFP ),
        cmocka_unit_test( test_vV80CurrentMonitor3V3QSFPFail ),
        cmocka_unit_test( test_vV80VoltageMonitor12VAUX0 ),
        cmocka_unit_test( test_vV80VoltageMonitor12VAUX0Fail ),
        cmocka_unit_test( test_vV80CurrentMonitor12VAUX0 ),
        cmocka_unit_test( test_vV80CurrentMonitor12VAUX0Fail ),
        cmocka_unit_test( test_vV80VoltageMonitor12VAUX1 ),
        cmocka_unit_test( test_vV80VoltageMonitor12VAUX1Fail ),
        cmocka_unit_test( test_vV80CurrentMonitor12VAUX1 ),
        cmocka_unit_test( test_vV80CurrentMonitor12VAUX1Fail ),
        cmocka_unit_test( test_vV80VoltageMonitorVccHBM ),
        cmocka_unit_test( test_vV80VoltageMonitorVccHBMFail ),
        cmocka_unit_test( test_vV80CurrentMonitorVccHBM ),
        cmocka_unit_test( test_vV80CurrentMonitorVccHBMFail ),
        cmocka_unit_test( test_vV80VoltageMonitor1V2VccoDimm ),
        cmocka_unit_test( test_vV80VoltageMonitor1V2VccoDimmFail ),
        cmocka_unit_test( test_vV80CurrentMonitor1V2VccoDimm ),
        cmocka_unit_test( test_vV80CurrentMonitor1V2VccoDimmFail ),
        cmocka_unit_test( test_vV80VoltageMonitor1V2GTXAVTT ),
        cmocka_unit_test( test_vV80VoltageMonitor1V2GTXAVTTFail ),
        cmocka_unit_test( test_vV80CurrentMonitor1V2GTXAVTT ),
        cmocka_unit_test( test_vV80CurrentMonitor1V2GTXAVTTFail ),
        cmocka_unit_test( test_vV80VoltageMonitorVccint ),
        cmocka_unit_test( test_vV80VoltageMonitorVccintFail ),
        cmocka_unit_test( test_vV80CurrentMonitorVccint ),
        cmocka_unit_test( test_vV80CurrentMonitorVccintFail ),
        cmocka_unit_test( test_scV80AsdmReadPower ),
        cmocka_unit_test( test_scV80AsdmReadPowerFail ),
        cmocka_unit_test( test_scV80AsdmReadVoltage12v ),
        cmocka_unit_test( test_scV80AsdmReadVoltage12vZero ),
        cmocka_unit_test( test_scV80AsdmReadVoltage12vFail ),
        cmocka_unit_test( test_scV80AsdmReadVoltage3v3 ),
        cmocka_unit_test( test_scV80AsdmReadVoltage3v3Zero ),
        cmocka_unit_test( test_scV80AsdmReadVoltage3v3Fail ),
        cmocka_unit_test( test_scV80AsdmReadVoltageVccint ),
        cmocka_unit_test( test_scV80AsdmReadVoltageVccintZero ),
        cmocka_unit_test( test_scV80AsdmReadVoltageVccintFail ),
        cmocka_unit_test( test_scV80AsdmReadVoltage12VAUX0 ),
        cmocka_unit_test( test_scV80AsdmReadVoltage12VAUX0Zero ),
        cmocka_unit_test( test_scV80AsdmReadVoltage12VAUX0Fail ),
        cmocka_unit_test( test_scV80AsdmReadVoltage12VAUX1 ),
        cmocka_unit_test( test_scV80AsdmReadVoltage12VAUX1Zero ),
        cmocka_unit_test( test_scV80AsdmReadVoltage12VAUX1Fail ),
        cmocka_unit_test( test_scV80AsdmReadVoltage1V5VCCAUX ),
        cmocka_unit_test( test_scV80AsdmReadVoltage1V5VCCAUXZero ),
        cmocka_unit_test( test_scV80AsdmReadVoltage1V5VCCAUXFail ),
        cmocka_unit_test( test_scV80AsdmReadVoltage3V3QSFP ),
        cmocka_unit_test( test_scV80AsdmReadVoltage3V3QSFPZero ),
        cmocka_unit_test( test_scV80AsdmReadVoltage3V3QSFPFail ),
        cmocka_unit_test( test_scV80AsdmReadVoltageVccHBM ),
        cmocka_unit_test( test_scV80AsdmReadVoltageVccHBMZero ),
        cmocka_unit_test( test_scV80AsdmReadVoltageVccHBMFail ),
        cmocka_unit_test( test_scV80AsdmReadVoltage1V2VccoDimm ),
        cmocka_unit_test( test_scV80AsdmReadVoltage1V2VccoDimmZero ),
        cmocka_unit_test( test_scV80AsdmReadVoltage1V2VccoDimmFail ),
        cmocka_unit_test( test_scV80AsdmReadVoltage1V2GTXAVT ),
        cmocka_unit_test( test_scV80AsdmReadVoltage1V2GTXAVTZero ),
        cmocka_unit_test( test_scV80AsdmReadVoltage1V2GTXAVTFail ),
        cmocka_unit_test( test_scV80AsdmReadCurrent12v ),
        cmocka_unit_test( test_scV80AsdmReadCurrent12vZero ),
        cmocka_unit_test( test_scV80AsdmReadCurrent12vFail ),
        cmocka_unit_test( test_scV80AsdmReadCurrent3v3 ),
        cmocka_unit_test( test_scV80AsdmReadCurrent3v3Zero ),
        cmocka_unit_test( test_scV80AsdmReadCurrent3v3Fail ),
        cmocka_unit_test( test_scV80AsdmReadCurrentVccint ),
        cmocka_unit_test( test_scV80AsdmReadCurrentVccintZero ),
        cmocka_unit_test( test_scV80AsdmReadCurrentVccintFail ),
        cmocka_unit_test( test_scV80AsdmReadCurrent12VAUX0 ),
        cmocka_unit_test( test_scV80AsdmReadCurrent12VAUX0Zero ),
        cmocka_unit_test( test_scV80AsdmReadCurrent12VAUX0Fail ),
        cmocka_unit_test( test_scV80AsdmReadCurrent12VAUX1 ),
        cmocka_unit_test( test_scV80AsdmReadCurrent12VAUX1Zero ),
        cmocka_unit_test( test_scV80AsdmReadCurrent12VAUX1Fail ),
        cmocka_unit_test( test_scV80AsdmReadCurrent1V5VCCAUX ),
        cmocka_unit_test( test_scV80AsdmReadCurrent1V5VCCAUXZero ),
        cmocka_unit_test( test_scV80AsdmReadCurrent1V5VCCAUXFail ),
        cmocka_unit_test( test_scV80AsdmReadCurrent3V3QSFP ),
        cmocka_unit_test( test_scV80AsdmReadCurrent3V3QSFPZero ),
        cmocka_unit_test( test_scV80AsdmReadCurrent3V3QSFPFail ),
        cmocka_unit_test( test_scV80AsdmReadCurrentVccHBM ),
        cmocka_unit_test( test_scV80AsdmReadCurrentVccHBMZero ),
        cmocka_unit_test( test_scV80AsdmReadCurrentVccHBMFail ),
        cmocka_unit_test( test_scV80AsdmReadCurrent1V2VccoDimm ),
        cmocka_unit_test( test_scV80AsdmReadCurrent1V2VccoDimmZero ),
        cmocka_unit_test( test_scV80AsdmReadCurrent1V2VccoDimmFail ),
        cmocka_unit_test( test_scV80AsdmReadCurrent1V2GTXAVT ),
        cmocka_unit_test( test_scV80AsdmReadCurrent1V2GTXAVTZero ),
        cmocka_unit_test( test_scV80AsdmReadCurrent1V2GTXAVTFail ),
        cmocka_unit_test( test_vV80PowerMonitor ),
        cmocka_unit_test( test_vV80PowerMonitorZero ),
        cmocka_unit_test( test_vV80PowerMonitorThresholdCross ), 
        cmocka_unit_test( test_scV80AsdmTemperatureReadVccint ),
        cmocka_unit_test( test_scV80AsdmTemperatureReadVccintThresholdCross ),
        cmocka_unit_test( test_scV80AsdmTemperatureReadVccintFail ),
        cmocka_unit_test( test_scV80AsdmTemperatureReadVccHbm ),
        cmocka_unit_test( test_scV80AsdmTemperatureReadVccHbmThresholdCross ),
        cmocka_unit_test( test_scV80AsdmTemperatureReadVccHbmFail ),      
        cmocka_unit_test( test_scV80AsdmTemperatureRead1V2VccoDimm ),
        cmocka_unit_test( test_scV80AsdmTemperatureRead1V2VccoDimmFail ),
        cmocka_unit_test( test_scV80AsdmTemperatureRead1V2GTXAVTT ),
        cmocka_unit_test( test_scV80AsdmTemperatureRead1V2GTXAVTTFail ),
        cmocka_unit_test( test_scV80AsdmGetTemperatureNamesFail ),
        cmocka_unit_test( test_scV80AsdmGetCurrentNamesFail ),
        cmocka_unit_test( test_scV80AsdmGetVoltageNamesFail ),
    };

    return cmocka_run_group_tests( tests, NULL, NULL );
}
