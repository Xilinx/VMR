/*----------------------------------------------------------------------------------------------*/
/*                                                                                              */
/* � Copyright 2019 Xilinx, Inc. All rights reserved.                                           */
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
 *  $Change: 3109135 $
 *  $Date: 2021/01/28 $
 *  $Revision: #12 $
 *
 */





#include "cmc_peripheral_axi_gpio_qsfp.h"


void PERIPHERAL_AXI_GPIO_QSFP_Read_GPIO_for_QSFP(PERIPHERAL_AXI_GPIO_QSFP_CONTEXT_TYPE *pContext, uint32_t iQSFP, uint8_t * pRegisterValue)
{
    *pRegisterValue=0;
    uint32_t RegisterOffset=(CMC_PERIPHERAL_AXI_GPIO_QSFP_XGPIO_DATA_OFFSET+(iQSFP*CMC_PERIPHERAL_AXI_GPIO_QSFP_XGPIO_DATA2_OFFSET));

    if(pContext->IsAvailable)
    {
        if((pContext->MaxCages)>iQSFP)
        {
            *pRegisterValue=(uint8_t)Peripheral_Read(pContext->pRequiredEnvironmentContext, pContext->BaseAddress, RegisterOffset);
        }
    }
}

void PERIPHERAL_AXI_GPIO_QSFP_Write_GPIO_for_QSFP(PERIPHERAL_AXI_GPIO_QSFP_CONTEXT_TYPE* pContext, uint32_t iQSFP, uint8_t RegisterValue)
{
    uint32_t RegisterOffset = (CMC_PERIPHERAL_AXI_GPIO_QSFP_XGPIO_DATA_OFFSET + (iQSFP * CMC_PERIPHERAL_AXI_GPIO_QSFP_XGPIO_DATA2_OFFSET));

    if (pContext->IsAvailable)
    {
        if ((pContext->MaxCages) > iQSFP)
        {
            Peripheral_Write(pContext->pRequiredEnvironmentContext, pContext->BaseAddress, RegisterOffset, (uint32_t)RegisterValue);
        }
    }

}

void PERIPHERAL_AXI_GPIO_QSFP_Initialize(PERIPHERAL_AXI_GPIO_QSFP_CONTEXT_TYPE *pContext, CMC_PERIPHERAL_BASE_ADDRESS_TYPE AnyBaseAddress, bool IsAvailable, CMC_REQUIRED_ENVIRONMENT_CONTEXT_TYPE * pRequiredEnvironmentContext, uint8_t NumberOfQSFPCages)
{
    pContext->BaseAddress=AnyBaseAddress;
    pContext->IsAvailable=IsAvailable;
    pContext->MaxCages= NumberOfQSFPCages;
    pContext->pRequiredEnvironmentContext=pRequiredEnvironmentContext;
}

