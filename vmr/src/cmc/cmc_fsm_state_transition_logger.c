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
 *  $Revision: #7 $
 *
 */




#include "cmc_fsm_state_transition_logger.h"



 
void FSM_StateTransitionLogger_Initialize(FSM_STATE_TRANSITION_LOGGER_CONTEXT_TYPE *pContext, PERIPHERAL_AXI_UART_LITE_USB_CONTEXT_TYPE *pUART_LITE_USB_Context)
{
    pContext->pUART_LITE_USB_Context=pUART_LITE_USB_Context;
    pContext->MessageNumber=0;
}


void FSM_StateTransitionLogger_SendLogElementToLogOutputDevice(FSM_STATE_TRANSITION_LOGGER_CONTEXT_TYPE *pContext, FSM_STATE_TRANSITION_LOGGER_ELEMENT_TYPE *pLogElement)
{
    int32_t SequenceLength=FSM_STATE_TRANSITION_LOGGER_OUTPUT_MAX_DELIMITER_BYTES+FSM_STATE_TRANSITION_LOGGER_MESSAGE_SIZE;
    char Buffer[FSM_STATE_TRANSITION_LOGGER_OUTPUT_MAX_DELIMITER_BYTES+FSM_STATE_TRANSITION_LOGGER_MESSAGE_SIZE];

    Buffer[0]=FSM_STATE_TRANSITION_LOGGER_OUTPUT_START_DELIMITER_A;
    Buffer[1]=FSM_STATE_TRANSITION_LOGGER_OUTPUT_START_DELIMITER_B;

    Buffer[FSM_STATE_TRANSITION_LOGGER_OUTPUT_MAX_DELIMITER_BYTES+0]= (pContext->MessageNumber)++;
    Buffer[FSM_STATE_TRANSITION_LOGGER_OUTPUT_MAX_DELIMITER_BYTES+1]= pLogElement->FSM_Identifier;
    Buffer[FSM_STATE_TRANSITION_LOGGER_OUTPUT_MAX_DELIMITER_BYTES+2]= pLogElement->CurrentState;
    Buffer[FSM_STATE_TRANSITION_LOGGER_OUTPUT_MAX_DELIMITER_BYTES+3]= pLogElement->Event;
    Buffer[FSM_STATE_TRANSITION_LOGGER_OUTPUT_MAX_DELIMITER_BYTES+4]= pLogElement->NewState;

    PERIPHERAL_AXI_UART_LITE_USB_SendByteSequenceBlocking(pContext->pUART_LITE_USB_Context, Buffer, SequenceLength);
}


void FSM_StateTransitionLogger_TryAdd(FSM_STATE_TRANSITION_LOGGER_CONTEXT_TYPE *pContext, FSM_STATE_TRANSITION_LOGGER_ELEMENT_TYPE *pLogElement)
{
    FSM_StateTransitionLogger_SendLogElementToLogOutputDevice(pContext, pLogElement);
}


