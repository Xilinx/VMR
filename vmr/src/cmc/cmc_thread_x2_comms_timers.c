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
 *  $Revision: #2 $
 *
 */

#include "cmc_thread_x2_comms.h"
#include "cmc_thread_x2_comms_constants.h"



void CMC_X2_StartTimer(CMC_X2_COMMS_CONTEXT* pContext)
{
    pContext->currentTimerInstance = TimerThread_Start(pContext->pThreadTimerContext,
                                                       T_X2_COMMS_TIMER,
                                                       X2_COMMS_TIMER_DURATION_MS,
                                                       pContext,
                                                       CMC_X2_TimerCallback);

}




void CMC_X2_StopTimer(CMC_X2_COMMS_CONTEXT* pContext)
{
    TimerThread_Stop(pContext->pThreadTimerContext, T_X2_COMMS_TIMER);
}






void CMC_X2_RestartTimer(CMC_X2_COMMS_CONTEXT* pContext)
{
    pContext->currentTimerInstance = TimerThread_Restart(pContext->pThreadTimerContext, T_X2_COMMS_TIMER);
}







void CMC_X2_TimerCallback(void* pCallbackContext, uint32_t timerInstance)
{
    CMC_X2_COMMS_CONTEXT* pX2CommsContext = (CMC_X2_COMMS_CONTEXT*)pCallbackContext;

    if (timerInstance == (pX2CommsContext->currentTimerInstance))
    {    
       CMC_X2_FSM_AddEvent(pX2CommsContext, EVENT_TIMER_EXPIRY);
    }
}






