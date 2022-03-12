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
 *  $Revision: #19 $
 *
 */




#include "cmc_thread_linkstate.h"


bool LinkState_CanSensorSupervisorUseLink(LINK_STATE_CONTEXT_TYPE * pContext)
{
    bool Result=true;

    switch(pContext->State)
    {
        case    S_INITIAL:                                  /* drop through */
        case    S_DETERMINE_LINK_USER:                      /* drop through */
        case    S_LINK_USER_IS_REMOTE_SENSOR_SUPERVISOR:    /* drop through */
        case    S_WAITING_FOR_REMOTE_BOOTLOADER_RESPONSE:   /* drop through */
        case    S_LINK_USER_IS_REMOTE_BOOTLOADER:           /* drop through */
                Result=false;
                break;

        case    S_LINK_USER_IS_REMOTE_SENSOR_SUPERVISOR_LINK_OPERATING:
                Result=true;
                break;

        default:
                Result=false;
                break;
    }
    return Result;
}


LINK_STATE_TYPE LinkState_FSM_GetState(LINK_STATE_CONTEXT_TYPE *pContext)
{
    return pContext->State;
}




void LinkStateThread_BindBroker( LINK_STATE_CONTEXT_TYPE * pContext, struct BROKER_CONTEXT_TYPE *    pBrokerContext)
{
    pContext->pBrokerContext=pBrokerContext;
}





void LinkStateThread_Initialize(    LINK_STATE_CONTEXT_TYPE * pContext,
                                    THREAD_TIMER_CONTEXT_TYPE *pThreadTimerContext,
                                    CMC_WATCHPOINT_CONTEXT_TYPE * pWatchPointContext,
                                    FSM_STATE_TRANSITION_LOGGER_CONTEXT_TYPE * pFSM_StateTransitionLoggerContext,
									CMC_VERSION_SUPPORT_CONTEXT_TYPE * pVersionSupportContext,
                                    CMC_FIRMWARE_VERSION_CONTEXT_TYPE * pFirmwareVersionContext,
									CMC_BUILD_PROFILE_TYPE * pProfile)
{
    pContext->pBrokerContext=NULL;

    pContext->Event=0;
    pContext->State=S_INITIAL;
    pContext->PreviousState=S_INITIAL;

    pContext->CurrentTimerInstance=0;
    pContext->pThreadTimerContext=pThreadTimerContext;
    pContext->pWatchPointContext=pWatchPointContext;
    pContext->pFSM_StateTransitionLoggerContext=pFSM_StateTransitionLoggerContext;
	pContext->pVersionSupportContext = pVersionSupportContext;
    pContext->pFirmwareVersionContext = pFirmwareVersionContext;
    pContext->bZync1Device = pProfile->UserSuppliedEnvironment.bU30_Zync1_Device;
	pContext->pProtocol = &(pProfile->Protocols.SENSOR);
    pContext->pProtocolBootloader = &(pProfile->Protocols.BOOTLOADER);
    pContext->pTransport = &(pProfile->Transports.SENSOR);
    pContext->bCardSupportsSC = pProfile->UserSuppliedEnvironment.bCardSupportsSC;
    pContext->bCardSupportsSUCUpgrade = pProfile->UserSuppliedEnvironment.bCardSupportsSUCUpgrade;

    CircularBuffer_Initialize(&(pContext->EventSourceCircularBuffer), pContext->CircularBuffer, MAX_EVENT_ELEMENTS);

    Watch_Set(pContext->pWatchPointContext, W_LINK_STATE_CURRENT,       (uint32_t)pContext->State);
    Watch_Set(pContext->pWatchPointContext, W_LINK_STATE_PREVIOUS,      (uint32_t)pContext->State);
    Watch_Set(pContext->pWatchPointContext, W_LINK_STATE_LAST_EVENT,    (uint32_t)pContext->Event);

	LinkState_CreateEvent_E_LS_RESET_REQUEST(pContext);
}

