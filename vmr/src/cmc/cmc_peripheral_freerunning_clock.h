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




#ifndef _CMC_PERIPHERAL_FREERUNNING_CLOCK_H_
#define _CMC_PERIPHERAL_FREERUNNING_CLOCK_H_


#ifdef __cplusplus
extern "C"
{
#endif

#include "cmc_common.h"
#include "cmc_peripherals_access.h"
#include "cmc_required_environment.h"
#include "cmc_entry_point_build_profile_clock.h"


/*
 * Single 32 bit register running at 50MHz
 *
 * 20ns increments, rollover at 85.89934592 seconds.
 *
 */

#define PERIPHERAL_FREERUNNING_CLOCK_REGISTER_SET_SIZE   (1) // TODO



typedef struct PERIPHERAL_FREERUNNING_CLOCK_CONTEXT_TYPE
{
    CMC_PERIPHERAL_BASE_ADDRESS_TYPE                    BaseAddress;
    bool                                                IsAvailable;
    CMC_REQUIRED_ENVIRONMENT_CONTEXT_TYPE       *       pRequiredEnvironmentContext;
    CMC_BUILD_PROFILE_CLOCK_SERVICES_TYPE               ClockServices;

} PERIPHERAL_FREERUNNING_CLOCK_CONTEXT_TYPE;


uint32_t PERIPHERAL_FREERUNNING_CLOCK_READ_TIME_ConvertMillisecondsClockTicks(PERIPHERAL_FREERUNNING_CLOCK_CONTEXT_TYPE * pContext, uint32_t Milliseconds);
uint32_t PERIPHERAL_FREERUNNING_CLOCK_READ_TIME_ConvertClockTicksToMilliseconds(PERIPHERAL_FREERUNNING_CLOCK_CONTEXT_TYPE * pContext, uint32_t ClockTicks);

uint32_t PERIPHERAL_FREERUNNING_CLOCK_READ_TIME_Ticks(PERIPHERAL_FREERUNNING_CLOCK_CONTEXT_TYPE * pPERIPHERAL_FREERUNNING_CLOCK_Context);

void PERIPHERAL_FREERUNNING_CLOCK_Initialize(   PERIPHERAL_FREERUNNING_CLOCK_CONTEXT_TYPE *pContext,
                                                CMC_BUILD_PROFILE_CLOCK_SERVICES_TYPE * pClockServices,
                                                CMC_PERIPHERAL_BASE_ADDRESS_TYPE AnyBaseAddress,
                                                bool IsAvailable,
                                                CMC_REQUIRED_ENVIRONMENT_CONTEXT_TYPE * pRequiredEnvironmentContext);

void PERIPHERAL_FREERUNNING_CLOCK_Start(PERIPHERAL_FREERUNNING_CLOCK_CONTEXT_TYPE *pContext);




#ifdef __cplusplus
}
#endif


#endif /* _CMC_PERIPHERAL_FREERUNNING_CLOCK_H_ */



















