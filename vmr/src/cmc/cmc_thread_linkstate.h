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
 *  $Change: 3217619 $
 *  $Date: 2021/05/13 $
 *  $Revision: #30 $
 *
 */



#ifndef _CMC_THREAD_LINKSTATE_H_
#define _CMC_THREAD_LINKSTATE_H_


#ifdef __cplusplus
extern "C"
{
#endif

#include <assert.h>


#include "cmc_common.h"
#include "cmc_watchpoint.h"
#include "cmc_thread_timer.h"
#include "cmc_circular_buffer.h"
#include "cmc_thread_communications_broker.h"
#include "cmc_local_sensor_supervisor_thread.h"
#include "cmc_host_user_proxy_thread.h"
#include "cmc_link_receive_thread.h"

#include "cmc_local_bootloader_thread.h"
#include "cmc_bootloader_message_formatter.h"
#include "cmc_fsm_state_transition_logger.h"
#include "cmc_peripheral_uart_lite_usb.h"

#include "cmc_thread_linkstate_constants.h"
#include "cmc_thread_linkstate_event.h"
#include "cmc_thread_linkstate_state.h"
#include "cmc_thread_linkstate_action.h"

#include "cmc_version_support.h"

#include "cmc_protocol_sensor.h"
#include "cmc_protocol_bootloader.h"
#include "cmc_transport_sensor.h"

#include "cmc_entry_point_build_profile.h"


typedef struct LINK_STATE_CONTEXT_TYPE
{
	char							                            TxFramedMsgBuffer[MAX_BOOTLOADER_MSG_SIZE];
	char														TxFramedMsgBufferSize;
	char							                            TxMsgBuffer[MAX_BOOTLOADER_MSG_SIZE];
	uint32_t						                            TxMsgBufferSize;

    uint32_t                                                    CurrentTimerInstance;
    uint32_t                                                    Action;
    CIRCULAR_BUFFER_TYPE                                        EventSourceCircularBuffer;
    CIRCULAR_BUFFER_ELEMENT_TYPE                                CircularBuffer[MAX_EVENT_ELEMENTS];
    CMC_WATCHPOINT_CONTEXT_TYPE *                               pWatchPointContext;
    THREAD_TIMER_CONTEXT_TYPE *                                 pThreadTimerContext;
    BROKER_CONTEXT_TYPE *										pBrokerContext;
    FSM_STATE_TRANSITION_LOGGER_CONTEXT_TYPE *                  pFSM_StateTransitionLoggerContext;
    PERIPHERAL_AXI_UART_LITE_SATELLITE_CONTEXT_TYPE *           pAXI_UART_LITE_Satellite_Context;
    LINK_STATE_TYPE                                             State;
    LINK_STATE_TYPE                                             PreviousState;
    uint8_t                                                     Event;
	CMC_VERSION_SUPPORT_CONTEXT_TYPE *							pVersionSupportContext;
    CMC_FIRMWARE_VERSION_CONTEXT_TYPE *                         pFirmwareVersionContext;
    bool                                                        bZync1Device;   // Used by U30
	CMC_BUILD_PROFILE_PROTOCOL_SENSOR_TYPE *					pProtocol;
    CMC_BUILD_PROFILE_PROTOCOL_BOOTLOADER_TYPE *                pProtocolBootloader;
    CMC_BUILD_PROFILE_TRANSPORT_SENSOR_TYPE *					pTransport;
    bool                                                        bCardSupportsSC;
    bool                                                        bCardSupportsSUCUpgrade;
} LINK_STATE_CONTEXT_TYPE;



void LinkStateThread_BindBroker(    LINK_STATE_CONTEXT_TYPE * pContext,
                                    BROKER_CONTEXT_TYPE * pBrokerContext);

LINK_STATE_TYPE LinkState_FSM_GetState(LINK_STATE_CONTEXT_TYPE *pContext);

void LinkStateThread_Initialize(LINK_STATE_CONTEXT_TYPE * pContext,
								THREAD_TIMER_CONTEXT_TYPE *pThreadTimerContext,
								CMC_WATCHPOINT_CONTEXT_TYPE * pWatchPointContext,
								FSM_STATE_TRANSITION_LOGGER_CONTEXT_TYPE * pFSM_StateTransitionLoggerContext,
								CMC_VERSION_SUPPORT_CONTEXT_TYPE * pVersionSupportContext,
								CMC_FIRMWARE_VERSION_CONTEXT_TYPE * pFirmwareVersionContext,
								CMC_BUILD_PROFILE_TYPE * pProfile);

bool LinkStateThread_Schedule(void *pContext);

void CLEAR_ACTION(LINK_STATE_CONTEXT_TYPE *pContext);
void ACTION(LINK_STATE_CONTEXT_TYPE *pContext, uint32_t AnyAction);
void LinkState_HandleAction(LINK_STATE_CONTEXT_TYPE *pContext);
void LinkState_FSM(LINK_STATE_CONTEXT_TYPE *pContext, uint8_t AnyEvent);
void LinkState_TimerCallback(void * pCallbackContext, uint32_t AnyInstance);
void LinkState_BSLTimerCallback(void* pCallbackContext, uint32_t AnyInstance);

void LinkState_CreateEvent_E_LS_RESET_REQUEST(LINK_STATE_CONTEXT_TYPE * pContext);
void LinkState_CreateEvent_E_LS_T_RESPONSE_EXPIRY(LINK_STATE_CONTEXT_TYPE * pContext);
void LinkState_CreateEvent_E_LS_LINK_USER_IS_REMOTE_SENSOR_SUPERVISOR(LINK_STATE_CONTEXT_TYPE * pContext);
void LinkState_CreateEvent_E_LS_LINK_USER_IS_REMOTE_BOOTLOADER(LINK_STATE_CONTEXT_TYPE * pContext);
void LinkState_CreateEvent_E_LS_LINK_USER_TO_BOOTLOADER_REQUEST(LINK_STATE_CONTEXT_TYPE * pContext);
void LinkState_CreateEvent_E_LS_LINK_OFFERED_VERSION_ACKNOWLEDGE(LINK_STATE_CONTEXT_TYPE * pContext);
void LinkState_CreateEvent_E_LS_LINK_OFFERED_VERSION_DECLINED(LINK_STATE_CONTEXT_TYPE* pContext);
void LinkState_CreateEvent_E_LS_LINK_ACCEPTED_PROTOCOL_VERSION_RANGE_RESPONSE(LINK_STATE_CONTEXT_TYPE * pContext);
void LinkState_CreateEvent_E_LS_LINK_ACCEPTED_PROTOCOL_VERSION_RANGE_RESPONSE_FAILED(LINK_STATE_CONTEXT_TYPE* pContext);
void LinkState_CreateEvent_E_LS_T_BSL_EXPIRY(LINK_STATE_CONTEXT_TYPE* pContext);
void LinkState_CreateEvent_E_LS_ENABLE_BSL_RESPONSE(LINK_STATE_CONTEXT_TYPE* pContext);
void LinkState_CreateEvent_E_LS_ENABLE_BSL_RESPONSE_FAILED(LINK_STATE_CONTEXT_TYPE* pContext);
void LinkState_CreateEvent_E_LS_LINK_I2C_ACKNOWLEDGE(LINK_STATE_CONTEXT_TYPE* pContext);
void LinkState_CreateEvent_E_LS_LINK_I2C_FAILED(LINK_STATE_CONTEXT_TYPE* pContext);

bool LinkState_CanSensorSupervisorUseLink(LINK_STATE_CONTEXT_TYPE * pContext);

#ifdef __cplusplus
}
#endif


#endif /* _CMC_THREAD_LINKSTATE_H_ */





