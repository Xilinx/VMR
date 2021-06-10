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
 *  $Revision: #40 $
 *
 */






#ifndef _CMC_HOST_USER_PROXY_THREAD_H_
#define _CMC_HOST_USER_PROXY_THREAD_H_


#ifdef __cplusplus
extern "C"
{
#endif



#include "cmc_common.h"
#include "cmc_watchpoint.h"
#include "cmc_circular_buffer.h"
#include "cmc_thread_timer.h"

#include "cmc_thread_communications_broker.h"
#include "cmc_local_sensor_supervisor_thread.h"
#include "cmc_local_bootloader_thread.h"
#include "cmc_thread_linkstate.h"
#include "cmc_link_receive_thread.h"

#include "cmc_peripherals.h"
#include "cmc_fsm_state_transition_logger.h"

#include "cmc_hardware_platform.h"


#include "cmc_host_user_proxy_constants.h"
#include "cmc_host_user_proxy_state.h"
#include "cmc_host_user_proxy_event.h"
#include "cmc_host_user_proxy_action.h"
#include "cmc_host_user_proxy_local_operation.h"
#include "cmc_host_user_proxy_remote_operation.h"




struct BROKER_CONTEXT_TYPE;

typedef struct HOST_USER_PROXY_THREAD_CONTEXT_TYPE
{
    CMC_WATCHPOINT_CONTEXT_TYPE *                       pWatchPointContext;
    struct BROKER_CONTEXT_TYPE *                        pBrokerContext;
    PERIPHERAL_REGMAP_RAM_CONTROLLER_CONTEXT_TYPE *     pPeripheralRegMapRamControllerContext;
    THREAD_TIMER_CONTEXT_TYPE *                         pThreadTimerContext;
    FSM_STATE_TRANSITION_LOGGER_CONTEXT_TYPE *          pFSM_StateTransitionLoggerContext;

    HARDWARE_PLATFORM_CONTEXT_TYPE *                     pHardwarePlatformContext;

    CIRCULAR_BUFFER_TYPE                EventSourceCircularBuffer;
    CIRCULAR_BUFFER_ELEMENT_TYPE        CircularBuffer[MAX_PROXY_EVENTS];

    HOST_PROXY_REMOTE_OPERATION_TYPE    AdditionalInformationRemote;
    HOST_PROXY_LOCAL_OPERATION_TYPE     AdditionalInformationLocal;
	uint8_t								DebugCaptureMsgID;

    uint32_t                            CurrentHostProxyTransactionTimerInstance;
    uint8_t                             Event;
    uint32_t                            Action;
    HOST_PROXY_STATE_TYPE               State;
    HOST_PROXY_STATE_TYPE               PreviousState;

    bool                                FSM_Running;
    bool                                bCardSupportsSCUpgrade;
    bool                                bCSDRInProgress;
    bool                                bQSFP_ReadDiagnosticsInProgress;
    bool                                bQSFP_ReadLowSpeedIOInProgress;
    bool                                bQSFP_WriteLowSpeedIOInProgress;
    bool                                bQSFP_ReadSingleByteInProgress;
    bool                                bQSFP_WriteSingleByteInProgress;
    uint32_t                            HBMSupportedCMS;
    uint32_t                            CardSupportsScalingFactorCMS;
    bool                                bBoardInfoInProgress;
    uint32_t                            CardSupportsLowSpeedQSFPFromGPIOCMS;
} HOST_USER_PROXY_THREAD_CONTEXT_TYPE;


bool HostUserProxyThread_Schedule(void *pContext);

void HostUserProxyThread_Initialize(    HOST_USER_PROXY_THREAD_CONTEXT_TYPE *               pContext,
                                        CMC_WATCHPOINT_CONTEXT_TYPE *                       pWatchPointContext,
                                        THREAD_TIMER_CONTEXT_TYPE *                         pThreadTimerContext,
                                        PERIPHERAL_REGMAP_RAM_CONTROLLER_CONTEXT_TYPE *     pPeripheralRegMapRamControllerContext,
                                        FSM_STATE_TRANSITION_LOGGER_CONTEXT_TYPE *          pFSM_StateTransitionLoggerContext,
                                        HARDWARE_PLATFORM_CONTEXT_TYPE *                    pHardwarePlatformContext,
                                        bool                                                bCardSupportsSCUpgrade);

void HostUserProxyThread_BindBroker(  HOST_USER_PROXY_THREAD_CONTEXT_TYPE * pContext,
                                        struct BROKER_CONTEXT_TYPE *    pBrokerContext);



void HostUserProxyThread_CreateEvent_E_HP_LOCAL_OPERATION_REQUEST(HOST_USER_PROXY_THREAD_CONTEXT_TYPE * pContext, HOST_PROXY_LOCAL_OPERATION_TYPE AdditionalInformationLocal);
void HostUserProxyThread_CreateEvent_E_HP_REMOTE_OPERATION_REQUEST(HOST_USER_PROXY_THREAD_CONTEXT_TYPE * pContext, HOST_PROXY_REMOTE_OPERATION_TYPE AdditionalInformationRemote);
void HostUserProxyThread_CreateEvent_E_HP_REMOTE_OPERATION_FAILED(HOST_USER_PROXY_THREAD_CONTEXT_TYPE * pContext);
void HostUserProxyThread_CreateEvent_E_HP_REMOTE_OPERATION_SUCCESS(HOST_USER_PROXY_THREAD_CONTEXT_TYPE * pContext);
void HostUserProxyThread_CreateEvent_E_HP_TRANSACTION_TIMER_EXPIRY(HOST_USER_PROXY_THREAD_CONTEXT_TYPE * pContext);
void HostUserProxyThread_CreateEvent_E_HP_LINK_USER_IS_BOOTLOADER(HOST_USER_PROXY_THREAD_CONTEXT_TYPE * pContext);
void HostUserProxyThread_CreateEvent_E_HP_LINK_USER_IS_SENSOR_SUPERVISOR(HOST_USER_PROXY_THREAD_CONTEXT_TYPE * pContext);
void HostUserProxyThread_CreateEvent_E_HP_LINK_USER_IS_SENSOR_SUPERVISOR_SUC(HOST_USER_PROXY_THREAD_CONTEXT_TYPE* pContext);
void HostUserProxyThread_CreateEvent_E_HP_REQUESTED_LINK_USER_AS_BOOTLOADER(HOST_USER_PROXY_THREAD_CONTEXT_TYPE * pContext);


void HostUserProxyThread_CLEAR_ACTION(HOST_USER_PROXY_THREAD_CONTEXT_TYPE *pContext);
void HostUserProxyThread_ACTION(HOST_USER_PROXY_THREAD_CONTEXT_TYPE *pContext, uint32_t AnyAction);
void HostUserProxyThread_HandleAction(HOST_USER_PROXY_THREAD_CONTEXT_TYPE *pContext);


void HostUserProxyThread_ClearCurrentControlRegisterRequest(HOST_USER_PROXY_THREAD_CONTEXT_TYPE *pContext);
void HostUserProxyThread_ReadControlRegister(HOST_USER_PROXY_THREAD_CONTEXT_TYPE *pContext);
void HostUserProxyThread_UpdateResultErrorCodeRegister(HOST_USER_PROXY_THREAD_CONTEXT_TYPE *pContext, uint32_t AnyErrorCode);

void HostUserProxyThread_TransactionTimerCallback(void * pCallbackContext, uint32_t AnyInstance);

void HostUserProxyThread_FSM(HOST_USER_PROXY_THREAD_CONTEXT_TYPE *pContext, uint8_t AnyEvent);

HOST_PROXY_STATE_TYPE HostUserProxyThread_GetState(HOST_USER_PROXY_THREAD_CONTEXT_TYPE *pContext);



void HostUserProxyThread_HandleLocalAction_HP_LOCAL_OPERATION_REBOOT_FIRMWARE(HOST_USER_PROXY_THREAD_CONTEXT_TYPE *pContext);
void HostUserProxyThread_HandleLocalAction_HP_LOCAL_OPERATION_STOP_FIRMWARE(HOST_USER_PROXY_THREAD_CONTEXT_TYPE *pContext);
void HostUserProxyThread_HandleLocalAction_HP_LOCAL_OPERATION_CLEAR_ERROR_REGISTER(HOST_USER_PROXY_THREAD_CONTEXT_TYPE *pContext);
void HostUserProxyThread_HandleLocalAction_HP_LOCAL_OPERATION_CLEAR_SENSOR_REGISTER(HOST_USER_PROXY_THREAD_CONTEXT_TYPE *pContext);
void HostUserProxyThread_HandleLocalAction_HP_LOCAL_OPERATION_ENABLE_DEBUG_CAPTURE(HOST_USER_PROXY_THREAD_CONTEXT_TYPE* pContext);
void HostUserProxyThread_HandleLocalAction_HP_LOCAL_OPERATION_ENABLE_REMOTE_DEBUG_UART(HOST_USER_PROXY_THREAD_CONTEXT_TYPE *pContext);
void HostUserProxyThread_HandleLocalAction_HP_LOCAL_OPERATION_DEBUG_CAPTURE_MSG_ID(HOST_USER_PROXY_THREAD_CONTEXT_TYPE* pContext);
void HostUserProxyThread_HandleLocalAction_HP_LOCAL_OPERATION_FIRST_CSDR_REQUEST(HOST_USER_PROXY_THREAD_CONTEXT_TYPE* pContext);
void HostUserProxyThread_HandleLocalAction_HP_LOCAL_OPERATION_ADDITIONAL_CSDR_REQUEST(HOST_USER_PROXY_THREAD_CONTEXT_TYPE* pContext);
void HostUserProxyThread_HandleLocalAction_HP_LOCAL_OPERATION_CSDR_COMPLETE(HOST_USER_PROXY_THREAD_CONTEXT_TYPE* pContext);
void HostUserProxyThread_HandleLocalAction_HP_LOCAL_OPERATION_CSDR_FAILED(HOST_USER_PROXY_THREAD_CONTEXT_TYPE* pContext);


void HostUserProxyThread_HandleLocalAction_HP_LOCAL_OPERATION_READ_QSFP_DIAGNOSTICS(HOST_USER_PROXY_THREAD_CONTEXT_TYPE* pContext);
void HostUserProxyThread_HandleLocalAction_HP_LOCAL_OPERATION_READ_QSFP_VALIDATE_LOW_SPEED_IO(HOST_USER_PROXY_THREAD_CONTEXT_TYPE* pContext);
void HostUserProxyThread_HandleLocalAction_HP_LOCAL_OPERATION_WRITE_QSFP_VALIDATE_LOW_SPEED_IO(HOST_USER_PROXY_THREAD_CONTEXT_TYPE* pContext);

void HostUserProxyThread_HandleLocalAction_HP_LOCAL_OPERATION_READ_QSFP_DIAGNOSTICS_COMPLETE(HOST_USER_PROXY_THREAD_CONTEXT_TYPE* pContext);
void HostUserProxyThread_HandleLocalAction_HP_LOCAL_OPERATION_READ_QSFP_VALIDATE_LOW_SPEED_IO_COMPLETE(HOST_USER_PROXY_THREAD_CONTEXT_TYPE* pContext);
void HostUserProxyThread_HandleLocalAction_HP_LOCAL_OPERATION_WRITE_QSFP_VALIDATE_LOW_SPEED_IO_COMPLETE(HOST_USER_PROXY_THREAD_CONTEXT_TYPE* pContext);
void HostUserProxyThread_HandleLocalAction_HP_LOCAL_OPERATION_READ_QSFP_DIAGNOSTICS_FAILED(HOST_USER_PROXY_THREAD_CONTEXT_TYPE* pContext);
void HostUserProxyThread_HandleLocalAction_HP_LOCAL_OPERATION_READ_QSFP_VALIDATE_LOW_SPEED_IO_FAILED(HOST_USER_PROXY_THREAD_CONTEXT_TYPE* pContext);
void HostUserProxyThread_HandleLocalAction_HP_LOCAL_OPERATION_WRITE_QSFP_VALIDATE_LOW_SPEED_IO_FAILED(HOST_USER_PROXY_THREAD_CONTEXT_TYPE* pContext);


void HostUserProxyThread_HandleLocalAction_HP_LOCAL_OPERATION_QSFP_READ_SINGLE_BYTE_REQUEST(HOST_USER_PROXY_THREAD_CONTEXT_TYPE* pContext);
void HostUserProxyThread_HandleLocalAction_HP_LOCAL_OPERATION_QSFP_READ_SINGLE_BYTE_COMPLETE(HOST_USER_PROXY_THREAD_CONTEXT_TYPE* pContext);
void HostUserProxyThread_HandleLocalAction_HP_LOCAL_OPERATION_QSFP_READ_SINGLE_BYTE_FAILED(HOST_USER_PROXY_THREAD_CONTEXT_TYPE* pContext);

void HostUserProxyThread_HandleLocalAction_HP_LOCAL_OPERATION_QSFP_WRITE_SINGLE_BYTE_REQUEST(HOST_USER_PROXY_THREAD_CONTEXT_TYPE* pContext);
void HostUserProxyThread_HandleLocalAction_HP_LOCAL_OPERATION_QSFP_WRITE_SINGLE_BYTE_COMPLETE(HOST_USER_PROXY_THREAD_CONTEXT_TYPE* pContext);
void HostUserProxyThread_HandleLocalAction_HP_LOCAL_OPERATION_QSFP_WRITE_SINGLE_BYTE_FAILED(HOST_USER_PROXY_THREAD_CONTEXT_TYPE* pContext);


void HostUserProxyThread_HandleLocalAction_HP_LOCAL_OPERATION_HBM_SUPPORT_CMS(HOST_USER_PROXY_THREAD_CONTEXT_TYPE* pContext);
void HostUserProxyThread_HandleLocalAction_HP_LOCAL_OPERATION_CLOCK_SCALING_CMS(HOST_USER_PROXY_THREAD_CONTEXT_TYPE* pContext);
void HostUserProxyThread_HandleLocalAction_HP_LOCAL_OPERATION_LOW_SPEED_QSFP_FROM_GPIO_CMS(HOST_USER_PROXY_THREAD_CONTEXT_TYPE* pContext);

void HostUserProxyThread_HandleLocalAction_HP_LOCAL_OPERATION_BOARDINFO_REQUEST(HOST_USER_PROXY_THREAD_CONTEXT_TYPE* pContext);
void HostUserProxyThread_HandleLocalAction_HP_LOCAL_OPERATION_BOARDINFO_COMPLETE(HOST_USER_PROXY_THREAD_CONTEXT_TYPE* pContext);

#ifdef __cplusplus
}
#endif


#endif /* _CMC_HOST_USER_PROXY_THREAD_H_ */







