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


/* PROFILE_LINUX_VERSAL_VCK5000_R5 */ 
/* LINUX_DIST */





/*
 *
 *  RCS Keyword Metadata
 *
 *  $Change: 3095780 $
 *  $Date: 2021/01/13 $
 *  $Revision: #9 $
 *
 */






#include "cmc_profile_versal_VCK5000_R5.h"


void CMC_BuildProfile_Versal_VCK5000_R5_clock_throttling(CMC_BUILD_PROFILE_TYPE * pProfile)
{
    pProfile->ClockThrottling.ProfileMode = CT_MODE_FEATURE_DISABLED;
    pProfile->ClockThrottling.ProfileAlgorithm = CT_ALGORITHM_STANDARD;
    pProfile->ClockThrottling.NumberOfSensors = 3;
    pProfile->ClockThrottling.VoltageSensorID[0] = SNSR_ID_12V_PEX;
    pProfile->ClockThrottling.VoltageSensorID[1] = SNSR_ID_3V3_PEX;
    pProfile->ClockThrottling.VoltageSensorID[2] = SNSR_ID_VCCINT;
    pProfile->ClockThrottling.CurrentSensorID[0] = SNSR_ID_12VPEX_I_IN;
    pProfile->ClockThrottling.CurrentSensorID[1] = SNSR_ID_3V3PEX_I_IN;
    pProfile->ClockThrottling.CurrentSensorID[2] = SNSR_ID_VCCINT_I;
    pProfile->ClockThrottling.NominalVoltage[0] = 12000;
    pProfile->ClockThrottling.NominalVoltage[1] = 3300;
    pProfile->ClockThrottling.NominalVoltage[2] = 12000;
    pProfile->ClockThrottling.bContributesToBoardPower[0] = true;
    pProfile->ClockThrottling.bContributesToBoardPower[1] = true;
    pProfile->ClockThrottling.bContributesToBoardPower[2] = false;
    pProfile->ClockThrottling.IdlePower = 9625000;

    pProfile->ClockThrottling.bVCCIntThermalThrottling = true;
    pProfile->ClockThrottling.TempGainKpFPGA = 3.000e+06;    //0x4a371b00;
    pProfile->ClockThrottling.TempGainKi = 1.000e+04;    //0x461c4000;
    pProfile->ClockThrottling.TempGainKpVCCInt = 1.000e+07;    //0x4b189680;
    pProfile->ClockThrottling.TempGainKaw = 2.400e-03;    //0x3b1d4951;;
}

