/******************************************************************************
 * Copyright (C) 2020 - 2022 Xilinx, Inc.  All rights reserved.
 * SPDX-License-Identifier: MIT
 *******************************************************************************/
#include "xil_types.h"
#include "stdbool.h"


#define MAX_RAILS_MONITORED 5
#define ACTIVITY_MAX            (128)
#define CLOCK_THROTTLING_AVERAGE_SIZE 10

#define MASK_CLOCKTHROTTLING_DISABLE_THROTTLING (0 << 20)
#define MASK_CLOCKTHROTTLING_ENABLE_THROTTLING  (1 << 20)
#define MASK_CLEAR_LATCHEDSHUTDOWNCLOCKS        (1 << 16)
#define MASK_GAPPING_DEMAND_CONTROL		(1 << 24)

#define FPGA_THROTTLING_TEMP_LIMIT    95
#define VCCINT_THROTTLING_TEMP_LIMIT 105

#define VCK5000_PEX_12V_I_IN_THROTTLING_LIMIT    5500
#define VCK5000_AUX_12V_I_IN_THROTTLING_LIMIT_2X3   6250
#define VCK5000_AUX_12V_I_IN_THROTTLING_LIMIT_2X4   12500

typedef struct __attribute__((packed)) Build_Clock_Throttling_Profile
{
	u8	NumberOfSensors;
	u8	VoltageSensorID[5];
	u8	CurrentSensorID[5];
	u16	NominalVoltage[5];
	u16	throttlingThresholdCurrent[5];
	float	IdlePower; //in uW
	bool	bVCCIntThermalThrottling;

	float	TempGainKpFPGA;
	float	TempGainKi;
	float	TempGainKpVCCInt;
	float	TempGainKaw;

	float	IntegrataionSumInitial;

}Build_Clock_Throttling_Profile;

typedef struct __attribute__((packed)) Clock_Throttling_Rail_Type
{
	u16    Voltage;
	u16    Current;
	u16    throttlingThresholdCurrent;

} Clock_Throttling_Rail_Type;

typedef struct __attribute__((packed)) Clock_Throttling_Per_Rail_Type
{
	u8				VoltageSensorID;
	u8 				CurrentSensorID;
	Clock_Throttling_Rail_Type	PreviousReading;
	Clock_Throttling_Rail_Type	LatestReading;
	u16				NominalVoltage;
	u32				MeasuredPower;
	u32				ThrottledThresholdPower;
	float				RailNormalizedPower;
}Clock_Throttling_Per_Rail_Type;

typedef struct __attribute__((packed)) Clock_Throttling_Algorithm
{
	u8					NumberOfRailsMonitored;
	Clock_Throttling_Per_Rail_Type		RailParameters[MAX_RAILS_MONITORED];
	u32					BoardThrottlingThresholdPower;
	u32					BoardMeasuredPower;
	float					UserNormalizedPower;
	float					BoardNormalizedPower;
	float					KernelPower;
	float					KernelTarget;
	float					IdlePower;
	u32					Activity;
	float					RateCurrent;
	float					RateLinear;

	float					TempGainKpFPGA;
	float					TempGainKi;
	float					TempGainKpVCCInt;
	float					TempGainKaw;

	float					ThermalThrottlingThresholdPower;        //Output from Thermal stage
	float					LastThermalThrottlingThresholdPower;

	float					ThermalNormalisedPower;
	float					VccIntThermalNormalisedPower;

	float					FPGAMeasuredTemp;
	u32					FPGAThrottlingTempLimit;

	bool					bVccIntThermalThrottlingEnabled;
	float					VccIntMeasuredTemp;
	u32					VccIntThrottlingTempLimit;

	bool					ThermalThrottlingLoopJustEnabled;
	float					IntegrataionSum;

	u32					FPGAMeasuredTempValues;
	u32					VCCIntMeasuredTempValues;
	u8					AverageArrayIndex;

	float					localVccIntThermalThrottlingThresholdPower;
	float					localThermalThrottlingThresholdPower_FPGA;

	float					ThrottlingTempLimit_FPGA;
	float					ThermalThrottlingThresholdPower_FPGA;
	float					DeltaTemp_FPGA;
	float					IntegrationAntiWindup_FPGA;

	// From XRT
	bool					FeatureEnabled;
	bool					PowerOverRideEnabled;
	bool					bUserThrottlingTempLimitEnabled;
	u32					XRTSuppliedBoardThrottlingThresholdPower;
	u32					XRTSuppliedUserThrottlingTempLimit;

}Clock_Throttling_Algorithm;

void clock_throttling_algorithm_temperature(Clock_Throttling_Algorithm  *pContext );
void clock_throttling_algorithm_power(Clock_Throttling_Algorithm  *pContext );
void ClockThrottling_Initialize(Clock_Throttling_Algorithm  *pContext, Build_Clock_Throttling_Profile *pThrottling);
