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
 *  $Change: 3143817 $
 *  $Date: 2021/03/09 $
 *  $Revision: #20 $
 *
 */






#include "cmc_host_user_proxy_thread.h"




void HostUserProxyThread_BindBroker(  HOST_USER_PROXY_THREAD_CONTEXT_TYPE * pContext,
                                        struct BROKER_CONTEXT_TYPE *    pBrokerContext)
{
    pContext->pBrokerContext=pBrokerContext;
}




void HostUserProxyThread_Initialize(    HOST_USER_PROXY_THREAD_CONTEXT_TYPE *               pContext,
                                        CMC_WATCHPOINT_CONTEXT_TYPE *                       pWatchPointContext,
                                        THREAD_TIMER_CONTEXT_TYPE *                         pThreadTimerContext,
                                        PERIPHERAL_REGMAP_RAM_CONTROLLER_CONTEXT_TYPE *     pPeripheralRegMapRamControllerContext,
                                        FSM_STATE_TRANSITION_LOGGER_CONTEXT_TYPE *          pFSM_StateTransitionLoggerContext,
                                        HARDWARE_PLATFORM_CONTEXT_TYPE *                    pHardwarePlatformContext,
                                        bool                                                bCardSupportsSCUpgrade)
{

    pContext->AdditionalInformationRemote=HP_REMOTE_OPERATION_NONE;
    pContext->AdditionalInformationLocal=HP_LOCAL_OPERATION_NONE;
 
    pContext->Event=0;
    pContext->State=S_HP_INITIAL;
    pContext->PreviousState=S_HP_INITIAL;
    pContext->CurrentHostProxyTransactionTimerInstance=0;

    pContext->pBrokerContext=NULL;
    pContext->pWatchPointContext=pWatchPointContext;
    pContext->pThreadTimerContext=pThreadTimerContext;
    pContext->pPeripheralRegMapRamControllerContext=pPeripheralRegMapRamControllerContext;
    pContext->pFSM_StateTransitionLoggerContext=pFSM_StateTransitionLoggerContext;
    pContext->pHardwarePlatformContext=pHardwarePlatformContext;

    CircularBuffer_Initialize(&(pContext->EventSourceCircularBuffer), pContext->CircularBuffer, MAX_PROXY_EVENTS);

    pContext->bCardSupportsSCUpgrade = bCardSupportsSCUpgrade;
    pContext->FSM_Running = true;
    pContext->bCSDRInProgress = false;
    pContext->bQSFP_ReadDiagnosticsInProgress = false;
    pContext->bQSFP_ReadLowSpeedIOInProgress = false;
    pContext->bQSFP_WriteLowSpeedIOInProgress = false;
    pContext->bQSFP_ReadSingleByteInProgress = false;
    pContext->bQSFP_WriteSingleByteInProgress = false;
    pContext->HBMSupportedCMS = 0;
    pContext->CardSupportsScalingFactorCMS = 0;
    pContext->CardSupportsLowSpeedQSFPFromGPIOCMS = 0;
}




