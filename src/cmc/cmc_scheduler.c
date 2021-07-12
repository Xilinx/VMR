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
 *  $Change: 2755114 $
 *  $Date: 2020/01/17 $
 *  $Revision: #14 $
 *
 */



#include "cmc_scheduler.h"
#include "FreeRTOS.h"
#include "task.h"


uint32_t NextThread(uint32_t AnyThread)
{
    uint32_t Result=AnyThread;

    Result++;
    if(THREAD_MAX_THREADS<=Result)
    {
        Result=0;
    }

    return Result;
}



void Scheduler(SCHEDULER_CONTEXT_TYPE *pContext)
{
    bool StillProcessing=true;
    bool ThreadFound;
    const TickType_t yieldms = pdMS_TO_TICKS( 100*1 );

    SchedulerEnter(pContext);
    while(StillProcessing)
    {
	vTaskDelay( yieldms );
        ThreadTimeDifferenceMetaData_UpdateEnter(&pContext->Element[pContext->iElement].Meta);
        StillProcessing=(*pContext->Element[pContext->iElement].pFN_Thread)(pContext->Element[pContext->iElement].pThreadContext);
        ThreadTimeDifferenceMetaData_UpdateExit(&pContext->Element[pContext->iElement].Meta);
        
        ThreadFound=false;
        while(!ThreadFound)
        {
            pContext->iElement=NextThread(pContext->iElement);
            if(!pContext->Element[pContext->iElement].IsFree)
            {
                ThreadFound=true;
            }
        }
    }
    SchedulerExit(pContext);
}



void Scheduler_AddThread(SCHEDULER_CONTEXT_TYPE *pContext, uint32_t iLocation, bool (*pFN_Thread)(void * pContext), void *  pThreadContext)
{
    uint32_t Watch_RuntimeTicks=(W_RUNTIME_THREAD_LOOP_ACTIVITY+iLocation);
    uint32_t Watch_MaxRuntimeTicks=(W_MAX_RUNTIME_THREAD_LOOP_ACTIVITY+iLocation);

    if(pContext->Element[iLocation].IsFree)
    {
        SchedulerElementAddThread(&(pContext->Element[iLocation]), pFN_Thread, pThreadContext);

        ThreadTimeDifferenceMetaData_Initialize(&(pContext->Element[iLocation].Meta),
                                                pContext->pPeripheral_FreeRunningClock_Context,
                                                pContext->pWatchPointContext,
                                                Watch_RuntimeTicks,
                                                Watch_MaxRuntimeTicks);

    }
}

void Scheduler_Initialize(          SCHEDULER_CONTEXT_TYPE *                            pContext,
                                    CMC_WATCHPOINT_CONTEXT_TYPE *                       pWatchPointContext, 
                                    PERIPHERAL_REGMAP_RAM_CONTROLLER_CONTEXT_TYPE *     pPeripheralRegMapRamControllerContext,
                                    PERIPHERAL_FREERUNNING_CLOCK_CONTEXT_TYPE *         pPeripheral_FreeRunningClock_Context,
                                    PERIPHERAL_AXI_GPIO_WDT_CONTEXT_TYPE*               pPeripheral_AXI_GPIO_Wdt_Context,
                                    PERIPHERAL_AXI_GPIO_MUTEX_CMC_CONTEXT_TYPE*         pPeripheral_AXI_GPIO_Mutex_CMC_Context
)
{
    uint32_t iElement;

    pContext->iElement=0;
    pContext->pWatchPointContext=pWatchPointContext;
    pContext->pPeripheralRegMapRamControllerContext=pPeripheralRegMapRamControllerContext;
    pContext->pPeripheral_FreeRunningClock_Context=pPeripheral_FreeRunningClock_Context;
    pContext->pPeripheral_AXI_GPIO_Wdt_Context = pPeripheral_AXI_GPIO_Wdt_Context;
    pContext->pPeripheral_AXI_GPIO_Mutex_CMC_Context = pPeripheral_AXI_GPIO_Mutex_CMC_Context;

    for(iElement=0;iElement<THREAD_MAX_THREADS;iElement++)
    {
        SchedulerElementInitialize(&(pContext->Element[iElement]));
    }
}






