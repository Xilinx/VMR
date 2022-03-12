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
 *  $Change: 2681821 $
 *  $Date: 2019/10/02 $
 *  $Revision: #10 $
 *
 */




#include "cmc_thread_timer.h"

bool TimerThread_Schedule(void *pContext)
{
    uint32_t iTimer;
    uint32_t Now_Ticks=0;
    uint32_t ElapsedTicks=0;
    uint32_t TimerDurationTicks=0;

    THREAD_TIMER_CONTEXT_TYPE *pThreadContext=(THREAD_TIMER_CONTEXT_TYPE *)pContext;

    Now_Ticks=PERIPHERAL_FREERUNNING_CLOCK_READ_TIME_Ticks(pThreadContext->pPeripheral_FreeRunningClock_Context);
    for(iTimer=0;iTimer<T_MAX_TIMERS;iTimer++)
    {
        if(pThreadContext->Element[iTimer].IsRunning)
        {
            ElapsedTicks=Now_Ticks-pThreadContext->Element[iTimer].StartTime_Ticks;
            TimerDurationTicks=PERIPHERAL_FREERUNNING_CLOCK_READ_TIME_ConvertMillisecondsClockTicks(    pThreadContext->pPeripheral_FreeRunningClock_Context,
                                                                                                        pThreadContext->Element[iTimer].Duration_ms);
            if(ElapsedTicks>=TimerDurationTicks)
            {
                if(NULL!=pThreadContext->Element[iTimer].pFn_Callback)
                {
                    (*pThreadContext->Element[iTimer].pFn_Callback)(    pThreadContext->Element[iTimer].pCallbackContext,
                                                                        pThreadContext->Element[iTimer].Instance);

                    pThreadContext->Element[iTimer].IsRunning=false;
                }
            }
        }
    }

    Watch_Inc(pThreadContext->pWatchPointContext, W_THREAD_TIMER);

    return true;
}






