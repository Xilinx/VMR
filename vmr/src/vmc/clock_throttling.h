/******************************************************************************
 * Copyright (C) 2024 Advanced Micro Devices, Inc.    All rights reserved.
 * SPDX-License-Identifier: MIT
 *******************************************************************************/
#include "xil_types.h"
#include "stdbool.h"


#define MAX_RAILS_MONITORED 5
#define ACTIVITY_MAX            (128)
#define CLOCK_THROTTLING_AVERAGE_SIZE 10

#define NOMINAL_VOLTAGE_12V_IN_MV 12000
#define NOMINAL_VOLTAGE_3V3_IN_MV 3300

#define MASK_CLOCKTHROTTLING_DISABLE_THROTTLING (0 << 20)
#define MASK_CLOCKTHROTTLING_ENABLE_THROTTLING  (1 << 20)
#define MASK_CLEAR_LATCHEDSHUTDOWNCLOCKS        (1 << 16)
#define MASK_GAPPING_DEMAND_CONTROL		(1 << 24)

#define FPGA_THROTTLING_TEMP_LIMIT    95
#define VCCINT_THROTTLING_TEMP_LIMIT 105

#define VCK5000_PEX_12V_I_IN_THROTTLING_LIMIT    5500
#define VCK5000_AUX_12V_I_IN_THROTTLING_LIMIT_2X3   6250
#define VCK5000_AUX_12V_I_IN_THROTTLING_LIMIT_2X4   12500

typedef struct __attribute__((packed)) Clock_Throttling_Profile_s
{
	u8	NumberOfSensors;
	u8	VoltageSensorID[5];
	u8	CurrentSensorID[5];
	u16	NominalVoltage[5];
	u16	throttlingThresholdCurrent[5];
	float	IdlePower; //in uW
	bool	bVCCIntThermalThrottling;
	u32	FPGATempThrottlingLimit;
	u32	VccIntTempThrottlingLimit;
	u16	PowerThrottlingLimit;
	u16 	TempThrottlingLimit;

	float	TempGainKpFPGA;
	float	TempGainKi;
	float	TempGainKpVCCInt;
	float	TempGainKaw;

	float	IntegrataionSumInitial;

}Clock_Throttling_Profile_t;

typedef struct __attribute__((packed)) Clock_Throttling_Rail_Type_s
{
	u16    Voltage;
	u16    Current;
	u16    throttlingThresholdCurrent;

} Clock_Throttling_Rail_Type_t;

typedef struct __attribute__((packed)) Clock_Throttling_Per_Rail_Type_s
{
	u8				VoltageSensorID;
	u8 				CurrentSensorID;
	Clock_Throttling_Rail_Type_t	PreviousReading;
	Clock_Throttling_Rail_Type_t	LatestReading;
	u16				NominalVoltage;
	u32				MeasuredPower;
	u32				ThrottledThresholdPower;
	float				RailNormalizedPower;
}Clock_Throttling_Per_Rail_Type_t;

typedef struct __attribute__((packed)) Clock_Throttling_Handle_s
{
	u8					NumberOfRailsMonitored;
	Clock_Throttling_Per_Rail_Type_t		RailParameters[MAX_RAILS_MONITORED];
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
	u32					FPGATempThrottlingLimit;

	bool					bVccIntThermalThrottlingEnabled;
	float					VccIntMeasuredTemp;
	u32					VccIntTempThrottlingLimit;

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

	// Power and Temp throttling limits to send it to XRT
	u16					PowerThrottlingLimit;
	u16					TempThrottlingLimit;

}Clock_Throttling_Handle_t;

typedef enum {
	eCLK_SCALING_MODE_PWR  = 0x0,
	eCLK_SCALING_MODE_TEMP = 0x1,
	eCLK_SCALING_MODE_BOTH = 0x2
}eClk_scaling_mode_t;

typedef struct __attribute__((packed)) Moving_Average_s
{
	float * buffer;
	float sum;
	u8 pos;
	u8 length;
	bool is_filled;
}Moving_Average_t;


void clock_throttling_algorithm_temperature(Clock_Throttling_Handle_t  *pContext );
void clock_throttling_algorithm_power(Clock_Throttling_Handle_t  *pContext );
void ClockThrottling_Initialize(Clock_Throttling_Handle_t  *pContext, Clock_Throttling_Profile_t *pThrottling);
