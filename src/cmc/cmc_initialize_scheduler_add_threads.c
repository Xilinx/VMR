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
 *  $Change: 3095780 $
 *  $Date: 2021/01/13 $
 *  $Revision: #12 $
 *
 */




#include "cmc_initialization.h"
#include "cmc_ram.h"
#include "cmc_scheduler.h"



void cmc_Initialize_SchedulerAddThreads(void)
{
    Scheduler_AddThread(                    &SchedulerContext,
                                            THREAD_LOOP_ACTIVITY,
                                            Scheduler_LoopActivityThread,
                                            &SchedulerContext);

    Scheduler_AddThread(                    &SchedulerContext,
                                            THREAD_WATCHDOG,
                                            WatchdogThread_Schedule,
                                            &WatchdogThreadContext);

    Scheduler_AddThread(                    &SchedulerContext,
                                            THREAD_TIMER,
                                            TimerThread_Schedule,
                                            &ThreadTimerContext);

    Scheduler_AddThread(                    &SchedulerContext,
                                            THREAD_LOCAL_LINK_SUPERVISOR,
                                            LinkStateThread_Schedule,
                                            &LinkStateThreadContext);

    Scheduler_AddThread(                    &SchedulerContext,
                                            THREAD_LINK_RECEIVE,
                                            LinkReceiveThread_Schedule,
                                            &LinkReceiveThreadContext);

    Scheduler_AddThread(                    &SchedulerContext,
                                            THREAD_LOCAL_BOOTLOADER,
                                            LocalBootloaderThread_Schedule,
                                            &LocalBootloaderThreadContext);

    Scheduler_AddThread(                    &SchedulerContext,
                                            THREAD_HOST_USER_PROXY,
                                            HostUserProxyThread_Schedule,
                                            &HostUserProxyThreadContext);

    Scheduler_AddThread(                    &SchedulerContext,
                                            THREAD_LOCAL_SENSOR_SUPERVISOR,
                                            LocalSensorSupervisorThread_Schedule,
                                            &LocalSensorSupervisorThreadContext);

    Scheduler_AddThread(                    &SchedulerContext,
                                            THREAD_MUTEX_OBSERVER,
                                            MutexObserverThread_Schedule,
                                            &ThreadMutexObserver_Context);

    /*Scheduler_AddThread(                    &SchedulerContext,
                                            THREAD_CLOCK_THROTTLING,
                                            CMC_ClockThrottlingThread_Schedule,
                                            &ClockThrottlingThreadContext);*/


    Scheduler_AddThread(                    &SchedulerContext,
                                            THREAD_X2_COMMS,
                                            X2_Comms_Thread_Schedule,
                                            &X2CommsContext);

}


