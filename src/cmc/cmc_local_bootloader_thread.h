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
 *  $Change: 3095780 $
 *  $Date: 2021/01/13 $
 *  $Revision: #30 $
 *
 */





#ifndef _CMC_LOCAL_BOOTLOADER_THREAD_H_
#define _CMC_LOCAL_BOOTLOADER_THREAD_H_


#ifdef __cplusplus
extern "C"
{
#endif



#include "cmc_common.h"
#include "cmc_watchpoint.h"
#include "cmc_circular_buffer.h"
#include "cmc_thread_timer.h"
#include "cmc_local_bootloader_thread.h"
#include "cmc_local_bootloader_thread_constants.h"
#include "cmc_fsm_state_transition_logger.h"
#include "cmc_local_bootloader_event.h"
#include "cmc_local_bootloader_action.h"
#include "cmc_local_bootloader_state.h"
#include "cmc_thread_communications_broker.h"
#include "cmc_local_sensor_supervisor_thread.h"
#include "cmc_host_user_proxy_thread.h"
#include "cmc_link_receive_thread.h"

#include "cmc_peripheral_uart_lite_usb.h"
#include "cmc_peripheral_regmap_ram_controller.h"
#include "cmc_user_supplied_environment.h"


#define MAX_BOOTLOADER_MSG		(266)
#define MAX_HOST_PAYLOAD_SIZE	(256)

typedef	enum HOST_MESSAGE_OPCODE_TYPE// host message opcodes
	{
		CMC_OP_MSP432_FW_SEC = 0x1,
		CMC_OP_MSP432_FW_DATA,
		CMC_OP_MSP432_JUMP,
		CMC_OP_BOARD_INFO_REQ,
		CMC_OP_MSP432_FW_ERASE,
		CMC_HOST_OP_MAX
	}HOST_MESSAGE_OPCODE_TYPE;


typedef struct LOCAL_BOOTLOADER_THREAD_CONTEXT_TYPE
{
    CIRCULAR_BUFFER_TYPE                                EventSourceCircularBuffer;
    CIRCULAR_BUFFER_ELEMENT_TYPE                        CircularBuffer[MAX_BOOTLOADER_RECEIVE_EVENTS];
    CMC_WATCHPOINT_CONTEXT_TYPE *                       pWatchPointContext;
    THREAD_TIMER_CONTEXT_TYPE *                         pThreadTimerContext;
    BROKER_CONTEXT_TYPE *								pBrokerContext;
    PERIPHERAL_REGMAP_RAM_CONTROLLER_CONTEXT_TYPE *     pPeripheralRegMapContext;
    PERIPHERAL_AXI_UART_LITE_SATELLITE_CONTEXT_TYPE *   pAXI_UART_LITE_Satellite_Context;
    FSM_STATE_TRANSITION_LOGGER_CONTEXT_TYPE *          pFSM_StateTransitionLoggerContext;
    uint32_t                                            CurrentResponseTimerInstance;
    uint32_t                                            CurrentDelayTimerInstance;
    uint32_t                                            RestartAttemptCount;
    uint8_t                                             Event;
    uint32_t                                            Action;
    LOCAL_BOOT_LOADER_STATE_TYPE                        State;
    LOCAL_BOOT_LOADER_STATE_TYPE                        PreviousState;
	char							                    TxMsgBuffer[MAX_BOOTLOADER_MSG];
	uint32_t						                    TxMsgBufferSize;
	char							                    PayloadFromHost[MAX_HOST_PAYLOAD_SIZE];
	uint32_t						                    PayloadFromHostSize;
	uint32_t											FirmwareDownloadRemaining;
	uint16_t											calculatedChecksum;
	char												checksumBody[2];
	uint32_t											DestinationAddress;
	uint32_t											Length;
	uint32_t											ChunkCount;
	uint32_t											IndexAddress;
	uint8_t *											pPayload;
	uint8_t												lastCommandSent;
	USER_SUPPLIED_ENVIRONMENT_CALCULATE_CRC16_CCITT_TYPE pFN_Calculate_CRC16_CCITT;
	void*												pUserContext;
	CMC_BUILD_PROFILE_PROTOCOL_BOOTLOADER_TYPE *        pProtocol;

} LOCAL_BOOTLOADER_THREAD_CONTEXT_TYPE;


bool LocalBootloaderThread_Schedule(void *pContext);

void LocalBootloaderThread_Initialize(  LOCAL_BOOTLOADER_THREAD_CONTEXT_TYPE * pContext,
                                        THREAD_TIMER_CONTEXT_TYPE * pThreadTimerContext,
                                        CMC_WATCHPOINT_CONTEXT_TYPE * pWatchPointContext,
                                        FSM_STATE_TRANSITION_LOGGER_CONTEXT_TYPE * pFSM_StateTransitionLoggerContext,
                                        PERIPHERAL_AXI_UART_LITE_SATELLITE_CONTEXT_TYPE * pAXI_UART_LITE_Satellite_Context,
                                        PERIPHERAL_REGMAP_RAM_CONTROLLER_CONTEXT_TYPE *     pPeripheralRegMapContext,
	                                    CMC_BUILD_PROFILE_TYPE* pProfile);


void LocalBootloaderThread_BindBroker(  LOCAL_BOOTLOADER_THREAD_CONTEXT_TYPE * pContext,
                                        BROKER_CONTEXT_TYPE *    pBrokerContext);



void LocalBootloader_ResponseTimerCallback(void * pCallbackContext, uint32_t AnyInstance);
void LocalBootloader_DelayTimerCallback(void * pCallbackContext, uint32_t AnyInstance);



void Bootloader_FSM(LOCAL_BOOTLOADER_THREAD_CONTEXT_TYPE *pContext, uint8_t AnyEvent);


void Bootloader_CLEAR_ACTION(LOCAL_BOOTLOADER_THREAD_CONTEXT_TYPE *pContext);
void Bootloader_ACTION(LOCAL_BOOTLOADER_THREAD_CONTEXT_TYPE *pContext, uint32_t AnyAction);
void Bootloader_HandleAction(LOCAL_BOOTLOADER_THREAD_CONTEXT_TYPE *pContext);


bool Bootloader_FSM_Helper_FramesRemaining(LOCAL_BOOTLOADER_THREAD_CONTEXT_TYPE *pContext);








void Bootloader_CreateEvent_E_LB_ACTIVATE_BOOTLOADER(LOCAL_BOOTLOADER_THREAD_CONTEXT_TYPE * pContext);
void Bootloader_CreateEvent_E_LB_RESET_BOOTLOADER(LOCAL_BOOTLOADER_THREAD_CONTEXT_TYPE * pContext);
void Bootloader_CreateEvent_E_LB_COMMAND_ERASE_FIRMWARE_SEQUENCE(LOCAL_BOOTLOADER_THREAD_CONTEXT_TYPE * pContext);
void Bootloader_CreateEvent_E_LB_COMMAND_RESTART_FIRMWARE(LOCAL_BOOTLOADER_THREAD_CONTEXT_TYPE * pContext);
void Bootloader_CreateEvent_E_LB_COMMAND_UPDATE_FIRMWARE(LOCAL_BOOTLOADER_THREAD_CONTEXT_TYPE * pContext);
void Bootloader_CreateEvent_E_LB_T_RESPONSE_TIMEOUT(LOCAL_BOOTLOADER_THREAD_CONTEXT_TYPE * pContext);
void Bootloader_CreateEvent_E_LB_T_RESTART_DELAY_TIMEOUT(LOCAL_BOOTLOADER_THREAD_CONTEXT_TYPE * pContext);
void Bootloader_CreateEvent_E_LB_REMOTE_ACK(LOCAL_BOOTLOADER_THREAD_CONTEXT_TYPE * pContext);
void Bootloader_CreateEvent_E_LB_REMOTE_ACK_FRAMES_REMAINING(LOCAL_BOOTLOADER_THREAD_CONTEXT_TYPE* pContext);
void Bootloader_CreateEvent_E_LB_REMOTE_ACK_NO_FRAMES_REMAINING(LOCAL_BOOTLOADER_THREAD_CONTEXT_TYPE* pContext);
void Bootloader_CreateEvent_E_LB_REMOTE_NAK(LOCAL_BOOTLOADER_THREAD_CONTEXT_TYPE * pContext);
void Bootloader_CreateEvent_E_LB_LINK_USER_IS_BOOTLOADER(LOCAL_BOOTLOADER_THREAD_CONTEXT_TYPE * pContext);
void Bootloader_CreateEvent_E_LB_LINK_USER_IS_SENSOR_SUPERVISOR(LOCAL_BOOTLOADER_THREAD_CONTEXT_TYPE * pContext);





#ifdef __cplusplus
}
#endif


#endif /* _CMC_LOCAL_BOOTLOADER_THREAD_H_ */






