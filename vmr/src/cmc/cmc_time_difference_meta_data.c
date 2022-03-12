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
 *  $Change: 2718162 $
 *  $Date: 2019/11/19 $
 *  $Revision: #5 $
 *
 */


#include "cmc_time_difference_meta_data.h"






void ThreadTimeDifferenceMetaData_Initialize(   TIME_DIFFERENCE_META_DATA_CONTEXT_TYPE *pContext,
                                                PERIPHERAL_FREERUNNING_CLOCK_CONTEXT_TYPE * pPeripheral_FreeRunningClock_Context,
                                                CMC_WATCHPOINT_CONTEXT_TYPE * pWatchPointContext,
                                                uint32_t Watch_RuntimeTicks,
                                                uint32_t Watch_MaxRuntimeTicks)
{
    pContext->pPeripheral_FreeRunningClock_Context=pPeripheral_FreeRunningClock_Context;
    pContext->pWatchPointContext=pWatchPointContext;

    pContext->EnterTicks=0;
    pContext->ExitTicks=0;
    pContext->MaxRunTimeTicks=0;
    pContext->RunTimeTicks=0;

    pContext->Watch_RuntimeTicks=Watch_RuntimeTicks;
    pContext->Watch_MaxRuntimeTicks=Watch_MaxRuntimeTicks;
}






void ThreadTimeDifferenceMetaData_UpdateEnter(TIME_DIFFERENCE_META_DATA_CONTEXT_TYPE *pContext)
{

    pContext->EnterTicks=PERIPHERAL_FREERUNNING_CLOCK_READ_TIME_Ticks(pContext->pPeripheral_FreeRunningClock_Context);
}


void ThreadTimeDifferenceMetaData_UpdateExit(TIME_DIFFERENCE_META_DATA_CONTEXT_TYPE *pContext)
{

    pContext->ExitTicks=PERIPHERAL_FREERUNNING_CLOCK_READ_TIME_Ticks(pContext->pPeripheral_FreeRunningClock_Context);
    pContext->RunTimeTicks=pContext->ExitTicks-pContext->EnterTicks;
    if(pContext->MaxRunTimeTicks<pContext->RunTimeTicks)
    {
        pContext->MaxRunTimeTicks=pContext->RunTimeTicks;
    }

    Watch_Set(pContext->pWatchPointContext, pContext->Watch_RuntimeTicks, pContext->RunTimeTicks);
    Watch_Set(pContext->pWatchPointContext, pContext->Watch_MaxRuntimeTicks, pContext->MaxRunTimeTicks);
} 