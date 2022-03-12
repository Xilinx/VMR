/*----------------------------------------------------------------------------------------------*/
/*                                                                                              */
/* © Copyright 2019 Xilinx, Inc. All rights reserved.                                           */
/*                                                                                              */
/* This file contains confidential and proprietary information of Xilinx, Inc.                  */
/* and is protected under U.S. and international copyright and other intellectual               */
/* property laws.                                                                               */
/*                                                                                              */
/*                                                                                              */
/* DISCLAIMER                                                                                   */
/*                                                                                              */
/* This disclaimer is not a license and does not grant any rights to the materials              */
/* distributed herewith. Except as otherwise provided in a valid license issued                 */
/* to you by Xilinx, and to the maximum extent permitted by applicable law:                     */
/*                                                                                              */
/* (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND WITH ALL FAULTS,                          */
/* AND XILINX HEREBY DISCLAIMS ALL WARRANTIES AND CONDITIONS, EXPRESS, IMPLIED,                 */
/* OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY,                    */
/* NON-INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE; and                                 */
/*                                                                                              */
/* (2) Xilinx shall not be liable (whether in contract or tort, including negligence,           */
/* or under any other theory of liability) for any loss or damage of any kind or                */
/* nature related to, arising under or in connection with these materials,                      */
/* including for any direct, or any indirect, special, incidental, or consequential             */
/* loss or damage (including loss of data, profits, goodwill, or any type of loss or            */
/* damage suffered as a result of any action brought by a third party) even if such             */
/* damage or loss was reasonably foreseeable or Xilinx had been advised of the                  */
/* possibility of the same.                                                                     */
/*                                                                                              */
/*                                                                                              */
/* CRITICAL APPLICATIONS                                                                        */
/*                                                                                              */
/* Xilinx products are not designed or intended to be fail-safe, or for use in                  */
/* any application requiring fail-safe performance, such as life-support or safety              */
/* devices or systems, Class III medical devices, nuclear facilities, applications              */
/* related to the deployment of airbags, or any other applications that could lead              */
/* to death, personal injury, or severe property or environmental damage (individually          */
/* and collectively, "Critical Applications"). Customer assumes the sole risk and               */
/* liability of any use of Xilinx products in Critical Applications, subject                    */
/* only to applicable laws and regulations governing limitations on product liability.          */
/*                                                                                              */
/*                                                                                              */
/* THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS PART OF THIS FILE AT ALL TIMES.     */
/*                                                                                              */
/*----------------------------------------------------------------------------------------------*/


/* COMMON_DIST */ 




/*
 *
 *  RCS Keyword Metadata
 *
 *  $Change: 3095780 $
 *  $Date: 2021/01/13 $
 *  $Revision: #30 $
 *
 */




#include "cmc_initialization.h"

#include "cmc_scheduler.h"
#include "cmc_fsm_state_transition_logger.h"
#include "cmc_sensor_relay_buffer.h"
#include "cmc_firmware_version.h"
#include "cmc_core_version.h"
#include "cmc_version_support.h"
#include "cmc_clock_throttling.h"
#include "cmc_feature_control.h"
#include "cmc_link_protocol_trace.h"
#include "cmc_ram.h"





void cmc_Initialize_Libraries(CMC_BUILD_PROFILE_TYPE * pProfile)
{

    
    Scheduler_Initialize(                   &SchedulerContext,
                                            &WatchpointContext,
                                            &RegMap_RAM_Controller_Context,
                                            &FreeRunningClock_Context,
                                            &AXI_GPIO_WDT_Context,
                                            &AXI_GPIO_Mutex_CMC_Context);

    

    FSM_StateTransitionLogger_Initialize(   &FSM_StateTransitionLoggerContext,
                                            &UART_LITE_USB_Context);

    SensorRelay_Initialize(                 &SensorRelayContext,
                                            SensorRelayCircularBuffer,
                                            MAX_SENSOR_RELAY_ELEMENTS);

    SensorRelay_Initialize(                 &ClockThrottlingRelayContext,
                                            ClockThrottlingRelayCircularBuffer,
                                            MAX_SENSOR_RELAY_ELEMENTS);


    CMC_FirmwareVersion_Initialize(         &FirmwareVersionContext,
                                            &pProfile->Version);

    CMC_CoreVersion_Initialize(             &CoreVersionContext);

	CMC_VersionSupport_Initialize(			&VersionSupportContext,
											&WatchpointContext,
                                            pProfile->CMCOfferedInterfaceVersion,
                                            pProfile->Version.Minor,
                                            pProfile->UserSuppliedEnvironment.bU30_Zync1_Device);

    CMC_ClockThrottling_Initialize(         &ClockThrottlingAlgorithmContext,
                                            &pProfile->ClockThrottling,
                                            &WatchpointContext,
                                            &ClockThrottlingPeripheral_Context,
                                            &ClockMonitorPeripheral_Context,
                                            &RegMap_RAM_Controller_Context,
                                            &AXI_GPIO_Mutex_CMC_Context);

    FeatureControl_Initialize(              &FeatureControlContext,
                                            &RegMap_RAM_Controller_Context,
                                            &ClockThrottlingAlgorithmContext,
                                            pProfile->FeatureIsEnabled);


	LinkProtocolTrace_Initialize(			&LinkProtocolTraceContext);
}



