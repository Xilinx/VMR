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
 *  $Revision: #16 $
 *
 */






#include "cmc_host_user_proxy_thread.h"

static void HostUserProxyThreadLogUnexpected(HOST_USER_PROXY_THREAD_CONTEXT_TYPE* pContext, uint8_t AnyEvent)
{
    Watch_Set(pContext->pWatchPointContext, W_HOST_PROXY_UNEXPECTED_EVENT, pContext->State << 24 | AnyEvent);
}
 
HOST_PROXY_STATE_TYPE HostUserProxyThread_GetState(HOST_USER_PROXY_THREAD_CONTEXT_TYPE *pContext)
{
    return pContext->State;
}

void HostUserProxyThread_NextStateDecoder(HOST_USER_PROXY_THREAD_CONTEXT_TYPE *pContext, HOST_PROXY_STATE_TYPE NewState)
{
    FSM_STATE_TRANSITION_LOGGER_ELEMENT_TYPE LogElement;
    if(NewState!=pContext->State)
    {
        LogElement.CurrentState=(uint8_t)pContext->State;
        LogElement.Event=(uint8_t)pContext->Event;
        LogElement.NewState=(uint8_t)NewState;
        LogElement.FSM_Identifier=FSM_ID_HOST_USER_PROXY;
        FSM_StateTransitionLogger_TryAdd(pContext->pFSM_StateTransitionLoggerContext, &LogElement);
    }
    pContext->PreviousState=pContext->State;
    pContext->State=NewState;

    Watch_Set(pContext->pWatchPointContext, W_HOST_PROXY_CURRENT,  (uint32_t)pContext->State);
    Watch_Set(pContext->pWatchPointContext, W_HOST_PROXY_PREVIOUS, (uint32_t)pContext->PreviousState);
}




void HostProxy_FSM_S_HP_TRANSACTION_RUNNING(HOST_USER_PROXY_THREAD_CONTEXT_TYPE *pContext, uint8_t AnyEvent)
{
    switch(AnyEvent)
    {
        case    E_HP_LINK_USER_IS_SENSOR_SUPERVISOR:
                HostUserProxyThread_ACTION(pContext,A_HP_T_TRANSACTION_STOP);
                HostUserProxyThread_ACTION(pContext, A_HP_CANCEL_REMOTE_OPERATION);
                HostUserProxyThread_NextStateDecoder(pContext, S_HP_INITIAL);
                break;

        case    E_HP_LINK_USER_IS_SENSOR_SUPERVISOR_SUC:
                HostUserProxyThread_ACTION(pContext, A_HP_T_TRANSACTION_STOP);
                HostUserProxyThread_ACTION(pContext, A_HP_CANCEL_REMOTE_OPERATION);
                HostUserProxyThread_NextStateDecoder(pContext, S_HP_INITIAL);
                break;

        case    E_HP_LOCAL_OPERATION_REQUEST:
                HostUserProxyThread_ACTION(pContext,A_HP_PERFORM_LOCAL_ACTION);
                HostUserProxyThread_NextStateDecoder(pContext, S_HP_TRANSACTION_RUNNING);
                break;

        case    E_HP_REMOTE_OPERATION_FAILED:
                HostUserProxyThread_ACTION(pContext,A_HP_T_TRANSACTION_STOP);
                HostUserProxyThread_ACTION(pContext,A_HP_ACKNOWLEDGE_REMOTE_OPERATION_FAILED);
                HostUserProxyThread_NextStateDecoder(pContext, S_HP_INITIAL);
                break;
 
        case    E_HP_TRANSACTION_TIMER_EXPIRY:
                HostUserProxyThread_ACTION(pContext,A_HP_ACKNOWLEDGE_REMOTE_OPERATION_FAILED);
                HostUserProxyThread_NextStateDecoder(pContext, S_HP_INITIAL);
                break;

        case    E_HP_REMOTE_OPERATION_SUCCESS:
                HostUserProxyThread_ACTION(pContext,A_HP_T_TRANSACTION_STOP);
                HostUserProxyThread_ACTION(pContext,A_HP_CANCEL_REMOTE_OPERATION);
                HostUserProxyThread_NextStateDecoder(pContext, S_HP_BOOTLOADER);
                break;

        default:
                HostUserProxyThreadLogUnexpected(pContext, AnyEvent);
                break;
    }
}


void HostProxy_FSM_S_HP_BOOTLOADER(HOST_USER_PROXY_THREAD_CONTEXT_TYPE *pContext, uint8_t AnyEvent)
{
    switch(AnyEvent)
    {
        case    E_HP_LINK_USER_IS_SENSOR_SUPERVISOR:
                HostUserProxyThread_NextStateDecoder(pContext, S_HP_INITIAL);
                break;
                
        case    E_HP_LOCAL_OPERATION_REQUEST:
                HostUserProxyThread_ACTION(pContext,A_HP_PERFORM_LOCAL_ACTION);
                HostUserProxyThread_NextStateDecoder(pContext, S_HP_BOOTLOADER);
                break;

        case    E_HP_REMOTE_OPERATION_REQUEST:
                HostUserProxyThread_ACTION(pContext,A_HP_T_TRANSACTION_START);
                HostUserProxyThread_ACTION(pContext,A_HP_START_REMOTE_ACTION);
                HostUserProxyThread_NextStateDecoder(pContext, S_HP_TRANSACTION_RUNNING);
                break;
        
        default:
                HostUserProxyThreadLogUnexpected(pContext, AnyEvent);
                break;
    }
}






void HostProxy_FSM_S_HP_WAITING_FOR_BOOTLOADER(HOST_USER_PROXY_THREAD_CONTEXT_TYPE *pContext, uint8_t AnyEvent)
{
    switch(AnyEvent)
    {
        case    E_HP_LINK_USER_IS_BOOTLOADER:
                HostUserProxyThread_ACTION(pContext,A_HP_T_TRANSACTION_STOP);
                HostUserProxyThread_ACTION(pContext,A_HP_ACTIVATE_BOOTLOADER);
                HostUserProxyThread_NextStateDecoder(pContext, S_HP_BOOTLOADER);
                break;
          
        case    E_HP_LINK_USER_IS_SENSOR_SUPERVISOR:
                HostUserProxyThread_ACTION(pContext,A_HP_T_TRANSACTION_STOP);
                HostUserProxyThread_ACTION(pContext,A_HP_ACKNOWLEDGE_REMOTE_OPERATION_FAILED);
                HostUserProxyThread_NextStateDecoder(pContext, S_HP_INITIAL);
                break;
                
        case    E_HP_LOCAL_OPERATION_REQUEST:
                HostUserProxyThread_ACTION(pContext,A_HP_PERFORM_LOCAL_ACTION);
                HostUserProxyThread_NextStateDecoder(pContext, S_HP_WAITING_FOR_BOOTLOADER);
                break;

        case    E_HP_TRANSACTION_TIMER_EXPIRY:
                HostUserProxyThread_ACTION(pContext,A_HP_ACKNOWLEDGE_REMOTE_OPERATION_FAILED);
                HostUserProxyThread_NextStateDecoder(pContext, S_HP_INITIAL);
                break;

        default:
                HostUserProxyThreadLogUnexpected(pContext, AnyEvent);
                break;
    }
}






void HostProxy_FSM_S_HP_INITIAL(HOST_USER_PROXY_THREAD_CONTEXT_TYPE *pContext, uint8_t AnyEvent)
{
    switch(AnyEvent)
    {
        case    E_HP_LINK_USER_IS_BOOTLOADER:
                HostUserProxyThread_NextStateDecoder(pContext, S_HP_BOOTLOADER);
                break;
          
        case    E_HP_REQUESTED_LINK_USER_AS_BOOTLOADER:
                HostUserProxyThread_ACTION(pContext,A_HP_T_TRANSACTION_START);
                HostUserProxyThread_NextStateDecoder(pContext, S_HP_WAITING_FOR_BOOTLOADER);
                break;

        case    E_HP_LOCAL_OPERATION_REQUEST:
                HostUserProxyThread_ACTION(pContext,A_HP_PERFORM_LOCAL_ACTION);
                HostUserProxyThread_NextStateDecoder(pContext, S_HP_INITIAL);
                break;

        case    E_HP_LINK_USER_IS_SENSOR_SUPERVISOR:
                HostUserProxyThread_NextStateDecoder(pContext, S_HP_INITIAL);
                break;

        case    E_HP_LINK_USER_IS_SENSOR_SUPERVISOR_SUC:
                HostUserProxyThread_NextStateDecoder(pContext, S_HP_INITIAL);
                break;

        default:
                HostUserProxyThreadLogUnexpected(pContext, AnyEvent);
                break;
    }
}






void HostUserProxyThread_FSM(HOST_USER_PROXY_THREAD_CONTEXT_TYPE *pContext, uint8_t AnyEvent)
{
    HostUserProxyThread_CLEAR_ACTION(pContext);

    pContext->Event=AnyEvent;
    Watch_Set(pContext->pWatchPointContext, W_HOST_PROXY_LAST_EVENT,    (uint32_t)pContext->Event);

    switch(pContext->State)
    {
        case    S_HP_INITIAL:
                HostProxy_FSM_S_HP_INITIAL(pContext, AnyEvent);
                break;

        case    S_HP_WAITING_FOR_BOOTLOADER:
                HostProxy_FSM_S_HP_WAITING_FOR_BOOTLOADER(pContext, AnyEvent);
                break;

        case    S_HP_BOOTLOADER:
                HostProxy_FSM_S_HP_BOOTLOADER(pContext, AnyEvent);
                break;

        case    S_HP_TRANSACTION_RUNNING:
                HostProxy_FSM_S_HP_TRANSACTION_RUNNING(pContext, AnyEvent);
                break;

        default:
                HostUserProxyThread_NextStateDecoder(pContext, S_HP_INITIAL);
                Watch_Inc(pContext->pWatchPointContext, W_HOST_PROXY_FSM_ILLEGAL_STATE);
                break;
    }

    HostUserProxyThread_HandleAction(pContext);

}
