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
 *  $Revision: #26 $
 *
 */






#include "cmc_local_bootloader_thread.h"

static void BootloaderLogUnexpected(LOCAL_BOOTLOADER_THREAD_CONTEXT_TYPE* pContext, uint8_t AnyEvent)
{
    Watch_Set(pContext->pWatchPointContext, W_BOOTLOADER_UNEXPECTED_EVENT, pContext->State << 24 | AnyEvent);
}

void Bootloader_NextStateDecoder(LOCAL_BOOTLOADER_THREAD_CONTEXT_TYPE *pContext, LOCAL_BOOT_LOADER_STATE_TYPE NewState)
{
    FSM_STATE_TRANSITION_LOGGER_ELEMENT_TYPE LogElement;
    if(NewState!=pContext->State)
    {
        LogElement.CurrentState=(uint8_t)pContext->State;
        LogElement.Event=(uint8_t)pContext->Event;
        LogElement.NewState=(uint8_t)NewState;
        LogElement.FSM_Identifier=FSM_ID_LOCAL_BOOTLOADER;
        FSM_StateTransitionLogger_TryAdd(pContext->pFSM_StateTransitionLoggerContext, &LogElement);
    }

    pContext->PreviousState=pContext->State;
    pContext->State=NewState;

    Watch_Set(pContext->pWatchPointContext, W_BOOTLOADER_CURRENT, (uint32_t)pContext->State);
    Watch_Set(pContext->pWatchPointContext, W_BOOTLOADER_PREVIOUS, (uint32_t)pContext->PreviousState);
}

static void Bootloader_ActionActionStateSequence(LOCAL_BOOTLOADER_THREAD_CONTEXT_TYPE* pContext, uint32_t AnyAction, uint32_t AnyAction2, LOCAL_BOOT_LOADER_STATE_TYPE NewState)
{
	Bootloader_ACTION(pContext, AnyAction);
	Bootloader_ACTION(pContext, AnyAction2);
	Bootloader_NextStateDecoder(pContext, NewState);
}

static void Bootloader_FailSequence(LOCAL_BOOTLOADER_THREAD_CONTEXT_TYPE* pContext)
{
	Bootloader_ActionActionStateSequence(pContext, A_LB_T_RESPONSE_STOP, A_LB_ANNOUNCE_COMMAND_FAILED, S_LB_BOOTLOAD_SEQUENCE_FAIL);
}

static void Bootloader_StopDoneSequence(LOCAL_BOOTLOADER_THREAD_CONTEXT_TYPE* pContext, LOCAL_BOOT_LOADER_STATE_TYPE NewState)
{
	Bootloader_ActionActionStateSequence(pContext, A_LB_T_RESPONSE_STOP, A_LB_ANNOUNCE_COMMAND_DONE, NewState);
}

void Bootloader_FSM_S_LB_INITIAL(LOCAL_BOOTLOADER_THREAD_CONTEXT_TYPE *pContext, uint8_t AnyEvent)
{
    switch(AnyEvent)
    {
		case	E_LB_LINK_USER_IS_BOOTLOADER:
        case    E_LB_ACTIVATE_BOOTLOADER:
                Bootloader_NextStateDecoder(pContext, S_LB_AWAITING_COMMAND);
                break;

        case    E_LB_LINK_USER_IS_SENSOR_SUPERVISOR:
                Bootloader_NextStateDecoder(pContext, S_LB_INITIAL);
                break;

        default:
                BootloaderLogUnexpected(pContext, AnyEvent);
                break;
    }
}


void Bootloader_FSM_S_LB_AWAITING_COMMAND(LOCAL_BOOTLOADER_THREAD_CONTEXT_TYPE *pContext, uint8_t AnyEvent)
{
    switch(AnyEvent)
    {
        case    E_LB_RESET_BOOTLOADER:
                Bootloader_NextStateDecoder(pContext, S_LB_INITIAL);
                break;

        case    E_LB_COMMAND_ERASE_FIRMWARE_SEQUENCE:
				Bootloader_ActionActionStateSequence(pContext, A_LB_T_RESPONSE_START, A_LB_SEND_PASSWORD, S_LB_AWAITING_PASSWORD_RESPONSE);
                break;

        case    E_LB_COMMAND_RESTART_FIRMWARE:
                pContext->RestartAttemptCount=0;
				Bootloader_ActionActionStateSequence(pContext, A_LB_T_RESTART_DELAY_START, A_LB_SEND_RESTART_FIRMWARE, S_LB_WAITING_FOR_FIRMWARE_RESTART);
                break;
       
        case    E_LB_COMMAND_UPDATE_FIRMWARE:
				Bootloader_ActionActionStateSequence(pContext, A_LB_T_RESPONSE_START, A_LB_SEND_FIRMWARE_SEGMENT, S_LB_AWAITING_FRAME_RESPONSE);
                break;

        default:
                BootloaderLogUnexpected(pContext, AnyEvent);
                break;
    }
}  


void Bootloader_FSM_S_LB_AWAITING_PASSWORD_RESPONSE(LOCAL_BOOTLOADER_THREAD_CONTEXT_TYPE *pContext, uint8_t AnyEvent)
{
    switch(AnyEvent)
    {
		case    E_LB_RESET_BOOTLOADER:
				Bootloader_NextStateDecoder(pContext, S_LB_INITIAL);
				break;

        case    E_LB_REMOTE_ACK:
                Bootloader_ActionActionStateSequence(pContext, A_LB_T_RESPONSE_RESTART, A_LB_SEND_ERASE_FIRMWARE, S_LB_AWAITING_ERASE_RESPONSE);
                break;

        case    E_LB_REMOTE_NAK:
        case    E_LB_T_RESPONSE_TIMEOUT:
				Bootloader_FailSequence(pContext);
                break;
                
        default:
                BootloaderLogUnexpected(pContext, AnyEvent);
                break;
    }
}



void Bootloader_FSM_S_LB_AWAITING_ERASE_RESPONSE(LOCAL_BOOTLOADER_THREAD_CONTEXT_TYPE *pContext, uint8_t AnyEvent)
{
    switch(AnyEvent)
    {
        case    E_LB_RESET_BOOTLOADER:
                Bootloader_NextStateDecoder(pContext, S_LB_INITIAL);
                break;

        case    E_LB_REMOTE_ACK:
				Bootloader_StopDoneSequence(pContext, S_LB_AWAITING_COMMAND);
                break;

        case    E_LB_REMOTE_NAK:
        case    E_LB_T_RESPONSE_TIMEOUT:
				Bootloader_FailSequence(pContext);
                break;


        default:
                BootloaderLogUnexpected(pContext, AnyEvent);
                break;
    }
}



void Bootloader_FSM_S_LB_AWAITING_FRAME_RESPONSE(LOCAL_BOOTLOADER_THREAD_CONTEXT_TYPE *pContext, uint8_t AnyEvent)
{
    switch(AnyEvent)
    {
        case    E_LB_RESET_BOOTLOADER:
                Bootloader_NextStateDecoder(pContext, S_LB_INITIAL);
                break;

        case    E_LB_REMOTE_ACK:
				Bootloader_ActionActionStateSequence(pContext, A_LB_T_RESPONSE_RESTART, A_LB_SEND_CHECKSUM, S_LB_AWAITING_CHECKSUM_RESPONSE);
                break;

        case    E_LB_REMOTE_NAK:
        case    E_LB_T_RESPONSE_TIMEOUT:
				Bootloader_FailSequence(pContext);
                break;

        default:
                BootloaderLogUnexpected(pContext, AnyEvent);
                break;
    }
}



void Bootloader_FSM_S_LB_AWAITING_CHECKSUM_RESPONSE(LOCAL_BOOTLOADER_THREAD_CONTEXT_TYPE *pContext, uint8_t AnyEvent)
{
    switch(AnyEvent)
    {
        case    E_LB_RESET_BOOTLOADER:
                Bootloader_NextStateDecoder(pContext, S_LB_INITIAL);
                break;

        case    E_LB_REMOTE_ACK:
				Bootloader_ActionActionStateSequence(pContext, A_LB_T_RESPONSE_RESTART, A_LB_SEND_ACK_FINAL_DECISION, S_LB_AWAITING_FINAL_FRAME_DECISION);
				break;

        case    E_LB_REMOTE_NAK:
        case    E_LB_T_RESPONSE_TIMEOUT:
				Bootloader_FailSequence(pContext);
                break;


        default:
                BootloaderLogUnexpected(pContext, AnyEvent);
                break;
    }
}

void Bootloader_FSM_S_LB_AWAITING_FINAL_FRAME_DECISION(LOCAL_BOOTLOADER_THREAD_CONTEXT_TYPE* pContext, uint8_t AnyEvent)
{
	switch (AnyEvent)
	{
	case    E_LB_RESET_BOOTLOADER:
			Bootloader_NextStateDecoder(pContext, S_LB_INITIAL);
			break;

	case    E_LB_REMOTE_ACK_FRAMES_REMAINING:
			Bootloader_ActionActionStateSequence(pContext, A_LB_T_RESPONSE_RESTART, A_LB_SEND_FIRMWARE_SEGMENT, S_LB_AWAITING_FRAME_RESPONSE);
			break;

	case    E_LB_REMOTE_ACK_NO_FRAMES_REMAINING:
			Bootloader_StopDoneSequence(pContext, S_LB_AWAITING_COMMAND);
			Broker_Announce_UserProxyRemoteBootloaderOperationSuccess(pContext->pBrokerContext);
			break;


	default:
            BootloaderLogUnexpected(pContext, AnyEvent);
			break;
	}
}


void Bootloader_FSM_S_LB_WAITING_FOR_FIRMWARE_RESTART(LOCAL_BOOTLOADER_THREAD_CONTEXT_TYPE *pContext, uint8_t AnyEvent)
{
    switch(AnyEvent)
    {
        case    E_LB_RESET_BOOTLOADER:
                Bootloader_ACTION(pContext, A_LB_T_RESTART_DELAY_STOP);
                Bootloader_NextStateDecoder(pContext, S_LB_INITIAL);
                break;

        case    E_LB_T_RESTART_DELAY_TIMEOUT:
			    Bootloader_ActionActionStateSequence(pContext, A_LB_REQUEST_LINK_USER, A_LB_T_RESPONSE_START, S_LB_WAITING_FOR_LINK_USER_CONFIRMATION);
                break;

        default:
                BootloaderLogUnexpected(pContext, AnyEvent);
                break;
    }
}


void Bootloader_FSM_S_LB_WAITING_FOR_LINK_USER_CONFIRMATION(LOCAL_BOOTLOADER_THREAD_CONTEXT_TYPE *pContext, uint8_t AnyEvent)
{
    switch(AnyEvent)
    {
        case    E_LB_RESET_BOOTLOADER:
                Bootloader_ACTION(pContext, A_LB_T_RESPONSE_STOP);
                Bootloader_NextStateDecoder(pContext, S_LB_INITIAL);
                break;


        case    E_LB_T_RESPONSE_TIMEOUT:
                (pContext->RestartAttemptCount)++;
                if(MAX_BOOTLOADER_FIRMWARE_RESTART_ATTEMPTS<(pContext->RestartAttemptCount))
                {
                    Bootloader_ACTION(pContext, A_LB_ANNOUNCE_COMMAND_FAILED);
                    Bootloader_NextStateDecoder(pContext, S_LB_BOOTLOAD_SEQUENCE_FAIL);
                }
                else
                {
					Bootloader_ActionActionStateSequence(pContext, A_LB_T_RESTART_DELAY_START, A_LB_SEND_RESTART_FIRMWARE, S_LB_WAITING_FOR_FIRMWARE_RESTART);
                }
                break;

        case    E_LB_LINK_USER_IS_BOOTLOADER:
                Bootloader_ACTION(pContext, A_LB_T_RESPONSE_STOP);
                (pContext->RestartAttemptCount)++;
                if(MAX_BOOTLOADER_FIRMWARE_RESTART_ATTEMPTS<(pContext->RestartAttemptCount))
                {
                    Bootloader_ACTION(pContext, A_LB_ANNOUNCE_COMMAND_FAILED);
                    Bootloader_NextStateDecoder(pContext, S_LB_BOOTLOAD_SEQUENCE_FAIL);
                }
                else
                {
					Bootloader_ActionActionStateSequence(pContext, A_LB_T_RESTART_DELAY_START, A_LB_SEND_RESTART_FIRMWARE, S_LB_WAITING_FOR_FIRMWARE_RESTART);
                }
                break;
       
        case    E_LB_LINK_USER_IS_SENSOR_SUPERVISOR:
				Bootloader_StopDoneSequence(pContext, S_LB_INITIAL);
                break;


        default:
                BootloaderLogUnexpected(pContext, AnyEvent);
                break;
    }
}


void Bootloader_FSM_S_LB_BOOTLOAD_SEQUENCE_FAIL(LOCAL_BOOTLOADER_THREAD_CONTEXT_TYPE *pContext, uint8_t AnyEvent)
{
    switch(AnyEvent)
    {
        case    E_LB_RESET_BOOTLOADER:
                Bootloader_NextStateDecoder(pContext, S_LB_INITIAL);
                break;

        default:
                BootloaderLogUnexpected(pContext, AnyEvent);
                break;
    }
}




void Bootloader_FSM(LOCAL_BOOTLOADER_THREAD_CONTEXT_TYPE *pContext, uint8_t AnyEvent)
{
    Bootloader_CLEAR_ACTION(pContext);

    pContext->Event=AnyEvent;
    Watch_Set(pContext->pWatchPointContext, W_BOOTLOADER_LAST_EVENT, (uint32_t)pContext->Event);

    switch(pContext->State)
    {
        case    S_LB_INITIAL:
                Bootloader_FSM_S_LB_INITIAL(pContext, AnyEvent);
                break;

        case    S_LB_AWAITING_COMMAND:
                Bootloader_FSM_S_LB_AWAITING_COMMAND(pContext, AnyEvent);
                break;

        case    S_LB_AWAITING_PASSWORD_RESPONSE:
                Bootloader_FSM_S_LB_AWAITING_PASSWORD_RESPONSE(pContext, AnyEvent);
                break;

        case    S_LB_AWAITING_ERASE_RESPONSE:
                Bootloader_FSM_S_LB_AWAITING_ERASE_RESPONSE(pContext, AnyEvent);
                break;

        case    S_LB_AWAITING_FRAME_RESPONSE:
                Bootloader_FSM_S_LB_AWAITING_FRAME_RESPONSE(pContext, AnyEvent);
                break;

        case    S_LB_AWAITING_CHECKSUM_RESPONSE:
                Bootloader_FSM_S_LB_AWAITING_CHECKSUM_RESPONSE(pContext, AnyEvent);
                break;

        case    S_LB_WAITING_FOR_FIRMWARE_RESTART:
                Bootloader_FSM_S_LB_WAITING_FOR_FIRMWARE_RESTART(pContext, AnyEvent);
                break;

        case    S_LB_WAITING_FOR_LINK_USER_CONFIRMATION:
                Bootloader_FSM_S_LB_WAITING_FOR_LINK_USER_CONFIRMATION(pContext, AnyEvent);
                break;

        case    S_LB_BOOTLOAD_SEQUENCE_FAIL:
                Bootloader_FSM_S_LB_BOOTLOAD_SEQUENCE_FAIL(pContext, AnyEvent);
                break;

		case    S_LB_AWAITING_FINAL_FRAME_DECISION:
				Bootloader_FSM_S_LB_AWAITING_FINAL_FRAME_DECISION(pContext, AnyEvent);
				break;

        default:
                Bootloader_NextStateDecoder(pContext, S_LB_INITIAL);
                Watch_Inc(pContext->pWatchPointContext, W_BOOTLOADER_FSM_ILLEGAL_STATE);
                break;
    }

    Bootloader_HandleAction(pContext);
}

