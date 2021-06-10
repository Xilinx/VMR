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
 *  $Revision: #48 $
 *
 */







#ifndef _CMC_INITIALIZATION_H_
#define _CMC_INITIALIZATION_H_


#ifdef __cplusplus
extern "C"
{
#endif

#if 0
#include "cmc_common.h"
#include "cmc_watchpoint.h"

#include "cmc_fsm_state_transition_logger.h"
#include "cmc_circular_buffer.h"
#include "cmc_initialization_constants.h"
#include "cmc_scheduler.h"
#include "cmc_thread_timer.h"
#include "cmc_thread_linkstate.h"
#include "cmc_link_receive_thread.h"
#include "cmc_local_bootloader_thread.h"
#include "cmc_watchdog_thread.h"
#include "cmc_host_user_proxy_thread.h"
#include "cmc_local_sensor_supervisor_thread.h"
#include "cmc_clock_throttling.h"

#include "cmc_peripherals.h"
#include "cmc_hardware_platform.h"

#include "cmc_supervisor_message_parser.h"
#include "cmc_supervisor_message_deframer.h"


#include "cmc_thread_communications_broker.h"

#include "cmc_bootloader_message_formatter.h"
#include "cmc_bootloader_message_parser.h"


#include "cmc_feature_control.h"

#include "cmc_ram.h"

#include "cmc_user_supplied_memory_map.h"

#include "cmc_sensor_relay_buffer.h"
#include "cmc_version_support.h"

#include "cmc_entry_point.h"
#include "cmc_firmware_version.h"
#include "cmc_link_protocol_trace.h"

#include "cmc_thread_mutex_observer.h"

#include "cmc_thread_x2_comms.h"

#include "cmc_protocol_sensor.h"

#include "cmc_transport_sensor.h"
#endif

#include "cmc_entry_point_build_profile.h"
#include "cmc_user_supplied_environment.h"
#include "cmc_peripheral_regmap_ram_controller.h"

void cmc_Initialize_Peripherals(struct CMC_BUILD_PROFILE_TYPE * pProfile);
void cmc_Initialize_SchedulerAddThreads(void);
void cmc_Initialize_BrokerDoBindings(void);
void cmc_Initialize_ParsersAndFormatters(CMC_USER_SUPPLIED_ENVIRONMENT_TYPE * pUserSuppliedEnvironment, PERIPHERAL_REGMAP_RAM_CONTROLLER_CONTEXT_TYPE * pRegMap_RAM_Controller_Context);
void cmc_Initialize_ThreadContexts(struct CMC_BUILD_PROFILE_TYPE * pProfile);
void cmc_Initialize_Libraries(struct CMC_BUILD_PROFILE_TYPE * pProfile);
void cmc_Initialize_PeripheralPushInterface(void);
void cmc_initialize_required_environment(   void *      pUserContext,
                                            uint32_t    (*pFN_Read)(void * pUserContext, CMC_PERIPHERAL_BASE_ADDRESS_TYPE PeripheralBaseAddress, uint32_t RegisterOffset),
                                            void        (*pFN_Write)(void * pUserContext, CMC_PERIPHERAL_BASE_ADDRESS_TYPE PeripheralBaseAddress, uint32_t RegisterOffset, uint32_t Value),
                                            void        (*pFN_WriteWithMask)(void * pUserContext, CMC_PERIPHERAL_BASE_ADDRESS_TYPE PeripheralBaseAddress, uint32_t RegisterOffset, uint32_t Value, uint32_t Mask));
void cmc_Initialize_ProtocolsAndTransports(CMC_BUILD_PROFILE_TYPE * pProfile);

void cmc_initialization(struct CMC_BUILD_PROFILE_TYPE * pProfile);


void cmc_peripheral_start(void);


#ifdef __cplusplus
}
#endif


#endif /* _CMC_INITIALIZATION_H_ */









