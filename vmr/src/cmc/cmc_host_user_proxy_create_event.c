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
 *  $Revision: #11 $
 *
 */





#include "cmc_host_user_proxy_thread.h"


 

void HostUserProxyThread_CreateEvent(  HOST_USER_PROXY_THREAD_CONTEXT_TYPE * pContext, char AnyEvent)
{
    if(!CircularBuffer_TryWrite(&(pContext->EventSourceCircularBuffer), AnyEvent))
    {
        Watch_Inc(pContext->pWatchPointContext, W_HOST_PROXY_FSM_FAILED_TO_ADD_EVENT);
    }
}



void HostUserProxyThread_CreateEvent_E_HP_REQUESTED_LINK_USER_AS_BOOTLOADER(  HOST_USER_PROXY_THREAD_CONTEXT_TYPE * pContext)
{
    HostUserProxyThread_CreateEvent(pContext, E_HP_REQUESTED_LINK_USER_AS_BOOTLOADER);
}

void HostUserProxyThread_CreateEvent_E_HP_LINK_USER_IS_BOOTLOADER(  HOST_USER_PROXY_THREAD_CONTEXT_TYPE * pContext)
{
    HostUserProxyThread_CreateEvent(pContext, E_HP_LINK_USER_IS_BOOTLOADER);
}
       
void HostUserProxyThread_CreateEvent_E_HP_LINK_USER_IS_SENSOR_SUPERVISOR(  HOST_USER_PROXY_THREAD_CONTEXT_TYPE * pContext)
{
    HostUserProxyThread_CreateEvent(pContext, E_HP_LINK_USER_IS_SENSOR_SUPERVISOR);
}

void HostUserProxyThread_CreateEvent_E_HP_LINK_USER_IS_SENSOR_SUPERVISOR_SUC(HOST_USER_PROXY_THREAD_CONTEXT_TYPE* pContext)
{
    HostUserProxyThread_CreateEvent(pContext, E_HP_LINK_USER_IS_SENSOR_SUPERVISOR_SUC);
}


void HostUserProxyThread_CreateEvent_E_HP_LOCAL_OPERATION_REQUEST(  HOST_USER_PROXY_THREAD_CONTEXT_TYPE * pContext,
                                                                    HOST_PROXY_LOCAL_OPERATION_TYPE AdditionalInformationLocal)
{
    pContext->AdditionalInformationLocal=AdditionalInformationLocal;

    HostUserProxyThread_CreateEvent(pContext, E_HP_LOCAL_OPERATION_REQUEST);
} 

        

void HostUserProxyThread_CreateEvent_E_HP_REMOTE_OPERATION_REQUEST(HOST_USER_PROXY_THREAD_CONTEXT_TYPE * pContext, HOST_PROXY_REMOTE_OPERATION_TYPE AdditionalInformationRemote)
{
    pContext->AdditionalInformationRemote=AdditionalInformationRemote;

    HostUserProxyThread_CreateEvent(pContext, E_HP_REMOTE_OPERATION_REQUEST);
} 

        

void HostUserProxyThread_CreateEvent_E_HP_REMOTE_OPERATION_FAILED(HOST_USER_PROXY_THREAD_CONTEXT_TYPE * pContext)
{
    HostUserProxyThread_CreateEvent(pContext, E_HP_REMOTE_OPERATION_FAILED);
} 



void HostUserProxyThread_CreateEvent_E_HP_REMOTE_OPERATION_SUCCESS(HOST_USER_PROXY_THREAD_CONTEXT_TYPE * pContext)
{
    HostUserProxyThread_CreateEvent(pContext, E_HP_REMOTE_OPERATION_SUCCESS);
} 

   
        

void HostUserProxyThread_CreateEvent_E_HP_TRANSACTION_TIMER_EXPIRY(HOST_USER_PROXY_THREAD_CONTEXT_TYPE * pContext)
{
    HostUserProxyThread_CreateEvent(pContext, E_HP_TRANSACTION_TIMER_EXPIRY);
} 

        
