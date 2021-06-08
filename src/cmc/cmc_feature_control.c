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
 *  $Revision: #21 $
 *
 */


#include "cmc_feature_control.h"

void FeatureControl_InterpretConfigurationRAM_0(CMC_FEATURE_CONTROL_CONTEXT_TYPE * pContext)
{
    uint32_t FeatureSupportedBySCVersionMask = (1 << 30);
    uint32_t RegisterValue= pContext->Raw_NewFeatureConfigurationUpdate[FEATURE_RAM_CLOCK_THROTTLING_FEATURE];
    long BaseAddress = RegisterValue & 0x01ffffff;
    uint32_t Flag_FeatureEnabled = ((RegisterValue >> 28) & 0x00000001);
    uint32_t Flag_FeaturePresent = ((RegisterValue >> 29) & 0x00000001);

    // Feature Control
    pContext->ClockThrottlingFeature.BaseAddress = (CMC_PERIPHERAL_BASE_ADDRESS_TYPE)BaseAddress;
    pContext->ClockThrottlingFeature.FeatureEnabled = Flag_FeatureEnabled ? true : false;
    pContext->ClockThrottlingFeature.FeaturePresent = Flag_FeaturePresent ? true : false;

    // Clock Throttling 
    CMC_ClockThrottling_FeatureEnable(pContext->pClockThrottlingContext, (Flag_FeatureEnabled ? true : false));

    if (pContext->pClockThrottlingContext->bFeatureSupportedBySCVersion)
    {
        PERIPHERAL_REGMAP_RAM_CONTROLLER_WriteWithMask(pContext->pRegMap_RAM_Controller_Context, HOST_REGISTER_LOCATION_NEW_FEATURE_REGISTER_00, 0x0, FeatureSupportedBySCVersionMask);
    }
    else
    {
        PERIPHERAL_REGMAP_RAM_CONTROLLER_WriteWithMask(pContext->pRegMap_RAM_Controller_Context, HOST_REGISTER_LOCATION_NEW_FEATURE_REGISTER_00, FeatureSupportedBySCVersionMask, FeatureSupportedBySCVersionMask);
    }

    switch( pContext->pClockThrottlingContext->Algorithm )
    {
        case CT_ALGORITHM_STANDARD:
        {
            // Peripheral
            pContext->pClockThrottlingContext->pClockThrottlingPeripheralContext->IsAvailable = Flag_FeaturePresent ? true : false;
            pContext->pClockThrottlingContext->pClockThrottlingPeripheralContext->BaseAddress = (CMC_PERIPHERAL_BASE_ADDRESS_TYPE)BaseAddress;
        }
        break;

        case CT_ALGORITHM_SSD:
            break;

        case CT_ALGORITHM_NONE:
        default:
            break;
    }
}




void FeatureControl_ReadConfigurationRAM_0(CMC_FEATURE_CONTROL_CONTEXT_TYPE * pContext)
{
    /*
     * For initial release only one of the 32 configuration registers is defined - only read this register.
     *
     */

    uint32_t RegisterValue=PERIPHERAL_REGMAP_RAM_CONTROLLER_Read(pContext->pRegMap_RAM_Controller_Context, HOST_REGISTER_LOCATION_NEW_FEATURE_REGISTER_00);

    pContext->Raw_NewFeatureConfigurationUpdate[FEATURE_RAM_CLOCK_THROTTLING_FEATURE]=RegisterValue;
}


//void FeatureControl_WriteConfigurationRAM_0_bFeatureSupportedBySCVersion(CMC_FEATURE_CONTROL_CONTEXT_TYPE* pContext, bool bFeatureSupportedBySCVersion)
//{
//    /*
//     * For initial release only one of the 32 configuration registers is defined - only read this register.
//     *
//     */
//    uint32_t Mask = (1 << 30);
//
//    if (bFeatureSupportedBySCVersion)
//    {
//        PERIPHERAL_REGMAP_RAM_CONTROLLER_WriteWithMask(pContext->pRegMap_RAM_Controller_Context, HOST_REGISTER_LOCATION_NEW_FEATURE_REGISTER_00, Mask, 0);
//    }
//    else
//    {
//        PERIPHERAL_REGMAP_RAM_CONTROLLER_WriteWithMask(pContext->pRegMap_RAM_Controller_Context, HOST_REGISTER_LOCATION_NEW_FEATURE_REGISTER_00, Mask, Mask);
//    }
//}


    

void FeatureControl_InterpretConfigurationRAM_1(CMC_FEATURE_CONTROL_CONTEXT_TYPE* pContext)
{
    uint32_t RegisterValue = pContext->Raw_NewFeatureConfigurationUpdate[FEATURE_RAM_CLOCK_THROTTLING_POWER];
    uint32_t PowerOverRideEnabled = ((RegisterValue >> 31) & 0x00000001);
    uint32_t PowerOverRideValue = ((RegisterValue) & 0x000000FF);

    // Feature Control
    pContext->ClockThrottlingFeature.PowerOverRideEnabled = PowerOverRideEnabled ? true : false;
    pContext->ClockThrottlingFeature.PowerOverRideValue = PowerOverRideValue;

    pContext->pClockThrottlingContext->XRTSuppliedBoardThrottlingThresholdPower = PowerOverRideValue * 1000000;
    pContext->pClockThrottlingContext->PowerOverRideEnabled = PowerOverRideEnabled ? true : false;

    switch (pContext->pClockThrottlingContext->Algorithm)
    {
        case CT_ALGORITHM_STANDARD:
        {
            // Clock Throttling
            PERIPHERAL_ClockThrottling_SetPowerOverRideEnabled(pContext->pClockThrottlingContext->pClockThrottlingPeripheralContext, pContext->ClockThrottlingFeature.PowerOverRideEnabled);
            PERIPHERAL_ClockThrottling_SetPowerOverRideValue(pContext->pClockThrottlingContext->pClockThrottlingPeripheralContext, pContext->ClockThrottlingFeature.PowerOverRideValue);
        }
        break;

        case CT_ALGORITHM_SSD:
            break;

        case CT_ALGORITHM_NONE:
        default:
            break;
    }

}




void FeatureControl_ReadConfigurationRAM_1(CMC_FEATURE_CONTROL_CONTEXT_TYPE* pContext)
{
    /*
     * For initial release only one of the 32 configuration registers is defined - only read this register.
     *
     */

    uint32_t RegisterValue = PERIPHERAL_REGMAP_RAM_CONTROLLER_Read(pContext->pRegMap_RAM_Controller_Context, HOST_REGISTER_LOCATION_NEW_FEATURE_REGISTER_01);

    pContext->Raw_NewFeatureConfigurationUpdate[FEATURE_RAM_CLOCK_THROTTLING_POWER] = RegisterValue;
}


void FeatureControl_InterpretConfigurationRAM_2(CMC_FEATURE_CONTROL_CONTEXT_TYPE* pContext)
{
    uint32_t RegisterValue = pContext->Raw_NewFeatureConfigurationUpdate[FEATURE_RAM_CLOCK_THROTTLING_TEMPERATURE];
    uint32_t bUserThrottlingTempEnabled = ((RegisterValue >> 31) & 0x00000001);
    uint32_t UserThrottlingTempValue = ((RegisterValue) & 0x000000FF);

    // Feature Control
    pContext->ClockThrottlingFeature.bUserThrottlingTempEnabled = bUserThrottlingTempEnabled ? true : false;
    pContext->ClockThrottlingFeature.UserThrottlingTempValue = UserThrottlingTempValue;

    pContext->pClockThrottlingContext->XRTSuppliedUserThrottlingTempLimit = UserThrottlingTempValue;
    pContext->pClockThrottlingContext->bUserThrottlingTempLimitEnabled = bUserThrottlingTempEnabled ? true : false;

    switch (pContext->pClockThrottlingContext->Algorithm)
    {
        case CT_ALGORITHM_STANDARD:
        {
            // Clock Throttling
            PERIPHERAL_ClockThrottling_SetUserThrottlingTempEnabled(pContext->pClockThrottlingContext->pClockThrottlingPeripheralContext, pContext->ClockThrottlingFeature.bUserThrottlingTempEnabled);
            PERIPHERAL_ClockThrottling_SetUserThrottlingTempValue(pContext->pClockThrottlingContext->pClockThrottlingPeripheralContext, pContext->ClockThrottlingFeature.UserThrottlingTempValue);
        }
        break;

        case CT_ALGORITHM_SSD:
            break;

        case CT_ALGORITHM_NONE:
        default:
            break;
    }

}




void FeatureControl_ReadConfigurationRAM_2(CMC_FEATURE_CONTROL_CONTEXT_TYPE* pContext)
{
    /*
     * For initial release only one of the 32 configuration registers is defined - only read this register.
     *
     */

    uint32_t RegisterValue = PERIPHERAL_REGMAP_RAM_CONTROLLER_Read(pContext->pRegMap_RAM_Controller_Context, HOST_REGISTER_LOCATION_NEW_FEATURE_REGISTER_02);

    pContext->Raw_NewFeatureConfigurationUpdate[FEATURE_RAM_CLOCK_THROTTLING_TEMPERATURE] = RegisterValue;
}
void FeatureControl_ConfigurationUpdateSequence(CMC_FEATURE_CONTROL_CONTEXT_TYPE * pContext)
{
    FeatureControl_ReadConfigurationRAM_0(pContext);
    FeatureControl_InterpretConfigurationRAM_0(pContext);
    //FeatureControl_PushConfiguration_0(pContext);

    FeatureControl_ReadConfigurationRAM_1(pContext);
    FeatureControl_InterpretConfigurationRAM_1(pContext);
    //FeatureControl_PushConfiguration_1(pContext);

    FeatureControl_ReadConfigurationRAM_2(pContext);
    FeatureControl_InterpretConfigurationRAM_2(pContext);
}


void FeatureControl_ReachOutInterfaceUpdate(CMC_FEATURE_CONTROL_CONTEXT_TYPE * pContext)
{ 
    FeatureControl_ConfigurationUpdateSequence(pContext);
}


void FeatureControl_GetFeatureStatus(CMC_FEATURE_CONTROL_CONTEXT_TYPE * pContext, FEATURE_CONTROL_ITEM_TYPE iFeature, bool * pIsEnabled)
{
    FeatureControl_FeatureSet_GetFeature_Status(pContext->FeatureIsEnabled, iFeature, pIsEnabled);
}




void FeatureControl_SetFeatureStatus(CMC_FEATURE_CONTROL_CONTEXT_TYPE * pContext, FEATURE_CONTROL_ITEM_TYPE iFeature, bool IsEnabled)
{
    FeatureControl_FeatureSet_SetFeature_Status(pContext->FeatureIsEnabled, iFeature, IsEnabled);
}




void FeatureControl_Initialize( CMC_FEATURE_CONTROL_CONTEXT_TYPE * pContext,
                                PERIPHERAL_REGMAP_RAM_CONTROLLER_CONTEXT_TYPE * pRegMap_RAM_Controller_Context,
                                CMC_CLOCK_THROTTLING_CONTEXT_TYPE *             pClockThrottlingContext,
                                bool * pFeatureSet)
{
    pContext->pRegMap_RAM_Controller_Context=pRegMap_RAM_Controller_Context;
    pContext->pClockThrottlingContext=pClockThrottlingContext;

    FeatureControl_FeatureSet_Initialize(pContext->FeatureIsEnabled, pFeatureSet);
}



