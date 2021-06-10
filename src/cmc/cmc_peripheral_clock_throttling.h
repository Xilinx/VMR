/*----------------------------------------------------------------------------------------------*/
/*                                                                                              */
/* � Copyright 2019 Xilinx, Inc. All rights reserved.                                           */
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
 *  $Change: 2921002 $
 *  $Date: 2020/06/22 $
 *  $Revision: #11 $
 *
 */





#ifndef _CMC_PERIPHERAL_CLOCK_THROTTLING_H_
#define _CMC_PERIPHERAL_CLOCK_THROTTLING_H_


#ifdef __cplusplus
extern "C"
{
#endif

#include "cmc_common.h"
#include "cmc_peripherals_access.h"
#include "cmc_required_environment.h"


#define PERIPHERAL_AXI_CLOCK_THROTTLING_REGISTER_SET_SIZE   (1)

#define PERIPHERAL_AXI_CLOCK_THROTTLING_OFFSET_THRT_EN      (0)


typedef struct PERIPHERAL_CLOCK_THROTTLING_CONTEXT_TYPE
{
    CMC_PERIPHERAL_BASE_ADDRESS_TYPE BaseAddress;
    bool                IsAvailable;
    bool                PowerOverRideEnabled;
    uint32_t            PowerOverRideValue;
    bool                bUserThrottlingTempEnabled;
    uint32_t            UserThrottlingTempValue;



    CMC_REQUIRED_ENVIRONMENT_CONTEXT_TYPE       * pRequiredEnvironmentContext;

} PERIPHERAL_CLOCK_THROTTLING_CONTEXT_TYPE;


void PERIPHERAL_ClockThrottling_DisableAccess(PERIPHERAL_CLOCK_THROTTLING_CONTEXT_TYPE* pContext);
void PERIPHERAL_ClockThrottling_EnableAccess(PERIPHERAL_CLOCK_THROTTLING_CONTEXT_TYPE* pContext, CMC_PERIPHERAL_BASE_ADDRESS_TYPE BaseAddress);
void PERIPHERAL_ClockThrottling_Initialize(PERIPHERAL_CLOCK_THROTTLING_CONTEXT_TYPE *pContext, CMC_PERIPHERAL_BASE_ADDRESS_TYPE, bool IsAvailable, CMC_REQUIRED_ENVIRONMENT_CONTEXT_TYPE * pRequiredEnvironmentContext);
void PERIPHERAL_ClockThrottling_Start(PERIPHERAL_CLOCK_THROTTLING_CONTEXT_TYPE *pContext);
void PERIPHERAL_ClockThrottling_UpdateThrottlingControl(PERIPHERAL_CLOCK_THROTTLING_CONTEXT_TYPE *pContext, uint32_t Value);
bool PERIPHERAL_ClockThrottling_PowerOverRideEnabled(PERIPHERAL_CLOCK_THROTTLING_CONTEXT_TYPE* pContext);
uint32_t PERIPHERAL_ClockThrottling_GetPowerOverRideValue(PERIPHERAL_CLOCK_THROTTLING_CONTEXT_TYPE* pContext);
void PERIPHERAL_ClockThrottling_SetPowerOverRideEnabled(PERIPHERAL_CLOCK_THROTTLING_CONTEXT_TYPE* pContext, bool Enable);
void PERIPHERAL_ClockThrottling_SetPowerOverRideValue(PERIPHERAL_CLOCK_THROTTLING_CONTEXT_TYPE* pContext, uint32_t PowerOverRideValue);


void PERIPHERAL_ClockThrottling_SetUserThrottlingTempEnabled(PERIPHERAL_CLOCK_THROTTLING_CONTEXT_TYPE* pContext, bool Enable);
void PERIPHERAL_ClockThrottling_SetUserThrottlingTempValue(PERIPHERAL_CLOCK_THROTTLING_CONTEXT_TYPE* pContext, uint32_t UserThrottlingTempValue);

bool PERIPHERAL_ClockThrottling_Read_LatchedClockShutdownRequest(PERIPHERAL_CLOCK_THROTTLING_CONTEXT_TYPE* pContext);
void PERIPHERAL_ClockThrottling_Enable_Throttling(PERIPHERAL_CLOCK_THROTTLING_CONTEXT_TYPE* pContext);
void PERIPHERAL_ClockThrottling_Disable_Throttling(PERIPHERAL_CLOCK_THROTTLING_CONTEXT_TYPE* pContext);
void PERIPHERAL_ClockThrottling_Clear_LatchedShutdownClocks(PERIPHERAL_CLOCK_THROTTLING_CONTEXT_TYPE* pContext);

#ifdef __cplusplus
}
#endif


#endif /* _CMC_PERIPHERAL_CLOCK_THROTTLING_H_ */




















