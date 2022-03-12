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
 *  $Change: 2712364 $
 *  $Date: 2019/11/12 $
 *  $Revision: #10 $
 *
 */




#include "cmc_thread_timer.h"


void TimerThread_TimeDifference_UpdateEnter(THREAD_TIMER_CONTEXT_TYPE *pContext)
{
    ThreadTimeDifferenceMetaData_UpdateEnter(&pContext->ArbitraryTimeDifferenceContext);
}


void TimerThread_TimeDifference_UpdateExit(THREAD_TIMER_CONTEXT_TYPE *pContext)
{
    ThreadTimeDifferenceMetaData_UpdateExit(&pContext->ArbitraryTimeDifferenceContext);
}


uint32_t TimerThread_Start(THREAD_TIMER_CONTEXT_TYPE *pContext, CMC_THREAD_TIMER_TYPE AnyTimer, uint32_t Duration_ms, void * pCallbackContext, TIMER_CALLBACK_TYPE pFn_Callback)
{
    uint32_t Result=0;
    uint32_t StartTime_Ticks=0;

    if(T_MAX_TIMERS>AnyTimer)
    {
        StartTime_Ticks=PERIPHERAL_FREERUNNING_CLOCK_READ_TIME_Ticks(pContext->pPeripheral_FreeRunningClock_Context);       
        Result=TimerThread_TimerElement_Start(&(pContext->Element[AnyTimer]), StartTime_Ticks, Duration_ms, pCallbackContext, pFn_Callback);
    }

    return Result;
}


uint32_t TimerThread_Restart(THREAD_TIMER_CONTEXT_TYPE *pContext, CMC_THREAD_TIMER_TYPE AnyTimer)
{
    uint32_t Result=0;
    uint32_t StartTime_Ticks=0;

    if(T_MAX_TIMERS>AnyTimer)
    {
        StartTime_Ticks=PERIPHERAL_FREERUNNING_CLOCK_READ_TIME_Ticks(pContext->pPeripheral_FreeRunningClock_Context);
        Result=TimerThread_TimerElement_Restart(&(pContext->Element[AnyTimer]), StartTime_Ticks);
    }

    return Result;
}


void TimerThread_Stop(THREAD_TIMER_CONTEXT_TYPE *pContext, CMC_THREAD_TIMER_TYPE AnyTimer)
{
    if(T_MAX_TIMERS>AnyTimer)
    {
        TimerThread_TimerElement_Stop(&(pContext->Element[AnyTimer]));
    }
}



void TimerThread_Initialize(THREAD_TIMER_CONTEXT_TYPE *pContext, PERIPHERAL_FREERUNNING_CLOCK_CONTEXT_TYPE * pPeripheral_FreeRunningClock_Context, CMC_WATCHPOINT_CONTEXT_TYPE * pWatchPointContext)
{
    uint32_t i;

    pContext->pPeripheral_FreeRunningClock_Context=pPeripheral_FreeRunningClock_Context;
    pContext->pWatchPointContext=pWatchPointContext;

    for(i=0;i<T_MAX_TIMERS;i++)
    {
        TimerThread_TimerElement_Initialize(&(pContext->Element[i]));
    }

    ThreadTimeDifferenceMetaData_Initialize(   &pContext->ArbitraryTimeDifferenceContext,
                                               pContext->pPeripheral_FreeRunningClock_Context,
                                               pContext->pWatchPointContext,
                                               W_ARBITRARY_TIME_DIFFERENCE_CURRENT,
                                               W_ARBITRARY_TIME_DIFFERENCE_MAX);  

}
