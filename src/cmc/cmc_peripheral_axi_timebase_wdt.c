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
 *  $Change: 2766318 $
 *  $Date: 2020/01/30 $
 *  $Revision: #13 $
 *
 */




#include "cmc_peripheral_axi_timebase_wdt.h"


void PERIPHERAL_AXI_TIMEBASE_WDT_WatchdogService(PERIPHERAL_AXI_TIMEBASE_WDT_CONTEXT_TYPE *pContext)
{
    uint32_t RegisterValue;

    if(pContext->IsAvailable)
    {
        RegisterValue=Peripheral_Read(pContext->pRequiredEnvironmentContext, pContext->BaseAddress, PERIPHERAL_AXI_TIMEBASE_WDT_XWT_TWCSR0_OFFSET);

        // store reset status bit from watchdog timer in cmc register map
        if (RegisterValue & PERIPHERAL_AXI_TIMEBASE_WDT_XWT_CSR0_WRS_MASK)
        {
            PERIPHERAL_REGMAP_RAM_CONTROLLER_WriteWithMask(pContext->pPeripheralRegMapRamControllerContext, HOST_REGISTER_STATUS, HOST_REGISTER_PATTERN_WDT_RESET_STATUS, HOST_REGISTER_PATTERN_WDT_RESET_STATUS);
        }
        else
        {
            PERIPHERAL_REGMAP_RAM_CONTROLLER_WriteWithMask(pContext->pPeripheralRegMapRamControllerContext, HOST_REGISTER_STATUS, 0, HOST_REGISTER_PATTERN_WDT_RESET_STATUS);
        }

        RegisterValue|=(PERIPHERAL_AXI_TIMEBASE_WDT_XWT_CSR0_WRS_MASK | PERIPHERAL_AXI_TIMEBASE_WDT_XWT_CSR0_WDS_MASK);

        Peripheral_Write(pContext->pRequiredEnvironmentContext, pContext->BaseAddress, PERIPHERAL_AXI_TIMEBASE_WDT_XWT_TWCSR0_OFFSET, RegisterValue);
    }
}




void PERIPHERAL_AXI_TIMEBASE_WDT_Initialize(PERIPHERAL_AXI_TIMEBASE_WDT_CONTEXT_TYPE *pContext, CMC_PERIPHERAL_BASE_ADDRESS_TYPE AnyBaseAddress, bool IsAvailable, CMC_REQUIRED_ENVIRONMENT_CONTEXT_TYPE       * pRequiredEnvironmentContext, PERIPHERAL_REGMAP_RAM_CONTROLLER_CONTEXT_TYPE *  pPeripheralRegMapRamControllerContext)
{
    pContext->BaseAddress=AnyBaseAddress;
    pContext->IsAvailable=IsAvailable;
    pContext->pRequiredEnvironmentContext=pRequiredEnvironmentContext;
    pContext->pPeripheralRegMapRamControllerContext = pPeripheralRegMapRamControllerContext;
}

