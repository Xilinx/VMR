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
 *  $Change: 2773736 $
 *  $Date: 2020/02/07 $
 *  $Revision: #8 $
 *
 */


#include "cmc_thread_x2_comms.h"



void X2_Comms_Thread_Initialize(CMC_X2_COMMS_CONTEXT* pContext,
                                THREAD_TIMER_CONTEXT_TYPE* pThreadTimerContext,
                                CMC_WATCHPOINT_CONTEXT_TYPE* pWatchPointContext,
                                FSM_STATE_TRANSITION_LOGGER_CONTEXT_TYPE* pFSM_StateTransitionLoggerContext,
                                PERIPHERAL_X2_I2C_CONTEXT_TYPE* pI2CContext,
                                PERIPHERAL_X2_GPIO_CONTEXT_TYPE* pGPIOContext,
                                CMC_FIRMWARE_VERSION_CONTEXT_TYPE* pFirmwareVersionContext)
{

    pContext->pFSM_StateTransitionLoggerContext = pFSM_StateTransitionLoggerContext;

    pContext->currentState = STATE_IDLE;
    pContext->previousState = STATE_IDLE;
    pContext->lastEventReceived = CMC_X2_MAX_EVENTS;

    CMC_X2_FSM_ClearActions(pContext);


    pContext->pThreadTimerContext = pThreadTimerContext;
    pContext->currentTimerInstance = 0;


    pContext->pWatchPointContext = pWatchPointContext;


    CircularBuffer_Initialize(&(pContext->eventBuffer), pContext->eventBufferElements, X2_COMMS_MAX_EVENT_BUFFER_ELEMENTS);


    CMC_X2_VirtualRegisterSet_Initialize(&(pContext->virtualRegisterSet));


    //Populate the version register with the version of the firmware...
    CMC_X2_VirtualRegisterSet_SetVersionInfo(&(pContext->virtualRegisterSet),   pFirmwareVersionContext->Version.Major, 
                                                                                pFirmwareVersionContext->Version.Minor, 
                                                                                pFirmwareVersionContext->Version.Increment, 
                                                                                0x00);


    Watch_Set(pContext->pWatchPointContext, W_X2_COMMS_FSM_CURRENT_STATE, (uint32_t)pContext->currentState);
    Watch_Set(pContext->pWatchPointContext, W_X2_COMMS_FSM_PREVIOUS_STATE, (uint32_t)pContext->previousState);
    Watch_Set(pContext->pWatchPointContext, W_X2_COMMS_FSM_LAST_EVENT, (uint32_t)pContext->lastEventReceived);



    pContext->responseCallback = NULL;
    pContext->pResponseContext = NULL;

    pContext->resetCallback = NULL;
    pContext->pResetContext = NULL;

    

    pContext->pI2C = pI2CContext;
    pContext->pGPIO = pGPIOContext;


    PERIPHERAL_X2_I2C_SetWriteRequestCallback(pContext->pI2C, pContext, CMC_X2_WriteRequestCallback);
    PERIPHERAL_X2_I2C_SetReadRequestCallback(pContext->pI2C, pContext, CMC_X2_ReadRequestCallback);

 
    PERIPHERAL_X2_GPIO_SetPinDirectionOutput(pContext->pGPIO, X2_COMMS_GPIO_CHANNEL, X2_COMMS_GPIO_PIN);
    PERIPHERAL_X2_GPIO_SetPinValue(pContext->pGPIO, X2_COMMS_GPIO_CHANNEL, X2_COMMS_GPIO_PIN, false);

}












bool X2_Comms_Thread_Schedule(void* pContext)
{
    CMC_X2_COMMS_CONTEXT* pThreadContext = (CMC_X2_COMMS_CONTEXT*)pContext;
    char AnyEvent;

    Watch_Inc(pThreadContext->pWatchPointContext, W_THREAD_X2_COMMS);


    while (CircularBuffer_TryRead(&(pThreadContext->eventBuffer), &AnyEvent))
    {
        CMC_X2_FSM_Run(pThreadContext, AnyEvent);
    }

    return true;
   
}







void CMC_X2_NewUserRequest(CMC_X2_COMMS_CONTEXT* pContext, uint8_t* pRequestData, uint8_t requestDataLength)
{
    cmcMemCopy((char*)pContext->pendingRequestData, (char*)pRequestData, requestDataLength);
    pContext->pendingRequestLength = requestDataLength;

    CMC_X2_FSM_AddEvent(pContext, EVENT_NEW_USER_REQUEST_ARRIVED);
}






void CMC_X2_SetResponseCallback(CMC_X2_COMMS_CONTEXT* pContext, CMC_X2_RESPONSE_PUSH_CALLBACK_TYPE callback, void* pCallbackContext)
{
    pContext->responseCallback = callback;
    pContext->pResponseContext = pCallbackContext;
}




void CMC_X2_SetResetCallback(CMC_X2_COMMS_CONTEXT* pContext, CMC_X2_RESET_CALLBACK_TYPE callback, void* pCallbackContext)
{
    pContext->resetCallback = callback;
    pContext->pResetContext = pCallbackContext;
}

