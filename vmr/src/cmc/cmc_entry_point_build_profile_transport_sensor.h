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





#ifndef _CMC_ENTRY_POINT_BUILD_PROFILE_TRANSPORT_SENSOR_H_
#define _CMC_ENTRY_POINT_BUILD_PROFILE_TRANSPORT_SENSOR_H_





#ifdef __cplusplus
extern "C"
{
#endif

#include "cmc_common.h"

typedef     void        (*TRANSPORT_INITIALIZE                       )   (void *pUserContext );
typedef     void        (*TRANSPORT_FRAME_MESSAGE_TYPE               )   (void *pUserContext, uint8_t* pMessage, uint8_t* pMessageLength, uint8_t* pFramedMessage, uint8_t* pFramedMessageLength);
typedef     bool        (*TRANSPORT_DEFRAME_TRY_ADD_TYPE             )   (void *pUserContext, char AnyCharacter);
typedef     void        (*TRANSPORT_DEFRAME_RESET_TYPE               )   (void *pUserContext );
typedef     char *      (*TRANSPORT_DEFRAME_GET_MESSAGE_TYPE         )   (void *pUserContext );
typedef     void        (*TRANSPORT_SEND_BYTE_SEQUENCE_BLOCKING_TYPE )   (void *pUserContext, char * pData, uint32_t SequenceLength );


typedef struct CMC_BUILD_PROFILE_TRANSPORT_SENSOR_TYPE
{
	bool                                               IsAvailable;
    void *                                             pUserContext;
	TRANSPORT_INITIALIZE                               pFN_Initialize;
	TRANSPORT_FRAME_MESSAGE_TYPE                       pFN_FrameMessage;
	TRANSPORT_DEFRAME_TRY_ADD_TYPE                     pFN_DeframeTryAdd;
	TRANSPORT_DEFRAME_RESET_TYPE                       pFN_DeframeReset;
	TRANSPORT_DEFRAME_GET_MESSAGE_TYPE                 pFN_DeframeGetMessage;
	TRANSPORT_SEND_BYTE_SEQUENCE_BLOCKING_TYPE         pFN_SendByteSequenceBlocking;

}CMC_BUILD_PROFILE_TRANSPORT_SENSOR_TYPE;



#ifdef __cplusplus
}
#endif





#endif /* _CMC_ENTRY_POINT_BUILD_PROFILE_TRANSPORT_SENSOR_H_ */





