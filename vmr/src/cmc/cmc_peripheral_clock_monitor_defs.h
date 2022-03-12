/*----------------------------------------------------------------------------------------------*/
/*                                                                                              */
/* © Copyright 2019-2020 Xilinx, Inc. All rights reserved.                                           */
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
 *  $Change: 3097075 $
 *  $Date: 2021/01/14 $
 *  $Revision: #2 $
 *
 */

#ifndef _CMC_PERIPHERAL_CLOCK_MONITOR_DEFS_H_
#define _CMC_PERIPHERAL_CLOCK_MONITOR_DEFS_H_

#define PATTERN_1_CLOCK   0x80000000
#define PATTERN_32_CLOCK  0xFFFFFFFF

#define TOTAL_CLOCK_BITS  256
#define MAX_PATTERN_REGS  (TOTAL_CLOCK_BITS / 32)

#define CLOCK_SCALING_PATTERN_REG_0 0x100
#define CLOCK_SCALING_PATTERN_REG_1 0x104
#define CLOCK_SCALING_PATTERN_REG_2 0x108
#define CLOCK_SCALING_PATTERN_REG_3 0x10C
#define CLOCK_SCALING_PATTERN_REG_4 0x110
#define CLOCK_SCALING_PATTERN_REG_5 0x114
#define CLOCK_SCALING_PATTERN_REG_6 0x118
#define CLOCK_SCALING_PATTERN_REG_7 0x11C

#define CLOCK_SCALING_BASEADDR  0x53000

#define CLOCK_SCALING_CTRL_REG    0x0
#define CLOCK_SCALING_CTRL_REG_LOAD   0x1
#define CLOCK_SCALING_CTRL_REG_DONE   0x2
#define CLOCK_SCALING_CTRL_REG_PATTERN_OFFSET 16

#define STORE_TEMPERATURE_VALUE_REG   0x00C

#define PI_ALGO_SELECT_REG    0x010
#define PI_ALGO_SELECT_POWER  0x0
#define PI_ALGO_SELECT_TEMPERATURE  0x1
#define PI_ALGO_SELECT_BOTH_POWER_TEMP  0x2

#define CLOCK_SCALING_TEMPERATURE_REG   0x014
#define CLOCK_SCALING_TEMPERATURE_TARGET_MASK  0xFF
#define CLOCK_SCALING_TEMPERATURE_BAND_MASK   0x000F0000
#define CLOCK_SCALING_TEMPERATURE_BAND_POS    16

#define CLOCK_SCALING_POWER_REG   0x018
#define CLOCK_SCALING_POWER_TARGET_MASK    0xFF
#define CLOCK_SCALING_POWER_BAND_MASK 0x000F0000
#define CLOCK_SCALING_POWER_BAND_POS  16
#define CLOCK_SCALING_POWER_I_CONST_MASK 0x0FF00000
#define CLOCK_SCALING_POWER_I_CONST_POS 20
#define CLOCK_SCALING_POWER_INTEGRAL_MASK 0x10000000
#define CLOCK_SCALING_POWER_INTEGRAL_POS 28

#define STORE_POWER_VALUE_REG   0x01C

#define PROPORTIONAL_GAIN_REG   0x020
#define CLOCK_SCALING_POWER_P_MASK 0xFF
#define CLOCK_SCALING_TEMPERATURE_P_CONST_MASK 0XFF00
#define CLOCK_SCALING_TEMPERATURE_P_CONST_POS 8

//This register will be set to 0x1 by XRT when it identifies clock scaling ip
//present on platform. XRT uses featureROM bit for this.
//CMC firmware will read this register and checks if clock scaling feature enabled.
#define CLOCK_SCALING_FEATURE_EN  0x024

#define FAN_SPEED   0x028

//This register is read-only register, values will be loaded by hw ip.
#define CLOCK_SCALING_THRESHOLD_REG    0x02C
#define CLOCK_SCALING_POWER_THRESHOLD_MASK    0xFF
#define CLOCK_SCALING_POWER_THRESHOLD_BIT_POS    8
#define CLOCK_SCALING_TEMP_THRESHOLD_MASK    0xFF
#define CLOCK_SCALING_TEMP_THRESHOLD_BIT_POS    0

#define STORE_ONES_ZEROES_COUNT_REG        0x030
#define STORE_ONES_ZEROES_COUNT_MASK       0xFFFF
#define STORE_ONES_ZEROES_COUNT_ONES_POS   0
#define STORE_ONES_ZEROES_COUNT_ZEROES_POS 16

#define CLOCK_FREQ_MULTIPLIER_REGS           0x034
#define CLOCK_FREQ_MULTIPLIER_MASK           0xFFFF
#define CLOCK_FREQ_MULTIPLIER_MIN_BIT_POS    0
#define CLOCK_FREQ_MULTIPLIER_MAX_BIT_POS    16
#define CLOCK_FREQ_MULTIPLIER_MIN_VAL        1
#define CLOCK_FREQ_MULTIPLIER_MAX_VAL        256

#define CLOCK_SCALING_CRIT_THRESHOLD_REG    0x03C
#define CLOCK_SCALING_CRIT_THRESHOLD_MASK    0xFF
#define MAX_CRIT_TEMP_COUNT (26)

#define CLOCK_SCALING_SOFT_RESET_REG	0x040
#define CLOCK_SCALING_SOFT_RESET_VAL	0x1

#define clk_bits_max	(TOTAL_CLOCK_BITS + 2)
#define zeros_count	(clk_bits_max - 2)
#define ones_count	(clk_bits_max - 1)

extern uint16_t clk_bits[clk_bits_max];
extern uint32_t pattern[MAX_PATTERN_REGS];
extern uint32_t CLOCK_SCALING_PATTERN_REG[MAX_PATTERN_REGS];

#endif
