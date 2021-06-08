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
 *  $Change: 3097075 $
 *  $Date: 2021/01/14 $
 *  $Revision: #4 $
 *
 */




#include "cmc_peripheral_clock_monitor.h"
#include "cmc_peripheral_clock_monitor_defs.h"

uint16_t clk_bits[clk_bits_max] = { 0 };
uint32_t pattern[MAX_PATTERN_REGS] = { 0 };
uint32_t CLOCK_SCALING_PATTERN_REG[MAX_PATTERN_REGS] = {
    CLOCK_SCALING_PATTERN_REG_0,
    CLOCK_SCALING_PATTERN_REG_1,
    CLOCK_SCALING_PATTERN_REG_2,
    CLOCK_SCALING_PATTERN_REG_3,
    CLOCK_SCALING_PATTERN_REG_4,
    CLOCK_SCALING_PATTERN_REG_5,
    CLOCK_SCALING_PATTERN_REG_6,
    CLOCK_SCALING_PATTERN_REG_7 };

void PERIPHERAL_Clock_Monitor_Initialize(PERIPHERAL_CLOCK_MONITOR_CONTEXT_TYPE* pContext, CMC_PERIPHERAL_BASE_ADDRESS_TYPE AnyBaseAddress, bool IsAvailable, CMC_REQUIRED_ENVIRONMENT_CONTEXT_TYPE* pRequiredEnvironmentContext, PERIPHERAL_REGMAP_RAM_CONTROLLER_CONTEXT_TYPE* pRegmapRamController)
{
    pContext->BaseAddress = AnyBaseAddress;
    pContext->IsAvailable = IsAvailable;
    pContext->pRequiredEnvironmentContext = pRequiredEnvironmentContext;
    pContext->pRegmapRamController = pRegmapRamController;
}


uint32_t PERIPHERAL_Clock_Monitor_Read_Reg(PERIPHERAL_CLOCK_MONITOR_CONTEXT_TYPE* pContext, uint32_t RegisterOffset)
{
    uint32_t reg_val;
    //uint32_t offset;

    reg_val = 0;
    //offset = (uint32_t) pContext->BaseAddress + RegisterOffset;
    //reg_val = PERIPHERAL_REGMAP_RAM_CONTROLLER_Read(pContext->pRegmapRamController, offset);
    reg_val = Peripheral_Read(pContext->pRequiredEnvironmentContext, pContext->BaseAddress, RegisterOffset);

    return reg_val;
}

void PERIPHERAL_Clock_Monitor_Write_Reg(PERIPHERAL_CLOCK_MONITOR_CONTEXT_TYPE* pContext, uint32_t RegisterOffset, uint32_t Value)
{
    //uint32_t offset;

    //offset = (uint32_t) pContext->BaseAddress + RegisterOffset;
    //PERIPHERAL_REGMAP_RAM_CONTROLLER_Write(pContext->pRegmapRamController, offset, Value);
    Peripheral_Write(pContext->pRequiredEnvironmentContext, pContext->BaseAddress, RegisterOffset, Value);
}


void PERIPHERAL_Clock_Monitor_Distribute_Main(uint32_t one, uint32_t zero)
{
    int i, j, clk_i;

    if (zero == 0)
    {
        for (i = 0; i < TOTAL_CLOCK_BITS; i++)
            clk_bits[i] = 1;

        clk_bits[zeros_count] = 0;
        clk_bits[ones_count] = 256;
    }
    else
    {
        double freq_one, freq_zero, p_one, p_zero;

        clk_bits[zeros_count] = 0;
        clk_bits[ones_count] = 0;
        freq_one = 256.0 / one;
        freq_zero = 256.0 / zero;
        p_one = freq_one;
        p_zero = freq_zero;

        for (clk_i = 0; clk_i < TOTAL_CLOCK_BITS; clk_i++)
        {
            if (p_one < p_zero)
            {
                clk_bits[clk_i] = 1;
                clk_bits[ones_count]++;
                p_one += freq_one;
            }
            else
            {
                clk_bits[clk_i] = 0;
                clk_bits[zeros_count]++;
                p_zero += freq_zero;
            }
        }
    }

    for (i = 0; i < MAX_PATTERN_REGS; i++)
    {
        for (j = 0; j < 32; j++)
            pattern[i] = (pattern[i] << 1) | clk_bits[i * 32 + j];
    }
}


void PERIPHERAL_Clock_Monitor_Load_All_Pattern(PERIPHERAL_CLOCK_MONITOR_CONTEXT_TYPE* pContext, bool reset)
{
    uint32_t ops;
    uint32_t i = 0;

    if (reset)
    {
        for (i = 0; i < MAX_PATTERN_REGS; i++)
            pattern[i] = PATTERN_32_CLOCK;
        clk_bits[zeros_count] = 0;
        clk_bits[ones_count] = TOTAL_CLOCK_BITS;
    }

    for (i = 0; i < MAX_PATTERN_REGS; i++)
        PERIPHERAL_Clock_Monitor_Write_Reg(pContext, CLOCK_SCALING_PATTERN_REG[i], pattern[i]);

    //Write the pattern register offset 0x53100 address and load bit as 1 in the control register
    ops = CLOCK_SCALING_PATTERN_REG[0] << CLOCK_SCALING_CTRL_REG_PATTERN_OFFSET;
    ops |= CLOCK_SCALING_CTRL_REG_LOAD;
    PERIPHERAL_Clock_Monitor_Write_Reg(pContext, CLOCK_SCALING_CTRL_REG, ops);
    while (!(PERIPHERAL_Clock_Monitor_Read_Reg(pContext, CLOCK_SCALING_CTRL_REG) & CLOCK_SCALING_CTRL_REG_DONE))
        ;
    PERIPHERAL_Clock_Monitor_Write_Reg(pContext, CLOCK_SCALING_CTRL_REG, CLOCK_SCALING_CTRL_REG_DONE);
}


void PERIPHERAL_Clock_Monitor_Update_Targets(PERIPHERAL_CLOCK_MONITOR_CONTEXT_TYPE* pContext, uint8_t power, uint8_t temp)
{
    uint32_t target_power, target_temperature, threshold, val;

    threshold = PERIPHERAL_Clock_Monitor_Read_Reg(pContext, CLOCK_SCALING_THRESHOLD_REG);
    target_power = (threshold >> CLOCK_SCALING_POWER_THRESHOLD_BIT_POS) &
        CLOCK_SCALING_POWER_THRESHOLD_MASK;
    target_temperature = (threshold >> CLOCK_SCALING_TEMP_THRESHOLD_BIT_POS) &
        CLOCK_SCALING_TEMP_THRESHOLD_MASK;

    //Update target power value
    if ((power > 0) && (power <= target_power))
        target_power = power;

    val = PERIPHERAL_Clock_Monitor_Read_Reg(pContext, CLOCK_SCALING_POWER_REG);
    val &= ~CLOCK_SCALING_POWER_TARGET_MASK;
    val |= (target_power & CLOCK_SCALING_POWER_TARGET_MASK);
    PERIPHERAL_Clock_Monitor_Write_Reg(pContext, CLOCK_SCALING_POWER_REG, val);

    //Update target temperature value
    if ((temp > 0) && (temp <= target_temperature))
        target_temperature = temp;

    val = PERIPHERAL_Clock_Monitor_Read_Reg(pContext, CLOCK_SCALING_TEMPERATURE_REG);
    val &= ~CLOCK_SCALING_TEMPERATURE_TARGET_MASK;
    val |= (target_temperature & CLOCK_SCALING_TEMPERATURE_TARGET_MASK);
    PERIPHERAL_Clock_Monitor_Write_Reg(pContext, CLOCK_SCALING_TEMPERATURE_REG, val);
}


void PERIPHERAL_Clock_Monitor_Update_Freq_Multipliers(PERIPHERAL_CLOCK_MONITOR_CONTEXT_TYPE* pContext, uint8_t min, uint8_t max)
{
    uint32_t val = 0;
    uint32_t max_ones = max;

    if (min == 0)
        min = CLOCK_FREQ_MULTIPLIER_MIN_VAL;
    if ((max == 0) || (max == 255))
        max_ones = CLOCK_FREQ_MULTIPLIER_MAX_VAL;

    val |= (max_ones << CLOCK_FREQ_MULTIPLIER_MAX_BIT_POS);
    val |= (min << CLOCK_FREQ_MULTIPLIER_MIN_BIT_POS);
    PERIPHERAL_Clock_Monitor_Write_Reg(pContext, CLOCK_FREQ_MULTIPLIER_REGS, val);
}

