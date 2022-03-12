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
 *  $Change: 2680455 $
 *  $Date: 2019/10/01 $
 *  $Revision: #4 $
 *
 */





#include "cmc_link_protocol_trace.h"



uint32_t CircularBufferLite_Inc(CIRCULAR_BUFFER_LITE_TYPE* pContext, uint32_t AnyIndex)
{
	uint32_t Result = AnyIndex;

	Result++;

	if ((pContext->MaxElements) <= Result)
	{
		Result = 0;
	}

	return Result;
}



void LinkProtocolTrace_TryAdd(LINK_PROTOCOL_TRACE_CONTEXT_TYPE* pContext, LINK_PROTOCOL_TRACE_ELEMENT_TYPE* pLogElement)
{
	// Check if we have been given RAM to use
	if (pContext->pHardwareRegisterSetContext)
	{
		PERIPHERAL_REGMAP_RAM_CONTROLLER_Write((PERIPHERAL_REGMAP_RAM_CONTROLLER_CONTEXT_TYPE*)pContext->pHardwareRegisterSetContext, CMC_SAT_TX_MSG_TRACE_REG + (pContext->CircularBufferLiteContext.iWrite * 4), 
												(pContext->IncrementingCounter++ << 24 | pLogElement->MsgID << 16 | pLogElement->FSMPrevState << 8 | pLogElement->FSMCurrentState));

		pContext->CircularBufferLiteContext.iWrite = CircularBufferLite_Inc(&pContext->CircularBufferLiteContext, pContext->CircularBufferLiteContext.iWrite);
	}
}

void CircularBufferLite_Initialize(CIRCULAR_BUFFER_LITE_TYPE* pContext)
{

	pContext->MaxElements = CMC_LINK_PROTOCOL_TRACE_MAX_LOGS;
	pContext->iWrite = 0;

}


void LinkProtocolTrace_Initialize(LINK_PROTOCOL_TRACE_CONTEXT_TYPE* pContext)
{
	pContext->IncrementingCounter = 0;
	CircularBufferLite_Initialize(&(pContext->CircularBufferLiteContext));
}

void LinkProtocolTrace_Update_RAM(LINK_PROTOCOL_TRACE_CONTEXT_TYPE* pContext, struct PERIPHERAL_REGMAP_RAM_CONTROLLER_CONTEXT_TYPE* pHardwareRegisterSetContext)
{
	uint32_t i;
	pContext->pHardwareRegisterSetContext = (PERIPHERAL_REGMAP_RAM_CONTROLLER_CONTEXT_TYPE*)pHardwareRegisterSetContext;


	for (i = 0; i < (pContext->CircularBufferLiteContext.MaxElements); i++)
	{
		PERIPHERAL_REGMAP_RAM_CONTROLLER_Write((PERIPHERAL_REGMAP_RAM_CONTROLLER_CONTEXT_TYPE*)pContext->pHardwareRegisterSetContext, CMC_SAT_TX_MSG_TRACE_REG + (i * 4), 0);
	}
}