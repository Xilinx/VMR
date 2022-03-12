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
 *  $Change: 2751699 $
 *  $Date: 2020/01/14 $
 *  $Revision: #3 $
 *
 */


#include "cmc_thread_x2_comms.h"



void CMC_X2_FSM_Run(CMC_X2_COMMS_CONTEXT* pContext, CMC_X2_FSM_EVENT event)
{

    Watch_Set(pContext->pWatchPointContext, W_X2_COMMS_FSM_LAST_EVENT, event);

    CMC_X2_FSM_ClearActions(pContext);

    switch (pContext->currentState)
    {
        case(STATE_IDLE):
        {
            CMC_X2_IDLE_StateHandler(pContext, event);
            break;
        }


        case(STATE_WAITING_FOR_X2_READ_STATUS):
        {
            CMC_X2_WAITING_FOR_X2_READ_STATUS_StateHandler(pContext, event);
            break;
        }



        case(STATE_WAITING_FOR_X2_READ_REQUEST_LENGTH):
        {
            CMC_X2_WAITING_FOR_X2_READ_REQUEST_LENGTH_StateHandler(pContext, event);
            break;
        }


        case(STATE_WAITING_FOR_X2_READ_REQUEST_DATA):
        {
            CMC_X2_WAITING_FOR_X2_READ_REQUEST_DATA_StateHandler(pContext, event);
            break;
        }



        case(STATE_WAITING_FOR_X2_RESPONSE_LENGTH):
        {
            CMC_X2_WAITING_FOR_X2_RESPONSE_LENGTH_StateHandler(pContext, event);
            break;
        }




        case(STATE_WAITING_FOR_X2_RESPONSE_DATA):
        {
            CMC_X2_WAITING_FOR_X2_RESPONSE_DATA_StateHandler(pContext, event);
            break;
        }


        default:
        {

            Watch_Inc(pContext->pWatchPointContext, W_X2_COMMS_FSM_ILLEGAL_STATE);
            break;
        }
    }



    //Perform any actions that may have resulted from the event....
    CMC_X2_FSM_PerformActions(pContext);
}









void CMC_X2_FSM_AddEvent(CMC_X2_COMMS_CONTEXT* pContext, CMC_X2_FSM_EVENT event)
{
    if (!CircularBuffer_TryWrite(&(pContext->eventBuffer), event))
    {
        Watch_Inc(pContext->pWatchPointContext, W_X2_COMMS_FSM_FAILED_TO_ADD_EVENT);
    }
}

