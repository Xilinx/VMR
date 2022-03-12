/*----------------------------------------------------------------------------------------------*/
/*                                                                                              */
/* ? Copyright 2019 Xilinx, Inc. All rights reserved.                                           */
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


/* PROFILE_LINUX_VERSAL_VCK5000_R5 */ 
/* LINUX_DIST */





/*
 *
 *  RCS Keyword Metadata
 *
 *  $Change: 3095780 $
 *  $Date: 2021/01/13 $
 *  $Revision: #5 $
 *
 */


//#define _POSIX_MONOTONIC_CLOCK
//#define _POSIX_TIMERS
//#define _POSIX_CLOCK_SELECTION


#include "cmc_profile_versal_VCK5000_R5.h"
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "timers.h"
#include "event_groups.h"
#include "xil_printf.h"

#define NANOSECS_PER_SEC  1000000000L
#define NANOSECS_PER_CLOCKTICK  20


uint32_t CMC_BuildProfile_Versal_VCK5000_R5_Peripheral_Clock_UserContext=0;



void CMC_BuildProfile_Versal_VCK5000_R5_Peripheral_Clock_Initialize(void *pUserContext, CMC_PERIPHERAL_BASE_ADDRESS_TYPE BaseAddress)
{
    UNUSED(pUserContext);
    UNUSED(BaseAddress);
}

uint32_t CMC_BuildProfile_Versal_VCK5000_R5_Peripheral_Clock_GetTicks(void *pUserContext, CMC_PERIPHERAL_BASE_ADDRESS_TYPE BaseAddress)
{
    TickType_t tickCount;

    UNUSED(pUserContext);
    UNUSED(BaseAddress);

    tickCount = xTaskGetTickCount();
    return (uint32_t)tickCount;
}


void CMC_BuildProfile_Versal_VCK5000_R5_Peripheral_Clock(CMC_BUILD_PROFILE_TYPE * pProfile)
{
    pProfile->Peripherals.CLOCK.pUserContext=&CMC_BuildProfile_Versal_VCK5000_R5_Peripheral_Clock_UserContext;
    pProfile->Peripherals.CLOCK.TicksPerMillisecond= 1; //portTICK_PERIOD_MS; //(1000000 / 20);
    pProfile->Peripherals.CLOCK.pFN_Initialize=CMC_BuildProfile_Versal_VCK5000_R5_Peripheral_Clock_Initialize;
    pProfile->Peripherals.CLOCK.pFN_GetTicks=CMC_BuildProfile_Versal_VCK5000_R5_Peripheral_Clock_GetTicks;
}



