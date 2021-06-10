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
 *  $Revision: #15 $
 *
 */





#include "cmc_clock_throttling.h"
#include "cmc_version_support.h"
#include "cmc_peripheral_clock_monitor.h"
#include "cmc_peripheral_clock_monitor_defs.h"

//float	Idle_Power;	            //Board power when no kernels are executing as a fraction of
//float	Unique_Measured_Power; 	//most recent measured power in mW that is not a duplicate
//float	Kernel_Power;		    //Power in mW assumed to have been consumed by kernel logic in last interval
//float	Kernel_Target;		    //Target power in mW at which throttling will activate
//int	Activity;			    //Control applied to Clock Throttling hardware, passed from 1 iteration to next
//int	ACTIVITY_MAX;		    //Maximum value for Activity(128)
//float	Rate_Current;		    //Activity expressed as a float in the range 0.0 to 1.0
//float	Rate_Linear;		    //Activity required for next interval meet Set_Power
extern CMC_VERSION_SUPPORT_CONTEXT_TYPE				VersionSupportContext;

/*
 * cmcCeil(): Returns the smallest integer greater than or equal to num,
 * (i.e : rounds up the nearest integer).
 */
int cmcCeil(double num)
{
    int res = (int)num;

    if ((double)res != num)
        return (int)(num + 1);

    return (int)num;
}



void cmc_clock_throttling_initialise_all(CMC_CLOCK_THROTTLING_CONTEXT_TYPE* pContext)
{
    pContext->algorithm_type.type1.KernelPower = 0;
    pContext->algorithm_type.type1.KernelTarget = 0;
    pContext->algorithm_type.type1.Activity = ACTIVITY_MAX;
    pContext->XRTSuppliedBoardThrottlingThresholdPower = 0;
    cmc_clock_throttling_phase1_initialise(pContext);
}



void cmc_clock_throttling_phase1_initialise(CMC_CLOCK_THROTTLING_CONTEXT_TYPE* pContext)
{
    pContext->algorithm_type.type1.RateCurrent = 0;
    pContext->algorithm_type.type1.RateLinear = 0;
    pContext->algorithm_type.type1.BoardNormalizedPower = 0;
    pContext->algorithm_type.type1.BoardThrottlingThresholdPower = 0;
    pContext->algorithm_type.type1.BoardMeasuredPower = 0;
    pContext->algorithm_type.type1.UserNormalizedPower = 0;
}



void cmc_clock_throttling_phase1_preprocessing_type1(CMC_CLOCK_THROTTLING_CONTEXT_TYPE* pContext)
{
    int i, j;
    bool ReadingsHaveChanged = false;
    cmc_clock_throttling_phase1_initialise(pContext);

    if (!CMC_VersionSupport_CheckMessageSupport(&VersionSupportContext, SAT_COMMS_SNSR_POWER_THROTTLING_THRESHOLDS_REQ))
    {
        pContext->bFeatureSupportedBySCVersion = false;
    }
    else
    {
        pContext->bFeatureSupportedBySCVersion = true;
    }

    if (pContext->FeatureEnabled && pContext->bFeatureSupportedBySCVersion)   // Controlled by FeatureControl
    {
        if (pContext->Mode == CT_MODE_POWER_ONLY || pContext->Mode == CT_MODE_BOTH || pContext->Mode == CT_MODE_TEMPERATURE_ONLY)    // Currently read from profile
        {
            for (i = 0; i < pContext->algorithm_type.type1.NumberOfRailsMonitored; i++)
            {
                // Update Voltage
                pContext->algorithm_type.type1.RailParameters[i].LatestReading.Voltage =
                    PERIPHERAL_REGMAP_RAM_CONTROLLER_Read(pContext->pHardwareRegisterSetContext, (pContext->algorithm_type.type1.RailParameters[i].VoltageSensorID) * 12 + HOST_REGISTER_SNSR_INS_REG_OFFSET);

                //Update Current
                pContext->algorithm_type.type1.RailParameters[i].LatestReading.Current =
                    PERIPHERAL_REGMAP_RAM_CONTROLLER_Read(pContext->pHardwareRegisterSetContext, (pContext->algorithm_type.type1.RailParameters[i].CurrentSensorID) * 12 + HOST_REGISTER_SNSR_INS_REG_OFFSET);

                //Update Threshold
                // These are stores in ID/value pairs - a maximum of 5 pairs
                for (j = 0; j < 10; j = j + 2)
                {
                    uint8_t readValue = (uint8_t)PERIPHERAL_REGMAP_RAM_CONTROLLER_Read(pContext->pHardwareRegisterSetContext, (j * 4) + HOST_REGISTER_POWER_THRESHOLD_BASE_REG);
                    if (pContext->algorithm_type.type1.RailParameters[i].CurrentSensorID == readValue)
                    {
                        pContext->algorithm_type.type1.RailParameters[i].LatestReading.throttlingThresholdCurrent =
                            PERIPHERAL_REGMAP_RAM_CONTROLLER_Read(pContext->pHardwareRegisterSetContext, ((j + 1) * 4) + HOST_REGISTER_POWER_THRESHOLD_BASE_REG);
                        break;

                    }
                }
            }

            // Check for duplicates
            for (i = 0; i < pContext->algorithm_type.type1.NumberOfRailsMonitored; i++)
            {
                if (pContext->algorithm_type.type1.RailParameters[i].LatestReading.Current != pContext->algorithm_type.type1.RailParameters[i].PreviousReading.Current)
                {
                    ReadingsHaveChanged = true;
                }
                else
                {
                    pContext->algorithm_type.type1.RailParameters[i].DuplicateReadingCount++;
                    // Reading hasn't changed
                }

                pContext->algorithm_type.type1.RailParameters[i].MeasuredPower = pContext->algorithm_type.type1.RailParameters[i].LatestReading.Current
                    * pContext->algorithm_type.type1.RailParameters[i].LatestReading.Voltage;
                pContext->algorithm_type.type1.RailParameters[i].ThrottledThresholdPower = pContext->algorithm_type.type1.RailParameters[i].NominalVoltage
                    * pContext->algorithm_type.type1.RailParameters[i].LatestReading.throttlingThresholdCurrent;

                if (pContext->algorithm_type.type1.RailParameters[i].ContributesToBoardThrottlingPower)
                {
                    pContext->algorithm_type.type1.BoardThrottlingThresholdPower += pContext->algorithm_type.type1.RailParameters[i].ThrottledThresholdPower;
                    pContext->algorithm_type.type1.BoardMeasuredPower += pContext->algorithm_type.type1.RailParameters[i].MeasuredPower;
                }

                pContext->algorithm_type.type1.RailParameters[i].RailNormalizedPower = ((float)pContext->algorithm_type.type1.RailParameters[i].MeasuredPower 
                    / (float)pContext->algorithm_type.type1.RailParameters[i].ThrottledThresholdPower);


                // Store new reading into previous
                pContext->algorithm_type.type1.RailParameters[i].PreviousReading.Current = pContext->algorithm_type.type1.RailParameters[i].LatestReading.Current;
                pContext->algorithm_type.type1.RailParameters[i].PreviousReading.Voltage = pContext->algorithm_type.type1.RailParameters[i].LatestReading.Voltage;
                pContext->algorithm_type.type1.RailParameters[i].PreviousReading.throttlingThresholdCurrent = pContext->algorithm_type.type1.RailParameters[i].LatestReading.throttlingThresholdCurrent;
            }

            // If Thermal is enabled include the ThermalThrottlingThresholdPower
            if (pContext->Mode == CT_MODE_BOTH)
            {
                //pContext->ThermalThrottlingThresholdPower 
                pContext->algorithm_type.type1.ThermalNormalisedPower = pContext->algorithm_type.type1.BoardMeasuredPower / pContext->algorithm_type.type1.ThermalThrottlingThresholdPower;
            }


            // Check if XRT has supplied a BoardThrottlingThresholdPower if so use it
            // This comes from Clock Throttling Power Management Register in the peripheral7
            if (pContext->PowerOverRideEnabled)
            {
                pContext->algorithm_type.type1.UserNormalizedPower = ((float)pContext->algorithm_type.type1.BoardMeasuredPower 
                    / (float)pContext->XRTSuppliedBoardThrottlingThresholdPower);
            }
            else
            {
                pContext->algorithm_type.type1.UserNormalizedPower = ((float)pContext->algorithm_type.type1.BoardMeasuredPower 
                    / (float)pContext->algorithm_type.type1.BoardThrottlingThresholdPower);
            }

            pContext->algorithm_type.type1.BoardNormalizedPower = 0;
            for (i = 0; i < pContext->algorithm_type.type1.NumberOfRailsMonitored; i++)
            {
                // Select the max of 
                if (pContext->algorithm_type.type1.BoardNormalizedPower < pContext->algorithm_type.type1.RailParameters[i].RailNormalizedPower)
                {
                    pContext->algorithm_type.type1.BoardNormalizedPower = pContext->algorithm_type.type1.RailParameters[i].RailNormalizedPower;
                }
            }

            if (pContext->algorithm_type.type1.BoardNormalizedPower < pContext->algorithm_type.type1.UserNormalizedPower)
            {
                pContext->algorithm_type.type1.BoardNormalizedPower = (float)pContext->algorithm_type.type1.UserNormalizedPower;
            }

            if ((pContext->Mode == CT_MODE_TEMPERATURE_ONLY) || 
                ((pContext->Mode == CT_MODE_BOTH) && (pContext->algorithm_type.type1.BoardNormalizedPower < pContext->algorithm_type.type1.ThermalNormalisedPower)))
            {
                pContext->algorithm_type.type1.BoardNormalizedPower = pContext->algorithm_type.type1.ThermalNormalisedPower;
            }




            // Now call the algorithm
            cmc_clock_throttling_phase1_algorithm(pContext, ReadingsHaveChanged);
        }
    }
    else
    {
        // Response to CLock Shutdown CR-1064421
        // Read the location & write activity to it
        pContext->algorithm_type.type1.Activity = 0x80;
        if (PERIPHERAL_AXI_GPIO_MUTEX_CMC_ReachOutInterfaceAccessIsGranted(pContext->algorithm_type.type1.pPeripheralMutexContext))
        {
            //          PERIPHERAL_ClockThrottling_Disable_Throttling(pContext->pClockThrottlingPeripheralContext);
            PERIPHERAL_ClockThrottling_UpdateThrottlingControl(pContext->pClockThrottlingPeripheralContext, 
                pContext->algorithm_type.type1.Activity | MASK_CLOCKTHROTTLING_DISABLE_THROTTLING);
        }

    }
    CMC_ClockThrottling_Update_Watchpoint(pContext);
}


void cmc_clock_throttling_phase1_preprocessing_type2(CMC_CLOCK_THROTTLING_CONTEXT_TYPE* pContext)
{
    if (pContext->FeatureEnabled)   // Controlled by FeatureControl
    {
        if (pContext->Mode == CT_MODE_POWER_ONLY || pContext->Mode == CT_MODE_BOTH || pContext->Mode == CT_MODE_TEMPERATURE_ONLY)    // Currently read from profile
        {
            uint32_t power_reg, temperature_reg, gain, crit_temp_threshold;
            uint32_t threshold, m12vPexCurrIns, m12VPexVoltIns, m3v3PexVoltIns;
            uint32_t m3v3PexCurrIns;

            //Read target power and target temperature values
            power_reg = PERIPHERAL_Clock_Monitor_Read_Reg(pContext->pClockMonitorPeripheralContext, 
                CLOCK_SCALING_POWER_REG);
            temperature_reg = PERIPHERAL_Clock_Monitor_Read_Reg(pContext->pClockMonitorPeripheralContext,
                CLOCK_SCALING_TEMPERATURE_REG);
            gain = PERIPHERAL_Clock_Monitor_Read_Reg(pContext->pClockMonitorPeripheralContext,
                PROPORTIONAL_GAIN_REG);
            crit_temp_threshold = PERIPHERAL_Clock_Monitor_Read_Reg(pContext->pClockMonitorPeripheralContext,
                CLOCK_SCALING_CRIT_THRESHOLD_REG) & CLOCK_SCALING_CRIT_THRESHOLD_MASK;

            //Retrieve all power related data
            pContext->algorithm_type.type2.p_integral = power_reg & CLOCK_SCALING_POWER_INTEGRAL_MASK;
            pContext->algorithm_type.type2.power_band = (power_reg & CLOCK_SCALING_POWER_BAND_MASK) >>
                CLOCK_SCALING_POWER_BAND_POS;
            pContext->algorithm_type.type2.target_power = power_reg & CLOCK_SCALING_POWER_TARGET_MASK;
            threshold = PERIPHERAL_Clock_Monitor_Read_Reg(pContext->pClockMonitorPeripheralContext,
                CLOCK_SCALING_THRESHOLD_REG);
            threshold = (threshold >> CLOCK_SCALING_POWER_THRESHOLD_BIT_POS) &
                CLOCK_SCALING_POWER_THRESHOLD_MASK;

            /* W -> mW conversion */
            pContext->algorithm_type.type2.power_band *= 1000;
            pContext->algorithm_type.type2.target_power *= 1000;
            threshold = threshold * 1000;

            if (pContext->algorithm_type.type2.target_power > threshold)
                pContext->algorithm_type.type2.target_power = threshold;
            pContext->algorithm_type.type2.target_power2 = pContext->algorithm_type.type2.target_power - pContext->algorithm_type.type2.power_band;

            //Integral constant for power based solution
            pContext->algorithm_type.type2.ki_p = ((power_reg & CLOCK_SCALING_POWER_I_CONST_MASK) >>
                CLOCK_SCALING_POWER_I_CONST_POS) / 100.0;
            //Proportional constant for power based solution
            pContext->algorithm_type.type2.kp_p = (gain & CLOCK_SCALING_POWER_P_MASK) / 10.0;

            
            //retrieve only first 16 bits for target power
            pContext->algorithm_type.type2.temperature_band = (temperature_reg & CLOCK_SCALING_TEMPERATURE_BAND_MASK) >>
                CLOCK_SCALING_TEMPERATURE_BAND_POS;
            pContext->algorithm_type.type2.target_temperature = temperature_reg & CLOCK_SCALING_TEMPERATURE_TARGET_MASK;
            threshold = PERIPHERAL_Clock_Monitor_Read_Reg(pContext->pClockMonitorPeripheralContext,
                CLOCK_SCALING_THRESHOLD_REG);
            threshold = (threshold >> CLOCK_SCALING_TEMP_THRESHOLD_BIT_POS) &
                CLOCK_SCALING_TEMP_THRESHOLD_MASK;
            if (pContext->algorithm_type.type2.target_temperature > threshold)
                pContext->algorithm_type.type2.target_temperature = threshold;
            pContext->algorithm_type.type2.target_temperature2 = pContext->algorithm_type.type2.target_temperature - pContext->algorithm_type.type2.temperature_band;
            //Proportional constant for temperature based solution
            pContext->algorithm_type.type2.kp_t = ((gain & CLOCK_SCALING_TEMPERATURE_P_CONST_MASK) >>
                CLOCK_SCALING_TEMPERATURE_P_CONST_POS) / 10.0;

            //Measure board power in terms of Watts and store it in register
            m12vPexCurrIns = PERIPHERAL_REGMAP_RAM_CONTROLLER_Read(pContext->pHardwareRegisterSetContext, 
                (SNSR_ID_12VPEX_I_IN * 12) + HOST_REGISTER_SNSR_INS_REG_OFFSET);
            m12VPexVoltIns = PERIPHERAL_REGMAP_RAM_CONTROLLER_Read(pContext->pHardwareRegisterSetContext, 
                (SNSR_ID_12V_PEX * 12) + HOST_REGISTER_SNSR_INS_REG_OFFSET);
            m3v3PexVoltIns = PERIPHERAL_REGMAP_RAM_CONTROLLER_Read(pContext->pHardwareRegisterSetContext,
                (SNSR_ID_3V3_PEX * 12) + HOST_REGISTER_SNSR_INS_REG_OFFSET);
            m3v3PexCurrIns = PERIPHERAL_REGMAP_RAM_CONTROLLER_Read(pContext->pHardwareRegisterSetContext,
                (SNSR_ID_3V3PEX_I_IN * 12) + HOST_REGISTER_SNSR_INS_REG_OFFSET);

            /* W -> mW conversion */
            pContext->algorithm_type.type2.board_power = ((m12vPexCurrIns * m12VPexVoltIns) +
                (m3v3PexCurrIns * m3v3PexVoltIns)) / 1000;

            PERIPHERAL_Clock_Monitor_Write_Reg(pContext->pClockMonitorPeripheralContext,
                STORE_POWER_VALUE_REG, pContext->algorithm_type.type2.board_power);

            //Read board temerature and store it in register.
            pContext->algorithm_type.type2.board_temperature = PERIPHERAL_REGMAP_RAM_CONTROLLER_Read(pContext->pHardwareRegisterSetContext,
                (SNSR_ID_FPGA_TEMP * 12) + HOST_REGISTER_SNSR_INS_REG_OFFSET);
            PERIPHERAL_Clock_Monitor_Write_Reg(pContext->pClockMonitorPeripheralContext,
                STORE_TEMPERATURE_VALUE_REG, pContext->algorithm_type.type2.board_temperature);

            //Check for critical temperature threshold breach
            if (pContext->algorithm_type.type2.board_temperature > crit_temp_threshold) {
                if (++pContext->algorithm_type.type2.crit_temp_count > MAX_CRIT_TEMP_COUNT) {
                    //Shortly after this the host should reset the card
                    int i;
                    for (i = 0; i < MAX_PATTERN_REGS; i++)
                        PERIPHERAL_Clock_Monitor_Write_Reg(pContext->pClockMonitorPeripheralContext,
                            CLOCK_SCALING_PATTERN_REG[i], 0);
                }
            }
            else {
                pContext->algorithm_type.type2.crit_temp_count = 0;
                cmc_clock_throttling_phase1_algorithm(pContext, true);
            }
        }
    }
}


void cmc_clock_throttling_phase1_preprocessing(CMC_CLOCK_THROTTLING_CONTEXT_TYPE* pContext)
{
    switch (pContext->Algorithm)
    {
        case CT_ALGORITHM_STANDARD:
            cmc_clock_throttling_phase1_preprocessing_type1(pContext);
            break;

        case CT_ALGORITHM_SSD:
            // Do SSD clock throttling
            cmc_clock_throttling_phase1_preprocessing_type2(pContext);
            break;

        case CT_ALGORITHM_NONE:
        default:
            break;
    }
}



void cmc_clock_throttling_phase1_algorithm_type1(CMC_CLOCK_THROTTLING_CONTEXT_TYPE* pContext, bool ReadingsHaveChanged)
{
    float RateCurrent;
    float RateLinear;
    bool  bClockShutdownRequest;

    if (PERIPHERAL_AXI_GPIO_MUTEX_CMC_ReachOutInterfaceAccessIsGranted(pContext->algorithm_type.type1.pPeripheralMutexContext))
    {
        // Response to CLock Shutdown CR-1064421
        // Read Latched Shutdown Clocks GPIO bit      
        bClockShutdownRequest = PERIPHERAL_ClockThrottling_Read_LatchedClockShutdownRequest(pContext->pClockThrottlingPeripheralContext);
        if (bClockShutdownRequest)
        {
            pContext->algorithm_type.type1.Activity = MIN(pContext->algorithm_type.type1.Activity, 0x20);
            PERIPHERAL_ClockThrottling_UpdateThrottlingControl(pContext->pClockThrottlingPeripheralContext, 
                pContext->algorithm_type.type1.Activity | MASK_CLOCKTHROTTLING_ENABLE_THROTTLING | MASK_CLEAR_LATCHEDSHUTDOWNCLOCKS);
        }
    }

    if ((pContext->PowerOverRideEnabled) &&
        (pContext->XRTSuppliedBoardThrottlingThresholdPower < pContext->algorithm_type.type1.BoardThrottlingThresholdPower))
    {
        pContext->algorithm_type.type1.KernelPower = pContext->algorithm_type.type1.BoardNormalizedPower 
            - (pContext->algorithm_type.type1.IdlePower / (float)pContext->XRTSuppliedBoardThrottlingThresholdPower);
        pContext->algorithm_type.type1.KernelTarget = (float)(1.0 - ((float)pContext->algorithm_type.type1.IdlePower / (float)pContext->XRTSuppliedBoardThrottlingThresholdPower));
    }
    else
    {
        pContext->algorithm_type.type1.KernelPower = pContext->algorithm_type.type1.BoardNormalizedPower 
            - (pContext->algorithm_type.type1.IdlePower / (float)pContext->algorithm_type.type1.BoardThrottlingThresholdPower);
        pContext->algorithm_type.type1.KernelTarget = (float)(1.0 - ((float)pContext->algorithm_type.type1.IdlePower / (float)pContext->algorithm_type.type1.BoardThrottlingThresholdPower));
    }


    if (pContext->algorithm_type.type1.KernelTarget < 0)
    {
        pContext->algorithm_type.type1.Activity = 0;
    }
    else if (pContext->algorithm_type.type1.KernelPower > 0)
    {
        RateCurrent = (float)((float)pContext->algorithm_type.type1.Activity / (float)ACTIVITY_MAX);

        if (ReadingsHaveChanged)
        {
            RateLinear = (pContext->algorithm_type.type1.KernelTarget * RateCurrent) / pContext->algorithm_type.type1.KernelPower;
        }
        else
        {
            RateLinear = RateCurrent;
        }

        if (RateLinear < RateCurrent)
        {
            pContext->algorithm_type.type1.Activity = (uint32_t)(RateLinear * (float)ACTIVITY_MAX);
        }
        else if (pContext->algorithm_type.type1.Activity < ACTIVITY_MAX)
        {
            pContext->algorithm_type.type1.Activity = pContext->algorithm_type.type1.Activity + 1;
        }
        else
        {
            pContext->algorithm_type.type1.Activity = ACTIVITY_MAX;
        }
    }
    else
    {
        pContext->algorithm_type.type1.Activity = ACTIVITY_MAX;
    }

    // Read the location & write activity to it
    if (PERIPHERAL_AXI_GPIO_MUTEX_CMC_ReachOutInterfaceAccessIsGranted(pContext->algorithm_type.type1.pPeripheralMutexContext))
    {
//      PERIPHERAL_ClockThrottling_Enable_Throttling(pContext->pClockThrottlingPeripheralContext);
        PERIPHERAL_ClockThrottling_UpdateThrottlingControl(pContext->pClockThrottlingPeripheralContext, 
            pContext->algorithm_type.type1.Activity | MASK_CLOCKTHROTTLING_ENABLE_THROTTLING);
    }
   
}


void cmc_clock_throttling_phase1_algorithm_type2(CMC_CLOCK_THROTTLING_CONTEXT_TYPE* pContext)
{
    uint32_t pi_mode = PERIPHERAL_Clock_Monitor_Read_Reg(pContext->pClockMonitorPeripheralContext, PI_ALGO_SELECT_REG);
    uint32_t ones, zeroes, changes;
    long delta_p = 0, delta_t = 0;
    double change;
    uint32_t multiplier, ones_min, ones_max;

    switch (pi_mode)
    {
    case 0://Power based PI algo
        if (pContext->algorithm_type.type2.board_power < pContext->algorithm_type.type2.target_power) {
            //If clock is not scaled down then do nothing.
            if (pContext->algorithm_type.type2.clk_scaled == false)
                return;
            //If measured power is within band range, then do nothing.
            if (pContext->algorithm_type.type2.board_power >= pContext->algorithm_type.type2.target_power2)
                return;
        }

        //Try to stabilize the power between target_power and target_power2
        pContext->algorithm_type.type2.target_power = pContext->algorithm_type.type2.target_power - (uint32_t)(pContext->algorithm_type.type2.power_band / 2.0);
        delta_p = pContext->algorithm_type.type2.target_power - pContext->algorithm_type.type2.board_power;
        change = delta_p * pContext->algorithm_type.type2.kp_p;
        if (pContext->algorithm_type.type2.p_integral) {
            //Add error (delta_p) to the integral sum in every cycle.
            pContext->algorithm_type.type2.integral_p_err += delta_p;
            change += (pContext->algorithm_type.type2.ki_p * pContext->algorithm_type.type2.integral_p_err);
        }

        change = change / 1000.0;
        changes = cmcCeil(change);
        ones = clk_bits[ones_count] + changes;
        zeroes = clk_bits[zeros_count] - changes;
        for (int i = 0; i < MAX_PATTERN_REGS; i++)
            pattern[i] = PERIPHERAL_Clock_Monitor_Read_Reg(pContext->pClockMonitorPeripheralContext, CLOCK_SCALING_PATTERN_REG[i]);

        if (ones >= TOTAL_CLOCK_BITS) {
            if ((pContext->algorithm_type.type2.clk_scaled == true) && pContext->algorithm_type.type2.p_integral)
                pContext->algorithm_type.type2.integral_p_err -= delta_p;
            ones = TOTAL_CLOCK_BITS;
            zeroes = 0;
            pContext->algorithm_type.type2.clk_scaled = false;
        }
        else {
            pContext->algorithm_type.type2.clk_scaled = true;
        }

        if (zeroes >= TOTAL_CLOCK_BITS) {
            zeroes = TOTAL_CLOCK_BITS - 1;
            ones = CLOCK_FREQ_MULTIPLIER_MIN_VAL;
            if (pContext->algorithm_type.type2.p_integral)
                pContext->algorithm_type.type2.integral_p_err -= delta_p;
        }

        multiplier = PERIPHERAL_Clock_Monitor_Read_Reg(pContext->pClockMonitorPeripheralContext, CLOCK_FREQ_MULTIPLIER_REGS);
        ones_min = (multiplier >> CLOCK_FREQ_MULTIPLIER_MIN_BIT_POS) &
            CLOCK_FREQ_MULTIPLIER_MASK;
        ones_max = (multiplier >> CLOCK_FREQ_MULTIPLIER_MAX_BIT_POS) &
            CLOCK_FREQ_MULTIPLIER_MASK;
        if (ones < ones_min) {
            ones = ones_min;
            zeroes = TOTAL_CLOCK_BITS - ones;
        }
        if (ones > ones_max) {
            ones = ones_max;
            zeroes = TOTAL_CLOCK_BITS - ones;
        }

        PERIPHERAL_Clock_Monitor_Write_Reg(pContext->pClockMonitorPeripheralContext, STORE_ONES_ZEROES_COUNT_REG,
            ((ones << STORE_ONES_ZEROES_COUNT_ONES_POS) |
                (zeroes << STORE_ONES_ZEROES_COUNT_ZEROES_POS)));
        PERIPHERAL_Clock_Monitor_Distribute_Main(ones, zeroes);
        PERIPHERAL_Clock_Monitor_Load_All_Pattern(pContext->pClockMonitorPeripheralContext, false);
        break;

    case 1: //temperature based PI algo chosen.
        if (pContext->algorithm_type.type2.board_temperature < pContext->algorithm_type.type2.target_temperature) {
            //If clock is not scaled down then do nothing.
            if (pContext->algorithm_type.type2.clk_scaled == false)
                return;
            //If measured power is within band range, then do nothing.
            if (pContext->algorithm_type.type2.board_temperature >= pContext->algorithm_type.type2.target_temperature2)
                return;
        }
        //Try to stabilize the temperature between target_temperature and target_temperature2
        pContext->algorithm_type.type2.target_temperature = pContext->algorithm_type.type2.target_temperature - (uint32_t)(pContext->algorithm_type.type2.temperature_band / 2.0);
        delta_t = pContext->algorithm_type.type2.target_temperature - pContext->algorithm_type.type2.board_temperature;
        change = delta_t * pContext->algorithm_type.type2.kp_t;
        changes = cmcCeil(change);
        ones = clk_bits[ones_count] + changes;
        zeroes = clk_bits[zeros_count] - changes;

        for (int i = 0; i < MAX_PATTERN_REGS; i++)
            pattern[i] = PERIPHERAL_Clock_Monitor_Read_Reg(pContext->pClockMonitorPeripheralContext, CLOCK_SCALING_PATTERN_REG[i]);

        if (ones > TOTAL_CLOCK_BITS) {
            ones = TOTAL_CLOCK_BITS;
            zeroes = 0;
            pContext->algorithm_type.type2.clk_scaled = false;
        }
        else {
            pContext->algorithm_type.type2.clk_scaled = true;
        }

        if (zeroes >= TOTAL_CLOCK_BITS) {
            zeroes = TOTAL_CLOCK_BITS - 1;
            ones = CLOCK_FREQ_MULTIPLIER_MIN_VAL;
        }

        multiplier = PERIPHERAL_Clock_Monitor_Read_Reg(pContext->pClockMonitorPeripheralContext, CLOCK_FREQ_MULTIPLIER_REGS);
        ones_min = (multiplier >> CLOCK_FREQ_MULTIPLIER_MIN_BIT_POS) &
            CLOCK_FREQ_MULTIPLIER_MASK;
        ones_max = (multiplier >> CLOCK_FREQ_MULTIPLIER_MAX_BIT_POS) &
            CLOCK_FREQ_MULTIPLIER_MASK;
        if (ones < ones_min) {
            ones = ones_min;
            zeroes = TOTAL_CLOCK_BITS - ones;
        }
        if (ones > ones_max) {
            ones = ones_max;
            zeroes = TOTAL_CLOCK_BITS - ones;
        }
        PERIPHERAL_Clock_Monitor_Write_Reg(pContext->pClockMonitorPeripheralContext, STORE_ONES_ZEROES_COUNT_REG,
            ((ones << STORE_ONES_ZEROES_COUNT_ONES_POS) |
                (zeroes << STORE_ONES_ZEROES_COUNT_ZEROES_POS)));
        PERIPHERAL_Clock_Monitor_Distribute_Main(ones, zeroes);
        PERIPHERAL_Clock_Monitor_Load_All_Pattern(pContext->pClockMonitorPeripheralContext, false);
        break;

    case 2: //temperature and power limit chosen
        if ((pContext->algorithm_type.type2.board_temperature < pContext->algorithm_type.type2.target_temperature) &&
            (pContext->algorithm_type.type2.board_power < pContext->algorithm_type.type2.target_power)) {
            //If clock is not scaled in power and temperature then do nothing.
            if ((pContext->algorithm_type.type2.clk_scaled_in_pwr == false) && (pContext->algorithm_type.type2.clk_scaled_in_temp == false))
            {
                return;
            }
            if ((pContext->algorithm_type.type2.board_power > pContext->algorithm_type.type2.target_power2) &&
                (pContext->algorithm_type.type2.board_temperature > pContext->algorithm_type.type2.target_temperature2))
            {
                return;
            }
            //Try to stabilize the power and temperature.
            pContext->algorithm_type.type2.target_temperature = pContext->algorithm_type.type2.target_temperature - pContext->algorithm_type.type2.temperature_band / 2;
            pContext->algorithm_type.type2.target_power = pContext->algorithm_type.type2.target_power - pContext->algorithm_type.type2.power_band / 2;
            if (pContext->algorithm_type.type2.clk_scaled_in_pwr == false) {
                //clock might be scaled down due to temperature
                if (pContext->algorithm_type.type2.board_temperature > pContext->algorithm_type.type2.target_temperature2)
                    return;
                //Measure delta/change to increase the clock
                delta_t = pContext->algorithm_type.type2.target_temperature - pContext->algorithm_type.type2.board_temperature;
                change = pContext->algorithm_type.type2.kp_t * delta_t;
            }
            else if (pContext->algorithm_type.type2.clk_scaled_in_temp == false) {
                //clock might be scaled down due to power
                if (pContext->algorithm_type.type2.board_power > pContext->algorithm_type.type2.target_power2)
                    return;
                //Measure delta/change to increase the clock
                delta_p = pContext->algorithm_type.type2.target_power - pContext->algorithm_type.type2.board_power;
                change = pContext->algorithm_type.type2.kp_p * delta_p;
                if (pContext->algorithm_type.type2.p_integral) {
                    pContext->algorithm_type.type2.integral_p_err += delta_p;
                    change += pContext->algorithm_type.type2.ki_p * pContext->algorithm_type.type2.integral_p_err;
                }
            }
            else {
                //clock might be scaled down due to both power and temperature.
                delta_p = pContext->algorithm_type.type2.target_power - pContext->algorithm_type.type2.board_power;
                delta_t = pContext->algorithm_type.type2.target_temperature - pContext->algorithm_type.type2.board_temperature;
                change = (pContext->algorithm_type.type2.kp_p / 2) * delta_p + (pContext->algorithm_type.type2.kp_t / 2) * delta_t;
                if (pContext->algorithm_type.type2.p_integral) {
                    pContext->algorithm_type.type2.integral_p_err += delta_p;
                    change += pContext->algorithm_type.type2.ki_p * pContext->algorithm_type.type2.integral_p_err;
                }
            }
        }
        else {
            if (pContext->algorithm_type.type2.board_power < pContext->algorithm_type.type2.target_power) {
                //clock to be scaled down due to temperature only
                delta_t = pContext->algorithm_type.type2.target_temperature - pContext->algorithm_type.type2.board_temperature;
                change = pContext->algorithm_type.type2.kp_t * delta_t;
                if (change < 0)
                    pContext->algorithm_type.type2.clk_scaled_in_temp = true;
            }
            else if (pContext->algorithm_type.type2.board_temperature < pContext->algorithm_type.type2.target_temperature) {
                //clock to be scaled down due to power only
                delta_p = pContext->algorithm_type.type2.target_power - pContext->algorithm_type.type2.board_power;
                change = pContext->algorithm_type.type2.kp_p * delta_p;
                if (pContext->algorithm_type.type2.p_integral) {
                    pContext->algorithm_type.type2.integral_p_err += delta_p;
                    change += pContext->algorithm_type.type2.ki_p * pContext->algorithm_type.type2.integral_p_err;
                }
                if (change < 0)
                    pContext->algorithm_type.type2.clk_scaled_in_pwr = true;
            }
            else {
                //clock to be scaled down due to both power and temperature
                delta_t = pContext->algorithm_type.type2.target_temperature - pContext->algorithm_type.type2.board_temperature;
                delta_p = pContext->algorithm_type.type2.target_power - pContext->algorithm_type.type2.board_power;
                change = (pContext->algorithm_type.type2.kp_p / 2) * delta_p + (pContext->algorithm_type.type2.kp_t / 2) * delta_t;
                if (pContext->algorithm_type.type2.p_integral) {
                    pContext->algorithm_type.type2.integral_p_err += delta_p;
                    change += pContext->algorithm_type.type2.ki_p * pContext->algorithm_type.type2.integral_p_err;
                }
                if (change < 0) {
                    pContext->algorithm_type.type2.clk_scaled_in_pwr = true;
                    pContext->algorithm_type.type2.clk_scaled_in_temp = true;
                }
            }
        }
        change = change / 1000.0;
        changes = cmcCeil(change);
        ones = clk_bits[ones_count] + changes;
        zeroes = clk_bits[zeros_count] - changes;

        for (int i = 0; i < MAX_PATTERN_REGS; i++)
            pattern[i] = PERIPHERAL_Clock_Monitor_Read_Reg(pContext->pClockMonitorPeripheralContext, CLOCK_SCALING_PATTERN_REG[i]);

        if (ones >= TOTAL_CLOCK_BITS) {
            ones = TOTAL_CLOCK_BITS;
            zeroes = 0;
            if ((pContext->algorithm_type.type2.clk_scaled_in_pwr == true) && pContext->algorithm_type.type2.p_integral)
                pContext->algorithm_type.type2.integral_p_err -= delta_p;
            pContext->algorithm_type.type2.clk_scaled_in_pwr = false;
            pContext->algorithm_type.type2.clk_scaled_in_temp = false;
        }
        if (zeroes >= TOTAL_CLOCK_BITS) {
            zeroes = TOTAL_CLOCK_BITS - 1;
            ones = CLOCK_FREQ_MULTIPLIER_MIN_VAL;
            if (pContext->algorithm_type.type2.p_integral)
                pContext->algorithm_type.type2.integral_p_err -= delta_p;
        }

        multiplier = PERIPHERAL_Clock_Monitor_Read_Reg(pContext->pClockMonitorPeripheralContext, CLOCK_FREQ_MULTIPLIER_REGS);
        ones_min = (multiplier >> CLOCK_FREQ_MULTIPLIER_MIN_BIT_POS) &
            CLOCK_FREQ_MULTIPLIER_MASK;
        ones_max = (multiplier >> CLOCK_FREQ_MULTIPLIER_MAX_BIT_POS) &
            CLOCK_FREQ_MULTIPLIER_MASK;
        if (ones < ones_min) {
            ones = ones_min;
            zeroes = TOTAL_CLOCK_BITS - ones;
        }
        if (ones > ones_max) {
            ones = ones_max;
            zeroes = TOTAL_CLOCK_BITS - ones;
        }

        PERIPHERAL_Clock_Monitor_Write_Reg(pContext->pClockMonitorPeripheralContext, STORE_ONES_ZEROES_COUNT_REG,
            ((ones << STORE_ONES_ZEROES_COUNT_ONES_POS) |
                (zeroes << STORE_ONES_ZEROES_COUNT_ZEROES_POS)));
        PERIPHERAL_Clock_Monitor_Distribute_Main(ones, zeroes);
        PERIPHERAL_Clock_Monitor_Load_All_Pattern(pContext->pClockMonitorPeripheralContext, false);
        break;
    }
}


void cmc_clock_throttling_phase1_algorithm(CMC_CLOCK_THROTTLING_CONTEXT_TYPE* pContext, bool ReadingsHaveChanged)
{
    switch (pContext->Algorithm)
    {
    case CT_ALGORITHM_STANDARD:
        cmc_clock_throttling_phase1_algorithm_type1(pContext, ReadingsHaveChanged);
        break;

    case CT_ALGORITHM_SSD:
        // Do SSD clock throttling
        cmc_clock_throttling_phase1_algorithm_type2(pContext);
        break;

    case CT_ALGORITHM_NONE:
    default:
        break;
    }
}
