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
 *  $Change: 3106443 $
 *  $Date: 2021/01/26 $
 *  $Revision: #26 $
 *
 */





#ifndef _CMC_LINK_RECEIVE_THREAD_H_
#define _CMC_LINK_RECEIVE_THREAD_H_


#ifdef __cplusplus
extern "C"
{
#endif



#include "cmc_common.h"
#include "cmc_watchpoint.h"
#include "cmc_circular_buffer.h"
#include "cmc_link_receive_thread_constants.h"
#include "cmc_bootloader_message_parser.h"
#include "cmc_sensor_relay_buffer.h"
#include "cmc_user_supplied_environment.h"
#include "cmc_link_protocol_trace.h"
#include "cmc_entry_point_build_profile.h"
#include "cmc_thread_communications_broker.h"
#include "cmc_local_sensor_supervisor_thread.h"
#include "cmc_local_bootloader_thread.h"
#include "cmc_thread_linkstate.h"
#include "cmc_host_user_proxy_thread.h"

//#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define DEBUG_BUFFER_SIZE       (0x300)

struct LINK_STATE_CONTEXT_TYPE;
struct LOCAL_SENSOR_SUPERVISOR_THREAD_CONTEXT_TYPE;

typedef struct LINK_RECEIVE_THREAD_CONTEXT_TYPE
{
	BROKER_CONTEXT_TYPE *					        pBrokerContext;
	BOOTLOADER_PARSER_CONTEXT_TYPE *				        BootloaderParserContext;
	LINK_PROTOCOL_TRACE_CONTEXT_TYPE*						LinkProtocolTraceContext;
    CIRCULAR_BUFFER_TYPE							        EventSourceCircularBuffer;
    CIRCULAR_BUFFER_ELEMENT_TYPE					        CircularBuffer[MAX_RECEIVE_OCTETS];
    struct LINK_STATE_CONTEXT_TYPE *				        pLinkStateContext;
	struct LOCAL_SENSOR_SUPERVISOR_THREAD_CONTEXT_TYPE *	pLocalSensorSupervisorContext;
    CMC_WATCHPOINT_CONTEXT_TYPE *					        pWatchPointContext;
    CMC_SENSOR_RELAY_BUFFER_CONTEXT_TYPE *                  pSensorSupervisorRelayBufferContext;
    CMC_SENSOR_RELAY_BUFFER_CONTEXT_TYPE*                   pClockThrottlingRelayBufferContext;
    USER_SUPPLIED_ENVIRONMENT_CALCULATE_CRC16_CCITT_TYPE    pFN_Calculate_CRC16_CCITT;
	bool													bPacketCaptureEnabled;
	uint8_t													PacketCaptureMsgID;
    uint8_t                                                 DebugBuffer[DEBUG_BUFFER_SIZE];
    uint8_t                                                 StartupDebugBuffer[DEBUG_BUFFER_SIZE];
    uint16_t                                                DebugBufferPosition;
	uint16_t                                                StartupLogBufferPosition;
	CMC_BUILD_PROFILE_PROTOCOL_SENSOR_TYPE *				pProtocol;
	CMC_BUILD_PROFILE_PROTOCOL_BOOTLOADER_TYPE *            pProtocolBootloader;
	CMC_BUILD_PROFILE_TRANSPORT_SENSOR_TYPE *				pTransport;
	uint8_t													msgIDReceived;
	uint8_t													payloadLength;


} LINK_RECEIVE_THREAD_CONTEXT_TYPE;



void LinkReceiveThread_Push(void * pContext, char AnyOctet);


bool LinkReceiveThread_Schedule(void *pContext);

void LinkReceiveThread_Initialize(BOOTLOADER_PARSER_CONTEXT_TYPE *				        pBootloaderParserContext,
								  LINK_RECEIVE_THREAD_CONTEXT_TYPE *				        pContext,
								  struct LINK_STATE_CONTEXT_TYPE *				        pLinkStateContext,
  								  struct LOCAL_SENSOR_SUPERVISOR_THREAD_CONTEXT_TYPE *	pLocalSensorSupervisorContext,
								  CMC_WATCHPOINT_CONTEXT_TYPE *					        pWatchPointContext,
								  CMC_SENSOR_RELAY_BUFFER_CONTEXT_TYPE *                  pSensorSupervisorRelayBufferContext,
								  CMC_SENSOR_RELAY_BUFFER_CONTEXT_TYPE *                  pClockThrottlingRelayBufferContext,
								  LINK_PROTOCOL_TRACE_CONTEXT_TYPE *						pLinkProtocolTraceContext,
								  CMC_BUILD_PROFILE_TYPE *								pProfile);

void LinkReceiveThread_BindBroker(	LINK_RECEIVE_THREAD_CONTEXT_TYPE* pContext,
									BROKER_CONTEXT_TYPE* pBrokerContext);

void LinkReceiveThread_Packet_Capture_Enable(LINK_RECEIVE_THREAD_CONTEXT_TYPE* pContext);
void LinkReceiveThread_Packet_Capture_Disable(LINK_RECEIVE_THREAD_CONTEXT_TYPE* pContext);

void LinkReceiveThread_Store_Debug_Capture_MsgID(LINK_RECEIVE_THREAD_CONTEXT_TYPE* pContext, uint8_t MsgID);

#ifdef __cplusplus
}
#endif


#endif /* _CMC_LINK_RECEIVE_THREAD_H_ */





