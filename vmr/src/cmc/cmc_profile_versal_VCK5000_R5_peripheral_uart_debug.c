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
 *  $Revision: #7 $
 *
 */





#include <stdio.h>
#include <sys/types.h>

#include "cmc_profile_versal_VCK5000_R5.h"


int CMC_BuildProfile_Versal_VCK5000_R5_Peripheral_uart_debug_Context;


void CMC_BuildProfile_Versal_VCK5000_R5_Peripheral_uart_debug_UART_SERVICES_SEND_BYTE_NON_BLOCKING(void *pUserContext, CMC_PERIPHERAL_BASE_ADDRESS_TYPE BaseAddress, char Data)
{
	UNUSED(pUserContext);
	UNUSED(BaseAddress);

	printf("%02d ", Data);
}




char  CMC_BuildProfile_Versal_VCK5000_R5_Peripheral_uart_debug_UART_SERVICES_RECV_BYTE_BLOCKING(void *pUserContext, CMC_PERIPHERAL_BASE_ADDRESS_TYPE BaseAddress)
{
	UNUSED(pUserContext);
	UNUSED(BaseAddress);
	return '\0';
}


bool CMC_BuildProfile_Versal_VCK5000_R5_Peripheral_uart_debug_UART_SERVICES_IS_TRANSMIT_FULL(void *pUserContext, CMC_PERIPHERAL_BASE_ADDRESS_TYPE BaseAddress)
{
	UNUSED(pUserContext);
	UNUSED(BaseAddress);
	return false;
}


bool CMC_BuildProfile_Versal_VCK5000_R5_Peripheral_uart_debug_UART_SERVICES_IS_RECEIVE_EMPTY(void *pUserContext, CMC_PERIPHERAL_BASE_ADDRESS_TYPE BaseAddress)
{
	UNUSED(pUserContext);
	UNUSED(BaseAddress);
	return true;
}


void CMC_BuildProfile_Versal_VCK5000_R5_Peripheral_uart_debug_UART_SERVICES_INITIALIZE(void *pUserContext, CMC_PERIPHERAL_BASE_ADDRESS_TYPE BaseAddress)
{
	UNUSED(pUserContext);
	UNUSED(BaseAddress);
}



void CMC_BuildProfile_Versal_VCK5000_R5_Peripheral_uart_debug(CMC_BUILD_PROFILE_TYPE * pProfile)
{
	pProfile->Peripherals.UART[UART_CATEGORY_DEBUG].pUserContext = &CMC_BuildProfile_Versal_VCK5000_R5_Peripheral_uart_debug_Context;
	pProfile->Peripherals.UART[UART_CATEGORY_DEBUG].pFN_Initialize = CMC_BuildProfile_Versal_VCK5000_R5_Peripheral_uart_debug_UART_SERVICES_INITIALIZE;
	pProfile->Peripherals.UART[UART_CATEGORY_DEBUG].pFN_IsReceiveEmpty = CMC_BuildProfile_Versal_VCK5000_R5_Peripheral_uart_debug_UART_SERVICES_IS_RECEIVE_EMPTY;
	pProfile->Peripherals.UART[UART_CATEGORY_DEBUG].pFN_IsTransmitFull = CMC_BuildProfile_Versal_VCK5000_R5_Peripheral_uart_debug_UART_SERVICES_IS_TRANSMIT_FULL;
	pProfile->Peripherals.UART[UART_CATEGORY_DEBUG].pFN_RecvByteBlocking = CMC_BuildProfile_Versal_VCK5000_R5_Peripheral_uart_debug_UART_SERVICES_RECV_BYTE_BLOCKING;
	pProfile->Peripherals.UART[UART_CATEGORY_DEBUG].pFN_SendByteNonBlocking = CMC_BuildProfile_Versal_VCK5000_R5_Peripheral_uart_debug_UART_SERVICES_SEND_BYTE_NON_BLOCKING;
}

