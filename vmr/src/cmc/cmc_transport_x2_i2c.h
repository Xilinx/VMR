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
 *  $Revision: #4 $
 *
 */


#ifndef _CMC_TRANSPORT_X2_I2C_H_
#define _CMC_TRANSPORT_X2_I2C_H_


#ifdef __cplusplus
extern "C"
{
#endif

#include "cmc_common.h"
#include "cmc_entry_point_build_profile_transport_sensor.h"
#include "cmc_peripheral_axi_uart_lite_satellite.h"
#include "cmc_supervisor_message_deframer.h"
#include "cmc_user_supplied_environment.h"
#include "cmc_ram.h"
#include "cmc_link_receive_thread.h"
#include "cmc_local_sensor_supervisor_thread.h"

typedef struct TRANSPORT_X2_I2C_CONTEXT_TYPE
{
    SENSOR_SUPERVISOR_DEFRAMER_CONTEXT_TYPE*            pDeframerContext;
    PERIPHERAL_AXI_UART_LITE_SATELLITE_CONTEXT_TYPE*    pUartContext;
    uint8_t											    X2CommsBuffer[X2_COMMS_MAX_RX_BUFFER_ELEMENT];
    bool												bX2CommsPacketReceived;
} TRANSPORT_X2_I2C_CONTEXT_TYPE;

    void TRANSPORT_X2_I2C_Initialize(void* pUserContext);
    void TRANSPORT_X2_I2C_FrameMessage(void* pUserContext, uint8_t* pMessage, uint8_t* pMessageLength, uint8_t* pFramedMessage, uint8_t* pFramedMessageLength);
    bool TRANSPORT_X2_I2C_DeframeTryAdd(void* pUserContext, char AnyCharacter);
    void TRANSPORT_X2_I2C_DeframeReset(void* pUserContext);
    uint8_t* TRANSPORT_X2_I2C_DeframeGetMessage(void* pUserContext);
    void TRANSPORT_X2_I2C_SendByteSequenceBlocking(void* pUserContext, char* pData, uint32_t SequenceLength);


#ifdef __cplusplus
}
#endif


#endif /* _CMC_TRANSPORT_X2_I2C_H_ */


