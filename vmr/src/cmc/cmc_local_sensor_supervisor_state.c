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
 *  $Change: 3164421 $
 *  $Date: 2021/03/30 $
 *  $Revision: #51 $
 *
 */




#include "cmc_local_sensor_supervisor_thread.h"
#include "cmc_feature_control.h"

static void checkIfExpectedMessageDidntArrive(LOCAL_SENSOR_SUPERVISOR_THREAD_CONTEXT_TYPE* pContext, SENSOR_SUPERVISOR_EXPECTED_MSG_TYPE thisMessage)
{
    uint32_t watchValue;
    uint32_t watchCount;

    if(pContext->ExpectedMessage != thisMessage)
    {
        Watch_Get(pContext->pWatchPointContext, W_SENSOR_SUPERVISOR_MESSAGE_DIDNT_ARRIVE, &watchValue);

        watchCount = watchValue & 0x00FFFFFF;
        watchCount++;
        watchValue = ((((uint32_t)pContext->ExpectedMessage << 24) & 0xFF000000) | (watchCount & 0x00FFFFFF));

        Watch_Set(pContext->pWatchPointContext, W_SENSOR_SUPERVISOR_MESSAGE_DIDNT_ARRIVE, watchValue);

        //On U30 Zync1 use this to re-request board info
        if (pContext->DataStoreContext.pFirmwareVersionContext->Version.Minor == 0x4) // Is this a U30
        {
            if (pContext->DataStoreContext.bZync1Device)
            {
                pContext->DataStoreContext.BoardInfo.IsValid = false;
            }
        }
    }
    else
    {
        pContext->ExpectedMessage = SS_EXPECTED_MSG_NONE;
    }
}

void SensorSupervisorLogUnexpected(LOCAL_SENSOR_SUPERVISOR_THREAD_CONTEXT_TYPE* pContext, uint8_t AnyEvent)
{
    Watch_Set(pContext->pWatchPointContext, W_SENSOR_SUPERVISOR_UNEXPECTED_EVENT, pContext->State << 24 | AnyEvent);
}

void SensorSupervisor_NextStateDecoder(LOCAL_SENSOR_SUPERVISOR_THREAD_CONTEXT_TYPE *pContext, SENSOR_SUPERVISOR_STATE_TYPE NewState)
{
    FSM_STATE_TRANSITION_LOGGER_ELEMENT_TYPE LogElement;
    if(NewState!= S_SS_INITIAL)
    {
        LogElement.CurrentState=(uint8_t)pContext->State;
        LogElement.Event=(uint8_t)pContext->Event;
        LogElement.NewState=(uint8_t)NewState;
        LogElement.FSM_Identifier=FSM_ID_SENSOR_SUPERVISOR;
        FSM_StateTransitionLogger_TryAdd(pContext->pFSM_StateTransitionLoggerContext, &LogElement);
    }

    pContext->PreviousState=pContext->State;
    pContext->State=NewState;

    Watch_Set(pContext->pWatchPointContext, W_SENSOR_SUPERVISOR_CURRENT, (uint32_t)pContext->State);
    Watch_Set(pContext->pWatchPointContext, W_SENSOR_SUPERVISOR_PREVIOUS, (uint32_t)pContext->PreviousState);
}

static void SensorSupervisor_ActionStateSequence(LOCAL_SENSOR_SUPERVISOR_THREAD_CONTEXT_TYPE* pContext, uint32_t AnyAction, SENSOR_SUPERVISOR_STATE_TYPE NewState)
{
	SensorSupervisor_ACTION(pContext, AnyAction);
	SensorSupervisor_NextStateDecoder(pContext, NewState);
}

static void SensorSupervisor_ActionActionStateSequence(LOCAL_SENSOR_SUPERVISOR_THREAD_CONTEXT_TYPE* pContext, uint32_t AnyAction, uint32_t AnyAction2, SENSOR_SUPERVISOR_STATE_TYPE NewState)
{
	SensorSupervisor_ACTION(pContext, AnyAction);
	SensorSupervisor_ActionStateSequence(pContext, AnyAction2, NewState);
}


void SensorSupervisor_FSM_S_SS_INITIAL(LOCAL_SENSOR_SUPERVISOR_THREAD_CONTEXT_TYPE *pContext, uint8_t AnyEvent)
{
    switch(AnyEvent)
    {
        case    E_SS_T_SENSOR_EXPIRY:
				SensorSupervisor_ActionStateSequence(pContext, A_SS_T_SENSOR_START, S_SS_INITIAL);
                break;

        case    E_SS_LINK_IS_AVAILABLE:
                SensorSupervisor_ACTION(pContext, A_SS_T_SENSOR_START);
                SensorSupervisor_ActionActionStateSequence(pContext, A_SS_INITIALIZE_CAGE_SENSOR_INFORMATION, A_SS_SEND_OEM_CMD_REQUEST, S_SS_FETCHING_OEM);
                break;

        default:
                SensorSupervisorLogUnexpected(pContext, AnyEvent);
                break;
    }
}

void SensorSupervisor_FSM_S_SS_FETCHING_OEM(LOCAL_SENSOR_SUPERVISOR_THREAD_CONTEXT_TYPE* pContext, uint8_t AnyEvent)
{
    switch (AnyEvent)
    {
    case    E_SS_T_SENSOR_EXPIRY:
        checkIfExpectedMessageDidntArrive(pContext, SS_EXPECTED_MSG_NONE);
        SensorSupervisor_ActionActionStateSequence(pContext, A_SS_T_SENSOR_START, A_SS_REQUEST_BOARD_INFO, S_SS_FETCHING_BOARD_INFORMATION);
        break;

    case    E_SS_MESSAGE_ARRIVAL:
        SensorSupervisor_ACTION(pContext, A_SS_PROCESS_MESSAGE);
        break;

    case    E_SS_LINK_IS_UNAVAILABLE:
        SensorSupervisor_ActionStateSequence(pContext, A_SS_T_SENSOR_RESTART, S_SS_INITIAL);
        break;

    case	E_SS_GPIO_INTERRUPT_ARRIVAL:
        SensorSupervisor_ActionActionStateSequence(pContext, A_SS_T_SENSOR_RESTART, A_SS_SEND_ALERT_REQUEST, S_SS_AWAITING_ALERT_RESPONSE);
        break;

    default:
        SensorSupervisorLogUnexpected(pContext, AnyEvent);
        break;
    }
}



void SensorSupervisor_FSM_S_SS_FETCHING_BOARD_INFORMATION(LOCAL_SENSOR_SUPERVISOR_THREAD_CONTEXT_TYPE *pContext, uint8_t AnyEvent)
{
    switch(AnyEvent)
    {
        case    E_SS_T_SENSOR_EXPIRY:
                checkIfExpectedMessageDidntArrive(pContext, SS_EXPECTED_MSG_NONE);
                pContext->EnableUartCount = 0;
                SensorSupervisor_ActionActionStateSequence(pContext, A_SS_T_SENSOR_START, A_SS_DEBUG_UART_ENABLE, S_SS_DEBUG_UART_ENABLE);
                break;

        case    E_SS_MESSAGE_ARRIVAL:
                SensorSupervisor_ACTION(pContext, A_SS_PROCESS_MESSAGE);
                break;

        case    E_SS_LINK_IS_UNAVAILABLE:
				SensorSupervisor_ActionStateSequence(pContext, A_SS_T_SENSOR_RESTART, S_SS_INITIAL);
                break;

		case	E_SS_GPIO_INTERRUPT_ARRIVAL:
				SensorSupervisor_ActionActionStateSequence(pContext, A_SS_T_SENSOR_RESTART, A_SS_SEND_ALERT_REQUEST, S_SS_AWAITING_ALERT_RESPONSE);
				break;

        case    E_SS_RELAYING_CAGE_INFORMATION_DONE:
                // You get one of these at the start due to sending the initialization 
                break;

        default:
                SensorSupervisorLogUnexpected(pContext, AnyEvent);
                break;
    }
}

void SensorSupervisor_FSM_S_SS_DEBUG_UART_ENABLE(LOCAL_SENSOR_SUPERVISOR_THREAD_CONTEXT_TYPE* pContext, uint8_t AnyEvent)
{
    switch (AnyEvent)
    {
    case    E_SS_T_SENSOR_EXPIRY:      
        if (pContext->EnableUartCount == 20)
        {
            checkIfExpectedMessageDidntArrive(pContext, SS_EXPECTED_MSG_NONE);
            SensorSupervisor_ActionActionStateSequence(pContext, A_SS_T_SENSOR_START, A_SS_REQUEST_VOLTAGE_SENSOR_INFORMATION, S_SS_FETCHING_SENSOR_VOLTAGE);
        }
        else
        {
            // For some unknown reason we get errored messages after sending the DEBUG_UART_ENABLE message
            // I've added a longer wait after sending to fix the issue
            // From timining tests this command seems to take 7 iterations of this loop !!!!!!!
            SensorSupervisor_ActionActionStateSequence(pContext, A_SS_T_SENSOR_START, A_SS_DEBUG_UART_ENABLE, S_SS_DEBUG_UART_ENABLE);
        }
        break;

    case    E_SS_MESSAGE_ARRIVAL:
        SensorSupervisor_ACTION(pContext, A_SS_PROCESS_MESSAGE);
        break;

    case    E_SS_LINK_IS_UNAVAILABLE:
        SensorSupervisor_ActionStateSequence(pContext, A_SS_T_SENSOR_RESTART, S_SS_INITIAL);
        break;

    case	E_SS_GPIO_INTERRUPT_ARRIVAL:
        SensorSupervisor_ActionActionStateSequence(pContext, A_SS_T_SENSOR_RESTART, A_SS_SEND_ALERT_REQUEST, S_SS_AWAITING_ALERT_RESPONSE);
        break;

    case    E_SS_T_ENABLE_UART_NOT_REQUIRED:
        SensorSupervisor_ActionActionStateSequence(pContext, A_SS_T_SENSOR_RESTART, A_SS_REQUEST_VOLTAGE_SENSOR_INFORMATION, S_SS_FETCHING_SENSOR_VOLTAGE);
        break;

    case    E_SS_DEBUG_UART_RESPONSE_ARRIVAL:
        checkIfExpectedMessageDidntArrive(pContext, SS_EXPECTED_MSG_UART_EN_RESP);
        SensorSupervisor_ActionActionStateSequence(pContext, A_SS_T_SENSOR_RESTART, A_SS_REQUEST_VOLTAGE_SENSOR_INFORMATION, S_SS_FETCHING_SENSOR_VOLTAGE);
        break;

    default:
        SensorSupervisorLogUnexpected(pContext, AnyEvent);
        break;
    }
}






void SensorSupervisor_FSM_S_SS_FETCHING_SENSOR_VOLTAGE(LOCAL_SENSOR_SUPERVISOR_THREAD_CONTEXT_TYPE *pContext, uint8_t AnyEvent)
{
    switch(AnyEvent)
    {
        case    E_SS_T_SENSOR_EXPIRY:
                checkIfExpectedMessageDidntArrive(pContext, SS_EXPECTED_MSG_NONE);
				
                SensorSupervisor_ActionActionStateSequence(pContext, A_SS_T_SENSOR_START, A_SS_REQUEST_POWER_SENSOR_INFORMATION, S_SS_FETCHING_SENSOR_POWER);
                break;

        case    E_SS_MESSAGE_ARRIVAL:
                SensorSupervisor_ACTION(pContext, A_SS_PROCESS_MESSAGE);
                break;

        case    E_SS_LINK_IS_UNAVAILABLE:
				SensorSupervisor_ActionStateSequence(pContext, A_SS_T_SENSOR_RESTART, S_SS_INITIAL);
                break;

		case	E_SS_GPIO_INTERRUPT_ARRIVAL:
				SensorSupervisor_ActionActionStateSequence(pContext, A_SS_T_SENSOR_RESTART, A_SS_SEND_ALERT_REQUEST, S_SS_AWAITING_ALERT_RESPONSE);
				break;

        default:
                SensorSupervisorLogUnexpected(pContext, AnyEvent);
                break;
    }
}




void SensorSupervisor_FSM_S_SS_FETCHING_SENSOR_POWER(LOCAL_SENSOR_SUPERVISOR_THREAD_CONTEXT_TYPE *pContext, uint8_t AnyEvent)
{
    switch(AnyEvent)
    {
        case    E_SS_T_SENSOR_EXPIRY:
                checkIfExpectedMessageDidntArrive(pContext, SS_EXPECTED_MSG_NONE);
				SensorSupervisor_ActionActionStateSequence(pContext, A_SS_T_SENSOR_START, A_SS_REQUEST_TEMPERATURE_SENSOR_INFORMATION, S_SS_FETCHING_SENSOR_TEMPERATURE);
                break;

        case    E_SS_MESSAGE_ARRIVAL:
                SensorSupervisor_ACTION(pContext, A_SS_PROCESS_MESSAGE);
                break;

        case    E_SS_LINK_IS_UNAVAILABLE:
				SensorSupervisor_ActionStateSequence(pContext, A_SS_T_SENSOR_RESTART, S_SS_INITIAL);
                break;

		case	E_SS_GPIO_INTERRUPT_ARRIVAL:
				SensorSupervisor_ActionActionStateSequence(pContext, A_SS_T_SENSOR_RESTART, A_SS_SEND_ALERT_REQUEST, S_SS_AWAITING_ALERT_RESPONSE);
				break;

        default:
                SensorSupervisorLogUnexpected(pContext, AnyEvent);
                break;
    }
}





void SensorSupervisor_FSM_S_SS_FETCHING_SENSOR_TEMPERATURE(LOCAL_SENSOR_SUPERVISOR_THREAD_CONTEXT_TYPE *pContext, uint8_t AnyEvent)
{
    switch(AnyEvent)
    {

        case    E_SS_T_SENSOR_EXPIRY:
                checkIfExpectedMessageDidntArrive(pContext, SS_EXPECTED_MSG_NONE);
				SensorSupervisor_ActionActionStateSequence(pContext, A_SS_T_SENSOR_START, A_SS_FETCHING_POWER_THROTTLING_THRESHOLDS, S_SS_FETCHING_POWER_THROTTLING_THRESHOLDS);
                break;

        case    E_SS_MESSAGE_ARRIVAL:
                SensorSupervisor_ACTION(pContext, A_SS_PROCESS_MESSAGE);
                break;

        case    E_SS_LINK_IS_UNAVAILABLE:
				SensorSupervisor_ActionStateSequence(pContext, A_SS_T_SENSOR_RESTART, S_SS_INITIAL);
                break;

		case	E_SS_GPIO_INTERRUPT_ARRIVAL:
				SensorSupervisor_ActionActionStateSequence(pContext, A_SS_T_SENSOR_RESTART, A_SS_SEND_ALERT_REQUEST, S_SS_AWAITING_ALERT_RESPONSE);
				break;

        default:
                SensorSupervisorLogUnexpected(pContext, AnyEvent);
                break;
    }
}

void SensorSupervisor_FSM_S_SS_FETCHING_POWER_THROTTLING_THRESHOLDS(LOCAL_SENSOR_SUPERVISOR_THREAD_CONTEXT_TYPE* pContext, uint8_t AnyEvent)
{
    switch (AnyEvent)
    {

    case    E_SS_T_SENSOR_EXPIRY:
        checkIfExpectedMessageDidntArrive(pContext, SS_EXPECTED_MSG_NONE);
        SensorSupervisor_ActionActionStateSequence(pContext, A_SS_T_SENSOR_START, A_SS_FETCHING_TEMP_THROTTLING_THRESHOLDS, S_SS_FETCHING_TEMP_THROTTLING_THRESHOLDS);
        break;

    case    E_SS_MESSAGE_ARRIVAL:
        SensorSupervisor_ACTION(pContext, A_SS_PROCESS_MESSAGE);
        break;

    case    E_SS_LINK_IS_UNAVAILABLE:
        SensorSupervisor_ActionStateSequence(pContext, A_SS_T_SENSOR_RESTART, S_SS_INITIAL);
        break;

    case	E_SS_GPIO_INTERRUPT_ARRIVAL:
        SensorSupervisor_ActionActionStateSequence(pContext, A_SS_T_SENSOR_RESTART, A_SS_SEND_ALERT_REQUEST, S_SS_AWAITING_ALERT_RESPONSE);
        break;

    default:
        SensorSupervisorLogUnexpected(pContext, AnyEvent);
        break;
    }
}

void SensorSupervisor_FSM_S_SS_FETCHING_TEMP_THROTTLING_THRESHOLDS(LOCAL_SENSOR_SUPERVISOR_THREAD_CONTEXT_TYPE* pContext, uint8_t AnyEvent)
{
    switch (AnyEvent)
    {

    case    E_SS_T_SENSOR_EXPIRY:
        checkIfExpectedMessageDidntArrive(pContext, SS_EXPECTED_MSG_NONE);
        SensorSupervisor_ActionActionStateSequence(pContext, A_SS_T_SENSOR_START, A_SS_RELAY_HBM_SENSOR_INFORMATION, S_SS_RELAYING_HBM);
        // And kick the clock throttling algorithm
        SensorSupervisor_ACTION(pContext, A_SS_KICK_CLOCK_THROTTLING);
        // Inform XRT that sensor data is available
        PERIPHERAL_REGMAP_RAM_CONTROLLER_WriteWithMask(pContext->DataStoreContext.pHardwareRegisterSetContext, HOST_REGISTER_STATUS2, HOST_STATUS2_SENSOR_DATA_AVAILABLE, HOST_STATUS2_SENSOR_DATA_AVAILABLE);

        /* Increment the average sensor counters position */
        if (pContext->DataStoreContext.SensorInfo.ProcessSensorCount == 0xFFFF)
        {
            /* When counter reaches 0xFFFF leave it there */
            pContext->DataStoreContext.SensorInfo.ProcessSensorCount = 0xFFFF;
        }
        else
        {
            pContext->DataStoreContext.SensorInfo.ProcessSensorCount++;
        }
        break;

    case    E_SS_MESSAGE_ARRIVAL:
        SensorSupervisor_ACTION(pContext, A_SS_PROCESS_MESSAGE);
        break;

    case    E_SS_LINK_IS_UNAVAILABLE:
        SensorSupervisor_ActionStateSequence(pContext, A_SS_T_SENSOR_RESTART, S_SS_INITIAL);
        break;

    case	E_SS_GPIO_INTERRUPT_ARRIVAL:
        SensorSupervisor_ActionActionStateSequence(pContext, A_SS_T_SENSOR_RESTART, A_SS_SEND_ALERT_REQUEST, S_SS_AWAITING_ALERT_RESPONSE);
        break;

    default:
        SensorSupervisorLogUnexpected(pContext, AnyEvent);
        break;
    }
}



void SensorSupervisor_FSM_S_SS_RELAYING_HBM(LOCAL_SENSOR_SUPERVISOR_THREAD_CONTEXT_TYPE *pContext, uint8_t AnyEvent)
{
    switch(AnyEvent)
    {

        case    E_SS_T_SENSOR_EXPIRY:
				pContext->iQSFP = 0;
                SensorSupervisor_ActionActionStateSequence(pContext, A_SS_T_SENSOR_START, A_SS_SEND_PCIE_ERRORS, S_SS_RELAY_PCIE);
                break;

        case    E_SS_MESSAGE_ARRIVAL:
                SensorSupervisor_ACTION(pContext, A_SS_PROCESS_MESSAGE);
                break;

        case    E_SS_LINK_IS_UNAVAILABLE:
				SensorSupervisor_ActionStateSequence(pContext, A_SS_T_SENSOR_RESTART, S_SS_INITIAL);
                break;

		case	E_SS_GPIO_INTERRUPT_ARRIVAL:
				SensorSupervisor_ActionActionStateSequence(pContext, A_SS_T_SENSOR_RESTART, A_SS_SEND_ALERT_REQUEST, S_SS_AWAITING_ALERT_RESPONSE);
				break;

        default:
                SensorSupervisorLogUnexpected(pContext, AnyEvent);
                break;
    }
}


void SensorSupervisor_FSM_S_SS_RELAY_PCIE(LOCAL_SENSOR_SUPERVISOR_THREAD_CONTEXT_TYPE* pContext, uint8_t AnyEvent)
{
    switch (AnyEvent)
    {

    case    E_SS_T_SENSOR_EXPIRY:
        checkIfExpectedMessageDidntArrive(pContext, SS_EXPECTED_MSG_NONE);
        SensorSupervisor_ActionActionStateSequence(pContext, A_SS_T_SENSOR_START, A_SS_SEND_ECC_ERRORS, S_SS_RELAY_ECC);
        break;

    case    E_SS_MESSAGE_ARRIVAL:
        SensorSupervisor_ACTION(pContext, A_SS_PROCESS_MESSAGE);
        break;

    case    E_SS_LINK_IS_UNAVAILABLE:
        SensorSupervisor_ActionStateSequence(pContext, A_SS_T_SENSOR_RESTART, S_SS_INITIAL);
        break;

    case	E_SS_GPIO_INTERRUPT_ARRIVAL:
        SensorSupervisor_ActionActionStateSequence(pContext, A_SS_T_SENSOR_RESTART, A_SS_SEND_ALERT_REQUEST, S_SS_AWAITING_ALERT_RESPONSE);
        break;

    default:
        SensorSupervisorLogUnexpected(pContext, AnyEvent);
        break;
    }
}

void SensorSupervisor_FSM_S_SS_RELAY_ECC(LOCAL_SENSOR_SUPERVISOR_THREAD_CONTEXT_TYPE* pContext, uint8_t AnyEvent)
{
    switch (AnyEvent)
    {

    case    E_SS_T_SENSOR_EXPIRY:
        checkIfExpectedMessageDidntArrive(pContext, SS_EXPECTED_MSG_NONE);
        SensorSupervisor_ActionActionStateSequence(pContext, A_SS_T_SENSOR_START, A_SS_SEND_KEEPALIVE, S_SS_KEEPALIVE);
        break;

    case    E_SS_MESSAGE_ARRIVAL:
        SensorSupervisor_ACTION(pContext, A_SS_PROCESS_MESSAGE);
        break;

    case    E_SS_LINK_IS_UNAVAILABLE:
        SensorSupervisor_ActionStateSequence(pContext, A_SS_T_SENSOR_RESTART, S_SS_INITIAL);
        break;

    case	E_SS_GPIO_INTERRUPT_ARRIVAL:
        SensorSupervisor_ActionActionStateSequence(pContext, A_SS_T_SENSOR_RESTART, A_SS_SEND_ALERT_REQUEST, S_SS_AWAITING_ALERT_RESPONSE);
        break;

    default:
        SensorSupervisorLogUnexpected(pContext, AnyEvent);
        break;
    }
}


void SensorSupervisor_FSM_S_SS_KEEPALIVE(LOCAL_SENSOR_SUPERVISOR_THREAD_CONTEXT_TYPE* pContext, uint8_t AnyEvent)
{
    switch (AnyEvent)
    {

    case    E_SS_T_SENSOR_EXPIRY:
        checkIfExpectedMessageDidntArrive(pContext, SS_EXPECTED_MSG_NONE);
        SensorSupervisor_ActionActionStateSequence(pContext, A_SS_T_SENSOR_START, A_SS_INTERRUPT_STATUS_REQUEST, S_SS_INTERRUPT_STATUS_REQUEST);
        break;

    case    E_SS_MESSAGE_ARRIVAL:
        SensorSupervisor_ACTION(pContext, A_SS_PROCESS_MESSAGE);
        break;

    case    E_SS_LINK_IS_UNAVAILABLE:
        SensorSupervisor_ActionStateSequence(pContext, A_SS_T_SENSOR_RESTART, S_SS_INITIAL);
        break;

    case	E_SS_GPIO_INTERRUPT_ARRIVAL:
        SensorSupervisor_ActionActionStateSequence(pContext, A_SS_T_SENSOR_RESTART, A_SS_SEND_ALERT_REQUEST, S_SS_AWAITING_ALERT_RESPONSE);
        break;

    default:
        SensorSupervisorLogUnexpected(pContext, AnyEvent);
        break;
    }
}

void SensorSupervisor_FSM_S_SS_INTERRUPT_STATUS_REQUEST(LOCAL_SENSOR_SUPERVISOR_THREAD_CONTEXT_TYPE* pContext, uint8_t AnyEvent)
{
    switch (AnyEvent)
    {

    case    E_SS_T_SENSOR_EXPIRY:
        pContext->CSDRResponseWaitCount = 0;
        SensorSupervisor_ActionActionStateSequence(pContext, A_SS_T_SENSOR_START, A_SS_COLLECT_CSDR, S_SS_COLLECT_CSDR);
        break;

    case    E_SS_MESSAGE_ARRIVAL:
        SensorSupervisor_ACTION(pContext, A_SS_PROCESS_MESSAGE);
        break;

    case    E_SS_LINK_IS_UNAVAILABLE:
        SensorSupervisor_ActionStateSequence(pContext, A_SS_T_SENSOR_RESTART, S_SS_INITIAL);
        break;

    case	E_SS_GPIO_INTERRUPT_ARRIVAL:
        SensorSupervisor_ActionActionStateSequence(pContext, A_SS_T_SENSOR_RESTART, A_SS_SEND_ALERT_REQUEST, S_SS_AWAITING_ALERT_RESPONSE);
        break;

    default:
        SensorSupervisorLogUnexpected(pContext, AnyEvent);
        break;
    }
}


void SensorSupervisor_FSM_S_SS_COLLECT_CSDR(LOCAL_SENSOR_SUPERVISOR_THREAD_CONTEXT_TYPE* pContext, uint8_t AnyEvent)
{
    switch (AnyEvent)
    {

    case    E_SS_T_SENSOR_EXPIRY:
        if (pContext->bCollectCSDR)
        {
            if(pContext->WaitingOnCSDRResponse == WAITING_ON_RESPONSE)
            {
                // Im waiting on a response
                // If timeout count reached fail
                // else start timer and wait again
                if (pContext->CSDRResponseWaitCount == CSDR_MESSGAE_TIMEOUT_COUNT)
                {
                    checkIfExpectedMessageDidntArrive(pContext, SS_EXPECTED_MSG_NONE);
                    Watch_Inc(pContext->pWatchPointContext, W_CSDR_EXPECTED_MSG_DIDNT_ARRIVE);
                    // previous message hasn't arrived mark as a failure
                    Broker_Announce_UserProxyCSDRFailed(pContext->pBrokerContext);
                   /* pContext->QSFPResponseWaitCount = 0;
                    pContext->bQSFPRequestSent = false;*/
                    SensorSupervisor_ActionActionStateSequence(pContext, A_SS_T_SENSOR_START, A_SS_QSFP_REQUEST, S_SS_QSFP_REQUEST);
                }
                else
                {
                    SensorSupervisor_ActionActionStateSequence(pContext, A_SS_T_SENSOR_START, A_SS_COLLECT_CSDR, S_SS_COLLECT_CSDR);
                }
            }
            else if(pContext->WaitingOnCSDRResponse == RESPONSE_RECEIVED)
            {
                // I received a response so lets go around the loop and collect some sensors beforg sending next CSDR request
                pContext->WaitingOnCSDRResponse = IDLE;
                /*pContext->QSFPResponseWaitCount = 0;
                pContext->bQSFPRequestSent = false;*/
                SensorSupervisor_ActionActionStateSequence(pContext, A_SS_T_SENSOR_START, A_SS_QSFP_REQUEST, S_SS_QSFP_REQUEST);
            }
            else if (pContext->WaitingOnCSDRResponse == IDLE)
            {
                // Start a new CMD - RESPONSE sequence
                SensorSupervisor_ActionActionStateSequence(pContext, A_SS_T_SENSOR_START, A_SS_COLLECT_CSDR, S_SS_COLLECT_CSDR);
            }

        }
        else
        {
            /*pContext->QSFPResponseWaitCount = 0;
            pContext->bQSFPRequestSent = false;*/
            SensorSupervisor_ActionActionStateSequence(pContext, A_SS_T_SENSOR_START, A_SS_QSFP_REQUEST, S_SS_QSFP_REQUEST);
        }
        break;

    case    E_SS_MESSAGE_ARRIVAL:
        SensorSupervisor_ACTION(pContext, A_SS_PROCESS_MESSAGE);
        break;

    case    E_SS_LINK_IS_UNAVAILABLE:
        SensorSupervisor_ActionStateSequence(pContext, A_SS_T_SENSOR_RESTART, S_SS_INITIAL);
        break;

    case	E_SS_GPIO_INTERRUPT_ARRIVAL:
        SensorSupervisor_ActionActionStateSequence(pContext, A_SS_T_SENSOR_RESTART, A_SS_SEND_ALERT_REQUEST, S_SS_AWAITING_ALERT_RESPONSE);
        break;

    default:
        SensorSupervisorLogUnexpected(pContext, AnyEvent);
        break;
    }
}

void SensorSupervisor_FSM_S_SS_QSFP_MESSAGE(LOCAL_SENSOR_SUPERVISOR_THREAD_CONTEXT_TYPE* pContext, uint8_t AnyEvent)
{
    switch (AnyEvent)
    {

    case    E_SS_T_SENSOR_EXPIRY:
        if (pContext->bReadQSFPDiagnostics || pContext->bReadQSFPValidateLowSpeedIO || pContext->bWriteQSFPValidateLowSpeedIO ||
            pContext->bQSFPReadSingleByte || pContext->bQSFPWriteSingleByte)
        {
            if (pContext->QSFPResponseWaitCount == QSFP_MESSGAE_TIMEOUT_COUNT)
            {
                checkIfExpectedMessageDidntArrive(pContext, SS_EXPECTED_MSG_NONE);
                Watch_Inc(pContext->pWatchPointContext, W_CSDR_EXPECTED_MSG_DIDNT_ARRIVE);
                // previous message hasn't arrived mark as a failure
                if (pContext->bQSFPReadSingleByte)
                {
                    Broker_Announce_SensorSupervisor_ReadQSFPSingleByteFailed(pContext->pBrokerContext);
                }
                if (pContext->bQSFPWriteSingleByte)
                {
                    Broker_Announce_SensorSupervisor_WriteQSFPSingleByteFailed(pContext->pBrokerContext);
                }
                if (pContext->bReadQSFPDiagnostics)
                {
                    Broker_Announce_SensorSupervisor_ReadQSFPDiagnosticsFailed(pContext->pBrokerContext);
                }
                if (pContext->bReadQSFPValidateLowSpeedIO)
                {
                    Broker_Announce_SensorSupervisor_ReadQSFPValidateLowSpeedIOFailed(pContext->pBrokerContext);
                }
                if (pContext->bWriteQSFPValidateLowSpeedIO)
                {
                    Broker_Announce_SensorSupervisor_WriteQSFPValidateLowSpeedIOFailed(pContext->pBrokerContext);
                }

                SensorSupervisor_ActionStateSequence(pContext, A_SS_T_SENSOR_START, S_SS_RELAYING_CAGE_INFORMATION);
            }
            else
            {
                // Wait again
                SensorSupervisor_ActionActionStateSequence(pContext, A_SS_T_SENSOR_START, A_SS_QSFP_REQUEST, S_SS_QSFP_REQUEST);
            }
        }
        else
        {
            SensorSupervisor_ActionStateSequence(pContext, A_SS_T_SENSOR_START, S_SS_RELAYING_CAGE_INFORMATION);
        }
        break;

    case    E_SS_MESSAGE_ARRIVAL:
        SensorSupervisor_ACTION(pContext, A_SS_PROCESS_MESSAGE);
        break;

    case    E_SS_LINK_IS_UNAVAILABLE:
        SensorSupervisor_ActionStateSequence(pContext, A_SS_T_SENSOR_RESTART, S_SS_INITIAL);
        break;

    case	E_SS_GPIO_INTERRUPT_ARRIVAL:
        SensorSupervisor_ActionActionStateSequence(pContext, A_SS_T_SENSOR_RESTART, A_SS_SEND_ALERT_REQUEST, S_SS_AWAITING_ALERT_RESPONSE);
        break;

    default:
        SensorSupervisorLogUnexpected(pContext, AnyEvent);
        break;
    }
}

void SensorSupervisor_FSM_S_SS_RELAYING_CAGE_INFORMATION(LOCAL_SENSOR_SUPERVISOR_THREAD_CONTEXT_TYPE *pContext, uint8_t AnyEvent)
{
	//bool IsEnabled;
    switch(AnyEvent)
    {
        case    E_SS_T_SENSOR_EXPIRY:
				SensorSupervisor_ActionActionStateSequence(pContext, A_SS_T_SENSOR_START, A_SS_RELAY_CAGE_SENSOR_INFORMATION, S_SS_RELAYING_CAGE_INFORMATION_COMPLETE);
                break;

        case    E_SS_MESSAGE_ARRIVAL:
                SensorSupervisor_ACTION(pContext, A_SS_PROCESS_MESSAGE);
                break;

        case    E_SS_LINK_IS_UNAVAILABLE:
				SensorSupervisor_ActionStateSequence(pContext, A_SS_T_SENSOR_RESTART, S_SS_INITIAL);
                break;

		case	E_SS_GPIO_INTERRUPT_ARRIVAL:
				SensorSupervisor_ActionActionStateSequence(pContext, A_SS_T_SENSOR_RESTART, A_SS_SEND_ALERT_REQUEST, S_SS_AWAITING_ALERT_RESPONSE);
				break;

        default:
                SensorSupervisorLogUnexpected(pContext, AnyEvent);
                break;
    }
}

void SensorSupervisor_FSM_S_SS_RELAYING_CAGE_INFORMATION_COMPLETE(LOCAL_SENSOR_SUPERVISOR_THREAD_CONTEXT_TYPE* pContext, uint8_t AnyEvent)
{
	switch (AnyEvent)
	{
	case    E_SS_T_SENSOR_EXPIRY:
            checkIfExpectedMessageDidntArrive(pContext, SS_EXPECTED_MSG_NONE);
		    SensorSupervisor_ActionActionStateSequence(pContext, A_SS_T_SENSOR_START, A_SS_SEND_OEM_CMD_REQUEST, S_SS_FETCHING_OEM);
		    break;

	case    E_SS_RELAYING_CAGE_INFORMATION_DONE:
		    if (pContext->iQSFP < pContext->pGPIO_QSFP_Context->MaxCages)
		    {
			    SensorSupervisor_ActionStateSequence(pContext, A_SS_T_SENSOR_START, S_SS_RELAYING_CAGE_INFORMATION);
		    }
		    else
		    {
			    SensorSupervisor_ActionActionStateSequence(pContext, A_SS_T_SENSOR_START, A_SS_SEND_OEM_CMD_REQUEST, S_SS_FETCHING_OEM);
		    }
		    break;

    case    E_SS_MESSAGE_ARRIVAL:
            SensorSupervisor_ACTION(pContext, A_SS_PROCESS_MESSAGE);
            break;

	case    E_SS_LINK_IS_UNAVAILABLE:
			SensorSupervisor_ActionStateSequence(pContext, A_SS_T_SENSOR_RESTART, S_SS_INITIAL);
			break;

	case	E_SS_GPIO_INTERRUPT_ARRIVAL:
			SensorSupervisor_ActionActionStateSequence(pContext, A_SS_T_SENSOR_RESTART, A_SS_SEND_ALERT_REQUEST, S_SS_AWAITING_ALERT_RESPONSE);
			break;

	default:
			SensorSupervisorLogUnexpected(pContext, AnyEvent);
			break;
	}
}

void SensorSupervisor_FSM_S_SS_AWAITING_ALERT_RESPONSE(LOCAL_SENSOR_SUPERVISOR_THREAD_CONTEXT_TYPE* pContext, uint8_t AnyEvent)
{
	//bool IsEnabled;
	switch (AnyEvent)
	{
	case    E_SS_T_SENSOR_EXPIRY:
            checkIfExpectedMessageDidntArrive(pContext, SS_EXPECTED_MSG_NONE);
			SensorSupervisor_ActionActionStateSequence(pContext, A_SS_T_SENSOR_START, A_SS_SEND_OEM_CMD_REQUEST, S_SS_FETCHING_OEM);
			break;

    case    E_SS_MESSAGE_ARRIVAL:
            SensorSupervisor_ACTION(pContext, A_SS_PROCESS_MESSAGE);
            break;

	case    E_SS_LINK_IS_UNAVAILABLE:
			SensorSupervisor_ActionStateSequence(pContext, A_SS_T_SENSOR_RESTART, S_SS_INITIAL);
			break;

	case	E_SS_GPIO_INTERRUPT_ARRIVAL:
			SensorSupervisor_ActionActionStateSequence(pContext, A_SS_T_SENSOR_RESTART, A_SS_SEND_ALERT_REQUEST, S_SS_AWAITING_ALERT_RESPONSE);
			break;

	default:
			SensorSupervisorLogUnexpected(pContext, AnyEvent);
			break;
	}
}

void SensorSupervisor_FSM_S_SS_AWAITING_SENSOR_STATE_RESPONSE(LOCAL_SENSOR_SUPERVISOR_THREAD_CONTEXT_TYPE* pContext, uint8_t AnyEvent)
{
	//bool IsEnabled;
	switch (AnyEvent)
	{
	case    E_SS_T_SENSOR_EXPIRY:
            checkIfExpectedMessageDidntArrive(pContext, SS_EXPECTED_MSG_NONE);
			SensorSupervisor_ActionActionStateSequence(pContext, A_SS_T_SENSOR_START, A_SS_SEND_OEM_CMD_REQUEST, S_SS_FETCHING_OEM);
			break;

    case    E_SS_MESSAGE_ARRIVAL:
            SensorSupervisor_ACTION(pContext, A_SS_PROCESS_MESSAGE);
            break;

	case    E_SS_LINK_IS_UNAVAILABLE:
			SensorSupervisor_ActionStateSequence(pContext, A_SS_T_SENSOR_RESTART, S_SS_INITIAL);
			break;

	case	E_SS_GPIO_INTERRUPT_ARRIVAL:
			SensorSupervisor_ActionActionStateSequence(pContext, A_SS_T_SENSOR_RESTART, A_SS_SEND_ALERT_REQUEST, S_SS_AWAITING_ALERT_RESPONSE);
			break;

	default:
			SensorSupervisorLogUnexpected(pContext, AnyEvent);
			break;
	}
}







void SensorSupervisor_FSM(LOCAL_SENSOR_SUPERVISOR_THREAD_CONTEXT_TYPE *pContext, uint8_t AnyEvent)
{
    SensorSupervisor_CLEAR_ACTION(pContext);

    pContext->Event=AnyEvent;
    Watch_Set(pContext->pWatchPointContext, W_SENSOR_SUPERVISOR_LAST_EVENT, (uint32_t)pContext->Event);

    switch(pContext->State)
    {
        case    S_SS_INITIAL:
                SensorSupervisor_FSM_S_SS_INITIAL(pContext, AnyEvent);
                break;

        case    S_SS_FETCHING_BOARD_INFORMATION:
                SensorSupervisor_FSM_S_SS_FETCHING_BOARD_INFORMATION(pContext, AnyEvent);
                break;


        case    S_SS_FETCHING_SENSOR_VOLTAGE:
                SensorSupervisor_FSM_S_SS_FETCHING_SENSOR_VOLTAGE(pContext, AnyEvent);
                break;


        case    S_SS_FETCHING_SENSOR_POWER:
                SensorSupervisor_FSM_S_SS_FETCHING_SENSOR_POWER(pContext, AnyEvent);
                break;


        case    S_SS_FETCHING_SENSOR_TEMPERATURE:
                SensorSupervisor_FSM_S_SS_FETCHING_SENSOR_TEMPERATURE(pContext, AnyEvent);
                break;


        case    S_SS_RELAYING_HBM:
                SensorSupervisor_FSM_S_SS_RELAYING_HBM(pContext, AnyEvent);
                break;


        case    S_SS_RELAYING_CAGE_INFORMATION:
                SensorSupervisor_FSM_S_SS_RELAYING_CAGE_INFORMATION(pContext, AnyEvent);
                break;

		case    S_SS_RELAYING_CAGE_INFORMATION_COMPLETE:
				SensorSupervisor_FSM_S_SS_RELAYING_CAGE_INFORMATION_COMPLETE(pContext, AnyEvent);
				break;

		case    S_SS_AWAITING_ALERT_RESPONSE:
				SensorSupervisor_FSM_S_SS_AWAITING_ALERT_RESPONSE(pContext, AnyEvent);
				break;

		case    S_SS_AWAITING_SENSOR_STATE_RESPONSE:
				SensorSupervisor_FSM_S_SS_AWAITING_SENSOR_STATE_RESPONSE(pContext, AnyEvent);
				break;

		case	S_SS_FETCHING_OEM:
				SensorSupervisor_FSM_S_SS_FETCHING_OEM(pContext, AnyEvent);
				break;

        case    S_SS_FETCHING_POWER_THROTTLING_THRESHOLDS:
                SensorSupervisor_FSM_S_SS_FETCHING_POWER_THROTTLING_THRESHOLDS(pContext, AnyEvent);
                break;

        case    S_SS_FETCHING_TEMP_THROTTLING_THRESHOLDS:
                SensorSupervisor_FSM_S_SS_FETCHING_TEMP_THROTTLING_THRESHOLDS(pContext, AnyEvent);
                break;

        case    S_SS_DEBUG_UART_ENABLE:
                SensorSupervisor_FSM_S_SS_DEBUG_UART_ENABLE(pContext, AnyEvent);
                break;

        case    S_SS_RELAY_PCIE:
                SensorSupervisor_FSM_S_SS_RELAY_PCIE(pContext, AnyEvent);
                break;

        case    S_SS_RELAY_ECC:
                SensorSupervisor_FSM_S_SS_RELAY_ECC(pContext, AnyEvent);
                break;

        case    S_SS_KEEPALIVE:
                SensorSupervisor_FSM_S_SS_KEEPALIVE(pContext, AnyEvent);
                break;

        case    S_SS_COLLECT_CSDR:
                SensorSupervisor_FSM_S_SS_COLLECT_CSDR(pContext, AnyEvent);
                break;

        case S_SS_QSFP_REQUEST:
                SensorSupervisor_FSM_S_SS_QSFP_MESSAGE(pContext, AnyEvent);
                break;
        
        case S_SS_INTERRUPT_STATUS_REQUEST:
                SensorSupervisor_FSM_S_SS_INTERRUPT_STATUS_REQUEST(pContext, AnyEvent);
                break;

        default:
                SensorSupervisor_NextStateDecoder(pContext, S_SS_INITIAL);
                Watch_Inc(pContext->pWatchPointContext, W_SENSOR_SUPERVISOR_FSM_ILLEGAL_STATE);
                break;
    }

    SensorSupervisor_HandleAction(pContext);
}

