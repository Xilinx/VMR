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
 *  $Revision: #17 $
 *
 */





#include "cmc_clock_throttling.h"
#include "cmc_peripheral_clock_monitor.h"
#include "cmc_peripheral_clock_monitor_defs.h"

void CMC_ClockThrottling_Initialize(CMC_CLOCK_THROTTLING_CONTEXT_TYPE *              pContext,
                                    CMC_BUILD_PROFILE_CLOCK_THROTTLING_TYPE *        pThrottling,
                                    CMC_WATCHPOINT_CONTEXT_TYPE *                    pWatchPointContext,
                                    PERIPHERAL_CLOCK_THROTTLING_CONTEXT_TYPE *       pClockThrottlingPeripheralContext,
                                    PERIPHERAL_CLOCK_MONITOR_CONTEXT_TYPE *          pClockMonitorPeripheralContext,
                                    PERIPHERAL_REGMAP_RAM_CONTROLLER_CONTEXT_TYPE *  pHardwareRegisterSetContext,
                                    PERIPHERAL_AXI_GPIO_MUTEX_CMC_CONTEXT_TYPE *     pPeripheralMutexContext)
{
    pContext->Algorithm = pThrottling->ProfileAlgorithm;
    pContext->bFeatureSupportedBySCVersion = true;
    pContext->pWatchPointContext = pWatchPointContext;
    pContext->pHardwareRegisterSetContext = pHardwareRegisterSetContext;
    pContext->pClockThrottlingPeripheralContext = pClockThrottlingPeripheralContext;
    pContext->pClockMonitorPeripheralContext = pClockMonitorPeripheralContext;
    CMC_ClockThrottling_FeatureEnable(pContext, false);
    CMC_ClockThrottling_Initialize_Mode(pContext, pThrottling->ProfileMode);

    switch (pThrottling->ProfileAlgorithm)
    {
        case CT_ALGORITHM_STANDARD:
        {
            int i;
            pContext->algorithm_type.type1.pPeripheralMutexContext = pPeripheralMutexContext;
            pContext->algorithm_type.type1.NumberOfRailsMonitored = pThrottling->NumberOfSensors;
            for (i = 0; i < pThrottling->NumberOfSensors; i++)
            {
                pContext->algorithm_type.type1.RailParameters[i].VoltageSensorID = pThrottling->VoltageSensorID[i];
                pContext->algorithm_type.type1.RailParameters[i].CurrentSensorID = pThrottling->CurrentSensorID[i];
                pContext->algorithm_type.type1.RailParameters[i].NominalVoltage = pThrottling->NominalVoltage[i];
                pContext->algorithm_type.type1.RailParameters[i].ContributesToBoardThrottlingPower = pThrottling->bContributesToBoardPower[i];
                pContext->algorithm_type.type1.RailParameters[i].DuplicateReadingCount = 0;
            }

            pContext->algorithm_type.type1.bVccIntThermalThrottlingEnabled = pThrottling->bVCCIntThermalThrottling;
            pContext->algorithm_type.type1.IdlePower = pThrottling->IdlePower;

            CMC_ClockThrottling_Initialize_Watchpoint(pContext);
            cmc_clock_throttling_initialise_all(pContext);

            for (i = 0; i < CLOCK_THROTTLING_AVERAGE_SIZE; i++)
            {
                pContext->algorithm_type.type1.FPGAMeasuredTempValues[i] = 0;
                pContext->algorithm_type.type1.VCCIntMeasuredTempValues[i] = 0;
            }
            pContext->algorithm_type.type1.AverageArrayIndex = 0;

            pContext->algorithm_type.type1.TempGainKpFPGA = pThrottling->TempGainKpFPGA;
            pContext->algorithm_type.type1.TempGainKi = pThrottling->TempGainKi;
            pContext->algorithm_type.type1.TempGainKpVCCInt = pThrottling->TempGainKpVCCInt;
            pContext->algorithm_type.type1.TempGainKaw = pThrottling->TempGainKaw;
        }
        break;

        case CT_ALGORITHM_SSD:
        {
            // TODO put default values somwehere else?
            uint8_t pow_kp = 25;
            uint8_t temp_kp = 30;
            uint8_t pow_ki = 7;
            uint32_t power;

            pContext->algorithm_type.type2.clk_scaled_in_temp = false;
            pContext->algorithm_type.type2.clk_scaled = false;
            pContext->algorithm_type.type2.clk_scaled_in_pwr = false;
            pContext->algorithm_type.type2.crit_temp_count = 0;
            pContext->algorithm_type.type2.integral_p_err = 0;

            clk_bits[zeros_count] = 0;
            clk_bits[ones_count] = TOTAL_CLOCK_BITS;

            //Reset clock scaling ip registers
            PERIPHERAL_Clock_Monitor_Write_Reg(pContext->pClockMonitorPeripheralContext, CLOCK_SCALING_SOFT_RESET_REG, CLOCK_SCALING_SOFT_RESET_VAL);

            //set 32_clocks initially
            PERIPHERAL_Clock_Monitor_Load_All_Pattern(pContext->pClockMonitorPeripheralContext, true);

            //Set power Integral constant
            power = PERIPHERAL_Clock_Monitor_Read_Reg(pContext->pClockMonitorPeripheralContext, CLOCK_SCALING_POWER_REG);
            power |= (pow_ki << CLOCK_SCALING_POWER_I_CONST_POS);
            PERIPHERAL_Clock_Monitor_Write_Reg(pContext->pClockMonitorPeripheralContext, CLOCK_SCALING_POWER_REG, power);

            //Read threshold register and update target power/temperature registers
            PERIPHERAL_Clock_Monitor_Update_Targets(pContext->pClockMonitorPeripheralContext, 0, 0);

            //Select power based clock scaling
            PERIPHERAL_Clock_Monitor_Write_Reg(pContext->pClockMonitorPeripheralContext, PI_ALGO_SELECT_REG, PI_ALGO_SELECT_BOTH_POWER_TEMP);

            //Set power Integral constant
            power = PERIPHERAL_Clock_Monitor_Read_Reg(pContext->pClockMonitorPeripheralContext, CLOCK_SCALING_POWER_REG);
            power |= (1 << CLOCK_SCALING_POWER_INTEGRAL_POS);
            PERIPHERAL_Clock_Monitor_Write_Reg(pContext->pClockMonitorPeripheralContext, CLOCK_SCALING_POWER_REG, power);

            //Update proportional constants for power and temperature
            PERIPHERAL_Clock_Monitor_Write_Reg(pContext->pClockMonitorPeripheralContext, PROPORTIONAL_GAIN_REG, (pow_kp |
                (temp_kp << CLOCK_SCALING_TEMPERATURE_P_CONST_POS)));

            //Set min = 1 and max = 255 clk multipliers.
            PERIPHERAL_Clock_Monitor_Update_Freq_Multipliers(pContext->pClockMonitorPeripheralContext, 1, 255);
        }
        break;

        case CT_ALGORITHM_NONE:
        default:
            break;
    }
}

