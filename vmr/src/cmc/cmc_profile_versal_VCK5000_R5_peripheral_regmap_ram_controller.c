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
 *  $Change: 3105474 $
 *  $Date: 2021/01/25 $
 *  $Revision: #12 $
 *
 */

#include <stdio.h>
#include "cmc_profile_versal_VCK5000_R5.h"
#include "cmc_peripheral_regmap_ram_controller.h"
#include "xil_printf.h"
#include "xil_io.h"
#include "xil_cache.h"
#define RAM_BUFFER_LENGTH	(1024)

typedef struct CMC_BuildProfile_Versal_VCK5000_R5_Peripheral_RegMap_RAM_Controller_UserContext_s
{
	CMC_PERIPHERAL_BASE_ADDRESS_TYPE BaseAddress;
} CMC_BuildProfile_Versal_VCK5000_R5_Peripheral_RegMap_RAM_Controller_UserContext_t;

CMC_BuildProfile_Versal_VCK5000_R5_Peripheral_RegMap_RAM_Controller_UserContext_t CMC_BuildProfile_Versal_VCK5000_R5_Peripheral_RegMap_RAM_Controller_UserContext;

void CMC_BuildProfile_Versal_VCK5000_R5_Peripheral_RegMap_RAM_Controller_SERVICES_INITIALIZE(void *pUserContext, CMC_PERIPHERAL_BASE_ADDRESS_TYPE BaseAddress)
{
	CMC_BuildProfile_Versal_VCK5000_R5_Peripheral_RegMap_RAM_Controller_UserContext_t * pContext=
	                                    (CMC_BuildProfile_Versal_VCK5000_R5_Peripheral_RegMap_RAM_Controller_UserContext_t *)pUserContext;

	pContext->BaseAddress=BaseAddress;

}


uint32_t CMC_BuildProfile_Versal_VCK5000_R5_Peripheral_RegMap_RAM_Controller_SERVICES_READ(void *pUserContext, uint32_t RegisterOffset)
{
	CMC_BuildProfile_Versal_VCK5000_R5_Peripheral_RegMap_RAM_Controller_UserContext_t *pContext = (CMC_BuildProfile_Versal_VCK5000_R5_Peripheral_RegMap_RAM_Controller_UserContext_t *)pUserContext;
    uint32_t  Address = (uint32_t)(pContext->BaseAddress + RegisterOffset/4);

    return IO_SYNC_READ32(Address);
}


void CMC_BuildProfile_Versal_VCK5000_R5_Peripheral_RegMap_RAM_Controller_SERVICES_WRITE(void *pUserContext, uint32_t RegisterOffset, uint32_t Value)
{
	CMC_BuildProfile_Versal_VCK5000_R5_Peripheral_RegMap_RAM_Controller_UserContext_t *pContext = (CMC_BuildProfile_Versal_VCK5000_R5_Peripheral_RegMap_RAM_Controller_UserContext_t *)pUserContext;
	uint32_t  Address = (uint32_t)(pContext->BaseAddress + RegisterOffset/4);

	Xil_Out32(Address,Value);
	Xil_DCacheFlushRange(Address, sizeof(u32));
	//IO_SYNC_WRITE32(Address,Value);
}


void CMC_BuildProfile_Versal_VCK5000_R5_Peripheral_RegMap_RAM_Controller_SERVICES_WRITE_WITH_MASK(void *pUserContext, uint32_t RegisterOffset, uint32_t Value, uint32_t Mask)
{
	CMC_BuildProfile_Versal_VCK5000_R5_Peripheral_RegMap_RAM_Controller_UserContext_t *pContext = (CMC_BuildProfile_Versal_VCK5000_R5_Peripheral_RegMap_RAM_Controller_UserContext_t *)pUserContext;
    volatile uint32_t  Address = (uint32_t)(pContext->BaseAddress + RegisterOffset/4);
    uint32_t current = IO_SYNC_READ32(Address);
    Xil_Out32(Address, (Value & Mask) | (current & ~Mask));
    //IO_SYNC_WRITE32(Address, (Value & Mask) | (current & ~Mask));
}

uint32_t* CMC_BuildProfile_Versal_VCK5000_R5_Peripheral_RegMap_RAM_Controller_SERVICES_GetRegisterAddress(void* pUserContext, uint32_t RegisterOffset)
{
    UNUSED(pUserContext);
    uint32_t RegisterWordOffset = RegisterOffset / 4;
    volatile uint32_t* pAddress = (volatile uint32_t*)& CMC_BuildProfile_Versal_VCK5000_R5_Peripheral_RegMap_RAM_Controller_UserContext.BaseAddress[RegisterWordOffset];

    Xil_DCacheFlushRange((INTPTR)pAddress, RAM_BUFFER_LENGTH * sizeof(u32));
    return (uint32_t*)pAddress;
}





void CMC_BuildProfile_Versal_VCK5000_R5_Peripheral_RegMap_RAM_Controller(CMC_BUILD_PROFILE_TYPE * pProfile)
{
    pProfile->Peripherals.RAM_Controller.pUserContext=&CMC_BuildProfile_Versal_VCK5000_R5_Peripheral_RegMap_RAM_Controller_UserContext;
    pProfile->Peripherals.RAM_Controller.pFN_Initialize=CMC_BuildProfile_Versal_VCK5000_R5_Peripheral_RegMap_RAM_Controller_SERVICES_INITIALIZE;
    pProfile->Peripherals.RAM_Controller.pFN_Read=CMC_BuildProfile_Versal_VCK5000_R5_Peripheral_RegMap_RAM_Controller_SERVICES_READ;
    pProfile->Peripherals.RAM_Controller.pFN_Write=CMC_BuildProfile_Versal_VCK5000_R5_Peripheral_RegMap_RAM_Controller_SERVICES_WRITE;
    pProfile->Peripherals.RAM_Controller.pFN_WriteWithMask=CMC_BuildProfile_Versal_VCK5000_R5_Peripheral_RegMap_RAM_Controller_SERVICES_WRITE_WITH_MASK;
    pProfile->Peripherals.RAM_Controller.pFN_GetRegisterAddress = CMC_BuildProfile_Versal_VCK5000_R5_Peripheral_RegMap_RAM_Controller_SERVICES_GetRegisterAddress;
}



