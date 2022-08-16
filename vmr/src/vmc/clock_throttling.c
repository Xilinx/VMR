/******************************************************************************
 * Copyright (C) 2020 - 2022 Xilinx, Inc.  All rights reserved.
 * SPDX-License-Identifier: MIT
 *******************************************************************************/
#include "clock_throttling.h"
#include "stdbool.h"
#include "vmc_sensors.h"
#include "vmc_sc_comms.h"
#include "vmr_common.h"
#include "vmc_main.h"

extern SC_VMC_Data sc_vmc_data;
extern Vmc_Sensors_Gl_t sensor_glvr;

void ClockThrottling_Initialize(Clock_Throttling_Algorithm  *pContext, Build_Clock_Throttling_Profile *pThrottling)
{
	u8 i = 0;
	pContext->NumberOfRailsMonitored = pThrottling->NumberOfSensors;
	for (i = 0; i < pThrottling->NumberOfSensors; i++)
	{
		pContext->RailParameters[i].VoltageSensorID = pThrottling->VoltageSensorID[i];
		pContext->RailParameters[i].CurrentSensorID = pThrottling->CurrentSensorID[i];
		pContext->RailParameters[i].NominalVoltage = pThrottling->NominalVoltage[i];
		pContext->RailParameters[i].LatestReading.throttlingThresholdCurrent =
				pThrottling->throttlingThresholdCurrent[i];
	}

	pContext->bVccIntThermalThrottlingEnabled = pThrottling->bVCCIntThermalThrottling;
	pContext->IdlePower = pThrottling->IdlePower;

	pContext->IntegrataionSum = pThrottling->IntegrataionSumInitial;
	pContext->LastThermalThrottlingThresholdPower = pThrottling->IntegrataionSumInitial;
	pContext->ThermalThrottlingThresholdPower = 1.0;

	pContext->FPGAMeasuredTemp = 0;
	pContext->VccIntMeasuredTemp = 0;

	pContext->KernelPower = 0;
	pContext->KernelTarget = 0;
	pContext->Activity = ACTIVITY_MAX;
	pContext->XRTSuppliedBoardThrottlingThresholdPower = 0;

	pContext->RateCurrent = 0;
	pContext->RateLinear = 0;
	pContext->BoardNormalizedPower = 0;
	pContext->BoardThrottlingThresholdPower = 0;
	pContext->BoardMeasuredPower = 0;
	pContext->UserNormalizedPower = 0;

	pContext->TempGainKpFPGA = pThrottling->TempGainKpFPGA;
	pContext->TempGainKi = pThrottling->TempGainKi;
	pContext->TempGainKpVCCInt = pThrottling->TempGainKpVCCInt;
	pContext->TempGainKaw = pThrottling->TempGainKaw;

	pContext->FPGAThrottlingTempLimit      = FPGA_THROTTLING_TEMP_LIMIT;
	pContext->VccIntThrottlingTempLimit    = VCCINT_THROTTLING_TEMP_LIMIT;
	pContext->ThermalThrottlingLoopJustEnabled = false;

	pContext->FeatureEnabled = false;
	pContext->PowerOverRideEnabled = false;
	pContext->bUserThrottlingTempLimitEnabled = false;
}
/*
 * PI Loop (Proportional , Integral) is used to only for FPGA temperature
 * Only Proportional is used for vccint temperature as change in temperature is
 * relatively slower.
 *
 * The minimum of FPGAThrottlingTempLimit and  XRTSuppliedUserThrottlingTempLimit
 * shall be calculated as ThrottlingTempLimit_FPGA.
 *
 * DeltaTemp = ThrottlingTempLimit_FPGA - MeasuredTemp ( error = setpoint - preset value)
 *
 * Integration anti-windup:
 * 		The back-calculation anti-windup method uses a feedback loop to
 * 		discharge the PID Controller internal integrator when the controller
 * 		hits specified saturation limits and enters nonlinear operation.
 *
 * In Integral Only mode, the controller simply multiplies the integral of the error
 * (accumulation of error or area under the error curve) by the Integral Gain (Ki) to get the controller output.
 *
 * Total controlled output = o/p of proportional + o/p of Integral (ThermalThrottlingThresholdPower_FPGA)
 *
 *
 */
float clock_throttling_PI_Loop_FPGA(Clock_Throttling_Algorithm* pContext, bool bUserThrottlingTempLimitEnabled, bool bUseIntegrationSum, float PassedInThrottlingTempLimit, float MeasuredTemp)
{
	/* FPGA Temperature*/
	if (bUserThrottlingTempLimitEnabled) // This comes from XRT
	{
		pContext->ThrottlingTempLimit_FPGA = MIN(PassedInThrottlingTempLimit, pContext->XRTSuppliedUserThrottlingTempLimit); //pContext->FPGATempThrottlingLimit sent in SC message
	}
	else
	{
		pContext->ThrottlingTempLimit_FPGA = PassedInThrottlingTempLimit;
	}
	pContext->DeltaTemp_FPGA = (pContext->ThrottlingTempLimit_FPGA - MeasuredTemp);

	if (bUseIntegrationSum) // Only for FPGA Temp
	{
		pContext->IntegrationAntiWindup_FPGA = pContext->TempGainKaw *
				(pContext->BoardMeasuredPower - pContext->LastThermalThrottlingThresholdPower);

		if (pContext->ThermalThrottlingLoopJustEnabled)
		{
			// Initial pContext->.IntegtaionSum will be value set in the profile
			pContext->ThermalThrottlingLoopJustEnabled = false;
		}
		else
		{
			pContext->IntegrataionSum = pContext->IntegrataionSum +
					(pContext->DeltaTemp_FPGA * pContext->TempGainKi) + pContext->IntegrationAntiWindup_FPGA;
		}

		pContext->ThermalThrottlingThresholdPower_FPGA = (pContext->DeltaTemp_FPGA * pContext->TempGainKpFPGA) +
				pContext->IntegrataionSum;
		pContext->LastThermalThrottlingThresholdPower = pContext->ThermalThrottlingThresholdPower_FPGA;
	}
	else // VCC Int Temp
	{
		pContext->ThermalThrottlingThresholdPower_FPGA = (pContext->DeltaTemp_FPGA * pContext->TempGainKpVCCInt);
	}

	return pContext->ThermalThrottlingThresholdPower_FPGA;
}

void clock_throttling_algorithm_temperature(Clock_Throttling_Algorithm  *pContext )
{
	bool bUseIntegrationSum = true;
	float localThermalThrottlingThresholdPower = 0.0;

	pContext->FPGAMeasuredTemp = sensor_glvr.sensor_readings.sysmon_max_temp;
	pContext->VccIntMeasuredTemp = (float)sc_vmc_data.sensor_values[eSC_VCCINT_TEMP];

	pContext->FPGAThrottlingTempLimit = FPGA_THROTTLING_TEMP_LIMIT;
	pContext->VccIntThrottlingTempLimit = VCCINT_THROTTLING_TEMP_LIMIT;

	// FPGA Temperature
	pContext->localThermalThrottlingThresholdPower_FPGA = clock_throttling_PI_Loop_FPGA(pContext,
			pContext->bUserThrottlingTempLimitEnabled, bUseIntegrationSum,
			(float)pContext->FPGAThrottlingTempLimit, pContext->FPGAMeasuredTemp);

	localThermalThrottlingThresholdPower = pContext->localThermalThrottlingThresholdPower_FPGA;

	if (pContext->bVccIntThermalThrottlingEnabled)
	{
		/* VCCInt Temperature*/
		pContext->localVccIntThermalThrottlingThresholdPower =
				((float)pContext->VccIntThrottlingTempLimit - pContext->VccIntMeasuredTemp) * pContext->TempGainKpVCCInt;
		// Use the smaller of the 2 values
		if (pContext->localVccIntThermalThrottlingThresholdPower < pContext->localThermalThrottlingThresholdPower_FPGA)
		{
			localThermalThrottlingThresholdPower = pContext->localVccIntThermalThrottlingThresholdPower;
		}
	}

	// Output of thermal stage
	pContext->ThermalThrottlingThresholdPower = localThermalThrottlingThresholdPower;

}
/*
 * If the power over ride/temperature over ride is enabled algorithm uses XRT supplied value,
 * to calculate Kernal Power and Kernal target. This only for user to test with different throttling limits.
 *
 * if power/temperature over ride not enabled default values will be used.
 *
 * Idle power should be the value measured after a system has been booted and before any xclbin loaded
 *
 * The Kernel_Power and Kernel_Target try to factor in the power consumption of the board.
 *
 * Rate_Current is a normalized measure of the toggle rate chosen by the previous iteration.
 *
 * Rate_Linear is the equivalently normalized value of how the current power measurement needs to be modified to reach the target power.
 *
 * The inner if (Rate_Linear < Rate_Current) is a "Fast attack, slow decay" control function.
 * It will rapidly decrease the power if the estimated toggle rate to meet the target power (Rate_Linear) is less than the current toggle rate (Rate_Current).
 * If the estimated toggle rate is greater it only ramps up slowly. This avoids the control function oscillating.
 *
 * when the input values voltage and power values are not changing for couple of iterations
 * algorithm tries to ramp up Activity values so that we can see changes in new readings.
 *
 * For VCK5000 getting readings from SC . But in case V70 all the voltage and current readings
 * can be read it from VMC only, that will be helpful to run the algorithm in better way
 */

void clock_throttling_algorithm(Clock_Throttling_Algorithm* pContext, bool ReadingsHaveChanged)
{
	float RateCurrent = 0.0;
	float RateLinear = 0.0;

	if (IO_SYNC_READ32(VMR_EP_UCS_CONTROL_STATUS_BASEADDR) & 0x01)
	{
		pContext->Activity = MIN(pContext->Activity, 0x20);
		IO_SYNC_WRITE32(pContext->Activity | MASK_CLOCKTHROTTLING_ENABLE_THROTTLING | MASK_CLEAR_LATCHEDSHUTDOWNCLOCKS, VMR_EP_GAPPING_DEMAND);
	}

	if ((pContext->PowerOverRideEnabled) &&
			(pContext->XRTSuppliedBoardThrottlingThresholdPower < pContext->BoardThrottlingThresholdPower))
	{
		pContext->KernelPower = pContext->BoardNormalizedPower
				- (pContext->IdlePower / (float)pContext->XRTSuppliedBoardThrottlingThresholdPower);
		pContext->KernelTarget = (float)(1.0 - (pContext->IdlePower / (float)pContext->XRTSuppliedBoardThrottlingThresholdPower));
	}
	else
	{
		pContext->KernelPower = pContext->BoardNormalizedPower
				- (pContext->IdlePower / (float)pContext->BoardThrottlingThresholdPower);
		pContext->KernelTarget = (float)(1.0 - (pContext->IdlePower / (float)pContext->BoardThrottlingThresholdPower));
	}

	if (pContext->KernelTarget < 0)
	{
		pContext->Activity = 0;
	}
	else if (pContext->KernelPower > 0)
	{
		RateCurrent = (float)((float)pContext->Activity / (float)ACTIVITY_MAX);

		if (ReadingsHaveChanged)
		{
			RateLinear = (pContext->KernelTarget * RateCurrent) / pContext->KernelPower;
		}
		else
		{
			RateLinear = RateCurrent;
		}
		if (RateLinear < RateCurrent)
		{
			pContext->Activity = (uint32_t)(RateLinear * (float)ACTIVITY_MAX);
		}
		else if (pContext->Activity < ACTIVITY_MAX)
		{
			pContext->Activity = pContext->Activity + 1;
		}
		else
		{
			pContext->Activity = ACTIVITY_MAX;
		}
	}
	else
	{
		pContext->Activity = ACTIVITY_MAX;
	}

	VMC_DBG(" %d 	%d	%f	%f",pContext->Activity,(pContext->BoardMeasuredPower)/1000000,pContext->FPGAMeasuredTemp,pContext->VccIntMeasuredTemp);
	IO_SYNC_WRITE32(pContext->Activity | MASK_CLOCKTHROTTLING_ENABLE_THROTTLING, VMR_EP_GAPPING_DEMAND);
}
/*
 * When clock throttling is disabled VMC shall periodically
 * write an activity value = 128  and disable throttling flag on versal registers
 *
 * When clock throttling enabled in clock_throttling_algorithm the latched shutdown
 * clock bit shall be checked to determine if the clock shutdown is requested
 * if so , pContext->Activity value from previous iteration shall be set to
 * MIN(pContext->Activity, 0x20);When the activity is written to the versal registers,
 * the throttling enabled bit shall be set to ‘1
 *
 * clock_throttling_algorithm_power this is used to calculate BoardNormalizedPower
 *
 * BoardNormalizedPower is the main input for clock_throttling_algorithm
 *
 * The algorithm will run for every 100 ms , Every time it will fetch latest voltage
 * and current readings from a global structure to calculate power of each power rail.
 *
 * BoardMeasuredPower is sum of the powers of all power rails.
 * BoardThrottlingThresholdPower is sum all throttling threshold powers
 *
 * Select the maximum value from all enabled instances of the per Rail Normalised Power ,
 * the User Normalised Power, the Thermal Normalised Power as the Board Normalised Power.
 *
 * Then call clock_throttling_algorithm
 */

void clock_throttling_algorithm_power(Clock_Throttling_Algorithm  *pContext )
{
	 u8 i;
	 u8 voltage_snsr_id = 0;
	 u8 current_snsr_id = 0;
	 bool ReadingsHaveChanged = false;
	 pContext->BoardThrottlingThresholdPower = 0;
	 pContext->BoardMeasuredPower = 0;

	 if (pContext->FeatureEnabled)
	 {
		 for (i = 0; i < pContext->NumberOfRailsMonitored; i++)
		 {
			 voltage_snsr_id = pContext->RailParameters[i].VoltageSensorID;
			 current_snsr_id = pContext->RailParameters[i].CurrentSensorID;
			 pContext->RailParameters[i].LatestReading.Voltage = sc_vmc_data.sensor_values[voltage_snsr_id];
			 pContext->RailParameters[i].LatestReading.Current = sc_vmc_data.sensor_values[current_snsr_id];
		 }

		 // Check for duplicates
		 for (i = 0; i < pContext->NumberOfRailsMonitored; i++)
		 {
			 if (pContext->RailParameters[i].LatestReading.Current != pContext->RailParameters[i].PreviousReading.Current)
			 {
				 ReadingsHaveChanged = true;
			 }

			 pContext->RailParameters[i].MeasuredPower = pContext->RailParameters[i].LatestReading.Current
					 * pContext->RailParameters[i].LatestReading.Voltage;

			 pContext->RailParameters[i].ThrottledThresholdPower = pContext->RailParameters[i].NominalVoltage
					 * pContext->RailParameters[i].LatestReading.throttlingThresholdCurrent;

			 pContext->BoardThrottlingThresholdPower += pContext->RailParameters[i].ThrottledThresholdPower;
			 pContext->BoardMeasuredPower += pContext->RailParameters[i].MeasuredPower;

			 pContext->RailParameters[i].RailNormalizedPower = ((float)pContext->RailParameters[i].MeasuredPower
					 / (float)pContext->RailParameters[i].ThrottledThresholdPower);

			 // Store new reading into previous
			 pContext->RailParameters[i].PreviousReading.Current = pContext->RailParameters[i].LatestReading.Current;
			 pContext->RailParameters[i].PreviousReading.Voltage = pContext->RailParameters[i].LatestReading.Voltage;
			 pContext->RailParameters[i].PreviousReading.throttlingThresholdCurrent = pContext->RailParameters[i].LatestReading.throttlingThresholdCurrent;
		 }

		 // If Thermal is enabled include the ThermalThrottlingThresholdPower
		 pContext->ThermalNormalisedPower = pContext->BoardMeasuredPower / pContext->ThermalThrottlingThresholdPower;

		 // Check if XRT has supplied a BoardThrottlingThresholdPower if so use it
		 // This comes from Clock Throttling Power Management Register in the peripheral7
		 if (pContext->PowerOverRideEnabled)
		 {
			 pContext->UserNormalizedPower = ((float)pContext->BoardMeasuredPower / (float)pContext->XRTSuppliedBoardThrottlingThresholdPower);
		 }
		 else
		 {
			 pContext->UserNormalizedPower = ((float)pContext->BoardMeasuredPower / (float)pContext->BoardThrottlingThresholdPower);
		 }

		 pContext->BoardNormalizedPower = 0;
		 for (i = 0; i < pContext->NumberOfRailsMonitored; i++)
		 {
			 // Select the max of
			 if (pContext->BoardNormalizedPower < pContext->RailParameters[i].RailNormalizedPower)
			 {
				 pContext->BoardNormalizedPower = pContext->RailParameters[i].RailNormalizedPower;
			 }
		 }

		 if (pContext->BoardNormalizedPower < pContext->UserNormalizedPower)
		 {
			 pContext->BoardNormalizedPower = (float)pContext->UserNormalizedPower;
		 }

		 if (pContext->BoardNormalizedPower < pContext->ThermalNormalisedPower)
		 {
			 pContext->BoardNormalizedPower = pContext->ThermalNormalisedPower;
		 }

		 // Now call the algorithm
		 clock_throttling_algorithm(pContext, ReadingsHaveChanged);

	 }
	 else
	 {
		 // Response to CLock Shutdown CR-1064421
		 // Read the location & write activity to it
		 pContext->Activity = ACTIVITY_MAX;
		 IO_SYNC_WRITE32(pContext->Activity | MASK_CLOCKTHROTTLING_DISABLE_THROTTLING, VMR_EP_GAPPING_DEMAND);

	 }

}
