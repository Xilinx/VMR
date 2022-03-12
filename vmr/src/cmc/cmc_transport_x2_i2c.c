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
 *  $Revision: #6 $
 *
 */

#include "cmc_transport_x2_i2c.h"

/* handle to the watchpoint context */
extern CMC_WATCHPOINT_CONTEXT_TYPE                      WatchpointContext;
extern LOCAL_SENSOR_SUPERVISOR_THREAD_CONTEXT_TYPE LocalSensorSupervisorThreadContext;

 /***************************************************************/
 /* Local Functions                                             */
 /***************************************************************/

void LinkReceiveThread_X2CommsCopyReceivedMsg(void* pUserContext, uint8_t* msg, uint8_t msgLength)
{
    TRANSPORT_X2_I2C_CONTEXT_TYPE* pContext = (TRANSPORT_X2_I2C_CONTEXT_TYPE*)pUserContext;

    if (pContext->bX2CommsPacketReceived == false)
    {
        cmcMemCopy((char*)pContext->X2CommsBuffer, (char*)msg, (size_t)msgLength);
        pContext->bX2CommsPacketReceived = true;
        LinkReceiveThread_Push(&LinkReceiveThreadContext, (char)msg[0]);
    }
}



void LinkReceiveThread_X2ResetHandler(LINK_RECEIVE_THREAD_CONTEXT_TYPE* pContext)
{
    /* When the X2 is reset, it may be as a result of a new firmware version being downloaded */
    /* This means we need to re-request the board info.										  */
    /* We will do this by setting the "IsValid"	flag to false.  This will cause the sensor	  */
    /* supervisor to re-request the info													  */

    pContext->pLocalSensorSupervisorContext->DataStoreContext.BoardInfo.IsValid = false;
}


/***************************************************************/
/* Global Functions                                            */
/***************************************************************/

void TRANSPORT_X2_I2C_Initialize(void* pUserContext)
{
    CMC_X2_SetResponseCallback(&X2CommsContext, (CMC_X2_RESPONSE_PUSH_CALLBACK_TYPE)LinkReceiveThread_X2CommsCopyReceivedMsg, pUserContext);
    CMC_X2_SetResetCallback(&X2CommsContext, (CMC_X2_RESET_CALLBACK_TYPE)LinkReceiveThread_X2ResetHandler, pUserContext);
}


bool TRANSPORT_X2_I2C_DeframeTryAdd(void* pUserContext, char AnyCharacter)
{
    UNUSED(AnyCharacter);
    bool Result = false;

    TRANSPORT_X2_I2C_CONTEXT_TYPE* pContext = (TRANSPORT_X2_I2C_CONTEXT_TYPE*)pUserContext;

    if (pContext->bX2CommsPacketReceived == true)
    {
        Result = true;
        pContext->bX2CommsPacketReceived = false;
    }

    return Result;
}


void TRANSPORT_X2_I2C_FrameMessage(void* pUserContext, uint8_t* pMessage, uint8_t* pMessageLength, uint8_t* pFramedMessage, uint8_t* pFramedMessageLength)
{
    UNUSED(pUserContext);
   
    cmcMemCopy((char*)pFramedMessage, (char*)pMessage, *pMessageLength);
    *pFramedMessageLength = *pMessageLength;
}


void TRANSPORT_X2_I2C_DeframeReset(void* pUserContext)
{
    UNUSED(pUserContext);
}


uint8_t* TRANSPORT_X2_I2C_DeframeGetMessage(void* pUserContext)
{
    TRANSPORT_X2_I2C_CONTEXT_TYPE* pContext = (TRANSPORT_X2_I2C_CONTEXT_TYPE*)pUserContext;

    return((uint8_t * )pContext->X2CommsBuffer);
}


void TRANSPORT_X2_I2C_SendByteSequenceBlocking(void* pUserContext, char* pData, uint32_t SequenceLength)
{
    UNUSED(pUserContext);
    CMC_X2_NewUserRequest(&X2CommsContext, (uint8_t*)pData, SequenceLength);
}
