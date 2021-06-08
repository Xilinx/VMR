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
 *  $Change: 3217619 $
 *  $Date: 2021/05/13 $
 *  $Revision: #37 $
 *
 */




#include "cmc_thread_linkstate.h"


void LinkState_NextStateDecoder(LINK_STATE_CONTEXT_TYPE *pContext, LINK_STATE_TYPE NewState)
{
    FSM_STATE_TRANSITION_LOGGER_ELEMENT_TYPE LogElement;
    if(NewState!=pContext->State)
    {
        LogElement.CurrentState=(uint8_t)pContext->State;
        LogElement.Event=(uint8_t)pContext->Event;
        LogElement.NewState=(uint8_t)NewState;
        LogElement.FSM_Identifier=FSM_IDE_LINK_STATE;
        FSM_StateTransitionLogger_TryAdd(pContext->pFSM_StateTransitionLoggerContext, &LogElement);
    }

    pContext->PreviousState=pContext->State;
    pContext->State=NewState;

    Watch_Set(pContext->pWatchPointContext, W_LINK_STATE_CURRENT,  (uint32_t)pContext->State);
    Watch_Set(pContext->pWatchPointContext, W_LINK_STATE_PREVIOUS, (uint32_t)pContext->PreviousState);
}

static void LinkState_ActionStateSequence(LINK_STATE_CONTEXT_TYPE* pContext, uint32_t AnyAction, LINK_STATE_TYPE NewState)
{
	ACTION(pContext, AnyAction);
	LinkState_NextStateDecoder(pContext, NewState);
}

static void LinkState_ActionActionStateSequence(LINK_STATE_CONTEXT_TYPE* pContext, uint32_t AnyAction, uint32_t AnyAction2, LINK_STATE_TYPE NewState)
{
	ACTION(pContext, AnyAction);
	LinkState_ActionStateSequence(pContext, AnyAction2, NewState);
}

static void LinkState_ActionActionActionStateSequence(LINK_STATE_CONTEXT_TYPE* pContext, uint32_t AnyAction, uint32_t AnyAction2, uint32_t AnyAction3, LINK_STATE_TYPE NewState)
{
	ACTION(pContext, AnyAction);
	LinkState_ActionActionStateSequence(pContext, AnyAction2, AnyAction3, NewState);
}

void LinkState_FSM_S_INITIAL(LINK_STATE_CONTEXT_TYPE *pContext, uint8_t AnyEvent)
{
    if (pContext->bCardSupportsSC) // Card with SC
    {
        switch (AnyEvent)
        {
        case    E_LS_RESET_REQUEST:
            LinkState_ActionStateSequence(pContext, A_LS_T_RESPONSE_START, S_DETERMINE_LINK_USER);
            LinkState_CreateEvent_E_LS_T_RESPONSE_EXPIRY(pContext); // Go straight to DETERMINE_LINK_USER_STATE
            break;

        case    E_LS_LINK_USER_TO_BOOTLOADER_REQUEST:
            LinkState_ActionActionStateSequence(pContext, A_LS_T_RESPONSE_START, A_LS_SEND_LINK_USER_TO_BOOTLOADER_COMMAND, S_WAITING_FOR_REMOTE_BOOTLOADER_RESPONSE);
            break;

        default:
            Watch_Inc(pContext->pWatchPointContext, W_LINK_STATE_UNEXPECTED_EVENT);
            break;
        }
    }
    else if (pContext->bCardSupportsSUCUpgrade)
    {
        switch (AnyEvent)
        {
        case    E_LS_RESET_REQUEST:
            LinkState_ActionActionStateSequence(pContext, A_LS_T_RESPONSE_START, A_LS_ANNOUNCE_LINK_USER_IS_SENSOR_SUPERVISOR_SUC, S_LINK_USER_IS_REMOTE_SENSOR_SUPERVISOR_SUC);
            break;

        case    E_LS_LINK_USER_TO_BOOTLOADER_REQUEST:
            break;

        default:
            Watch_Inc(pContext->pWatchPointContext, W_LINK_STATE_UNEXPECTED_EVENT);
            break;
        }
    }
    else
    {
        switch (AnyEvent)
        {
        case    E_LS_RESET_REQUEST:
            LinkState_ActionActionStateSequence(pContext, A_LS_T_RESPONSE_STOP, A_LS_ANNOUNCE_LINK_USER_IS_SENSOR_SUPERVISOR_LINK_OPERATING, S_LINK_USER_IS_REMOTE_SENSOR_SUPERVISOR_LINK_OPERATING);
            break;

        case    E_LS_LINK_USER_TO_BOOTLOADER_REQUEST:
            break;

        default:
            Watch_Inc(pContext->pWatchPointContext, W_LINK_STATE_UNEXPECTED_EVENT);
            break;
        }
    }
}



void LinkState_FSM_S_DETERMINE_LINK_USER_STATE(LINK_STATE_CONTEXT_TYPE *pContext, uint8_t AnyEvent)
{
    if (pContext->bCardSupportsSC) // Card with SC
    {
        switch (AnyEvent)
        {

        case    E_LS_T_RESPONSE_EXPIRY:
            LinkState_ActionActionStateSequence(pContext, A_LS_T_RESPONSE_START, A_LS_SEND_LINK_USER_QUERY, S_DETERMINE_LINK_USER);
            break;

        case    E_LS_LINK_USER_IS_REMOTE_SENSOR_SUPERVISOR:
            LinkState_ActionActionActionStateSequence(pContext, A_LS_T_RESPONSE_RESTART, A_LS_ANNOUNCE_LINK_USER_IS_SENSOR_SUPERVISOR, A_LS_SEND_LINK_OFFERED_PROTOCOL_VERSION, S_LINK_USER_IS_REMOTE_SENSOR_SUPERVISOR);
            break;

        case    E_LS_LINK_USER_IS_REMOTE_BOOTLOADER:
            LinkState_ActionActionStateSequence(pContext, A_LS_T_RESPONSE_STOP, A_LS_ANNOUNCE_LINK_USER_IS_BOOTLOADER, S_LINK_USER_IS_REMOTE_BOOTLOADER);
            break;

        case    E_LS_LINK_USER_TO_BOOTLOADER_REQUEST:
            LinkState_ActionActionStateSequence(pContext, A_LS_T_RESPONSE_RESTART, A_LS_SEND_LINK_USER_TO_BOOTLOADER_COMMAND, S_WAITING_FOR_REMOTE_BOOTLOADER_RESPONSE);
            break;

        default:
            Watch_Inc(pContext->pWatchPointContext, W_LINK_STATE_UNEXPECTED_EVENT);
            break;
        }
    }
    else
    {
        switch (AnyEvent)
        {
        case    E_LS_LINK_USER_IS_REMOTE_SENSOR_SUPERVISOR:
            LinkState_ActionActionStateSequence(pContext, A_LS_T_RESPONSE_STOP, A_LS_ANNOUNCE_LINK_USER_IS_SENSOR_SUPERVISOR_LINK_OPERATING, S_LINK_USER_IS_REMOTE_SENSOR_SUPERVISOR_LINK_OPERATING);
            break;
            
        case    E_LS_T_RESPONSE_EXPIRY:
        case    E_LS_LINK_USER_IS_REMOTE_BOOTLOADER:
        case    E_LS_LINK_USER_TO_BOOTLOADER_REQUEST:
            break;

        default:
            Watch_Inc(pContext->pWatchPointContext, W_LINK_STATE_UNEXPECTED_EVENT);
            break;
        }
    }
}



void LinkState_FSM_S_LINK_USER_IS_REMOTE_SENSOR_SUPERVISOR(LINK_STATE_CONTEXT_TYPE *pContext, uint8_t AnyEvent)
{

    switch(AnyEvent)
    {
        case    E_LS_RESET_REQUEST:
				LinkState_ActionStateSequence(pContext, A_LS_T_RESPONSE_RESTART, S_DETERMINE_LINK_USER);
                break;
                          
        case    E_LS_T_RESPONSE_EXPIRY:
				LinkState_ActionActionStateSequence(pContext, A_LS_T_RESPONSE_START, A_LS_SEND_LINK_OFFERED_PROTOCOL_VERSION, S_LINK_USER_IS_REMOTE_SENSOR_SUPERVISOR);
                break;
                       
        case    E_LS_LINK_USER_TO_BOOTLOADER_REQUEST:
				LinkState_ActionActionStateSequence(pContext, A_LS_T_RESPONSE_RESTART, A_LS_SEND_LINK_USER_TO_BOOTLOADER_COMMAND, S_WAITING_FOR_REMOTE_BOOTLOADER_RESPONSE);  
                break;
        
		case    E_LS_LINK_OFFERED_VERSION_DECLINED:
                LinkState_ActionActionStateSequence(pContext, A_LS_T_RESPONSE_RESTART, A_LS_SEND_LINK_OFFERED_PROTOCOL_VERSION, S_LINK_USER_IS_REMOTE_SENSOR_SUPERVISOR);
                break;

        case    E_LS_LINK_OFFERED_VERSION_ACKNOWLEDGE:
				LinkState_ActionActionStateSequence(pContext, A_LS_T_RESPONSE_RESTART, A_LS_SEND_LINK_ACCEPTED_PROTOCOL_VERSION_RANGE_QUERY, S_LINK_USER_IS_REMOTE_SENSOR_SUPERVISOR);
                break;

		case	E_LS_LINK_ACCEPTED_PROTOCOL_VERSION_RANGE_RESPONSE_FAILED:
				LinkState_ActionActionStateSequence(pContext, A_LS_T_RESPONSE_RESTART, A_LS_SEND_LINK_OFFERED_PROTOCOL_VERSION, S_LINK_USER_IS_REMOTE_SENSOR_SUPERVISOR);
				break;
       
        case    E_LS_LINK_ACCEPTED_PROTOCOL_VERSION_RANGE_RESPONSE:
                LinkState_ActionActionStateSequence(pContext, A_LS_T_RESPONSE_RESTART, A_LS_SEND_LINK_FPGA_I2C_BUS_ARB, S_LINK_USER_IS_REMOTE_SENSOR_SUPERVISOR);
                break;

        case    E_LS_LINK_I2C_ACKNOWLEDGE:
        case    E_LS_LINK_I2C_FAILED:
            LinkState_ActionActionActionStateSequence(pContext, A_LS_T_RESPONSE_RESTART, A_LS_CACHE_LINK_ACCEPTED_PROTOCOL_VERSION_RANGE, A_LS_ANNOUNCE_LINK_USER_IS_SENSOR_SUPERVISOR_LINK_OPERATING, S_LINK_USER_IS_REMOTE_SENSOR_SUPERVISOR_LINK_OPERATING);
            break;


        default:
                Watch_Inc(pContext->pWatchPointContext, W_LINK_STATE_UNEXPECTED_EVENT);
                break;
    }
}

void LinkState_FSM_S_LINK_USER_IS_REMOTE_SENSOR_SUPERVISOR_SUC(LINK_STATE_CONTEXT_TYPE* pContext, uint8_t AnyEvent)
{

    switch (AnyEvent)
    {
    case    E_LS_RESET_REQUEST:
            LinkState_ActionStateSequence(pContext, A_LS_T_RESPONSE_RESTART, S_LINK_USER_IS_REMOTE_SENSOR_SUPERVISOR_SUC);
            break;

    case    E_LS_T_RESPONSE_EXPIRY:
            LinkState_ActionActionStateSequence(pContext, A_LS_T_RESPONSE_START, A_LS_SEND_LINK_OFFERED_PROTOCOL_VERSION, S_LINK_USER_IS_REMOTE_SENSOR_SUPERVISOR_SUC);
            break;

    case    E_LS_LINK_OFFERED_VERSION_DECLINED:
            LinkState_ActionActionStateSequence(pContext, A_LS_T_RESPONSE_RESTART, A_LS_SEND_LINK_OFFERED_PROTOCOL_VERSION, S_LINK_USER_IS_REMOTE_SENSOR_SUPERVISOR_SUC);
            break;

    case    E_LS_LINK_OFFERED_VERSION_ACKNOWLEDGE:
            LinkState_ActionActionStateSequence(pContext, A_LS_T_RESPONSE_RESTART, A_LS_SEND_LINK_ACCEPTED_PROTOCOL_VERSION_RANGE_QUERY, S_LINK_USER_IS_REMOTE_SENSOR_SUPERVISOR_SUC);
            break;

    case    E_LS_LINK_ACCEPTED_PROTOCOL_VERSION_RANGE_RESPONSE_FAILED:
            LinkState_ActionActionStateSequence(pContext, A_LS_T_RESPONSE_RESTART, A_LS_SEND_LINK_OFFERED_PROTOCOL_VERSION, S_LINK_USER_IS_REMOTE_SENSOR_SUPERVISOR_SUC);
            break;

    case    E_LS_LINK_ACCEPTED_PROTOCOL_VERSION_RANGE_RESPONSE:
            //LinkState_ActionStateSequence(pContext, A_LS_T_RESPONSE_RESTART, S_LINK_USER_IS_REMOTE_SENSOR_SUPERVISOR_LINK_OPERATING);
            LinkState_ActionActionActionStateSequence(pContext, A_LS_T_RESPONSE_RESTART, A_LS_CACHE_LINK_ACCEPTED_PROTOCOL_VERSION_RANGE, A_LS_ANNOUNCE_LINK_USER_IS_SENSOR_SUPERVISOR_LINK_OPERATING, S_LINK_USER_IS_REMOTE_SENSOR_SUPERVISOR_LINK_OPERATING);
            break;

    default:
            Watch_Inc(pContext->pWatchPointContext, W_LINK_STATE_UNEXPECTED_EVENT);
            break;
    }
}


void LinkState_FSM_S_LINK_USER_IS_REMOTE_SENSOR_SUPERVISOR_LINK_OPERATING(LINK_STATE_CONTEXT_TYPE *pContext, uint8_t AnyEvent)
{
    switch(AnyEvent)
    {
        case    E_LS_RESET_REQUEST:
				LinkState_ActionStateSequence(pContext, A_LS_T_RESPONSE_RESTART, S_DETERMINE_LINK_USER);
                break;

        case    E_LS_LINK_USER_IS_REMOTE_SENSOR_SUPERVISOR:
                LinkState_ActionActionStateSequence(pContext, A_LS_T_RESPONSE_START, A_LS_SEND_LINK_OFFERED_PROTOCOL_VERSION, S_LINK_USER_IS_REMOTE_SENSOR_SUPERVISOR);
                break;

        case    E_LS_LINK_USER_TO_BOOTLOADER_REQUEST:
				LinkState_ActionActionStateSequence(pContext, A_LS_T_RESPONSE_RESTART, A_LS_SEND_LINK_USER_TO_BOOTLOADER_COMMAND, S_LINK_USER_IS_REMOTE_SENSOR_SUPERVISOR_LINK_OPERATING);
				break;

		case    E_LS_ENABLE_BSL_RESPONSE:
				LinkState_ActionStateSequence(pContext, A_LS_T_BSL_START, S_WAITING_FOR_REMOTE_BOOTLOADER_RESPONSE);
                break;

        case    E_LS_ENABLE_BSL_RESPONSE_FAILED:
                LinkState_ActionActionStateSequence(pContext, A_LS_ANNOUNCE_LINK_USER_IS_SENSOR_SUPERVISOR_LINK_OPERATING, A_LS_ANNOUNCE_COMMAND_FAILED, S_LINK_USER_IS_REMOTE_SENSOR_SUPERVISOR_LINK_OPERATING);
                break;

        default:
                Watch_Inc(pContext->pWatchPointContext, W_LINK_STATE_UNEXPECTED_EVENT);
                break;
    }
}



void LinkState_FSM_S_WAITING_FOR_REMOTE_BOOTLOADER_RESPONSE(LINK_STATE_CONTEXT_TYPE *pContext, uint8_t AnyEvent)
{
    switch(AnyEvent)
    {
        case    E_LS_RESET_REQUEST:
				LinkState_ActionStateSequence(pContext, A_LS_T_RESPONSE_RESTART, S_DETERMINE_LINK_USER);
                break;

        case    E_LS_LINK_USER_IS_REMOTE_SENSOR_SUPERVISOR:
		case    E_LS_LINK_USER_TO_BOOTLOADER_REQUEST:
				LinkState_ActionActionStateSequence(pContext, A_LS_T_RESPONSE_RESTART, A_LS_SEND_LINK_USER_TO_BOOTLOADER_COMMAND, S_WAITING_FOR_REMOTE_BOOTLOADER_RESPONSE);
                break;

        case    E_LS_T_RESPONSE_EXPIRY:
				LinkState_ActionActionStateSequence(pContext, A_LS_T_RESPONSE_RESTART, A_LS_SEND_LINK_USER_QUERY, S_WAITING_FOR_REMOTE_BOOTLOADER_RESPONSE);
                break;

		case	E_LS_T_BSL_EXPIRY:
				LinkState_ActionActionStateSequence(pContext, A_LS_T_RESPONSE_RESTART, A_LS_SEND_LINK_USER_QUERY, S_WAITING_FOR_REMOTE_BOOTLOADER_RESPONSE);
				break;

		case    E_LS_LINK_USER_IS_REMOTE_BOOTLOADER:
				LinkState_ActionActionStateSequence(pContext, A_LS_T_RESPONSE_STOP, A_LS_ANNOUNCE_LINK_USER_IS_BOOTLOADER, S_LINK_USER_IS_REMOTE_BOOTLOADER);
				break;

        default:
                Watch_Inc(pContext->pWatchPointContext, W_LINK_STATE_UNEXPECTED_EVENT);
                break;
    }
}


void LinkState_FSM_S_LINK_USER_IS_REMOTE_BOOTLOADER(LINK_STATE_CONTEXT_TYPE *pContext, uint8_t AnyEvent)
{
    switch(AnyEvent)
    {
        case    E_LS_RESET_REQUEST:
				LinkState_ActionStateSequence(pContext, A_LS_T_RESPONSE_START, S_DETERMINE_LINK_USER);
                break;

        case    E_LS_LINK_USER_TO_BOOTLOADER_REQUEST:
				LinkState_ActionActionStateSequence(pContext, A_LS_T_RESPONSE_RESTART, A_LS_SEND_LINK_USER_TO_BOOTLOADER_COMMAND, S_WAITING_FOR_REMOTE_BOOTLOADER_RESPONSE);
                break;

        default:
                Watch_Inc(pContext->pWatchPointContext, W_LINK_STATE_UNEXPECTED_EVENT);
                break;
    }
}






void LinkState_FSM(LINK_STATE_CONTEXT_TYPE *pContext, uint8_t AnyEvent)
{
    CLEAR_ACTION(pContext);

    pContext->Event=AnyEvent;
    Watch_Set(pContext->pWatchPointContext, W_LINK_STATE_LAST_EVENT,    (uint32_t)pContext->Event);

    switch(pContext->State)
    {
        case    S_INITIAL:
                LinkState_FSM_S_INITIAL(pContext, AnyEvent);
                break;

        case    S_DETERMINE_LINK_USER:
                LinkState_FSM_S_DETERMINE_LINK_USER_STATE(pContext, AnyEvent);
                break;

        case    S_LINK_USER_IS_REMOTE_SENSOR_SUPERVISOR:
                LinkState_FSM_S_LINK_USER_IS_REMOTE_SENSOR_SUPERVISOR(pContext, AnyEvent);
                break;

        case    S_LINK_USER_IS_REMOTE_SENSOR_SUPERVISOR_SUC:
                LinkState_FSM_S_LINK_USER_IS_REMOTE_SENSOR_SUPERVISOR_SUC(pContext, AnyEvent);
                break;

        case    S_LINK_USER_IS_REMOTE_SENSOR_SUPERVISOR_LINK_OPERATING:
                LinkState_FSM_S_LINK_USER_IS_REMOTE_SENSOR_SUPERVISOR_LINK_OPERATING(pContext, AnyEvent);
                break;

        case    S_WAITING_FOR_REMOTE_BOOTLOADER_RESPONSE:
                LinkState_FSM_S_WAITING_FOR_REMOTE_BOOTLOADER_RESPONSE(pContext, AnyEvent);
                break;

        case    S_LINK_USER_IS_REMOTE_BOOTLOADER:
                LinkState_FSM_S_LINK_USER_IS_REMOTE_BOOTLOADER(pContext, AnyEvent);
                break;

        default:
                LinkState_NextStateDecoder(pContext, S_INITIAL);
                Watch_Inc(pContext->pWatchPointContext, W_LINK_STATE_FSM_ILLEGAL_STATE);
                break;
    }

    LinkState_HandleAction(pContext);

}

