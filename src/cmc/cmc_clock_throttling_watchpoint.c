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




#include "cmc_clock_throttling.h"
#define FLOAT_TO_INT_SCALING_FACTOR         (65536)     //(2^16)

void CMC_ClockThrottling_Update_Watchpoint(CMC_CLOCK_THROTTLING_CONTEXT_TYPE * pContext)
{
    Watch_Set(pContext->pWatchPointContext, W_CT_MODE, (uint32_t)pContext->Mode);
    Watch_Set(pContext->pWatchPointContext, W_CT_XRTSUPPLIEDBOARDTHROTTLINGTHRESHOLDPOWER, pContext->XRTSuppliedBoardThrottlingThresholdPower);
    Watch_Set(pContext->pWatchPointContext, W_CT_BUSERTHROTTLINGTEMPLIMITENABLED, (uint32_t)pContext->bUserThrottlingTempLimitEnabled);
    Watch_Set(pContext->pWatchPointContext, W_CT_XRTSUPPLIEDUSERTHROTTLINGTEMPLIMIT, (uint32_t)pContext->XRTSuppliedUserThrottlingTempLimit);

    if (pContext->Algorithm == CT_ALGORITHM_STANDARD)
    {
        Watch_Set(pContext->pWatchPointContext, W_CT_BOARDMEASUREDPOWER, pContext->algorithm_type.type1.BoardMeasuredPower);
        Watch_Set(pContext->pWatchPointContext, W_CT_USERNORMALIZEDPOWER, (uint32_t)(pContext->algorithm_type.type1.UserNormalizedPower * FLOAT_TO_INT_SCALING_FACTOR));
        Watch_Set(pContext->pWatchPointContext, W_CT_BOARDNORMALIZEDPOWER, (uint32_t)(pContext->algorithm_type.type1.BoardNormalizedPower * FLOAT_TO_INT_SCALING_FACTOR));
        Watch_Set(pContext->pWatchPointContext, W_CT_KERNELPOWER, (uint32_t)(pContext->algorithm_type.type1.KernelPower * FLOAT_TO_INT_SCALING_FACTOR));
        Watch_Set(pContext->pWatchPointContext, W_CT_KERNELTARGET, (uint32_t)(pContext->algorithm_type.type1.KernelTarget * FLOAT_TO_INT_SCALING_FACTOR));
        Watch_Set(pContext->pWatchPointContext, W_CT_IDLEPOWER, (uint32_t)(pContext->algorithm_type.type1.IdlePower));
        Watch_Set(pContext->pWatchPointContext, W_CT_ACTIVITY, pContext->algorithm_type.type1.Activity);
        Watch_Set(pContext->pWatchPointContext, W_CT_RATECURRENT, (uint32_t)(pContext->algorithm_type.type1.RateCurrent * FLOAT_TO_INT_SCALING_FACTOR));
        Watch_Set(pContext->pWatchPointContext, W_CT_RATELINEAR, (uint32_t)(pContext->algorithm_type.type1.RateLinear * FLOAT_TO_INT_SCALING_FACTOR));

        Watch_Set(pContext->pWatchPointContext, W_CT_0_VOLTAGESENSORID, pContext->algorithm_type.type1.RailParameters[0].VoltageSensorID);
        Watch_Set(pContext->pWatchPointContext, W_CT_0_CURRENTSENSORID, pContext->algorithm_type.type1.RailParameters[0].CurrentSensorID);
        Watch_Set(pContext->pWatchPointContext, W_CT_0_DUPLICATEREADINGCURRENT, (uint32_t)pContext->algorithm_type.type1.RailParameters[0].DuplicateReadingCount);
        Watch_Set(pContext->pWatchPointContext, W_CT_0_LATESTREADINGCURRENT, (uint32_t)pContext->algorithm_type.type1.RailParameters[0].LatestReading.Current);
        Watch_Set(pContext->pWatchPointContext, W_CT_0_LATESTREADINGVOLTAGE, (uint32_t)pContext->algorithm_type.type1.RailParameters[0].LatestReading.Voltage);
        Watch_Set(pContext->pWatchPointContext, W_CT_0_LATESTREADINGTHRESHCURRENT, (uint32_t)pContext->algorithm_type.type1.RailParameters[0].LatestReading.throttlingThresholdCurrent);
        Watch_Set(pContext->pWatchPointContext, W_CT_0_NOMINALVOLTAGE, pContext->algorithm_type.type1.RailParameters[0].NominalVoltage);
        Watch_Set(pContext->pWatchPointContext, W_CT_0_MEASUREDPOWER, pContext->algorithm_type.type1.RailParameters[0].MeasuredPower);
        Watch_Set(pContext->pWatchPointContext, W_CT_0_THROTTLEDTHRESHOLDPOWER, pContext->algorithm_type.type1.RailParameters[0].ThrottledThresholdPower);
        Watch_Set(pContext->pWatchPointContext, W_CT_0_CONTRIBUTESTOBOARDTHROTTLINGPOWER, pContext->algorithm_type.type1.RailParameters[0].ContributesToBoardThrottlingPower);
        Watch_Set(pContext->pWatchPointContext, W_CT_0_RAILNORMALIZEDPOWER, (uint32_t)(pContext->algorithm_type.type1.RailParameters[0].RailNormalizedPower * FLOAT_TO_INT_SCALING_FACTOR));

        Watch_Set(pContext->pWatchPointContext, W_CT_1_VOLTAGESENSORID, pContext->algorithm_type.type1.RailParameters[1].VoltageSensorID);
        Watch_Set(pContext->pWatchPointContext, W_CT_1_CURRENTSENSORID, pContext->algorithm_type.type1.RailParameters[1].CurrentSensorID);
        Watch_Set(pContext->pWatchPointContext, W_CT_1_DUPLICATEREADINGCURRENT, (uint32_t)pContext->algorithm_type.type1.RailParameters[1].DuplicateReadingCount);
        Watch_Set(pContext->pWatchPointContext, W_CT_1_LATESTREADINGCURRENT, (uint32_t)pContext->algorithm_type.type1.RailParameters[1].LatestReading.Current);
        Watch_Set(pContext->pWatchPointContext, W_CT_1_LATESTREADINGVOLTAGE, (uint32_t)pContext->algorithm_type.type1.RailParameters[1].LatestReading.Voltage);
        Watch_Set(pContext->pWatchPointContext, W_CT_1_LATESTREADINGTHRESHCURRENT, (uint32_t)pContext->algorithm_type.type1.RailParameters[1].LatestReading.throttlingThresholdCurrent);
        Watch_Set(pContext->pWatchPointContext, W_CT_1_NOMINALVOLTAGE, pContext->algorithm_type.type1.RailParameters[1].NominalVoltage);
        Watch_Set(pContext->pWatchPointContext, W_CT_1_MEASUREDPOWER, pContext->algorithm_type.type1.RailParameters[1].MeasuredPower);
        Watch_Set(pContext->pWatchPointContext, W_CT_1_THROTTLEDTHRESHOLDPOWER, pContext->algorithm_type.type1.RailParameters[1].ThrottledThresholdPower);
        Watch_Set(pContext->pWatchPointContext, W_CT_1_CONTRIBUTESTOBOARDTHROTTLINGPOWER, pContext->algorithm_type.type1.RailParameters[1].ContributesToBoardThrottlingPower);
        Watch_Set(pContext->pWatchPointContext, W_CT_1_RAILNORMALIZEDPOWER, (uint32_t)(pContext->algorithm_type.type1.RailParameters[1].RailNormalizedPower * FLOAT_TO_INT_SCALING_FACTOR));

        Watch_Set(pContext->pWatchPointContext, W_CT_2_VOLTAGESENSORID, pContext->algorithm_type.type1.RailParameters[2].VoltageSensorID);
        Watch_Set(pContext->pWatchPointContext, W_CT_2_CURRENTSENSORID, pContext->algorithm_type.type1.RailParameters[2].CurrentSensorID);
        Watch_Set(pContext->pWatchPointContext, W_CT_2_DUPLICATEREADINGCURRENT, (uint32_t)pContext->algorithm_type.type1.RailParameters[2].DuplicateReadingCount);
        Watch_Set(pContext->pWatchPointContext, W_CT_2_LATESTREADINGCURRENT, (uint32_t)pContext->algorithm_type.type1.RailParameters[2].LatestReading.Current);
        Watch_Set(pContext->pWatchPointContext, W_CT_2_LATESTREADINGVOLTAGE, (uint32_t)pContext->algorithm_type.type1.RailParameters[2].LatestReading.Voltage);
        Watch_Set(pContext->pWatchPointContext, W_CT_2_LATESTREADINGTHRESHCURRENT, (uint32_t)pContext->algorithm_type.type1.RailParameters[2].LatestReading.throttlingThresholdCurrent);
        Watch_Set(pContext->pWatchPointContext, W_CT_2_NOMINALVOLTAGE, pContext->algorithm_type.type1.RailParameters[2].NominalVoltage);
        Watch_Set(pContext->pWatchPointContext, W_CT_2_MEASUREDPOWER, pContext->algorithm_type.type1.RailParameters[2].MeasuredPower);
        Watch_Set(pContext->pWatchPointContext, W_CT_2_THROTTLEDTHRESHOLDPOWER, pContext->algorithm_type.type1.RailParameters[2].ThrottledThresholdPower);
        Watch_Set(pContext->pWatchPointContext, W_CT_2_CONTRIBUTESTOBOARDTHROTTLINGPOWER, pContext->algorithm_type.type1.RailParameters[2].ContributesToBoardThrottlingPower);
        Watch_Set(pContext->pWatchPointContext, W_CT_2_RAILNORMALIZEDPOWER, (uint32_t)(pContext->algorithm_type.type1.RailParameters[2].RailNormalizedPower * FLOAT_TO_INT_SCALING_FACTOR));

        Watch_Set(pContext->pWatchPointContext, W_CT_FPGAMEASUREDTEMP, (uint32_t)(pContext->algorithm_type.type1.FPGAMeasuredTemp * FLOAT_TO_INT_SCALING_FACTOR));
        Watch_Set(pContext->pWatchPointContext, W_CT_FPGATHROTTLINGTEMPLIMIT, (uint32_t)pContext->algorithm_type.type1.FPGAThrottlingTempLimit);
        Watch_Set(pContext->pWatchPointContext, W_CT_BVCCINTTHERMALTHROTTLINGENABLED, (uint32_t)pContext->algorithm_type.type1.bVccIntThermalThrottlingEnabled);
        Watch_Set(pContext->pWatchPointContext, W_CT_VCCINTMEASUREDTEMP, (uint32_t)(pContext->algorithm_type.type1.VccIntMeasuredTemp * FLOAT_TO_INT_SCALING_FACTOR));
        Watch_Set(pContext->pWatchPointContext, W_CT_VCCINTTHROTTLINGTEMPLIMIT, (uint32_t)pContext->algorithm_type.type1.VccIntThrottlingTempLimit);
        Watch_Set(pContext->pWatchPointContext, W_CT_THERMALTHROTTLINGTHRESHOLDPOWER, (uint32_t)pContext->algorithm_type.type1.ThermalThrottlingThresholdPower);
        Watch_Set(pContext->pWatchPointContext, W_CT_THERMALNORMALISEDPOWER, (uint32_t)(pContext->algorithm_type.type1.ThermalNormalisedPower * FLOAT_TO_INT_SCALING_FACTOR));
        Watch_Set(pContext->pWatchPointContext, W_CT_VCCINTTHERMALNORMALISEDPOWER, (uint32_t)(pContext->algorithm_type.type1.VccIntThermalNormalisedPower * FLOAT_TO_INT_SCALING_FACTOR));

        Watch_Set(pContext->pWatchPointContext, W_CT_THERMALTHROTTLINGLOOPJUSTENABLED, (uint32_t)pContext->algorithm_type.type1.ThermalThrottlingLoopJustEnabled);
        Watch_Set(pContext->pWatchPointContext, W_CT_INTEGTAIONSUM, (uint32_t)pContext->algorithm_type.type1.IntegtaionSum);

        union union_convert {
            uint32_t uint_val;
            float    float_val;
        } union_convert;

        union_convert.float_val = pContext->algorithm_type.type1.TempGainKpFPGA;
        Watch_Set(pContext->pWatchPointContext, W_CT_TEMPGAINKPFPGA, union_convert.uint_val);
        union_convert.float_val = pContext->algorithm_type.type1.TempGainKi;
        Watch_Set(pContext->pWatchPointContext, W_CT_TEMPGAINKI, union_convert.uint_val);
        union_convert.float_val = pContext->algorithm_type.type1.TempGainKpVCCInt;
        Watch_Set(pContext->pWatchPointContext, W_CT_TEMPGAINKPVCCINT, union_convert.uint_val);
        union_convert.float_val = pContext->algorithm_type.type1.TempGainKaw;
        Watch_Set(pContext->pWatchPointContext, W_CT_TEMPGAINKAW, union_convert.uint_val);
    }
}

void CMC_ClockThrottling_Initialize_Watchpoint(CMC_CLOCK_THROTTLING_CONTEXT_TYPE * pContext)
{
    CMC_ClockThrottling_Update_Watchpoint(pContext);
}

