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
 *  $Change: 2714436 $
 *  $Date: 2019/11/14 $
 *  $Revision: #4 $
 *
 */





#include "cmc_sensor_relay_buffer.h"



void SensorRelay_Initialize(CMC_SENSOR_RELAY_BUFFER_CONTEXT_TYPE *pContext, CIRCULAR_BUFFER_ELEMENT_TYPE * pCircularBuffer, uint32_t MaxElements)
{
    CircularBuffer_Initialize(&(pContext->CircularBufferContext), pCircularBuffer, MaxElements);
}




bool SensorRelay_TryAdd(CMC_SENSOR_RELAY_BUFFER_CONTEXT_TYPE *pContext, CMC_SENSOR_RELAY_ELEMENT_TYPE *pLogElement)
{
    bool Result=false;
    uint32_t i;

    if(CircularBuffer_SpaceIsAvailable(&(pContext->CircularBufferContext), CMC_SENSOR_RELAY_MAX_ELEMENT_BUFFER_SIZE))
    {
        Result=true;
        for(i=0;i<CMC_SENSOR_RELAY_MAX_ELEMENT_BUFFER_SIZE; i++)
        {
            (void)CircularBuffer_TryWrite(&(pContext->CircularBufferContext), pLogElement->Buffer[i]);
        }
    }
    else
    {
        CircularBuffer_Initialize(&(pContext->CircularBufferContext), pContext->CircularBufferContext.pCircularBuffer, pContext->CircularBufferContext.MaxElements);
    }

    return Result;
}




bool SensorRelay_TryRemove(  CMC_SENSOR_RELAY_BUFFER_CONTEXT_TYPE *pContext, CMC_SENSOR_RELAY_ELEMENT_TYPE *pLogElement)
{
    bool Result=CircularBuffer_Verify_AtLeast_Count_Unread(&(pContext->CircularBufferContext), CMC_SENSOR_RELAY_MAX_ELEMENT_BUFFER_SIZE);
    uint32_t i;

    if(Result)
    {
            for(i=0;i<CMC_SENSOR_RELAY_MAX_ELEMENT_BUFFER_SIZE; i++)
            {
                (void)CircularBuffer_TryRead(&(pContext->CircularBufferContext), (char *)&(pLogElement->Buffer[i]));
            }
    }

    return Result;
}

