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
 *  $Revision: #5 $
 *
 */





#include "cmc_clock_throttling.h"


void cmc_clock_throttling_phase2_initialise(CMC_CLOCK_THROTTLING_CONTEXT_TYPE* pContext)
{
    UNUSED(pContext);
}

float cmc_clock_throttling_PI_Loop(CMC_CLOCK_THROTTLING_CONTEXT_TYPE* pContext, bool bUserThrottlingTempLimitEnabled, bool bUseIntegrationSum, float PassedInThrottlingTempLimit, float MeasuredTemp)
{
    float ThrottlingTempLimit;
    float ThermalThrottlingThresholdPower;
    float DeltaTemp;
    float IntegrationAntiWindup;

    /* FPGA Temperature*/

    if (bUserThrottlingTempLimitEnabled) // This comes from XRT
    {
        ThrottlingTempLimit = MIN(PassedInThrottlingTempLimit, pContext->XRTSuppliedUserThrottlingTempLimit); //pContext->FPGATempThrottlingLimit sent in SC message
    }
    else
    {
        ThrottlingTempLimit = PassedInThrottlingTempLimit;
    }

    DeltaTemp = (ThrottlingTempLimit - MeasuredTemp);

    if (bUseIntegrationSum) // Only for FPGA Temp
    {
        IntegrationAntiWindup = pContext->algorithm_type.type1.TempGainKaw * (pContext->algorithm_type.type1.BoardMeasuredPower - pContext->algorithm_type.type1.LastThermalThrottlingThresholdPower);

        if (pContext->algorithm_type.type1.ThermalThrottlingLoopJustEnabled)
        {
            pContext->algorithm_type.type1.IntegtaionSum = 0;
            pContext->algorithm_type.type1.ThermalThrottlingLoopJustEnabled = false;
        }
        else
        {
            pContext->algorithm_type.type1.IntegtaionSum = pContext->algorithm_type.type1.IntegtaionSum + (DeltaTemp * pContext->algorithm_type.type1.TempGainKi) + IntegrationAntiWindup;
        }

        ThermalThrottlingThresholdPower = (DeltaTemp * pContext->algorithm_type.type1.TempGainKpFPGA) + pContext->algorithm_type.type1.IntegtaionSum;
        pContext->algorithm_type.type1.LastThermalThrottlingThresholdPower = ThermalThrottlingThresholdPower;
    }
    else // VCC Int Temp
    {
        ThermalThrottlingThresholdPower = (DeltaTemp * pContext->algorithm_type.type1.TempGainKpVCCInt);
    }

    return ThermalThrottlingThresholdPower;
}



void cmc_clock_throttling_phase2_algorithm(CMC_CLOCK_THROTTLING_CONTEXT_TYPE* pContext)
{
    bool bUseIntegrationSum;
    float localThermalThrottlingThresholdPower;
    float localVccIntThermalThrottlingThresholdPower;

    // FPGA Temperature
    bUseIntegrationSum = true;
    localThermalThrottlingThresholdPower = cmc_clock_throttling_PI_Loop(pContext,
        pContext->bUserThrottlingTempLimitEnabled, bUseIntegrationSum,
        (float)pContext->algorithm_type.type1.FPGAThrottlingTempLimit, pContext->algorithm_type.type1.FPGAMeasuredTemp);


    if (pContext->algorithm_type.type1.bVccIntThermalThrottlingEnabled)
    {
        /* VCCInt Temperature*/
        bUseIntegrationSum = false;
        localVccIntThermalThrottlingThresholdPower = cmc_clock_throttling_PI_Loop(pContext,
            false, bUseIntegrationSum, (float)pContext->algorithm_type.type1.VccIntThrottlingTempLimit,
            pContext->algorithm_type.type1.VccIntMeasuredTemp);

        // Use the smaller of the 2 values
        if (localVccIntThermalThrottlingThresholdPower < localThermalThrottlingThresholdPower)
        {
            localThermalThrottlingThresholdPower = localVccIntThermalThrottlingThresholdPower;
        }
    }

    // Output of thermal stage
    pContext->algorithm_type.type1.ThermalThrottlingThresholdPower = localThermalThrottlingThresholdPower;

    if (pContext->Mode != CT_MODE_BOTH)
    {
        // Only thermal enabled
        // What do we do ?
    }
}




void cmc_clock_throttling_phase2_preprocessing_type1(CMC_CLOCK_THROTTLING_CONTEXT_TYPE* pContext)
{
    int i, j;
    cmc_clock_throttling_phase2_initialise(pContext);
    uint32_t FPGATemperatureSum = 0;
    uint32_t VCCIntTemperatureSum = 0;

    /*union union_convert {
        uint32_t uint_val;
        float    float_val;
    } union_convert;
*/
//For debug lets Read Temperature Debug Registers
//if (PERIPHERAL_REGMAP_RAM_CONTROLLER_Read(pContext->pHardwareRegisterSetContext, HOST_REGISTER_CT_MUTEX)) // Used as a mutex
//{
//    /*union_convert.uint_val = PERIPHERAL_REGMAP_RAM_CONTROLLER_Read(pContext->pHardwareRegisterSetContext, HOST_REGISTER_CT_KP_FPGA);
//    pContext->TempGainKpFPGA = union_convert.float_val;
//    union_convert.uint_val = PERIPHERAL_REGMAP_RAM_CONTROLLER_Read(pContext->pHardwareRegisterSetContext, HOST_REGISTER_CT_KI);
//    pContext->TempGainKi = union_convert.float_val;
//    union_convert.uint_val = PERIPHERAL_REGMAP_RAM_CONTROLLER_Read(pContext->pHardwareRegisterSetContext, HOST_REGISTER_CT_KP_VCCINT);
//    pContext->TempGainKpVCCInt = union_convert.float_val;
//    union_convert.uint_val = PERIPHERAL_REGMAP_RAM_CONTROLLER_Read(pContext->pHardwareRegisterSetContext, HOST_REGISTER_CT_KAW);
//    pContext->TempGainKaw = union_convert.float_val;*/

//    DebugRegister = PERIPHERAL_REGMAP_RAM_CONTROLLER_Read(pContext->pHardwareRegisterSetContext, HOST_REGISTER_CT_DEBUG);

//    //pContext->FPGAThrottlingTempLimit = DebugRegister & 0xFF;
//    //pContext->Mode = ((DebugRegister >> 8) & 0xFF);
//    //pContext->VccIntThrottlingTempLimit = ((DebugRegister >> 16) & 0xFF);
//    //pContext->bVccIntThermalThrottlingEnabled = ((DebugRegister >> 24) & 0x01);

//    /*pContext->FPGAThrottlingTempLimit = (PERIPHERAL_REGMAP_RAM_CONTROLLER_Read(pContext->pHardwareRegisterSetContext, HOST_REGISTER_CT_DEBUG )) & 0xFF;
//    pContext->VccIntThrottlingTempLimit = ((PERIPHERAL_REGMAP_RAM_CONTROLLER_Read(pContext->pHardwareRegisterSetContext, HOST_REGISTER_CT_DEBUG) >> 16) & 0xFF);
//    pContext->bVccIntThermalThrottlingEnabled = ((PERIPHERAL_REGMAP_RAM_CONTROLLER_Read(pContext->pHardwareRegisterSetContext, HOST_REGISTER_CT_DEBUG) >> 24) & 0x01);
//    */


//    //pContext->bUsingVccInt = (bool)PERIPHERAL_REGMAP_RAM_CONTROLLER_Read(pContext->pHardwareRegisterSetContext, HOST_REGISTER_CT_KAW);

//    PERIPHERAL_REGMAP_RAM_CONTROLLER_Write(pContext->pHardwareRegisterSetContext, HOST_REGISTER_CT_MUTEX, 0); // Clear the mutex           
//}

// Actual measured temperature with averaging performed
    pContext->algorithm_type.type1.FPGAMeasuredTempValues[pContext->algorithm_type.type1.AverageArrayIndex] =
        PERIPHERAL_REGMAP_RAM_CONTROLLER_Read(pContext->pHardwareRegisterSetContext, SNSR_ID_FPGA_TEMP * 12 + HOST_REGISTER_SNSR_INS_REG_OFFSET);

    pContext->algorithm_type.type1.VCCIntMeasuredTempValues[pContext->algorithm_type.type1.AverageArrayIndex] =
        PERIPHERAL_REGMAP_RAM_CONTROLLER_Read(pContext->pHardwareRegisterSetContext, SNSR_ID_VCCINT_TEMP * 12 + HOST_REGISTER_SNSR_INS_REG_OFFSET);

    for (i = 0; i < CLOCK_THROTTLING_AVERAGE_SIZE; i++)
    {
        FPGATemperatureSum += pContext->algorithm_type.type1.FPGAMeasuredTempValues[i];
        VCCIntTemperatureSum += pContext->algorithm_type.type1.VCCIntMeasuredTempValues[i];
    }

    pContext->algorithm_type.type1.FPGAMeasuredTemp = (float)FPGATemperatureSum / (float)CLOCK_THROTTLING_AVERAGE_SIZE;
    pContext->algorithm_type.type1.VccIntMeasuredTemp = (float)VCCIntTemperatureSum / (float)CLOCK_THROTTLING_AVERAGE_SIZE;
    pContext->algorithm_type.type1.AverageArrayIndex++;

    if (pContext->algorithm_type.type1.AverageArrayIndex > CLOCK_THROTTLING_AVERAGE_SIZE) pContext->algorithm_type.type1.AverageArrayIndex = 0;


    if (pContext->FeatureEnabled && pContext->bFeatureSupportedBySCVersion)   // Controlled by FeatureControl
    {
        if (pContext->Mode == CT_MODE_TEMPERATURE_ONLY || pContext->Mode == CT_MODE_BOTH)    // Currently read from profile
        {

            // Look up the FPGA Temperature and VCCInt Temperature Threshold received from the SC
            for (j = 0; j < 10; j = j + 2)
            {
                // Temporarily disable 
                uint8_t readValue = (uint8_t)PERIPHERAL_REGMAP_RAM_CONTROLLER_Read(pContext->pHardwareRegisterSetContext,
                    (j * 4) + HOST_REGISTER_TEMP_THRESHOLD_BASE_REG);
                if (SNSR_ID_FPGA_TEMP == readValue)
                {
                    pContext->algorithm_type.type1.FPGAThrottlingTempLimit =
                        PERIPHERAL_REGMAP_RAM_CONTROLLER_Read(pContext->pHardwareRegisterSetContext,
                        ((j + 1) * 4) + HOST_REGISTER_TEMP_THRESHOLD_BASE_REG);
                }
                if (SNSR_ID_VCCINT_TEMP == readValue)
                {
                    pContext->algorithm_type.type1.VccIntThrottlingTempLimit =
                        PERIPHERAL_REGMAP_RAM_CONTROLLER_Read(pContext->pHardwareRegisterSetContext,
                        ((j + 1) * 4) + HOST_REGISTER_TEMP_THRESHOLD_BASE_REG);
                }
            }

            //// Now call the algorithm
            cmc_clock_throttling_phase2_algorithm(pContext);
        }
    }

    CMC_ClockThrottling_Update_Watchpoint(pContext);
}


void cmc_clock_throttling_phase2_preprocessing(CMC_CLOCK_THROTTLING_CONTEXT_TYPE* pContext)
{
    switch (pContext->Algorithm)
    {
    case CT_ALGORITHM_STANDARD:
        cmc_clock_throttling_phase2_preprocessing_type1(pContext);
        break;

    case CT_ALGORITHM_SSD:
        // Do SSD clock throttling
        break;

    case CT_ALGORITHM_NONE:
    default:
        break;
    }
}

