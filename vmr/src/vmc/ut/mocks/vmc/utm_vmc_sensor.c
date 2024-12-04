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

#include "platforms/vck5000.h"
#include "platforms/v70.h"

/* VCK5000 Specific sensors */
#define VCK5000_TEMPERATURE_SENSORS_INSTANCES   ( 0 )
#define VCK5000_CURRENT_SENSORS_INSTANCES       ( 3 )
#define VCK5000_VOLTAGE_SENSORS_INSTANCES       ( 5 )

/* V70 Specific sensors */
#define V70_TEMPERATURE_SENSORS_INSTANCES   ( 0 )
#define V70_CURRENT_SENSORS_INSTANCES       ( 3 )
#define V70_VOLTAGE_SENSORS_INSTANCES       ( 3 )

/* V80 Specific sensors */
#define V80_TEMPERATURE_SENSORS_INSTANCES   ( 3 )
#define V80_CURRENT_SENSORS_INSTANCES       ( 10 )
#define V80_VOLTAGE_SENSORS_INSTANCES       ( 10 )

#define MIN_TEMPERATURE_SENSORS_INSTANCES   ( 0 )
#define MIN_CURRENT_SENSORS_INSTANCES       ( 3 )
#define MIN_VOLTAGE_SENSORS_INSTANCES       ( 3 )

sensorMonitorFunc Temperature_Read_Board_Ptr;
sensorMonitorFunc Temperature_Read_QSFP_Ptr;
sensorMonitorFunc Temperature_Read_VCCINT_Ptr;
sensorMonitorFunc Power_Read_Ptr;

snsrNameFunc Temperature_Read_Ptr;
snsrNameFunc Voltage_Read_Ptr;
snsrNameFunc Current_Read_Ptr;
snsrNameFunc QSFP_Read_Ptr;

platform_sensors_monitor_ptr Monitor_Sensors;
Vmc_Sensors_Gl_t sensor_glvr;

clk_throttling_params_t g_clk_throttling_params;

/*****************************Real function definitions*******************************/
/*This definition is same as real implementation.
 * It does not need mock features, since it is very simple definition*/
s8 Temperature_Read_Board( snsrRead_t *snsrData )
{
    if( Temperature_Read_Board_Ptr == NULL )
    {
        return XST_SUCCESS;
    }

    return ( *Temperature_Read_Board_Ptr )( snsrData );
}

s8 Temperature_Read_QSFP( snsrRead_t *snsrData )
{
    if( Temperature_Read_QSFP_Ptr == NULL )
    {
        return XST_SUCCESS;
    }

    return ( *Temperature_Read_QSFP_Ptr )( snsrData );
}

s8 Temperature_Read_VCCINT( snsrRead_t *snsrData )
{
    if( Temperature_Read_VCCINT_Ptr == NULL )
    {
        return XST_SUCCESS;
    }

    return ( *Temperature_Read_VCCINT_Ptr )( snsrData );
}

s8 Asdm_Read_Power( snsrRead_t *snsrData )
{
    if( NULL == Power_Read_Ptr )
    {
        return XST_SUCCESS;
    }

    return ( *Power_Read_Ptr )( snsrData );
}

s8 getVoltagesName( u8 index, char8* snsrName, u8 *sensorId,sensorMonitorFunc *sensor_handler )
{
    if( NULL == Voltage_Read_Ptr )
    {
        return XST_SUCCESS;
    }

    return ( *Voltage_Read_Ptr )( index,snsrName,sensorId,sensor_handler );
}

s8 getCurrentNames( u8 index, char8* snsrName, u8 *sensorId,sensorMonitorFunc *sensor_handler )
{
    if( NULL == Current_Read_Ptr )
    {
        return XST_SUCCESS;
    }

    return ( *Current_Read_Ptr )( index,snsrName,sensorId,sensor_handler );
}

s8 scGetTemperatureName( u8 ucIndex, char8* pucSnsrName, u8 *pucSensorId, sensorMonitorFunc *pxSensorHandler )
{
    if( NULL == Temperature_Read_Ptr )
    {
        return XST_SUCCESS;
    }

    return ( *Temperature_Read_Ptr )( ucIndex, pucSnsrName, pucSensorId, pxSensorHandler );
}

s8 getQSFPName( u8 index, char8* snsrName, u8 *sensorId,sensorMonitorFunc *sensor_handler )
{
    if( NULL == QSFP_Read_Ptr )
    {
        return XST_SUCCESS;
    }

    return ( *QSFP_Read_Ptr )( index,snsrName,sensorId,sensor_handler );
}

u8 getVoltageSensorNum( void )
{
    if( Vmc_Get_PlatformType( ) == eVCK5000 )
    {
    return VCK5000_VOLTAGE_SENSORS_INSTANCES;
    }
    else if( Vmc_Get_PlatformType( ) == eV70 )
    {
        return V70_VOLTAGE_SENSORS_INSTANCES;
    }
    else if( Vmc_Get_PlatformType( ) == eV80 )
    {
        return V80_VOLTAGE_SENSORS_INSTANCES;
    }

    return MIN_VOLTAGE_SENSORS_INSTANCES;
}

u8 getCurrentSensorNum( void )
{
    if( Vmc_Get_PlatformType( ) == eVCK5000 )
    {
        return VCK5000_CURRENT_SENSORS_INSTANCES;
    }
    else if( Vmc_Get_PlatformType( ) == eV70 )
    {
        return V70_CURRENT_SENSORS_INSTANCES;
    }
    else if( Vmc_Get_PlatformType( ) == eV80 )
    {
        return V80_CURRENT_SENSORS_INSTANCES;
    }

    return MIN_CURRENT_SENSORS_INSTANCES;
}

u8 ucGetTemperatureSensorNum( void )
{
    if( Vmc_Get_PlatformType( ) == eVCK5000 )
    {
        return VCK5000_TEMPERATURE_SENSORS_INSTANCES;
    }
    else if( Vmc_Get_PlatformType( ) == eV70 )
    {
        return V70_TEMPERATURE_SENSORS_INSTANCES;
    }
    else if( Vmc_Get_PlatformType( ) == eV80 )
    {
        return V80_TEMPERATURE_SENSORS_INSTANCES;
    }

    return MIN_TEMPERATURE_SENSORS_INSTANCES;
}

/*****************************Mock functions *******************************/
void __wrap_ucs_clock_shutdown( )
{
    /*Do Nothing for now*/
}

s8 __wrap_Temperature_Read_ACAP_Device_Sysmon( snsrRead_t *snsrData )
{
    s8 status = XST_SUCCESS;
    float TempReading = 71;

    u16 roundedOffVal = ( TempReading > 0 ) ? TempReading : 0;
    Cl_SecureMemcpy( &snsrData->snsrValue[0],sizeof( roundedOffVal ),&roundedOffVal,sizeof( roundedOffVal ) );
    snsrData->sensorValueSize = sizeof( roundedOffVal );
    snsrData->snsrSatus = Vmc_Snsr_State_Normal;

    return status;
}

s8 __wrap_VCCINT_Read_ACAP_Device_Sysmon( snsrRead_t *snsrData )
{
    s8 status = XST_SUCCESS;
    float VCCINT_reading = 5;
    u32 VCCINT_mv = 0;
    VCCINT_mv = VCCINT_reading *1000 ;

    Cl_SecureMemcpy( &snsrData->snsrValue[0], ( sizeof( u8 )*4 ), &VCCINT_mv,sizeof( VCCINT_mv ) );
    snsrData->sensorValueSize = sizeof( VCCINT_mv );
    snsrData->snsrSatus = Vmc_Snsr_State_Normal;

    return status;
}

s8 __wrap_PMBUS_SC_Sensor_Read( snsrRead_t *snsrData )
{
    s8 status = XST_SUCCESS;
    u16 sensorReading = 10;

    Cl_SecureMemcpy( &snsrData->snsrValue[0],sizeof( sensorReading ),&sensorReading,sizeof( sensorReading ) );
    snsrData->sensorValueSize = sizeof( sensorReading );
    snsrData->snsrSatus = Vmc_Snsr_State_Normal;

    return status;
}

s8 __wrap_PMBUS_SC_Vccint_Read( snsrRead_t *snsrData )
{
    s8 status = XST_SUCCESS;
    u32 vccint_i_sensorReading = 75;

    memcpy( &snsrData->snsrValue[0],&vccint_i_sensorReading,sizeof( vccint_i_sensorReading ) );
    snsrData->sensorValueSize = sizeof( vccint_i_sensorReading );
    snsrData->snsrSatus = Vmc_Snsr_State_Normal;

    return status;
}

