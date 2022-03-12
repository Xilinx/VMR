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
 *  $Revision: #19 $
 *
 */




#ifndef _CMC_FEATURE_CONTROL_H_
#define _CMC_FEATURE_CONTROL_H_


#ifdef __cplusplus
extern "C"
{
#endif



#include "cmc_common.h"
#include "cmc_peripheral_regmap_ram_controller.h"
#include "cmc_peripheral_clock_throttling.h"
#include "cmc_feature_control_feature_list.h"
#include "cmc_feature_control_feature_set.h"
#include "cmc_feature_control_constants.h"
#include "cmc_clock_throttling.h"

typedef struct CMC_FEATURE_CONTROL_CLOCK_THROTTLING_TYPE
{
    CMC_PERIPHERAL_BASE_ADDRESS_TYPE    BaseAddress;
    bool                                FeaturePresent;
    bool                                FeatureEnabled;
    bool                                PowerOverRideEnabled;
    uint32_t                            PowerOverRideValue;

    bool                                bUserThrottlingTempEnabled;
    uint32_t                            UserThrottlingTempValue;

} CMC_FEATURE_CONTROL_CLOCK_THROTTLING_TYPE;


typedef struct CMC_FEATURE_CONTROL_CONTEXT_TYPE
{
    PERIPHERAL_REGMAP_RAM_CONTROLLER_CONTEXT_TYPE * pRegMap_RAM_Controller_Context;
    CMC_CLOCK_THROTTLING_CONTEXT_TYPE *             pClockThrottlingContext;
    uint32_t                                        Raw_NewFeatureConfigurationUpdate[MAX_NEW_FEATURE_REGISTERS];
    bool                                            ReachoutInterfaceAccessIsGranted;

    bool    FeatureIsEnabled[FC_MAX_FEATURES];
    uint8_t MaxNumberOfCages;
    CMC_FEATURE_CONTROL_CLOCK_THROTTLING_TYPE       ClockThrottlingFeature;

} CMC_FEATURE_CONTROL_CONTEXT_TYPE;



void FeatureControl_ReachOutInterfaceUpdate(CMC_FEATURE_CONTROL_CONTEXT_TYPE * pContext);

void FeatureControl_GetFeatureStatus(CMC_FEATURE_CONTROL_CONTEXT_TYPE * pContext, FEATURE_CONTROL_ITEM_TYPE iFeature, bool * pIsEnabled);

void FeatureControl_SetFeatureStatus(CMC_FEATURE_CONTROL_CONTEXT_TYPE * pContext, FEATURE_CONTROL_ITEM_TYPE iFeature, bool IsEnabled);

void FeatureControl_Initialize( CMC_FEATURE_CONTROL_CONTEXT_TYPE * pContext,
                                PERIPHERAL_REGMAP_RAM_CONTROLLER_CONTEXT_TYPE * pRegMap_RAM_Controller_Context,
                                CMC_CLOCK_THROTTLING_CONTEXT_TYPE *             pClockThrottlingContext,
                                bool * pFeatureSet);


//void FeatureControl_WriteConfigurationRAM_0_bFeatureSupportedBySCVersion(CMC_FEATURE_CONTROL_CONTEXT_TYPE* pContext, bool bFeatureSupportedBySCVersion);


#ifdef __cplusplus
}
#endif


#endif /* _CMC_FEATURE_CONTROL_H_ */



