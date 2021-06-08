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



void CMC_X2_FSM_ChangeState(CMC_X2_COMMS_CONTEXT* pContext, CMC_X2_FSM_STATE newState)
{
    FSM_STATE_TRANSITION_LOGGER_ELEMENT_TYPE loggerElement;

    if (pContext->currentState != newState)
    {
        loggerElement.CurrentState   = pContext->currentState;
        loggerElement.Event          = pContext->lastEventReceived;
        loggerElement.FSM_Identifier = FSM_ID_X2_COMMS;
        loggerElement.NewState       = newState;
        
        FSM_StateTransitionLogger_TryAdd(pContext->pFSM_StateTransitionLoggerContext, &loggerElement);
    }

    pContext->previousState = pContext->currentState;
    pContext->currentState = newState;

    Watch_Set(pContext->pWatchPointContext, W_X2_COMMS_FSM_CURRENT_STATE, (uint32_t)pContext->currentState);
    Watch_Set(pContext->pWatchPointContext, W_X2_COMMS_FSM_PREVIOUS_STATE, (uint32_t)pContext->previousState);
}







void CMC_X2_IDLE_StateHandler(CMC_X2_COMMS_CONTEXT* pContext, CMC_X2_FSM_EVENT event)
{
    switch (event)
    {
        case(EVENT_NEW_USER_REQUEST_ARRIVED):
        {
            CMC_X2_FSM_AddAction(pContext, ACTION_COPY_AND_SIGNAL_NEW_REQUEST);
            CMC_X2_FSM_AddAction(pContext, ACTION_START_TIMER);
            CMC_X2_FSM_ChangeState(pContext, STATE_WAITING_FOR_X2_READ_STATUS);
            break;
        }


        case(EVENT_RESET_REQUEST):
        {
            CMC_X2_FSM_AddAction(pContext, ACTION_DEASSERT_GPIO);
            CMC_X2_FSM_AddAction(pContext, ACTION_CLEAR_STATUS);
            CMC_X2_FSM_AddAction(pContext, ACTION_RAISE_RESET_REQUEST);
            CMC_X2_FSM_ChangeState(pContext, STATE_IDLE);
            break;
        }


        case(EVENT_X2_READ_STATUS):
        case(EVENT_X2_READ_REQUEST_LENGTH):
        case(EVENT_X2_READ_REQUEST_DATA):
        {
            /* Ignore any reads while we are in idle state...X2 may poll for status at any time...*/
            break;
        }




        default:
        {
            Watch_Inc(pContext->pWatchPointContext, W_X2_COMMS_FSM_UNEXPECTED_EVENT);
            break;
        }
    }
}









void CMC_X2_WAITING_FOR_X2_READ_STATUS_StateHandler(CMC_X2_COMMS_CONTEXT* pContext, CMC_X2_FSM_EVENT event)
{
    switch (event)
    {
        case(EVENT_RESET_REQUEST):
        {
            CMC_X2_FSM_AddAction(pContext, ACTION_STOP_TIMER);
            CMC_X2_FSM_AddAction(pContext, ACTION_DEASSERT_GPIO);
            CMC_X2_FSM_AddAction(pContext, ACTION_CLEAR_STATUS);
            CMC_X2_FSM_AddAction(pContext, ACTION_RAISE_RESET_REQUEST);
            CMC_X2_FSM_ChangeState(pContext, STATE_IDLE);
            break;
        }
    
        case(EVENT_TIMER_EXPIRY):
        {
            CMC_X2_FSM_AddAction(pContext, ACTION_STOP_TIMER);
            CMC_X2_FSM_AddAction(pContext, ACTION_DEASSERT_GPIO);
            CMC_X2_FSM_AddAction(pContext, ACTION_CLEAR_STATUS);
            CMC_X2_FSM_AddAction(pContext, ACTION_INCREMENT_RESPONSE_TIMEOUT_COUNT);
            CMC_X2_FSM_ChangeState(pContext, STATE_IDLE);
            break;
        }


        case(EVENT_NEW_USER_REQUEST_ARRIVED):
        {
            CMC_X2_FSM_AddAction(pContext, ACTION_INCREMENT_REQUEST_ALREADY_IN_PROGRESS_COUNT);
            break;
        }


        case(EVENT_X2_READ_STATUS):
        {
            CMC_X2_FSM_AddAction(pContext, ACTION_DEASSERT_GPIO);
            CMC_X2_FSM_AddAction(pContext, ACTION_RESTART_TIMER);
            CMC_X2_FSM_ChangeState(pContext, STATE_WAITING_FOR_X2_READ_REQUEST_LENGTH);
            break;
        }


        default:
        {
            Watch_Inc(pContext->pWatchPointContext, W_X2_COMMS_FSM_UNEXPECTED_EVENT);
            break;
        }
           
    }
}





void CMC_X2_WAITING_FOR_X2_READ_REQUEST_LENGTH_StateHandler(CMC_X2_COMMS_CONTEXT* pContext, CMC_X2_FSM_EVENT event)
{
    switch (event)
    {
        case(EVENT_RESET_REQUEST):
        {
            CMC_X2_FSM_AddAction(pContext, ACTION_STOP_TIMER);
            CMC_X2_FSM_AddAction(pContext, ACTION_DEASSERT_GPIO);
            CMC_X2_FSM_AddAction(pContext, ACTION_CLEAR_STATUS);
            CMC_X2_FSM_AddAction(pContext, ACTION_RAISE_RESET_REQUEST);
            CMC_X2_FSM_ChangeState(pContext, STATE_IDLE);
            break;
        }


        case(EVENT_TIMER_EXPIRY):
        {
            CMC_X2_FSM_AddAction(pContext, ACTION_STOP_TIMER);
            CMC_X2_FSM_AddAction(pContext, ACTION_DEASSERT_GPIO);
            CMC_X2_FSM_AddAction(pContext, ACTION_CLEAR_STATUS);
            CMC_X2_FSM_AddAction(pContext, ACTION_INCREMENT_RESPONSE_TIMEOUT_COUNT);
            CMC_X2_FSM_ChangeState(pContext, STATE_IDLE);
            break;
        }



        case(EVENT_NEW_USER_REQUEST_ARRIVED):
        {
            CMC_X2_FSM_AddAction(pContext, ACTION_INCREMENT_REQUEST_ALREADY_IN_PROGRESS_COUNT);
            break;
        }


        case(EVENT_X2_READ_REQUEST_LENGTH):
        {
            CMC_X2_FSM_AddAction(pContext, ACTION_RESTART_TIMER);
            CMC_X2_FSM_ChangeState(pContext, STATE_WAITING_FOR_X2_READ_REQUEST_DATA);
            break;
        }


        default:
        {
            Watch_Inc(pContext->pWatchPointContext, W_X2_COMMS_FSM_UNEXPECTED_EVENT);
            break;
        }


    }

}





void CMC_X2_WAITING_FOR_X2_READ_REQUEST_DATA_StateHandler(CMC_X2_COMMS_CONTEXT* pContext, CMC_X2_FSM_EVENT event)
{
   switch (event)
    {
        case(EVENT_RESET_REQUEST):
        {
            CMC_X2_FSM_AddAction(pContext, ACTION_STOP_TIMER);
            CMC_X2_FSM_AddAction(pContext, ACTION_DEASSERT_GPIO);
            CMC_X2_FSM_AddAction(pContext, ACTION_CLEAR_STATUS);
            CMC_X2_FSM_AddAction(pContext, ACTION_RAISE_RESET_REQUEST);
            CMC_X2_FSM_ChangeState(pContext, STATE_IDLE);
            break;
        }


        case(EVENT_TIMER_EXPIRY):
        {
            CMC_X2_FSM_AddAction(pContext, ACTION_STOP_TIMER);
            CMC_X2_FSM_AddAction(pContext, ACTION_DEASSERT_GPIO);
            CMC_X2_FSM_AddAction(pContext, ACTION_CLEAR_STATUS);
            CMC_X2_FSM_AddAction(pContext, ACTION_INCREMENT_RESPONSE_TIMEOUT_COUNT);
            CMC_X2_FSM_ChangeState(pContext, STATE_IDLE);
            break;
        }


        case(EVENT_NEW_USER_REQUEST_ARRIVED):
        {
            CMC_X2_FSM_AddAction(pContext, ACTION_INCREMENT_REQUEST_ALREADY_IN_PROGRESS_COUNT);
            break;
        }


        case(EVENT_X2_READ_REQUEST_DATA):
        {
            CMC_X2_FSM_AddAction(pContext, ACTION_RESTART_TIMER);
            CMC_X2_FSM_ChangeState(pContext, STATE_WAITING_FOR_X2_RESPONSE_LENGTH);
            break;
        }


        default:
        {
            Watch_Inc(pContext->pWatchPointContext, W_X2_COMMS_FSM_UNEXPECTED_EVENT);
            break;
        }


    }
}





void CMC_X2_WAITING_FOR_X2_RESPONSE_LENGTH_StateHandler(CMC_X2_COMMS_CONTEXT* pContext, CMC_X2_FSM_EVENT event)
{
    switch (event)
    {
        case(EVENT_RESET_REQUEST):
        {
            CMC_X2_FSM_AddAction(pContext, ACTION_STOP_TIMER);
            CMC_X2_FSM_AddAction(pContext, ACTION_DEASSERT_GPIO);
            CMC_X2_FSM_AddAction(pContext, ACTION_CLEAR_STATUS);
            CMC_X2_FSM_AddAction(pContext, ACTION_RAISE_RESET_REQUEST);
            CMC_X2_FSM_ChangeState(pContext, STATE_IDLE);
            break;
        }


        case(EVENT_TIMER_EXPIRY):
        {
            CMC_X2_FSM_AddAction(pContext, ACTION_STOP_TIMER);
            CMC_X2_FSM_AddAction(pContext, ACTION_DEASSERT_GPIO);
            CMC_X2_FSM_AddAction(pContext, ACTION_CLEAR_STATUS);
            CMC_X2_FSM_AddAction(pContext, ACTION_INCREMENT_RESPONSE_TIMEOUT_COUNT);
            CMC_X2_FSM_ChangeState(pContext, STATE_IDLE);
            break;
        }


        case(EVENT_NEW_USER_REQUEST_ARRIVED):
        {
            CMC_X2_FSM_AddAction(pContext, ACTION_INCREMENT_REQUEST_ALREADY_IN_PROGRESS_COUNT);
            break;
        }


        case(EVENT_X2_WROTE_RESPONSE_LENGTH):
        {
            CMC_X2_FSM_AddAction(pContext, ACTION_RESTART_TIMER);
            CMC_X2_FSM_ChangeState(pContext, STATE_WAITING_FOR_X2_RESPONSE_DATA);
            break;
        }


        default:
        {
            Watch_Inc(pContext->pWatchPointContext, W_X2_COMMS_FSM_UNEXPECTED_EVENT);
            break;
        }

    }
}





void CMC_X2_WAITING_FOR_X2_RESPONSE_DATA_StateHandler(CMC_X2_COMMS_CONTEXT* pContext, CMC_X2_FSM_EVENT event)
{
    switch (event)
    {
        case(EVENT_RESET_REQUEST):
        {
            CMC_X2_FSM_AddAction(pContext, ACTION_STOP_TIMER);
            CMC_X2_FSM_AddAction(pContext, ACTION_DEASSERT_GPIO);
            CMC_X2_FSM_AddAction(pContext, ACTION_CLEAR_STATUS);
            CMC_X2_FSM_AddAction(pContext, ACTION_RAISE_RESET_REQUEST);
            CMC_X2_FSM_ChangeState(pContext, STATE_IDLE);
            break;
        }


        case(EVENT_TIMER_EXPIRY):
        {
            CMC_X2_FSM_AddAction(pContext, ACTION_STOP_TIMER);
            CMC_X2_FSM_AddAction(pContext, ACTION_DEASSERT_GPIO);
            CMC_X2_FSM_AddAction(pContext, ACTION_CLEAR_STATUS);
            CMC_X2_FSM_AddAction(pContext, ACTION_INCREMENT_RESPONSE_TIMEOUT_COUNT);
            CMC_X2_FSM_ChangeState(pContext, STATE_IDLE);
            break;
        }


        case(EVENT_NEW_USER_REQUEST_ARRIVED):
        {
            CMC_X2_FSM_AddAction(pContext, ACTION_INCREMENT_REQUEST_ALREADY_IN_PROGRESS_COUNT);
            break;
        }


        case(EVENT_X2_WROTE_RESPONSE_DATA):
        {
            CMC_X2_FSM_AddAction(pContext, ACTION_STOP_TIMER);
            CMC_X2_FSM_AddAction(pContext, ACTION_CLEAR_STATUS);
            CMC_X2_FSM_AddAction(pContext, ACTION_SEND_RESPONSE);
            CMC_X2_FSM_ChangeState(pContext, STATE_IDLE);
            break;
        }


        default:
        {
            Watch_Inc(pContext->pWatchPointContext, W_X2_COMMS_FSM_UNEXPECTED_EVENT);
            break;
        }


    }
}



