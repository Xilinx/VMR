/*----------------------------------------------------------------------------------------------*/
/*                                                                                              */
/* � Copyright 2020 Xilinx, Inc. All rights reserved.                                           */
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





#include "cmc_protocol_cmc_sc.h"
#include "cmc_supervisor_message_formatter.h"
#include "cmc_supervisor_message_parser.h"


void PROTOCOL_CMC_SC_FormatSETMessage(void *pUserContext, uint8_t* pREQMessage, uint8_t* pREQMessageLength, uint8_t msgID, uint8_t* pPayload, uint8_t payloadLength)
{
	UNUSED(pUserContext);
    bool CRCrequired;

    CRCrequired = true;
	cmcSensorSupervisorFormatSETMessage(pREQMessage, pREQMessageLength, msgID, pPayload, payloadLength, CRCrequired);
}



void PROTOCOL_CMC_SC_FormatREQMessage(void *pUserContext, uint8_t* pREQMessage, uint8_t* pREQMessageLength, uint8_t msgID)
{
	UNUSED(pUserContext);
    bool CRCrequired;

    CRCrequired = true;
	cmcSensorSupervisorFormatREQMessage(pREQMessage, pREQMessageLength, msgID, CRCrequired);
}


int PROTOCOL_CMC_SC_ProcessMessage(void *pUserContext, char* message, uint8_t *pPayloadLength, uint8_t *pMsgID)
{
    int Result=1;
    bool CRCrequired;

    PROTOCOL_CMC_SC_CONTEXT_TYPE *protocolContext = (PROTOCOL_CMC_SC_CONTEXT_TYPE *)pUserContext;
    CRCrequired = true;

    Result = cmc_sensor_supervisor_process_message(protocolContext->pParserContext, (uint8_t *)message, CRCrequired);

    *pPayloadLength = protocolContext->pParserContext->payloadLength;
    *pMsgID = protocolContext->pParserContext->msgIDReceived;

    return Result;
}


void PROTOCOL_CMC_SC_Initialize(void *pUserContext)
{
	UNUSED(pUserContext);
}


