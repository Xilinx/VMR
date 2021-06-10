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
#include "cmc_thread_x2_comms_constants.h"


/* Forward Declarations */
void CMC_X2_FSM_ActionHandler_AssertGPIO(CMC_X2_COMMS_CONTEXT* pContext);






void CMC_X2_FSM_ActionHandler_StartTimer(CMC_X2_COMMS_CONTEXT* pContext)
{
    CMC_X2_StartTimer(pContext);
}





void CMC_X2_FSM_ActionHandler_StopTimer(CMC_X2_COMMS_CONTEXT* pContext)
{
    CMC_X2_StopTimer(pContext);
}




void CMC_X2_FSM_ActionHandler_RestartTimer(CMC_X2_COMMS_CONTEXT* pContext)
{
    CMC_X2_RestartTimer(pContext);
}






void CMC_X2_FSM_ActionHandler_CopyAndSignalNewRequest(CMC_X2_COMMS_CONTEXT* pContext)
{  
    //copy the request details into the interanl virtual address map...
    cmcMemCopy((char*)pContext->virtualRegisterSet.Request_Data, (char*)pContext->pendingRequestData, (size_t)pContext->pendingRequestLength);
    pContext->virtualRegisterSet.Request_Length = pContext->pendingRequestLength;


    //...set the "requesting service" bit in the virtual status register...
    pContext->virtualRegisterSet.Status = 0x1;

    //...and signal that we have a new request
    CMC_X2_FSM_ActionHandler_AssertGPIO(pContext);
   
}






void CMC_X2_FSM_ActionHandler_AssertGPIO(CMC_X2_COMMS_CONTEXT* pContext)
{ 
    PERIPHERAL_X2_GPIO_SetPinValue(pContext->pGPIO, X2_COMMS_GPIO_CHANNEL, X2_COMMS_GPIO_PIN, true);
}




void CMC_X2_FSM_ActionHandler_DeassertGPIO(CMC_X2_COMMS_CONTEXT* pContext)
{
    PERIPHERAL_X2_GPIO_SetPinValue(pContext->pGPIO, X2_COMMS_GPIO_CHANNEL, X2_COMMS_GPIO_PIN, false);
}




void CMC_X2_FSM_ActionHandler_ClearStatus(CMC_X2_COMMS_CONTEXT* pContext)
{
    pContext->virtualRegisterSet.Status = 0;
}





void CMC_X2_FSM_ActionHandler_SendResponse(CMC_X2_COMMS_CONTEXT* pContext)
{
    if (pContext->responseCallback != NULL)
    {
        (*pContext->responseCallback)(pContext->pResponseContext, pContext->virtualRegisterSet.Response_Data, pContext->virtualRegisterSet.Response_Length);
    }
}





void CMC_X2_FSM_ActionHandler_IncrementResponseTimeoutCount(CMC_X2_COMMS_CONTEXT* pContext)
{
    Watch_Inc(pContext->pWatchPointContext, W_X2_COMMS_RESPONSE_SEQUENCE_TIMEOUT);
}







void CMC_X2_FSM_ActionHandler_IncrementRequestAlreadyInProgressCount(CMC_X2_COMMS_CONTEXT* pContext)
{
    Watch_Inc(pContext->pWatchPointContext, W_X2_COMMS_REQUEST_ALREADY_IN_PROGRESS);
}




void CMC_X2_FSM_ActionHandler_RaiseResetRequest(CMC_X2_COMMS_CONTEXT* pContext)
{
    if (pContext->resetCallback != NULL)
    {
        (*pContext->resetCallback)(pContext->pResetContext);
    }
}






/*******************************************************************/




void CMC_X2_FSM_ClearActions(CMC_X2_COMMS_CONTEXT* pContext)
{
    pContext->actions = 0;
}






void CMC_X2_FSM_AddAction(CMC_X2_COMMS_CONTEXT* pContext, CMC_X2_FSM_ACTION action)
{
    pContext->actions |= 1 << action;
}






void CMC_X2_FSM_PerformActions(CMC_X2_COMMS_CONTEXT* pContext)
{
    uint32_t i;
    uint32_t mask;

    for (i = 0; i < CMC_X2_MAX_ACTIONS; i++)
    {
        mask = 1 << i;

        if (pContext->actions & mask)
        {

            switch (i)
            {
                case(ACTION_START_TIMER):
                {
                    CMC_X2_FSM_ActionHandler_StartTimer(pContext);
                    break;
                }


                case(ACTION_STOP_TIMER):
                {
                    CMC_X2_FSM_ActionHandler_StopTimer(pContext);
                    break;
                }


                case(ACTION_RESTART_TIMER):
                {
                    CMC_X2_FSM_ActionHandler_RestartTimer(pContext);
                    break;
                }


                case(ACTION_COPY_AND_SIGNAL_NEW_REQUEST):
                {
                    CMC_X2_FSM_ActionHandler_CopyAndSignalNewRequest(pContext);
                    break;
                }


              
                case(ACTION_DEASSERT_GPIO):
                {
                    CMC_X2_FSM_ActionHandler_DeassertGPIO(pContext);
                    break;
                }


                

                case(ACTION_CLEAR_STATUS):
                {
                    CMC_X2_FSM_ActionHandler_ClearStatus(pContext);
                    break;
                }



                case(ACTION_SEND_RESPONSE):
                {
                    CMC_X2_FSM_ActionHandler_SendResponse(pContext);
                    break;
                }


                case(ACTION_INCREMENT_RESPONSE_TIMEOUT_COUNT):
                {
                    CMC_X2_FSM_ActionHandler_IncrementResponseTimeoutCount(pContext);
                    break;
                }


                case(ACTION_INCREMENT_REQUEST_ALREADY_IN_PROGRESS_COUNT):
                {
                    CMC_X2_FSM_ActionHandler_IncrementRequestAlreadyInProgressCount(pContext);
                    break;
                }


                case(ACTION_RAISE_RESET_REQUEST):
                {
                    CMC_X2_FSM_ActionHandler_RaiseResetRequest(pContext);
                    break;
                }

                  

                default:
                {
                   
                    break;
                }

            }

        }
    }

}