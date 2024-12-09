/******************************************************************************
* Copyright (C) 2024 Advanced Micro Devices, Inc.    All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#ifndef INC_PLATFORMS_V80_H_
#define INC_PLATFORMS_V80_H_

#include "../vmc_asdm.h"

#define V80_NUM_POWER_RAILS                     ( 2 )
#define V80_PEX_12V_I_IN_THROTTLING_LIMIT       ( 5500 )
#define V80_PEX_3V3_I_IN_THROTTLING_LIMIT       ( 3000 )

#define V80_IDLE_POWER                          ( 14250000  )
#define V80_TEMP_GAIN_KP_FPGA                   ( 3.000e+06 )
#define V80_TEMP_GAIN_KI                        ( 1.000e+04 )
#define V80_TEMP_GAIN_KP_VCCINT                 ( 1.000e+07 )
#define V80_TEMP_GAIN_KAW                       ( 2.400e-03 )

#define V80_INTEGRATION_SUM_INITIAL             ( 7.300e+07 )

#ifndef  INCREASE_LIMITS_FOR_TEST   
/*Power threshold limits for V80 */ 
#define V80_POWER_12VPEX_CRITICAL_THRESHOLD     ( 69.000 )
#define V80_POWER_3V3PEX_CRITICAL_THRESHOLD     ( 10.400 )

#define V80_POWER_12VPEX_THROTTLING_THRESHOLD       ( 66.000 )
#define V80_POWER_3V3PEX_THROTTLING_THRESHOLD       ( 9.900 )

#define V80_TEMP_VCCINT_CRITICAL_THRESHOLD      ( 110.000 )

#define V80_TEMP_FPGA_CRITICAL_THRESHOLD_LIMIT  ( 100.000 )

#define V80_POWER_CRITICAL_THRESHOLD_LIMIT      ( V80_POWER_12VPEX_CRITICAL_THRESHOLD + \
                        V80_POWER_3V3PEX_CRITICAL_THRESHOLD )

/*Throttling limits */
#define V80_TEMP_THROTTLING_THRESHOLD_LIMIT     ( 97.000 )

#define V80_POWER_THROTTLING_THRESHOLD_LIMIT    ( V80_POWER_12VPEX_THROTTLING_THRESHOLD + \
                        V80_POWER_3V3PEX_THROTTLING_THRESHOLD )

#define V80_FPGA_THROTTLING_TEMP_LIMIT          ( 97 )
#define V80_VCCINT_THROTTLING_TEMP_LIMIT        ( 105 )

#else
/*Power threshold limits for V80 */
#define INCREASE_Y_PERCENT( x )                 ( x * ( 0.25 ) )

#define POWER_12VPEX_CRITICAL_THRESHOLD         ( 69.000 + INCREASE_Y_PERCENT( 69.000 ) )
#define POWER_3V3PEX_CRITICAL_THRESHOLD         ( 10.400 + INCREASE_Y_PERCENT( 10.400 ) )

#define POWER_12VPEX_THROTTLING_THRESHOLD       ( 66.000 + INCREASE_Y_PERCENT( 66.000 ) )
#define POWER_3V3PEX_THROTTLING_THRESHOLD       ( 9.900 + INCREASE_Y_PERCENT( 9.900 ) )

#define V80_TEMP_VCCINT_CRITICAL_THRESHOLD      ( 110.000 + INCREASE_Y_PERCENT( 110.000 ) )

#define V80_TEMP_FPGA_CRITICAL_THRESHOLD_LIMIT  ( 100.000 + INCREASE_Y_PERCENT( 100.000 ) )

#define V80_POWER_CRITICAL_THRESHOLD_LIMIT      ( POWER_12VPEX_CRITICAL_THRESHOLD + \
                        POWER_3V3PEX_CRITICAL_THRESHOLD )

/*Throttling limits */
#define V80_TEMP_THROTTLING_THRESHOLD_LIMIT     ( 97.000 + INCREASE_Y_PERCENT( 97.000 ) )

#define V80_POWER_THROTTLING_THRESHOLD_LIMIT    ( POWER_12VPEX_THROTTLING_THRESHOLD + \
                        POWER_3V3PEX_THROTTLING_THRESHOLD )

#define V80_FPGA_THROTTLING_TEMP_LIMIT          ( 97 + INCREASE_Y_PERCENT( 97 ) )
#define V80_VCCINT_THROTTLING_TEMP_LIMIT        ( 10 + INCREASE_Y_PERCENT( 10 ) )

#endif

u8 ucV80Init( void );
s8 scV80AsdmTemperatureReadInlet( snsrRead_t *pxSnsrData );
s8 scV80AsdmTemperatureReadOutlet( snsrRead_t *pxSnsrData );
s8 scV80AsdmTemperatureReadVccint(  snsrRead_t *pxSnsrData  );
s8 scV80AsdmTemperatureReadBoard( snsrRead_t *pxSnsrData );
s8 scV80AsdmTemperatureReadVccHbm( snsrRead_t *pxSnsrData );
s8 scV80AsdmTemperatureRead1V2VccoDimm( snsrRead_t *pxSnsrData );
s8 scV80AsdmTemperatureRead1V2GTXAVTT( snsrRead_t *pxSnsrData );
s8 scV80AsdmTemperatureReadQSFP( snsrRead_t *pxSnsrData );
s32 slV80VMCFetchBoardInfo( u8 *pucBoardSnsrData );
s8 scV80AsdmReadPower( snsrRead_t *pxSnsrData );

s8 scV80AsdmGetTemperatureNames( u8 ucIndex, char8* pucSnsrName, u8 *pucSensorId, sensorMonitorFunc *pxSensorHandler );
s8 scV80AsdmGetVoltageNames( u8 ucIndex, char8* pucSnsrName, u8 *pucSensorId, sensorMonitorFunc *pxSensorHandler );
s8 scV80AsdmGetCurrentNames( u8 ucIndex, char8* pucSnsrName, u8 *pucSensorId, sensorMonitorFunc *pxSensorHandler );

s8 scV80AsdmGetQSFPName( u8 ucIndex, char8* pucSnsrName, u8 *pucSensorId, sensorMonitorFunc *pxSensorHandler );

void vV80VoltageMonitor12VPEX( void );
void vV80VoltageMonitor3v3PEX( void );
void vV80VoltageMonitor1V5VCCAUX( void );
void vV80VoltageMonitor3V3QSFP( void );
void vV80VoltageMonitor12VAUX0( void );
void vV80VoltageMonitor12VAUX1( void );
void vV80VoltageMonitorVccHBM( void );
void vV80VoltageMonitor1V2VccoDimm( void );
void vV80VoltageMonitor1V2GTXAVTT( void );

void vV80CurrentMonitor12VPEX( void );
void vV80CurrentMonitor3v3PEX( void );
void vV80CurrentMonitorVccint( void );
void vV80CurrentMonitor1V5VCCAUX( void );
void vV80CurrentMonitor3V3QSFP( void );
void vV80CurrentMonitor12VAUX0( void );
void vV80CurrentMonitor12VAUX1( void );
void vV80CurrentMonitorVccHBM( void );
void vV80CurrentMonitor1V2VccoDimm( void );
void vV80CurrentMonitor1V2GTXAVTT( void );

void vV80PowerMonitor( void );
void vV80VoltageMonitorVccint( void );

#endif
