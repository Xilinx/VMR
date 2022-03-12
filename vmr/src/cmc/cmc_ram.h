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
 *  $Revision: #43 $
 *
 */





#ifndef _CMC_RAM_H_
#define _CMC_RAM_H_


#ifdef __cplusplus
extern "C"
{
#endif


#include "cmc_common.h"
#include "cmc_watchpoint.h"

#include "cmc_fsm_state_transition_logger.h"
#include "cmc_circular_buffer.h"
#include "cmc_initialization_constants.h"
#include "cmc_scheduler.h"
#include "cmc_thread_timer.h"
#include "cmc_thread_linkstate.h"
#include "cmc_local_bootloader_thread.h"
#include "cmc_watchdog_thread.h"
#include "cmc_host_user_proxy_thread.h"
#include "cmc_local_sensor_supervisor_thread.h"
#include "cmc_link_receive_thread.h"
#include "cmc_required_environment.h"
#include "cmc_version_support.h"
#include "cmc_clock_throttling.h"
#include "cmc_hardware_platform.h"
#include "cmc_firmware_version.h"
#include "cmc_link_protocol_trace.h"
#include "cmc_thread_mutex_observer.h"
#include "cmc_thread_x2_comms.h"
#include "cmc_supervisor_message_parser.h"
#include "cmc_supervisor_message_deframer.h"


extern PERIPHERAL_X2_I2C_CONTEXT_TYPE                       X2_I2C_Device_Context;
extern PERIPHERAL_X2_GPIO_CONTEXT_TYPE                      X2_GPIO_Context;
extern PERIPHERAL_AXI_GPIO_QSFP_CONTEXT_TYPE                AXI_GPIO_QSFP_Context;
extern PERIPHERAL_AXI_GPIO_HBM_CONTEXT_TYPE                 AXI_GPIO_HBM_Context;
extern PERIPHERAL_AXI_GPIO_MB_INTERRUPTS_CONTEXT_TYPE       AXI_GPIO_MB_Interrupts_Context;
extern PERIPHERAL_AXI_GPIO_WDT_CONTEXT_TYPE                 AXI_GPIO_WDT_Context;
extern PERIPHERAL_AXI_INTC_CONTEXT_TYPE                     AXI_INTC_Context;
extern PERIPHERAL_AXI_TIMEBASE_WDT_CONTEXT_TYPE             AXI_Timebase_WDT_Context;

extern PERIPHERAL_AXI_UART_LITE_SATELLITE_CONTEXT_TYPE      AXI_UART_LITE_Satellite_Context;
extern PERIPHERAL_AXI_UART_LITE_USB_CONTEXT_TYPE            UART_LITE_USB_Context;
extern PERIPHERAL_AXI_GPIO_MUTEX_CMC_CONTEXT_TYPE           AXI_GPIO_Mutex_CMC_Context;
extern PERIPHERAL_REGMAP_RAM_CONTROLLER_CONTEXT_TYPE        RegMap_RAM_Controller_Context;  
extern PERIPHERAL_FREERUNNING_CLOCK_CONTEXT_TYPE            FreeRunningClock_Context;
extern PERIPHERAL_CLOCK_THROTTLING_CONTEXT_TYPE             ClockThrottlingPeripheral_Context;
extern PERIPHERAL_CLOCK_MONITOR_CONTEXT_TYPE                ClockMonitorPeripheral_Context;

extern HARDWARE_PLATFORM_CONTEXT_TYPE                       HardwarePlatformContext;

extern CMC_FIRMWARE_VERSION_CONTEXT_TYPE                    FirmwareVersionContext;

extern CMC_CORE_VERSION_CONTEXT_TYPE                    CoreVersionContext;

extern CMC_WATCHPOINT_CONTEXT_TYPE                  WatchpointContext;


extern FSM_STATE_TRANSITION_LOGGER_CONTEXT_TYPE     FSM_StateTransitionLoggerContext;



extern SCHEDULER_CONTEXT_TYPE                       SchedulerContext;
extern THREAD_TIMER_CONTEXT_TYPE                    ThreadTimerContext;
extern LINK_STATE_CONTEXT_TYPE                      LinkStateThreadContext;
extern LINK_RECEIVE_THREAD_CONTEXT_TYPE             LinkReceiveThreadContext;
extern LOCAL_BOOTLOADER_THREAD_CONTEXT_TYPE         LocalBootloaderThreadContext;
extern WATCHDOG_THREAD_CONTEXT_TYPE                 WatchdogThreadContext;
extern HOST_USER_PROXY_THREAD_CONTEXT_TYPE          HostUserProxyThreadContext;
extern LOCAL_SENSOR_SUPERVISOR_THREAD_CONTEXT_TYPE  LocalSensorSupervisorThreadContext;
extern SENSOR_SUPERVISOR_PARSER_CONTEXT_TYPE		SensorSupervisorParserContext;
extern SENSOR_SUPERVISOR_DEFRAMER_CONTEXT_TYPE		SensorSupervisorDeFramerContext;

extern BOOTLOADER_PARSER_CONTEXT_TYPE				BootloaderParserContext;



extern BROKER_CONTEXT_TYPE							BrokerContext;

extern CMC_FEATURE_CONTROL_CONTEXT_TYPE             FeatureControlContext;

extern CMC_REQUIRED_ENVIRONMENT_CONTEXT_TYPE        RequiredEnvironmentContext;

extern CMC_SENSOR_RELAY_BUFFER_CONTEXT_TYPE         SensorRelayContext;

extern CIRCULAR_BUFFER_ELEMENT_TYPE                 SensorRelayCircularBuffer[MAX_SENSOR_RELAY_ELEMENTS];

extern CMC_SENSOR_RELAY_BUFFER_CONTEXT_TYPE         ClockThrottlingRelayContext;

extern CIRCULAR_BUFFER_ELEMENT_TYPE                 ClockThrottlingRelayCircularBuffer[MAX_SENSOR_RELAY_ELEMENTS];

extern CMC_VERSION_SUPPORT_CONTEXT_TYPE				VersionSupportContext;

extern CMC_CLOCK_THROTTLING_CONTEXT_TYPE            ClockThrottlingAlgorithmContext;

extern LINK_PROTOCOL_TRACE_CONTEXT_TYPE				LinkProtocolTraceContext;

extern PERIPHERAL_HW_BUILD_INFO_CONTEXT_TYPE        HWBuildInfoPeripheral_Context;

extern MUTEX_OBSERVER_CONTEXT_TYPE                  ThreadMutexObserver_Context;

extern CMC_X2_COMMS_CONTEXT                         X2CommsContext;

extern PERIPHERAL_SYSMON_CONTEXT_TYPE               SysMonPeripheral_Context;

extern PERIPHERAL_DNA_CONTEXT_TYPE                  DNAPeripheral_Context;

#ifdef __cplusplus
}
#endif


#endif /* _CMC_RAM_H_ */










