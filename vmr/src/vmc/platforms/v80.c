/******************************************************************************
* Copyright ( C ) 2023 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#include "cl_vmc.h"
#include "cl_mem.h"
#include "cl_i2c.h"
#include "../vmc_api.h"
#include "v80.h"
#include "cl_i2c.h"
#include "../sensors/inc/lm75.h"
#include "../sensors/inc/ina3221.h"
#include "../sensors/inc/isl68221.h"
#include "../sensors/inc/cat34ts02.h"
#include "../sensors/inc/tca6416a.h"
#include "../vmc_main.h"
#include "vmr_common.h"
#include "../vmc_sc_comms.h"
#include "../vmc_sensors.h"
#include "../clock_throttling.h"

#define SLAVE_ADDRESS_LM75_0_V80  ( 0x48 )

#define SLAVE_ADDRESS_INA3221_0   ( 0x40 )
#define SLAVE_ADDRESS_INA3221_1   ( 0x41 )
#define SLAVE_ADDRESS_ISL68221_0  ( 0x60 )
#define SLAVE_ADDRESS_ISL68221_1  ( 0x61 )

extern Vmc_Sensors_Gl_t sensor_glvr;
extern msg_id_ptr msg_id_handler_ptr;
extern Fetch_BoardInfo_Func fetch_boardinfo_ptr;
extern clk_throttling_params_t g_clk_throttling_params;

static u8 ucI2cMain = LPD_I2C_0;

#define MAX( a,b )    ( ( a>b ) ? a : b )

#define V80_NUM_BOARD_INFO_SENSORS      ( 11 )
#define V80_NUM_TEMPERATURE_SENSORS     ( 9 )
#define V80_NUM_SC_VOLTAGE_SENSORS      ( 10 )
#define V80_NUM_SC_CURRENT_SENSORS      ( 10 )
#define V80_NUM_POWER_SENSORS           ( 1 )

extern platform_sensors_monitor_ptr Monitor_Sensors;
extern supported_sdr_info_ptr get_supported_sdr_info;
extern asdm_update_record_count_ptr asdm_update_record_count;

Clock_Throttling_Profile_t xV80ClockThrottlingProfile;
extern Clock_Throttling_Handle_t clock_throttling_std_algorithm;

void vV80VoltageMonitor12VPEX( void );
void vV80VoltageMonitor3v3PEX( void );
void vV80VoltageMonitorVccint( void );
void vV80VoltageMonitor1V5VCCAUX( void );
void vV80VoltageMonitor3V3QSFP( void );
void vV80VoltageMonitor12VAUX0( void );
void vV80VoltageMonitor12VAUX1( void );
void vV80CurrentMonitor12VPEX( void );
void vV80CurrentMonitor3v3PEX( void );
void vV80CurrentMonitorVccint( void );
void vV80CurrentMonitor1V5VCCAUX( void );
void vV80CurrentMonitor3V3QSFP( void );
void vV80CurrentMonitor12VAUX0( void );
void vV80CurrentMonitor12VAUX1( void );

static AsdmHeader_info_t xV80ASDMHeaderInfo[] = 
{
        { BoardInfoSDR,      V80_NUM_BOARD_INFO_SENSORS },
        { TemperatureSDR,    V80_NUM_TEMPERATURE_SENSORS },
        { VoltageSDR,        V80_NUM_SC_VOLTAGE_SENSORS },
        { CurrentSDR,        V80_NUM_SC_CURRENT_SENSORS },
        { PowerSDR,          V80_NUM_POWER_SENSORS },
};
#define MAX_SDR_REPO    ( sizeof( xV80ASDMHeaderInfo )/sizeof( xV80ASDMHeaderInfo[0] ) )

static u32 ulV80SupportedSensors[] = {
    eProduct_Name,
    eSerial_Number,
    ePart_Number,
    eRevision,
    eMfg_Date,
    eUUID,
    eMAC_0,
    eFpga_Fan_1,
    eActive_SC_Ver,
    eTarget_SC_Ver,
    eOEM_Id,

    /* Temperature SDR */
    eTemp_Board,
    eTemp_Vccint,
    eTemp_Group_Sensors,
    eTemp_Qsfp,

    /* Voltage SDR */
    eVoltage_Group_Sensors,

    /* Current SDR */
    eCurrent_Group_Sensors,

    /* Power SDR */
    ePower_Total
};

s8 scV80AsdmGetQSFPName( u8 ucIndex, char8* pucSnsrName, u8 *pucSensorId, sensorMonitorFunc *pxSensorHandler )
{
    struct sensorData
    {
        u8                  ucSensorId;
        char8               ucSensorName[SENSOR_NAME_MAX];
        sensorMonitorFunc   xSensorHandler;
    };

    struct sensorData qsfpData[] =
    {
        { 0,  TEMP_CAGE0_NAME, scV80AsdmTemperatureReadQSFP },
        { 1,  TEMP_CAGE1_NAME, scV80AsdmTemperatureReadQSFP },
        { 2,  TEMP_CAGE2_NAME, scV80AsdmTemperatureReadQSFP },
        { 3,  TEMP_CAGE3_NAME, scV80AsdmTemperatureReadQSFP }
    };

    if( ( sizeof( qsfpData ) / sizeof( struct sensorData ) ) <= ucIndex )
    {
        return XST_FAILURE;
    }

    if( NULL != pucSnsrName )
    {
        Cl_SecureMemcpy( pucSnsrName, SENSOR_NAME_MAX, &qsfpData[ucIndex].ucSensorName[0], SENSOR_NAME_MAX );
    }
    else
    {
        return XST_FAILURE;
    }

    if( NULL != pucSensorId )
    {
        *pucSensorId = qsfpData[ucIndex].ucSensorId;
    }
    else
    {
        return XST_FAILURE;
    }

    if( NULL != pxSensorHandler )
    {
        *pxSensorHandler = qsfpData[ucIndex].xSensorHandler;
    }
    else
    {
        return XST_FAILURE;
    }

    return XST_SUCCESS;
}

void vV80GetSupportedSdrInfo( u32 *pulPlatformSupportedSensors, u32 *pulSdrCount )
{
    if( ( NULL != pulPlatformSupportedSensors ) &&
        ( NULL != pulSdrCount ) )
    {
        *pulSdrCount = ( sizeof( ulV80SupportedSensors ) / sizeof( ulV80SupportedSensors[0] ) );
        Cl_SecureMemcpy( pulPlatformSupportedSensors, sizeof( ulV80SupportedSensors ),
                ulV80SupportedSensors,sizeof( ulV80SupportedSensors ) );
    }
    
    return;
}

void vV80AsdmUpdateRecordCount( Asdm_Header_t *pxHeaderInfo )
{
    u8 i = 0;

    if( NULL != pxHeaderInfo )
    {
        for( i = 0; i < MAX_SDR_REPO; i++ )
        {
            if( pxHeaderInfo[i].repository_type == xV80ASDMHeaderInfo[i].record_type )
            {
                pxHeaderInfo[i].no_of_records = xV80ASDMHeaderInfo[i].record_count;
            }
        }
    }
    return;
}

s8 scV80AsdmTemperatureReadInlet( snsrRead_t *pxSnsrData )
{
    s8 scStatus = XST_FAILURE;
    s16 ssTempValue = 0;

    if( NULL != pxSnsrData )
    {
        scStatus = LM75_ReadTemperature( ucI2cMain, SLAVE_ADDRESS_LM75_0_V80, &ssTempValue );
        if( XST_SUCCESS == scStatus )
        {
            Cl_SecureMemcpy( pxSnsrData->snsrValue,sizeof( ssTempValue ),&ssTempValue,sizeof( ssTempValue ) );
            pxSnsrData->sensorValueSize = sizeof( ssTempValue );
            pxSnsrData->snsrSatus = Vmc_Snsr_State_Normal;
        }
        else
        {
            pxSnsrData->snsrSatus = Vmc_Snsr_State_Comms_failure;
            VMC_DBG( "Failed to read slave : 0x%0.2x \n\r", SLAVE_ADDRESS_LM75_0_V80 );
        }
    }
    
    return scStatus;
}

/* This temperature sensor is on the CAT34TS02 EEPROM */
s8 scV80AsdmTemperatureReadOutlet( snsrRead_t *pxSnsrData )
{
    s8 scStatus = XST_FAILURE;
    s16 ssTempValue = 0;

    if( NULL != pxSnsrData )
    {
        scStatus = ucCAT34TS02ReadTemperature( ucI2cMain, CAT34TS02_SLAVE_ADDRESS_TEMPERATURE, &ssTempValue );
        if( XST_SUCCESS == scStatus )
        {
            Cl_SecureMemcpy( pxSnsrData->snsrValue,sizeof( ssTempValue ),&ssTempValue,sizeof( ssTempValue ) );
            pxSnsrData->sensorValueSize = sizeof( ssTempValue );
            pxSnsrData->snsrSatus = Vmc_Snsr_State_Normal;
        }
        else
        {
            pxSnsrData->snsrSatus = Vmc_Snsr_State_Comms_failure;
            VMC_DBG( "Failed to read slave : 0x%.2x \n\r", CAT34TS02_SLAVE_ADDRESS_TEMPERATURE );
        }
    }
    
    return scStatus;
}

s8 scV80AsdmTemperatureReadBoard( snsrRead_t *pxSnsrData )
{
    s8 scStatus = XST_FAILURE;
    s16 ssTempReading = 0;
    if( NULL != pxSnsrData )
    {
        ssTempReading = MAX( sensor_glvr.sensor_readings.board_temp[0], sensor_glvr.sensor_readings.board_temp[1] );

        Cl_SecureMemcpy( &pxSnsrData->snsrValue[0],sizeof( ssTempReading ),&ssTempReading,sizeof( ssTempReading ) );
        pxSnsrData->sensorValueSize = sizeof( ssTempReading );
        pxSnsrData->snsrSatus = Vmc_Snsr_State_Normal;

        scStatus = XST_SUCCESS;
    }
    
    return scStatus;
}

s8 scV80TemperatureReadVCCHBM( snsrRead_t *pxSnsrData )
{
    s8 scStatus         = XST_FAILURE;
    s16 ssTempValue     = 0;
    float fTempValue    = 0;

    if( NULL != pxSnsrData )
    {
        scStatus =  ucISL68221ReadTemperature0( ucI2cMain, SLAVE_ADDRESS_ISL68221_1, &fTempValue );
        ssTempValue = ( s16 )fTempValue;
        if( XST_SUCCESS == scStatus )
        {
            Cl_SecureMemcpy( pxSnsrData->snsrValue,sizeof( ssTempValue ),&ssTempValue,sizeof( ssTempValue ) );
            pxSnsrData->sensorValueSize = sizeof( ssTempValue );
            pxSnsrData->snsrSatus = Vmc_Snsr_State_Normal;
        }
        else
        {
            pxSnsrData->snsrSatus = Vmc_Snsr_State_Comms_failure;
            VMC_DBG( "Failed to read slave : 0x%.2x \n\r", SLAVE_ADDRESS_ISL68221_1 );
        }
    }
    
    return scStatus;
}

s8 scV80TemperatureReadVCCODIMM( snsrRead_t *pxSnsrData )
{
    s8 scStatus         = XST_FAILURE;
    s16 ssTempValue     = 0;
    float fTempValue    = 0;

    if( NULL != pxSnsrData )
    {
        scStatus =  ucISL68221ReadTemperature1( ucI2cMain, SLAVE_ADDRESS_ISL68221_1, &fTempValue );
        ssTempValue = ( s16 )fTempValue;
        if( XST_SUCCESS == scStatus )
        {
            Cl_SecureMemcpy( pxSnsrData->snsrValue,sizeof( ssTempValue ),&ssTempValue,sizeof( ssTempValue ) );
            pxSnsrData->sensorValueSize = sizeof( ssTempValue );
            pxSnsrData->snsrSatus = Vmc_Snsr_State_Normal;
        }
        else
        {
            pxSnsrData->snsrSatus = Vmc_Snsr_State_Comms_failure;
            VMC_DBG( "Failed to read slave : 0x%.2x \n\r", SLAVE_ADDRESS_ISL68221_1 );
        }
    }
    
    return scStatus;
}

s8 scV80TemperatureReadGTXAVTT( snsrRead_t *pxSnsrData )
{
    s8 scStatus         = XST_FAILURE;
    s16 ssTempValue     = 0;
    float fTempValue    = 0;

    if( NULL != pxSnsrData )
    {
        scStatus =  ucISL68221ReadTemperature2( ucI2cMain, SLAVE_ADDRESS_ISL68221_1, &fTempValue );
        ssTempValue = ( s16 )fTempValue;
        if( XST_SUCCESS == scStatus )
        {
            Cl_SecureMemcpy( pxSnsrData->snsrValue,sizeof( ssTempValue ),&ssTempValue,sizeof( ssTempValue ) );
            pxSnsrData->sensorValueSize = sizeof( ssTempValue );
            pxSnsrData->snsrSatus = Vmc_Snsr_State_Normal;
        }
        else
        {
            pxSnsrData->snsrSatus = Vmc_Snsr_State_Comms_failure;
            VMC_DBG( "Failed to read slave : 0x%.2x \n\r", SLAVE_ADDRESS_ISL68221_1 );
        }
    }
    
    return scStatus;
}



void vV80TemperatureMonitor( void )
{
    u8 scStatus = XST_FAILURE;

    /* Temp sensor IO Bracket Area ( Inlet Temp ? )*/
    scStatus = LM75_ReadTemperature( ucI2cMain, SLAVE_ADDRESS_LM75_0_V80, &sensor_glvr.sensor_readings.board_temp[0] );
    if( XST_FAILURE == scStatus )
    {
        VMC_DBG( "Failed to read LM75_0 \n\r" );
    }

    /* Temp sensor on EEPROM - Retainer Bracket Area */
    scStatus = ucCAT34TS02ReadTemperature( ucI2cMain, CAT34TS02_SLAVE_ADDRESS_TEMPERATURE, &sensor_glvr.sensor_readings.board_temp[1] );
    if( XST_FAILURE == scStatus )
    {
        VMC_DBG( "Failed to read CAT34TS02 \n\r" );
    }

    /* VCCINT Temp*/
    scStatus =  ucISL68221ReadTemperature0( ucI2cMain, SLAVE_ADDRESS_ISL68221_0, &sensor_glvr.sensor_readings.vccint_temp );
    if( XST_FAILURE == scStatus )
    {
        VMC_DBG( "Failed to read Vccint temp from : 0x%x", SLAVE_ADDRESS_ISL68221_0 );
    }

    /* VCC_HBM Temp*/
    scStatus =  ucISL68221ReadTemperature0( ucI2cMain, SLAVE_ADDRESS_ISL68221_1, &sensor_glvr.sensor_readings.fVccHbmTemp );

    if( XST_FAILURE == scStatus )
    {
        VMC_DBG( "Failed to read VCC_HBM temp from : 0x%x", SLAVE_ADDRESS_ISL68221_1 );
    }

    /* 1V2 VCCO DIMM Temp*/
    scStatus =  ucISL68221ReadTemperature1( ucI2cMain, SLAVE_ADDRESS_ISL68221_1, &sensor_glvr.sensor_readings.f1V2VccoDimmTemp );

    if( XST_FAILURE == scStatus )
    {
        VMC_DBG( "Failed to read VCCO DIMM temp from : 0x%x", SLAVE_ADDRESS_ISL68221_1 );
    }

    /* GTXAVTT Temp*/
    scStatus =  ucISL68221ReadTemperature2( ucI2cMain, SLAVE_ADDRESS_ISL68221_1, &sensor_glvr.sensor_readings.f1V2GtxaVttTemp );

    if( XST_FAILURE == scStatus )
    {
        VMC_DBG( "Failed to read GTXAVTT temp from : 0x%x", SLAVE_ADDRESS_ISL68221_1 );
    }

    return;
}

s8 scV80AsdmTemperatureReadQSFP(snsrRead_t *pxSnsrData)
{
    u8 scStatus         = XST_SUCCESS;
    float fTempReading  = 0.0;

    if( NULL != pxSnsrData )
    {
        scStatus = ucQSFPI2CMuxReadTemperature(&fTempReading, pxSnsrData->sensorInstance);

        if( XST_SUCCESS == scStatus )
        {
            u16 roundedOffVal = (fTempReading > 0) ? fTempReading : 0;
            Cl_SecureMemcpy(&pxSnsrData->snsrValue[0],sizeof(roundedOffVal),&roundedOffVal,sizeof(roundedOffVal));
            pxSnsrData->sensorValueSize = sizeof(roundedOffVal);
            pxSnsrData->snsrSatus = Vmc_Snsr_State_Normal;
        }
        else if( XST_FAILURE == scStatus )
        {
            pxSnsrData->snsrSatus = Vmc_Snsr_State_Comms_failure;
            VMC_ERR("Failed to read slave : %d \n\r",QSFP_SLAVE_ADDRESS);
        }
        else
        {
            pxSnsrData->snsrSatus = Vmc_Snsr_State_Unavailable;
            VMC_DBG("QSFP_%d module not present \n\r",(pxSnsrData->sensorInstance));
        }

        if( TEMP_QSFP_CRITICAL_THRESHOLD <= fTempReading )
        {
            ucs_clock_shutdown();
        }
    }
    else
    {
        scStatus = XST_FAILURE;
    }
    return scStatus;
}

void vV80QSFPMonitor( void )
{
    u8 ucQSFPIndex          = 0;
    float fTemperatureValue = 0;
    u8 scStatus             = XST_FAILURE;

    for( ucQSFPIndex = 0; ucQSFPIndex < QSFP_TEMPERATURE_SENSOR_NUM; ucQSFPIndex++ )
    {
        scStatus = ucQSFPI2CMuxReadTemperature( &fTemperatureValue, ucQSFPIndex );
        if( XST_SUCCESS == scStatus )
        {
            sensor_glvr.sensor_readings.qsfp_temp[ucQSFPIndex] = fTemperatureValue;
        }
        if( XST_FAILURE == scStatus )
        {
            VMC_PRNT( "\n\r Failed to read QSFP_%d temp \n\r", ucQSFPIndex );
        }
        if( XST_DEVICE_NOT_FOUND == scStatus )
        {
            /* VMC_PRNT( "QSFP_%d module not present", ucQSFPIndex ); */
        }

    }
    return;
}

s32 slV80VMCFetchBoardInfo( u8 *board_snsr_data )
{
    s32 slDataSize             = -1;
    Versal_BoardInfo xBoardInfo = {0};
    /* ulByteCount will indicate the length of the response payload being generated */
    u32 ulByteCount             = 0;

    if( NULL != board_snsr_data )
    {
        ( void )VMC_Get_BoardInfo( &xBoardInfo );

        Cl_SecureMemcpy( board_snsr_data, sizeof( Versal_BoardInfo ), &xBoardInfo, sizeof( Versal_BoardInfo ) );
        ulByteCount = sizeof( Versal_BoardInfo );

        /* Check and return -1 if size of response is > 256 */
        slDataSize =  ( ulByteCount <= MAX_VMC_SC_UART_BUF_SIZE ) ? ( ulByteCount ) : ( -1 );
    }
    
    return ( slDataSize );
}

void vV80VoltageMonitor12VPEX( void )
{
    u8 scStatus     = XST_FAILURE;
    float fVoltage  = 0.0;

    scStatus = INA3221_ReadVoltage( ucI2cMain, SLAVE_ADDRESS_INA3221_0, 0, &fVoltage );
    if( XST_SUCCESS != scStatus )
    {
        VMC_ERR( "Failed to read 12vPex voltage" );
    }
    else
    {
        sensor_glvr.sensor_readings.voltage[e12V_PEX] = fVoltage;
    }  
}

void vV80CurrentMonitor12VPEX( void )
{
    u8 scStatus     = XST_FAILURE;
    float fCurrent  = 0.0;

    scStatus = INA3221_ReadCurrent( ucI2cMain, SLAVE_ADDRESS_INA3221_0, 0, &fCurrent );
    if( XST_SUCCESS != scStatus )
    {
        VMC_ERR( "Failed to read 12vPex current" );
    }
    else
    {
        sensor_glvr.sensor_readings.current[e12V_PEX] = fCurrent;
    }
}

void vV80VoltageMonitor3v3PEX( void )
{
    u8 scStatus     = XST_FAILURE;
    float fVoltage  = 0.0;

    scStatus = INA3221_ReadVoltage( ucI2cMain, SLAVE_ADDRESS_INA3221_0, 1, &fVoltage );
    if( XST_SUCCESS != scStatus )
    {
        VMC_ERR( "Failed to read 3v3 pex voltage" );
    }
    else
    {
        sensor_glvr.sensor_readings.voltage[e3V3_PEX] = fVoltage;
    }   
}

void vV80CurrentMonitor3v3PEX( void )
{
    u8 scStatus     = XST_FAILURE;
    float fCurrent  = 0.0;

    scStatus = INA3221_ReadCurrent( ucI2cMain, SLAVE_ADDRESS_INA3221_0, 1, &fCurrent );
    if( XST_SUCCESS != scStatus )
    {
        VMC_ERR( "Failed to read 3v3 Pex current" );
    }
    else
    {
        sensor_glvr.sensor_readings.current[e3V3_PEX] = fCurrent;
    }  
}

void vV80VoltageMonitor1V5VCCAUX( void )
{
    u8 scStatus     = XST_FAILURE;
    float fVoltage  = 0.0;

    scStatus = INA3221_ReadVoltage( ucI2cMain, SLAVE_ADDRESS_INA3221_0, 2, &fVoltage );
    if( XST_SUCCESS != scStatus )
    {
        VMC_ERR( "Failed to read 1V5_VCC_AUX voltage" );
    }
    else
    {
        sensor_glvr.sensor_readings.voltage[e1V5_VCC_AUX] = fVoltage;
    }
}

void vV80CurrentMonitor1V5VCCAUX( void )
{
    u8 scStatus     = XST_FAILURE;
    float fCurrent  = 0.0;

    scStatus = INA3221_ReadCurrent( ucI2cMain, SLAVE_ADDRESS_INA3221_0, 2, &fCurrent );
    if( XST_SUCCESS != scStatus )
    {
        VMC_ERR( "Failed to read 1V5_VCC_AUX current" );
    }
    else
    {
        sensor_glvr.sensor_readings.current[e1V5_VCC_AUX] = fCurrent;
    }
}

void vV80VoltageMonitor3V3QSFP( void )
{ 
    float fVoltage  = 0.0;
    u8 scStatus     = XST_FAILURE;
    scStatus = INA3221_ReadVoltage( ucI2cMain, SLAVE_ADDRESS_INA3221_1, 0, &fVoltage );
    if( XST_SUCCESS != scStatus )
    {
        VMC_ERR( "Failed to read 3V3_QSFP voltage" );
    }
    else
    {
        sensor_glvr.sensor_readings.voltage[e3V3_QSFP] = fVoltage;
    }

}

void vV80CurrentMonitor3V3QSFP( void )
{  
    float fCurrent  = 0.0;
    u8 scStatus     = XST_FAILURE;
    scStatus = INA3221_ReadCurrent( ucI2cMain, SLAVE_ADDRESS_INA3221_1, 0, &fCurrent );
    if( XST_SUCCESS != scStatus )
    {
        VMC_ERR( "Failed to read 3V3_QSFP current" );
    }
    else
    {
        sensor_glvr.sensor_readings.current[e3V3_QSFP] = fCurrent;
    }

}

void vV80VoltageMonitor12VAUX0( void )
{
    float fVoltage  = 0.0;
    u8 scStatus     = XST_FAILURE;
    scStatus = INA3221_ReadVoltage( ucI2cMain, SLAVE_ADDRESS_INA3221_1, 1, &fVoltage );
    if( XST_SUCCESS != scStatus )
    {
        VMC_ERR( "Failed to read 12V_AUX_0 voltage" );
    }
    else
    {
        sensor_glvr.sensor_readings.voltage[e12V_AUX0] = fVoltage;
    }
}

void vV80CurrentMonitor12VAUX0( void )
{
    float fCurrent  = 0.0;
    u8 scStatus     = XST_FAILURE;
    scStatus = INA3221_ReadCurrent( ucI2cMain, SLAVE_ADDRESS_INA3221_1, 1, &fCurrent );
    if( XST_SUCCESS != scStatus )
    {
        VMC_ERR( "Failed to read 12V_AUX_0 current" );
    }
    else
    {
        sensor_glvr.sensor_readings.current[e12V_AUX0] = fCurrent;
    }
}

void vV80VoltageMonitor12VAUX1( void )
{
    float fVoltage  = 0.0;
    u8 scStatus     = XST_FAILURE;
    scStatus = INA3221_ReadVoltage( ucI2cMain, SLAVE_ADDRESS_INA3221_1, 2, &fVoltage );
    if( XST_SUCCESS != scStatus )
    {
        VMC_ERR( "Failed to read 12V_AUX_1 voltage" );
    }
    else
    {
        sensor_glvr.sensor_readings.voltage[e12V_AUX1] = fVoltage;
    }
}

void vV80CurrentMonitor12VAUX1( void )
{
    float fCurrent  = 0.0;
    u8 scStatus     = XST_FAILURE;
    scStatus = INA3221_ReadCurrent( ucI2cMain, SLAVE_ADDRESS_INA3221_1, 2, &fCurrent );
    if( XST_SUCCESS != scStatus )
    {
        VMC_ERR( "Failed to read 12V_AUX_1 current" );
    }
    else
    {
        sensor_glvr.sensor_readings.current[e12V_AUX1] = fCurrent;
    } 
}

void vV80VoltageMonitorVccHBM( void )
{
    float fVoltage  = 0.0;
    u8 scStatus     = XST_FAILURE;
    scStatus =  ucISL68221ReadVoltage0( ucI2cMain, SLAVE_ADDRESS_ISL68221_1, &fVoltage );
    if( XST_SUCCESS != scStatus )
    {
        VMC_ERR( "Failed to read Vcc HBM Voltage " );
    }
    else
    {
        sensor_glvr.sensor_readings.voltage[eVCC_HBM] = fVoltage;
    }
}

void vV80CurrentMonitorVccHBM( void )
{
    float fCurrentInA   = 0.0;
    u8 scStatus         = XST_SUCCESS;
    scStatus =  ucISL68221ReadCurrent0( ucI2cMain, SLAVE_ADDRESS_ISL68221_1, &fCurrentInA );
    if( XST_SUCCESS != scStatus )
    {
        VMC_ERR( "Failed to read Vcc HBM Current " );
    }
    else
    {
        sensor_glvr.sensor_readings.current[eVCC_HBM] = fCurrentInA; /* In Amps */
    } 
}

void vV80VoltageMonitor1V2VccoDimm( void )
{
    float fVoltage  = 0.0;
    u8 scStatus     = XST_SUCCESS;
    scStatus =  ucISL68221ReadVoltage1( ucI2cMain, SLAVE_ADDRESS_ISL68221_1, &fVoltage );
    if( XST_SUCCESS != scStatus )
    {
        VMC_ERR( "Failed to read 1V2 Vcco Dimm Voltage " );
    }
    else
    {
        sensor_glvr.sensor_readings.voltage[e1V2_VCCO_DIMM] = fVoltage;
    }
}

void vV80CurrentMonitor1V2VccoDimm( void )
{
    float fCurrentInA   = 0.0;
    u8 scStatus         = XST_SUCCESS;
    scStatus =  ucISL68221ReadCurrent1( ucI2cMain, SLAVE_ADDRESS_ISL68221_1, &fCurrentInA );
    if( XST_SUCCESS != scStatus )
    {
        VMC_ERR( "Failed to read 1V2 Vcco Dimm Current " );
    }
    else
    {
        sensor_glvr.sensor_readings.current[e1V2_VCCO_DIMM] = fCurrentInA; /* In Amps */
    }
}

void vV80VoltageMonitor1V2GTXAVTT( void )
{
    float fVoltage  = 0.0;
    u8 scStatus     = XST_SUCCESS;
    scStatus =  ucISL68221ReadVoltage2( ucI2cMain, SLAVE_ADDRESS_ISL68221_1, &fVoltage );
    if( XST_SUCCESS != scStatus )
    {
        VMC_ERR( "Failed to read 1V2GTXAVT Voltage " );
    }
    else
    {
        sensor_glvr.sensor_readings.voltage[e1V2_GTXAVTT] = fVoltage;
    }
}

void vV80CurrentMonitor1V2GTXAVTT( void )
{
    float fCurrentInA   = 0.0;
    u8 scStatus         = XST_SUCCESS;
    scStatus =  ucISL68221ReadCurrent2( ucI2cMain, SLAVE_ADDRESS_ISL68221_1, &fCurrentInA );
    if( XST_SUCCESS != scStatus )
    {
        VMC_ERR( "Failed to read 1V2GTXAVT Current " );
    }
    else
    {
        sensor_glvr.sensor_readings.current[e1V2_GTXAVTT] = fCurrentInA; /* In Amps */
    }
}


void vV80PowerMonitor( void )
{
    float fPower12vPex = 0.0;
    float fPower3v3pex = 0.0;

    static u8 ucCount12vpex = 1;
    static u8 ucCount3v3pex = 1;

    fPower12vPex = ( sensor_glvr.sensor_readings.current[e12V_PEX]/1000.0 ) * ( sensor_glvr.sensor_readings.voltage[e12V_PEX]/1000.0 );
    fPower3v3pex = ( sensor_glvr.sensor_readings.current[e3V3_PEX]/1000.0 ) * ( sensor_glvr.sensor_readings.voltage[e3V3_PEX]/1000.0 );

    /*
     * If any of 12v or 3v3 pex is zero that means Vmc_Snsr_State_Comms_failure occurred,
     * so set total_power to zero.
     * */
    if( fPower12vPex == 0 || fPower3v3pex == 0 )
    {
        sensor_glvr.sensor_readings.total_power = 0;
    }
    else
    {
        sensor_glvr.sensor_readings.total_power = ( fPower12vPex + fPower3v3pex );
    }

    /* shutdown clock only if power reached critical threshold continuously for 1sec ( 100ms*10 ) */
    if( V80_POWER_12VPEX_CRITICAL_THRESHOLD  <= ( fPower12vPex ) )
    {
        if( MAX_COUNT_TO_WAIT_1SEC == ucCount12vpex )
        {
            ucs_clock_shutdown( );
            ucCount12vpex = 0;
        }
        ucCount12vpex = ucCount12vpex + 1;
    }
    else /* Reset count value to zero when power values below critical limit and count is less than 10 */
    {
        if( ( 0 != ucCount12vpex ) && ( V80_POWER_12VPEX_CRITICAL_THRESHOLD > ( fPower12vPex ) ) )
        {
            ucCount12vpex = 0;
        }
    }
    if( V80_POWER_3V3PEX_CRITICAL_THRESHOLD <= ( fPower3v3pex ) )
    {
        if ( MAX_COUNT_TO_WAIT_1SEC == ucCount3v3pex )
        {
            ucs_clock_shutdown( );
            ucCount3v3pex = 0;
        }
        ucCount3v3pex = ucCount3v3pex + 1;
    }
    else /* Reset count value to zero when power values below critical limit and count is less than 10 */
    {
        if( ( 0 != ucCount3v3pex ) && ( V80_POWER_3V3PEX_CRITICAL_THRESHOLD > ( fPower3v3pex )  ) )
        {
            ucCount3v3pex = 0;
        }

    }
}

void vV80VoltageMonitorVccint( void )
{
    u8 scStatus     = XST_SUCCESS;
    float fVoltageInmV  = 0.0;

    scStatus =  ucISL68221ReadVoltage0( ucI2cMain, SLAVE_ADDRESS_ISL68221_0, &fVoltageInmV );
    if( XST_SUCCESS != scStatus )
    {
        VMC_ERR( "Failed to read Vccint Current " );
    }
    else
    {
        sensor_glvr.sensor_readings.voltage[eVCCINT] = fVoltageInmV;
    }   
}

void vV80CurrentMonitorVccint( void )
{
    u8 scStatus         = XST_SUCCESS;
    float fCurrentInA   = 0.0;

    scStatus =  ucISL68221ReadCurrent0( ucI2cMain, SLAVE_ADDRESS_ISL68221_0, &fCurrentInA );
    if( XST_SUCCESS != scStatus )
    {
        VMC_ERR( "Failed to read Vccint Current " );
    }
    else
    {
        sensor_glvr.sensor_readings.current[eVCCINT] = fCurrentInA; /* In Amps */
    }
}

s8 scV80AsdmReadPower( snsrRead_t *pxSnsrData )
{

    s8 scStatus         = XST_SUCCESS;
    u16 usTotalPower    = sensor_glvr.sensor_readings.total_power;

    if( NULL != pxSnsrData )
    {
        if( 0 != usTotalPower )
        {
            Cl_SecureMemcpy( &pxSnsrData->snsrValue[0],sizeof( usTotalPower ),&usTotalPower,sizeof( usTotalPower ) );
            pxSnsrData->sensorValueSize = sizeof( usTotalPower );
            pxSnsrData->snsrSatus = Vmc_Snsr_State_Normal;
        }
        else
        {
            pxSnsrData->snsrSatus = Vmc_Snsr_State_Comms_failure;
        }
    }
    else
    {
        scStatus = XST_FAILURE;
    }
    
    return scStatus;
}

s8 scV80AsdmReadVoltage12v( snsrRead_t *pxSnsrData )
{
    s8 scStatus     = XST_SUCCESS;
    u16 usVoltage   = sensor_glvr.sensor_readings.voltage[e12V_PEX];

    if( NULL != pxSnsrData )
    {
        if( 0 != usVoltage )
        {
            Cl_SecureMemcpy( &pxSnsrData->snsrValue[0],sizeof( usVoltage ),&usVoltage,sizeof( usVoltage ) );
            pxSnsrData->sensorValueSize = sizeof( usVoltage );
            pxSnsrData->snsrSatus = Vmc_Snsr_State_Normal;
        } 
        else
        {
            pxSnsrData->snsrSatus = Vmc_Snsr_State_Comms_failure;
        }
    }
    else
    {
        scStatus = XST_FAILURE;
    }
    
    return scStatus;
}

s8 scV80AsdmReadVoltage3v3( snsrRead_t *pxSnsrData )
{
    s8 scStatus = XST_SUCCESS;
    u16 usVoltage = sensor_glvr.sensor_readings.voltage[e3V3_PEX];

    if( NULL != pxSnsrData )
    {
        if( 0 != usVoltage )
        {
            Cl_SecureMemcpy( &pxSnsrData->snsrValue[0],sizeof( usVoltage ),&usVoltage,sizeof( usVoltage ) );
            pxSnsrData->sensorValueSize = sizeof( usVoltage );
            pxSnsrData->snsrSatus = Vmc_Snsr_State_Normal;
        } 
        else
        {
            pxSnsrData->snsrSatus = Vmc_Snsr_State_Comms_failure;
        }
    }
    else
    {
        scStatus = XST_FAILURE;
    }
    
    return scStatus;
}

s8 scV80AsdmReadVoltageVccint( snsrRead_t *pxSnsrData )
{
    s8 scStatus = XST_SUCCESS;
    u16 usVoltage = sensor_glvr.sensor_readings.voltage[eVCCINT];

    if( NULL != pxSnsrData )
    {
        if( 0 != usVoltage )
        {
            Cl_SecureMemcpy( &pxSnsrData->snsrValue[0],sizeof( usVoltage ),&usVoltage,sizeof( usVoltage ) );
            pxSnsrData->sensorValueSize = sizeof( usVoltage );
            pxSnsrData->snsrSatus = Vmc_Snsr_State_Normal;
        } 
        else
        {
            pxSnsrData->snsrSatus = Vmc_Snsr_State_Comms_failure;
        }
    }
    else
    {
        scStatus = XST_FAILURE;
    }
    
    return scStatus;
}

s8 scV80AsdmReadVoltage12VAUX0( snsrRead_t *pxSnsrData )
{
    s8 scStatus = XST_SUCCESS;
    u16 usVoltage = sensor_glvr.sensor_readings.voltage[e12V_AUX0];

    if( NULL != pxSnsrData )
    {
        if( 0 != usVoltage )
        {
            Cl_SecureMemcpy( &pxSnsrData->snsrValue[0],sizeof( usVoltage ),&usVoltage,sizeof( usVoltage ) );
            pxSnsrData->sensorValueSize = sizeof( usVoltage );
            pxSnsrData->snsrSatus = Vmc_Snsr_State_Normal;
        } 
        else
        {
            pxSnsrData->snsrSatus = Vmc_Snsr_State_Comms_failure;
        }
    }
    else
    {
        scStatus = XST_FAILURE;
    }
    
    return scStatus;
}

s8 scV80AsdmReadVoltage12VAUX1( snsrRead_t *pxSnsrData )
{
    s8 scStatus = XST_SUCCESS;
    u16 usVoltage = sensor_glvr.sensor_readings.voltage[e12V_AUX1];

    if( NULL != pxSnsrData )
    {
        if( 0 != usVoltage )
        {
            Cl_SecureMemcpy( &pxSnsrData->snsrValue[0],sizeof( usVoltage ),&usVoltage,sizeof( usVoltage ) );
            pxSnsrData->sensorValueSize = sizeof( usVoltage );
            pxSnsrData->snsrSatus = Vmc_Snsr_State_Normal;
        } 
        else
        {
            pxSnsrData->snsrSatus = Vmc_Snsr_State_Comms_failure;
        }
    }
    else
    {
        scStatus = XST_FAILURE;
    }
    
    return scStatus;
}

s8 scV80AsdmReadVoltage1V5VCCAUX( snsrRead_t *pxSnsrData )
{
    s8 scStatus = XST_SUCCESS;
    u16 usVoltage = sensor_glvr.sensor_readings.voltage[e1V5_VCC_AUX];

    if( NULL != pxSnsrData )
    {
        if( 0 != usVoltage )
        {
            Cl_SecureMemcpy( &pxSnsrData->snsrValue[0],sizeof( usVoltage ),&usVoltage,sizeof( usVoltage ) );
            pxSnsrData->sensorValueSize = sizeof( usVoltage );
            pxSnsrData->snsrSatus = Vmc_Snsr_State_Normal;
        } 
        else
        {
            pxSnsrData->snsrSatus = Vmc_Snsr_State_Comms_failure;
        }
    }
    else
    {
        scStatus = XST_FAILURE;
    }
    
    return scStatus;
}

s8 scV80AsdmReadVoltage3V3QSFP( snsrRead_t *pxSnsrData )
{
    s8 scStatus = XST_SUCCESS;
    u16 usVoltage = sensor_glvr.sensor_readings.voltage[e3V3_QSFP];

    if( NULL != pxSnsrData )
    {
        if( 0 != usVoltage )
        {
            Cl_SecureMemcpy( &pxSnsrData->snsrValue[0],sizeof( usVoltage ),&usVoltage,sizeof( usVoltage ) );
            pxSnsrData->sensorValueSize = sizeof( usVoltage );
            pxSnsrData->snsrSatus = Vmc_Snsr_State_Normal;
        } 
        else
        {
            pxSnsrData->snsrSatus = Vmc_Snsr_State_Comms_failure;
        }
    }
    else
    {
        scStatus = XST_FAILURE;
    }
    
    return scStatus;
}

s8 scV80AsdmReadVoltageVccHBM( snsrRead_t *pxSnsrData )
{
    s8 scStatus = XST_SUCCESS;
    u16 usVoltage = sensor_glvr.sensor_readings.voltage[eVCC_HBM];

    if( NULL != pxSnsrData )
    {
        if( 0 != usVoltage )
        {
            Cl_SecureMemcpy( &pxSnsrData->snsrValue[0],sizeof( usVoltage ),&usVoltage,sizeof( usVoltage ) );
            pxSnsrData->sensorValueSize = sizeof( usVoltage );
            pxSnsrData->snsrSatus = Vmc_Snsr_State_Normal;
        } 
        else
        {
            pxSnsrData->snsrSatus = Vmc_Snsr_State_Comms_failure;
        }
    }
    else
    {
        scStatus = XST_FAILURE;
    }
    
    return scStatus;
}

s8 scV80AsdmReadVoltage1V2VccoDimm( snsrRead_t *pxSnsrData )
{
    s8 scStatus = XST_SUCCESS;
    u16 usVoltage = sensor_glvr.sensor_readings.voltage[e1V2_VCCO_DIMM];

    if( NULL != pxSnsrData )
    {
        if( 0 != usVoltage )
        {
            Cl_SecureMemcpy( &pxSnsrData->snsrValue[0],sizeof( usVoltage ),&usVoltage,sizeof( usVoltage ) );
            pxSnsrData->sensorValueSize = sizeof( usVoltage );
            pxSnsrData->snsrSatus = Vmc_Snsr_State_Normal;
        } 
        else
        {
            pxSnsrData->snsrSatus = Vmc_Snsr_State_Comms_failure;
        }
    }
    else
    {
        scStatus = XST_FAILURE;
    }
    
    return scStatus;
}
        
s8 scV80AsdmReadVoltage1V2GTXAVT( snsrRead_t *pxSnsrData )
{
    s8 scStatus = XST_SUCCESS;
    u16 usVoltage = sensor_glvr.sensor_readings.voltage[e1V2_GTXAVTT];

    if( NULL != pxSnsrData )
    {
        if( 0 != usVoltage )
        {
            Cl_SecureMemcpy( &pxSnsrData->snsrValue[0],sizeof( usVoltage ),&usVoltage,sizeof( usVoltage ) );
            pxSnsrData->sensorValueSize = sizeof( usVoltage );
            pxSnsrData->snsrSatus = Vmc_Snsr_State_Normal;
        } 
        else
        {
            pxSnsrData->snsrSatus = Vmc_Snsr_State_Comms_failure;
        }
    }
    else
    {
        scStatus = XST_FAILURE;
    }
    
    return scStatus;
}

s8 scV80AsdmReadCurrent12v( snsrRead_t *pxSnsrData )
{
    s8 scStatus = XST_SUCCESS;
    u16 usCurrent = sensor_glvr.sensor_readings.current[e12V_PEX];

    if( NULL != pxSnsrData )
    {
        if( 0 != usCurrent )
        {
            Cl_SecureMemcpy( &pxSnsrData->snsrValue[0],sizeof( usCurrent ),&usCurrent,sizeof( usCurrent ) );
            pxSnsrData->sensorValueSize = sizeof( usCurrent );
            pxSnsrData->snsrSatus = Vmc_Snsr_State_Normal;
        } 
        else
        {
            pxSnsrData->snsrSatus = Vmc_Snsr_State_Comms_failure;
        }
    }
    else
    {
        scStatus = XST_FAILURE;
    }
    
    return scStatus;
}

s8 scV80AsdmReadCurrent3v3( snsrRead_t *pxSnsrData )
{
    s8 scStatus = XST_SUCCESS;
    u16 usCurrent = sensor_glvr.sensor_readings.current[e3V3_PEX];

    if( NULL != pxSnsrData )
    {
        if( 0 != usCurrent )
        {
            Cl_SecureMemcpy( &pxSnsrData->snsrValue[0],sizeof( usCurrent ),&usCurrent,sizeof( usCurrent ) );
            pxSnsrData->sensorValueSize = sizeof( usCurrent );
            pxSnsrData->snsrSatus = Vmc_Snsr_State_Normal;
        } 
        else
        {
            pxSnsrData->snsrSatus = Vmc_Snsr_State_Comms_failure;
        }
    }
    else
    {
        scStatus = XST_FAILURE;
    }
    
    return scStatus;
}

s8 scV80AsdmReadCurrentVccint( snsrRead_t *pxSnsrData )
{
    s8 scStatus = XST_SUCCESS;
    u16 usCurrent = sensor_glvr.sensor_readings.current[eVCCINT] * 1000;

    if( NULL != pxSnsrData )
    {
        if( 0 != usCurrent )
        {
            Cl_SecureMemcpy( &pxSnsrData->snsrValue[0],sizeof( usCurrent ),&usCurrent,sizeof( usCurrent ) );
            pxSnsrData->sensorValueSize = sizeof( usCurrent );
            pxSnsrData->snsrSatus = Vmc_Snsr_State_Normal;
        } 
        else
        {
            pxSnsrData->snsrSatus = Vmc_Snsr_State_Comms_failure;
        }
    }
    else
    {
        scStatus = XST_FAILURE;
    }
    
    return scStatus;
}

s8 scV80AsdmReadCurrent12VAUX0( snsrRead_t *pxSnsrData )
{
    s8 scStatus = XST_SUCCESS;
    u32 usCurrent = ( sensor_glvr.sensor_readings.current[e12V_AUX0] );

    if( NULL != pxSnsrData )
    {
        if( 0 != usCurrent )
        {
            Cl_SecureMemcpy( &pxSnsrData->snsrValue[0],sizeof( usCurrent ),&usCurrent,sizeof( usCurrent ) );
            pxSnsrData->sensorValueSize = sizeof( usCurrent );
            pxSnsrData->snsrSatus = Vmc_Snsr_State_Normal;
        } 
        else 
        {
            pxSnsrData->snsrSatus = Vmc_Snsr_State_Comms_failure;
        }
    }
    else
    {
        scStatus = XST_FAILURE;
    }
    
    return scStatus;
}

s8 scV80AsdmReadCurrent12VAUX1( snsrRead_t *pxSnsrData )
{
    s8 scStatus = XST_SUCCESS;
    u32 usCurrent = ( sensor_glvr.sensor_readings.current[e12V_AUX1] );

    if( NULL != pxSnsrData )
    {
        if( 0 != usCurrent )
        {
            Cl_SecureMemcpy( &pxSnsrData->snsrValue[0],sizeof( usCurrent ),&usCurrent,sizeof( usCurrent ) );
            pxSnsrData->sensorValueSize = sizeof( usCurrent );
            pxSnsrData->snsrSatus = Vmc_Snsr_State_Normal;
        } 
        else 
        {
            pxSnsrData->snsrSatus = Vmc_Snsr_State_Comms_failure;
        }
    }
    else
    {
        scStatus = XST_FAILURE;
    }
    
    return scStatus;
}

s8 scV80AsdmReadCurrent1V5VCCAUX( snsrRead_t *pxSnsrData )
{
    s8 scStatus = XST_SUCCESS;
    u32 usCurrent = ( sensor_glvr.sensor_readings.current[e1V5_VCC_AUX] );

    if( NULL != pxSnsrData )
    {
        if( 0 != usCurrent )
        {
            Cl_SecureMemcpy( &pxSnsrData->snsrValue[0],sizeof( usCurrent ),&usCurrent,sizeof( usCurrent ) );
            pxSnsrData->sensorValueSize = sizeof( usCurrent );
            pxSnsrData->snsrSatus = Vmc_Snsr_State_Normal;
        } 
        else 
        {
            pxSnsrData->snsrSatus = Vmc_Snsr_State_Comms_failure;
        }
    }
    else
    {
        scStatus = XST_FAILURE;
    }
    
    return scStatus;
}

s8 scV80AsdmReadCurrent3V3QSFP( snsrRead_t *pxSnsrData )
{
    s8 scStatus = XST_SUCCESS;
    u32 usCurrent = ( sensor_glvr.sensor_readings.current[e3V3_QSFP] );

    if( NULL != pxSnsrData )
    {
        if( 0 != usCurrent )
        {
            Cl_SecureMemcpy( &pxSnsrData->snsrValue[0],sizeof( usCurrent ),&usCurrent,sizeof( usCurrent ) );
            pxSnsrData->sensorValueSize = sizeof( usCurrent );
            pxSnsrData->snsrSatus = Vmc_Snsr_State_Normal;
        } 
        else 
        {
            pxSnsrData->snsrSatus = Vmc_Snsr_State_Comms_failure;
        }
    }
    else
    {
        scStatus = XST_FAILURE;
    }
    
    return scStatus;
}

s8 scV80AsdmReadCurrentVccHBM( snsrRead_t *pxSnsrData )
{
    s8 scStatus = XST_SUCCESS;
    u32 usCurrent = ( sensor_glvr.sensor_readings.current[eVCC_HBM] * 1000 );

    if( NULL != pxSnsrData )
    {
        if( 0 != usCurrent )
        {
            Cl_SecureMemcpy( &pxSnsrData->snsrValue[0],sizeof( usCurrent ),&usCurrent,sizeof( usCurrent ) );
            pxSnsrData->sensorValueSize = sizeof( usCurrent );
            pxSnsrData->snsrSatus = Vmc_Snsr_State_Normal;
        } 
        else 
        {
            pxSnsrData->snsrSatus = Vmc_Snsr_State_Comms_failure;
        }
    }
    else
    {
        scStatus = XST_FAILURE;
    }
    
    return scStatus;
}

s8 scV80AsdmReadCurrent1V2VccoDimm( snsrRead_t *pxSnsrData )
{
    s8 scStatus = XST_SUCCESS;
    u32 usCurrent = ( sensor_glvr.sensor_readings.current[e1V2_VCCO_DIMM] * 1000 );

    if( NULL != pxSnsrData )
    {
        if( 0 != usCurrent )
        {
            Cl_SecureMemcpy( &pxSnsrData->snsrValue[0],sizeof( usCurrent ),&usCurrent,sizeof( usCurrent ) );
            pxSnsrData->sensorValueSize = sizeof( usCurrent );
            pxSnsrData->snsrSatus = Vmc_Snsr_State_Normal;
        } 
        else 
        {
            pxSnsrData->snsrSatus = Vmc_Snsr_State_Comms_failure;
        }
    }
    else
    {
        scStatus = XST_FAILURE;
    }
    
    return scStatus;
}

s8 scV80AsdmReadCurrent1V2GTXAVT( snsrRead_t *pxSnsrData )
{
    s8 scStatus = XST_SUCCESS;
    u32 usCurrent = ( sensor_glvr.sensor_readings.current[e1V2_GTXAVTT] * 1000 );

    if( NULL != pxSnsrData )
    {
        if( 0 != usCurrent )
        {
            Cl_SecureMemcpy( &pxSnsrData->snsrValue[0],sizeof( usCurrent ),&usCurrent,sizeof( usCurrent ) );
            pxSnsrData->sensorValueSize = sizeof( usCurrent );
            pxSnsrData->snsrSatus = Vmc_Snsr_State_Normal;
        } 
        else 
        {
            pxSnsrData->snsrSatus = Vmc_Snsr_State_Comms_failure;
        }
    }
    else
    {
        scStatus = XST_FAILURE;
    }
    

    return scStatus;
}

s8 scV80AsdmTemperatureReadVccint( snsrRead_t *pxSnsrData )
{
    s8 scStatus = XST_SUCCESS;
    u16 usTemp = ( sensor_glvr.sensor_readings.vccint_temp );

    if( NULL != pxSnsrData )
    {
        Cl_SecureMemcpy( &pxSnsrData->snsrValue[0],sizeof( usTemp ),&usTemp,sizeof( usTemp ) );
        pxSnsrData->sensorValueSize = sizeof( usTemp );
        pxSnsrData->snsrSatus = Vmc_Snsr_State_Normal;

        /* Shutdown user clock when vccint temperature value goes beyond critical value */
        if ( V80_TEMP_VCCINT_CRITICAL_THRESHOLD <= usTemp )
        {
            ucs_clock_shutdown( );
        }
    }
    else
    {
        scStatus = XST_FAILURE;
    }
    
    return scStatus;
}

s8 scV80AsdmTemperatureReadVccHbm( snsrRead_t *pxSnsrData )
{
    s8 scStatus = XST_SUCCESS;
    u16 usTemp = ( sensor_glvr.sensor_readings.fVccHbmTemp );

    if( NULL != pxSnsrData )
    {
        Cl_SecureMemcpy( &pxSnsrData->snsrValue[0],sizeof( usTemp ),&usTemp,sizeof( usTemp ) );
        pxSnsrData->sensorValueSize = sizeof( usTemp );
        pxSnsrData->snsrSatus = Vmc_Snsr_State_Normal;
    }
    else
    {
        scStatus = XST_FAILURE;
    }
    
    
    return scStatus;
}

s8 scV80AsdmTemperatureRead1V2VccoDimm( snsrRead_t *pxSnsrData )
{
    s8 scStatus = XST_SUCCESS;
    u16 usTemp = ( sensor_glvr.sensor_readings.f1V2VccoDimmTemp );

    if( NULL != pxSnsrData )
    {
        Cl_SecureMemcpy( &pxSnsrData->snsrValue[0],sizeof( usTemp ),&usTemp,sizeof( usTemp ) );
        pxSnsrData->sensorValueSize = sizeof( usTemp );
        pxSnsrData->snsrSatus = Vmc_Snsr_State_Normal;
    }
    else
    {
        scStatus = XST_FAILURE;
    }
    
    return scStatus;
}

s8 scV80AsdmTemperatureRead1V2GTXAVTT( snsrRead_t *pxSnsrData )
{
    s8 scStatus = XST_SUCCESS;
    u16 usTemp = ( sensor_glvr.sensor_readings.f1V2GtxaVttTemp );

    if( NULL != pxSnsrData )
    {
        Cl_SecureMemcpy( &pxSnsrData->snsrValue[0],sizeof( usTemp ),&usTemp,sizeof( usTemp ) );
        pxSnsrData->sensorValueSize = sizeof( usTemp );
        pxSnsrData->snsrSatus = Vmc_Snsr_State_Normal;
    }
    else
    {
        scStatus = XST_FAILURE;
    }
    
    return scStatus;
}

s8 scV80AsdmGetTemperatureNames( u8 ucIndex, char8* pucSnsrName, u8 *pucSensorId, sensorMonitorFunc *pxSensorHandler )
{
    s8 scReturnCode = XST_SUCCESS;
    
    if( ( NULL != pucSnsrName ) &&
        ( NULL != pucSensorId ) &&
        ( NULL != pxSensorHandler ) )
    {
        struct sensorData
        {
            char8 ucSensorName[SENSOR_NAME_MAX];
            sensorMonitorFunc   xSensorHandler;
        };

        struct sensorData xSnsrData[] =    {
            {  "VCCHBM\0",          scV80AsdmTemperatureReadVccHbm },
            {  "1V2VCCODIMM\0",     scV80AsdmTemperatureRead1V2VccoDimm },
            {  "1V2GTXAVTT\0",      scV80AsdmTemperatureRead1V2GTXAVTT }
        };

        Cl_SecureMemcpy( pucSnsrName,SENSOR_NAME_MAX,&xSnsrData[ucIndex].ucSensorName[0],SENSOR_NAME_MAX );
        *pxSensorHandler = xSnsrData[ucIndex].xSensorHandler;
    }
    else
    {
        scReturnCode = XST_FAILURE;
    }

    return scReturnCode;
}



s8 scV80AsdmGetCurrentNames( u8 ucIndex, char8* pucSnsrName, u8 *pucSensorId, sensorMonitorFunc *pxSensorHandler )
{
    s8 scReturnCode = XST_SUCCESS;
    
    if( ( NULL != pucSnsrName ) &&
        ( NULL != pucSensorId ) &&
        ( NULL != pxSensorHandler ) )
    {
        struct sensorData
        {
            char8 ucSensorName[SENSOR_NAME_MAX];
            sensorMonitorFunc   xSensorHandler;
        };

        struct sensorData xSnsrData[] =    {
            {  "12v_pex\0",         scV80AsdmReadCurrent12v },
            {  "3v3_pex\0",         scV80AsdmReadCurrent3v3 },
            {  "vccint\0",          scV80AsdmReadCurrentVccint },
            {  "1V5_VCC_AUX\0",     scV80AsdmReadCurrent1V5VCCAUX },
            {  "3V3QSFP\0",         scV80AsdmReadCurrent3V3QSFP },
            {  "12VAUX0\0",         scV80AsdmReadCurrent12VAUX0 },
            {  "12VAUX1\0",         scV80AsdmReadCurrent12VAUX1 },
            {  "VCC_HBM\0",         scV80AsdmReadCurrentVccHBM },
            {  "1V2_VCCO_DIMM\0",   scV80AsdmReadCurrent1V2VccoDimm },
            {  "1V2_GTXAVTT\0",     scV80AsdmReadCurrent1V2GTXAVT }
        };
    
        Cl_SecureMemcpy( pucSnsrName,SENSOR_NAME_MAX,&xSnsrData[ucIndex].ucSensorName[0],SENSOR_NAME_MAX );
        *pxSensorHandler = xSnsrData[ucIndex].xSensorHandler;
    }
    else
    {
        scReturnCode = XST_FAILURE;
    }

    return scReturnCode;
}

s8 scV80AsdmGetVoltageNames( u8 ucIndex, char8* pucSnsrName, u8 *pucSensorId, sensorMonitorFunc *pxSensorHandler )
{
    s8 scReturnCode = XST_SUCCESS;
    
    if( ( NULL != pucSnsrName ) &&
        ( NULL != pucSensorId ) &&
        ( NULL != pxSensorHandler ) )
    {
        struct sensorData
        {
            char8 ucSensorName[SENSOR_NAME_MAX];
            sensorMonitorFunc   xSensorHandler;
        };

        struct sensorData xVoltageData[] =    {
            {  "12v_pex\0",         scV80AsdmReadVoltage12v },
            {  "3v3_pex\0",         scV80AsdmReadVoltage3v3 },
            {  "vccint\0",          scV80AsdmReadVoltageVccint },
            {  "1V5_VCC_AUX\0",     scV80AsdmReadVoltage1V5VCCAUX },
            {  "3V3QSFP\0",         scV80AsdmReadVoltage3V3QSFP },
            {  "12VAUX0\0",         scV80AsdmReadVoltage12VAUX0 },
            {  "12VAUX1\0",         scV80AsdmReadVoltage12VAUX1 },
            {  "VCC_HBM\0",         scV80AsdmReadVoltageVccHBM },
            {  "1V2_VCCO_DIMM\0",   scV80AsdmReadVoltage1V2VccoDimm },
            {  "1V2_GTXAVTT\0",     scV80AsdmReadVoltage1V2GTXAVT }
        };

        Cl_SecureMemcpy( pucSnsrName, SENSOR_NAME_MAX, &xVoltageData[ucIndex].ucSensorName[0], SENSOR_NAME_MAX );
        *pxSensorHandler = xVoltageData[ucIndex].xSensorHandler;
    }
    else
    {
        scReturnCode = XST_FAILURE;
    }

    return scReturnCode;
}

void vV80MonitorSensors( void )
{
    /* Read Temp Sensors */
    vV80TemperatureMonitor( );

    /* Read QSFP Temp Sensors */
    vV80QSFPMonitor( );

    /* Read Voltage Sensors */
    vV80VoltageMonitor12VPEX( );
    vV80VoltageMonitor3v3PEX( );
    vV80VoltageMonitorVccint( );
    vV80VoltageMonitor1V5VCCAUX( );
    vV80VoltageMonitor3V3QSFP( );
    vV80VoltageMonitor12VAUX0( );
    vV80VoltageMonitor12VAUX1( );
    vV80VoltageMonitorVccHBM( );
    vV80VoltageMonitor1V2VccoDimm( );
    vV80VoltageMonitor1V2GTXAVTT( );

    /* Read Current Sensors */
    vV80CurrentMonitor12VPEX( );
    vV80CurrentMonitor3v3PEX( );
    vV80CurrentMonitorVccint( );
    vV80CurrentMonitor1V5VCCAUX( );
    vV80CurrentMonitor3V3QSFP( );
    vV80CurrentMonitor12VAUX0( );
    vV80CurrentMonitor12VAUX1( );
    vV80CurrentMonitorVccHBM( );
    vV80CurrentMonitor1V2VccoDimm( );
    vV80CurrentMonitor1V2GTXAVTT( );

    /* Read Power Sensors */
    vV80PowerMonitor( );
}

static void vV80ClkScalingParamsInit( void ) {

    g_clk_throttling_params.is_clk_scaling_supported = true;
    g_clk_throttling_params.clk_scaling_mode = eCLK_SCALING_MODE_BOTH;
    g_clk_throttling_params.clk_scaling_enable = false;
    g_clk_throttling_params.limits.shutdown_limit_temp = V80_TEMP_FPGA_CRITICAL_THRESHOLD_LIMIT;
    g_clk_throttling_params.limits.shutdown_limit_pwr = V80_POWER_CRITICAL_THRESHOLD_LIMIT;
    g_clk_throttling_params.limits.throttle_limit_temp = V80_TEMP_THROTTLING_THRESHOLD_LIMIT;
    g_clk_throttling_params.limits.throttle_limit_pwr = V80_POWER_THROTTLING_THRESHOLD_LIMIT;
    return;
}

void vV80BuildClockThrottlingProfile( Clock_Throttling_Profile_t * pProfile )
{
    pProfile->NumberOfSensors = V80_NUM_POWER_RAILS;

    pProfile->VoltageSensorID[0] = e12V_PEX;
    pProfile->VoltageSensorID[1] = e3V3_PEX;

    pProfile->CurrentSensorID[0] = e12V_PEX;
    pProfile->CurrentSensorID[1] = e3V3_PEX;

    pProfile->throttlingThresholdCurrent[0] = V80_PEX_12V_I_IN_THROTTLING_LIMIT;
    pProfile->throttlingThresholdCurrent[1] = V80_PEX_3V3_I_IN_THROTTLING_LIMIT;

    pProfile->NominalVoltage[0] = NOMINAL_VOLTAGE_12V_IN_MV;
    pProfile->NominalVoltage[1] = NOMINAL_VOLTAGE_3V3_IN_MV;
    pProfile->IdlePower = V80_IDLE_POWER;

    pProfile->FPGATempThrottlingLimit = V80_FPGA_THROTTLING_TEMP_LIMIT;
    pProfile->VccIntTempThrottlingLimit = V80_VCCINT_THROTTLING_TEMP_LIMIT;
    pProfile->PowerThrottlingLimit = V80_POWER_THROTTLING_THRESHOLD_LIMIT;
    pProfile->TempThrottlingLimit = V80_TEMP_THROTTLING_THRESHOLD_LIMIT;

    pProfile->bVCCIntThermalThrottling = true;
    pProfile->TempGainKpFPGA    = V80_TEMP_GAIN_KP_FPGA;
    pProfile->TempGainKi        = V80_TEMP_GAIN_KI;
    pProfile->TempGainKpVCCInt  = V80_TEMP_GAIN_KP_VCCINT;
    pProfile->TempGainKaw       = V80_TEMP_GAIN_KAW;

    pProfile->IntegrataionSumInitial = V80_INTEGRATION_SUM_INITIAL;

}

u8 ucV80Init( void )
{
    msg_id_handler_ptr = NULL; /* Send nothing to SC */

    set_total_req_size(0);

    fetch_boardinfo_ptr = &slV80VMCFetchBoardInfo;
    
    Monitor_Sensors = vV80MonitorSensors;

    /* Enable DDR DIMM */
    ucEnableDDRDIMM( );

    vV80ClkScalingParamsInit( );

    /* platform specific initialization */
    vV80BuildClockThrottlingProfile( &xV80ClockThrottlingProfile );

    /* clock throttling initialization */
    ClockThrottling_Initialize( &clock_throttling_std_algorithm, &xV80ClockThrottlingProfile );

    get_supported_sdr_info = vV80GetSupportedSdrInfo;
    asdm_update_record_count = vV80AsdmUpdateRecordCount;
    return XST_SUCCESS;
}
