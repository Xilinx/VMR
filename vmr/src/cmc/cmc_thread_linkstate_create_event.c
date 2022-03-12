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
 *  $Change: 3137936 $
 *  $Date: 2021/03/03 $
 *  $Revision: #16 $
 *
 */




#include "cmc_thread_linkstate.h"


void LinkState_CreateEvent(LINK_STATE_CONTEXT_TYPE * pContext, char AnyEvent)
{
        if(!CircularBuffer_TryWrite(&(pContext->EventSourceCircularBuffer), AnyEvent))
        {
            Watch_Inc(pContext->pWatchPointContext, W_LINK_STATE_FSM_FAILED_TO_ADD_EVENT);
        }
} 



void LinkState_CreateEvent_E_LS_RESET_REQUEST(LINK_STATE_CONTEXT_TYPE * pContext)
{
    LinkState_CreateEvent(pContext, E_LS_RESET_REQUEST);
} 

                          
void LinkState_CreateEvent_E_LS_T_RESPONSE_EXPIRY(LINK_STATE_CONTEXT_TYPE * pContext)
{
    LinkState_CreateEvent(pContext, E_LS_T_RESPONSE_EXPIRY);
} 

void LinkState_CreateEvent_E_LS_T_BSL_EXPIRY(LINK_STATE_CONTEXT_TYPE* pContext)
{
	LinkState_CreateEvent(pContext, E_LS_T_BSL_EXPIRY);
}

                                                  
void LinkState_CreateEvent_E_LS_LINK_USER_IS_REMOTE_SENSOR_SUPERVISOR(LINK_STATE_CONTEXT_TYPE * pContext)
{
    LinkState_CreateEvent(pContext, E_LS_LINK_USER_IS_REMOTE_SENSOR_SUPERVISOR);
} 

                             
void LinkState_CreateEvent_E_LS_LINK_USER_IS_REMOTE_BOOTLOADER(LINK_STATE_CONTEXT_TYPE * pContext)
{
    LinkState_CreateEvent(pContext, E_LS_LINK_USER_IS_REMOTE_BOOTLOADER);
} 

                                    
void LinkState_CreateEvent_E_LS_LINK_USER_TO_BOOTLOADER_REQUEST(LINK_STATE_CONTEXT_TYPE * pContext)
{
    LinkState_CreateEvent(pContext, E_LS_LINK_USER_TO_BOOTLOADER_REQUEST);
} 

                                   
void LinkState_CreateEvent_E_LS_LINK_OFFERED_VERSION_ACKNOWLEDGE(LINK_STATE_CONTEXT_TYPE * pContext)
{
    LinkState_CreateEvent(pContext, E_LS_LINK_OFFERED_VERSION_ACKNOWLEDGE);
} 

void LinkState_CreateEvent_E_LS_LINK_OFFERED_VERSION_DECLINED(LINK_STATE_CONTEXT_TYPE* pContext)
{
    LinkState_CreateEvent(pContext, E_LS_LINK_OFFERED_VERSION_DECLINED);
}
                                  
void LinkState_CreateEvent_E_LS_LINK_ACCEPTED_PROTOCOL_VERSION_RANGE_RESPONSE(LINK_STATE_CONTEXT_TYPE * pContext)
{
    LinkState_CreateEvent(pContext, E_LS_LINK_ACCEPTED_PROTOCOL_VERSION_RANGE_RESPONSE);
} 



void LinkState_CreateEvent_E_LS_LINK_ACCEPTED_PROTOCOL_VERSION_RANGE_RESPONSE_FAILED(LINK_STATE_CONTEXT_TYPE* pContext)
{
    LinkState_CreateEvent(pContext, E_LS_LINK_ACCEPTED_PROTOCOL_VERSION_RANGE_RESPONSE_FAILED);
}

void LinkState_CreateEvent_E_LS_ENABLE_BSL_RESPONSE(LINK_STATE_CONTEXT_TYPE* pContext)
{
	LinkState_CreateEvent(pContext, E_LS_ENABLE_BSL_RESPONSE);
}

void LinkState_CreateEvent_E_LS_ENABLE_BSL_RESPONSE_FAILED(LINK_STATE_CONTEXT_TYPE* pContext)
{
    LinkState_CreateEvent(pContext, E_LS_ENABLE_BSL_RESPONSE_FAILED);
}

void LinkState_CreateEvent_E_LS_LINK_I2C_ACKNOWLEDGE(LINK_STATE_CONTEXT_TYPE* pContext)
{
    LinkState_CreateEvent(pContext, E_LS_LINK_I2C_ACKNOWLEDGE);
}

void LinkState_CreateEvent_E_LS_LINK_I2C_FAILED(LINK_STATE_CONTEXT_TYPE* pContext)
{
    LinkState_CreateEvent(pContext, E_LS_LINK_I2C_FAILED);
}