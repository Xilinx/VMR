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
 *  $Revision: #25 $
 *
 */




#ifndef _CMC_CLOCK_THROTTLING_H_
#define _CMC_CLOCK_THROTTLING_H_


#ifdef __cplusplus
extern "C"
{
#endif

#include "cmc_common.h"
#include "cmc_watchpoint.h"

#include "cmc_peripherals.h"
#include "cmc_data_store.h"




#include "cmc_clock_throttling_mode.h"
#include "cmc_clock_throttling_phase1.h"

#define MAX_RAILS_MONITORED     (5)
#define ACTIVITY_MAX            (128)
#define CLOCK_THROTTLING_AVERAGE_SIZE (10)
#define MIN(a,b) (((a)<(b))?(a):(b))

#define MASK_CLOCKTHROTTLING_DISABLE_THROTTLING (0 << 20)  
#define MASK_CLOCKTHROTTLING_ENABLE_THROTTLING  (0 << 20)       // CR-1071357 Please update CMC Firmware to disable Clock Throttling mitigation
#define MASK_CLEAR_LATCHEDSHUTDOWNCLOCKS        (1 << 16)  

    typedef struct CMC_BUILD_PROFILE_CLOCK_THROTTLING_TYPE
    {
        CMC_CLOCK_THROTTLING_MODE_TYPE                      ProfileMode;
        CMC_CLOCK_THROTTLING_ALGORITHM_TYPE                 ProfileAlgorithm;
        void *                                              pPeripheralContext;
        uint8_t                                             NumberOfSensors;
        uint8_t                                             VoltageSensorID[5];
        uint8_t                                             CurrentSensorID[5];
        uint16_t                                            NominalVoltage[5];
        bool                                                bContributesToBoardPower[5];
        float                                               IdlePower; //in uW
        bool                                                bVCCIntThermalThrottling;

        float                                               TempGainKpFPGA;
        float                                               TempGainKi;
        float                                               TempGainKpVCCInt;
        float                                               TempGainKaw;

    } CMC_BUILD_PROFILE_CLOCK_THROTTLING_TYPE;





    typedef struct CMC_CLOCK_THROTTLING_RAIL_TYPE
    {
        uint32_t    Voltage;
        uint32_t    Current;
        uint32_t    throttlingThresholdCurrent;

    } CMC_CLOCK_THROTTLING_RAIL_TYPE;

    typedef struct CMC_CLOCK_THROTTLING_PER_RAIL_TYPE
    {
        uint8_t                                             VoltageSensorID;
        uint8_t                                             CurrentSensorID;
        CMC_CLOCK_THROTTLING_RAIL_TYPE                      PreviousReading;
        CMC_CLOCK_THROTTLING_RAIL_TYPE                      LatestReading;
        uint16_t                                            NominalVoltage;
        uint32_t                                            MeasuredPower;
        uint32_t                                            ThrottledThresholdPower;
        bool                                                ContributesToBoardThrottlingPower;
        float                                               RailNormalizedPower;
        uint32_t                                            DuplicateReadingCount;
    }CMC_CLOCK_THROTTLING_PER_RAIL_TYPE;

    typedef struct CMC_CLOCK_THROTTLING_CONTEXT_ALOGRITHM_STD_TYPE 
    {
        uint8_t                                             NumberOfRailsMonitored;
        CMC_CLOCK_THROTTLING_PER_RAIL_TYPE                  RailParameters[MAX_RAILS_MONITORED];
        uint32_t                                            BoardThrottlingThresholdPower;
        uint32_t                                            BoardMeasuredPower;
        float                                               UserNormalizedPower;
        float                                               BoardNormalizedPower;
        float                                               KernelPower;
        float                                               KernelTarget;
        float                                               IdlePower;                  // Where does this come from set to zero 
        uint32_t                                            Activity;                   //0 to 128
        float                                               RateCurrent;
        float                                               RateLinear;

        PERIPHERAL_AXI_GPIO_MUTEX_CMC_CONTEXT_TYPE* pPeripheralMutexContext;

        float                                               TempGainKpFPGA;
        float                                               TempGainKi;
        float                                               TempGainKpVCCInt;
        float                                               TempGainKaw;

        float                                               ThermalThrottlingThresholdPower;        //Output from Thermal stage
        float                                               LastThermalThrottlingThresholdPower;

        float                                               ThermalNormalisedPower;
        float                                               VccIntThermalNormalisedPower;

        float                                               FPGAMeasuredTemp;
        uint32_t                                            FPGAThrottlingTempLimit;

        bool                                                bVccIntThermalThrottlingEnabled;
        float                                               VccIntMeasuredTemp;
        uint32_t                                            VccIntThrottlingTempLimit;

        bool                                                ThermalThrottlingLoopJustEnabled;
        float                                               IntegtaionSum;

        uint32_t                                            FPGAMeasuredTempValues[CLOCK_THROTTLING_AVERAGE_SIZE];
        uint32_t                                            VCCIntMeasuredTempValues[CLOCK_THROTTLING_AVERAGE_SIZE];
        uint8_t                                             AverageArrayIndex;
    } CMC_CLOCK_THROTTLING_CONTEXT_ALOGRITHM_STD_TYPE;

    typedef struct CMC_CLOCK_THROTTLING_CONTEXT_ALOGRITHM_SSD_TYPE {
        uint32_t crit_temp_count;
        uint32_t p_integral;
        uint32_t power_band;
        uint32_t target_power;
        uint32_t target_power2;
        uint32_t temperature_band;
        uint32_t target_temperature;
        uint32_t target_temperature2;
        uint32_t board_power;
        uint32_t board_temperature;
        int integral_p_err;
        double ki_p;
        double kp_p;
        double kp_t;
        bool clk_scaled;
        bool clk_scaled_in_pwr;
        bool clk_scaled_in_temp;
    } CMC_CLOCK_THROTTLING_CONTEXT_ALOGRITHM_SSD_TYPE;


    typedef union {
        CMC_CLOCK_THROTTLING_CONTEXT_ALOGRITHM_STD_TYPE type1;
        CMC_CLOCK_THROTTLING_CONTEXT_ALOGRITHM_SSD_TYPE type2;
    } algorithm_type;

typedef struct CMC_CLOCK_THROTTLING_CONTEXT_TYPE
{
    PERIPHERAL_REGMAP_RAM_CONTROLLER_CONTEXT_TYPE *     pHardwareRegisterSetContext;
    CMC_CLOCK_THROTTLING_MODE_TYPE                      Mode;
    CMC_CLOCK_THROTTLING_ALGORITHM_TYPE                 Algorithm;
    CMC_WATCHPOINT_CONTEXT_TYPE *                       pWatchPointContext;
    PERIPHERAL_CLOCK_THROTTLING_CONTEXT_TYPE *          pClockThrottlingPeripheralContext;
    PERIPHERAL_CLOCK_MONITOR_CONTEXT_TYPE *             pClockMonitorPeripheralContext;
    bool                                                bFeatureSupportedBySCVersion;

    // From XRT
    bool                                                FeatureEnabled;
    bool                                                PowerOverRideEnabled;
    bool                                                bUserThrottlingTempLimitEnabled;
    uint32_t                                            XRTSuppliedBoardThrottlingThresholdPower;
    uint32_t                                            XRTSuppliedUserThrottlingTempLimit;
    algorithm_type                                      algorithm_type;
    

} CMC_CLOCK_THROTTLING_CONTEXT_TYPE;

void CMC_ClockThrottling_FeatureEnable(CMC_CLOCK_THROTTLING_CONTEXT_TYPE* pContext, bool bEnable);

void CMC_ClockThrottling_Initialize(CMC_CLOCK_THROTTLING_CONTEXT_TYPE *             pContext,
                                    CMC_BUILD_PROFILE_CLOCK_THROTTLING_TYPE*        pThrottling,
                                    CMC_WATCHPOINT_CONTEXT_TYPE *                   pWatchPointContext,
                                    PERIPHERAL_CLOCK_THROTTLING_CONTEXT_TYPE *      pClockThrottlingPeripheralContext,
                                    PERIPHERAL_CLOCK_MONITOR_CONTEXT_TYPE *          pClockMonitorPeripheralContext,
                                    PERIPHERAL_REGMAP_RAM_CONTROLLER_CONTEXT_TYPE * pHardwareRegisterSetContext,
                                    PERIPHERAL_AXI_GPIO_MUTEX_CMC_CONTEXT_TYPE *    pPeripheralMutexContext);


//void CMC_ClockThrottling_NewPowerSample(CMC_CLOCK_THROTTLING_CONTEXT_TYPE *pContext, CMC_CLOCK_THROTTLING_SAMPLE_LIST_TYPE * pSampleList);
//void CMC_ClockThrottling_NewTemperatureSample(CMC_CLOCK_THROTTLING_CONTEXT_TYPE *pContext, CMC_CLOCK_THROTTLING_SAMPLE_LIST_TYPE * pSampleList);

void CMC_ClockThrottling_Initialize_Watchpoint(CMC_CLOCK_THROTTLING_CONTEXT_TYPE* pContext);
void cmc_clock_throttling_phase1_initialise(CMC_CLOCK_THROTTLING_CONTEXT_TYPE* pContext);
void cmc_clock_throttling_initialise_all(CMC_CLOCK_THROTTLING_CONTEXT_TYPE* pContext);

void CMC_ClockThrottling_Initialize_Mode(CMC_CLOCK_THROTTLING_CONTEXT_TYPE* pContext, CMC_CLOCK_THROTTLING_MODE_TYPE Value);

void cmc_clock_throttling_phase1_algorithm(CMC_CLOCK_THROTTLING_CONTEXT_TYPE* pContext, bool ReadingsHaveChanged);
void cmc_clock_throttling_phase1_preprocessing(CMC_CLOCK_THROTTLING_CONTEXT_TYPE* pContext);
void cmc_clock_throttling_phase2_preprocessing(CMC_CLOCK_THROTTLING_CONTEXT_TYPE* pContext);

void CMC_ClockThrottling_Update_Watchpoint(CMC_CLOCK_THROTTLING_CONTEXT_TYPE* pContext);

#ifdef __cplusplus
}
#endif


#endif /* _CMC_CLOCK_THROTTLING_H_ */






