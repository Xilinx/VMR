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
 *  $Change: 2773736 $
 *  $Date: 2020/02/07 $
 *  $Revision: #8 $
 *
 */


#ifndef CMC_THREAD_X2_COMMS_H
#define CMC_THREAD_X2_COMMS_H



#include <stdint.h>

#include "cmc_common.h"
#include "cmc_circular_buffer.h"
#include "cmc_fsm_state_transition_logger.h"
#include "cmc_thread_timer.h"
#include "cmc_watchpoint.h"

#include "cmc_peripheral_x2_interface_i2c_device.h"
#include "cmc_peripheral_x2_interface_gpio.h"
#include "cmc_x2_virtual_register_set.h"

#include "cmc_thread_x2_comms_fsm.h"
#include "cmc_thread_x2_comms_constants.h"




#ifdef __cplusplus
extern "C"
{
#endif



typedef void (*CMC_X2_RESPONSE_PUSH_CALLBACK_TYPE)(void* pResponseContext, uint8_t* pResponseData, uint8_t dataLength);

typedef void (*CMC_X2_RESET_CALLBACK_TYPE)(void* pResetContext);


typedef struct CMC_X2_COMMS_CONTEXT
{
    FSM_STATE_TRANSITION_LOGGER_CONTEXT_TYPE* pFSM_StateTransitionLoggerContext;
    CMC_X2_FSM_EVENT lastEventReceived;
    CMC_X2_FSM_STATE currentState;
    CMC_X2_FSM_STATE previousState;
    uint32_t actions;

    THREAD_TIMER_CONTEXT_TYPE* pThreadTimerContext;
    uint32_t currentTimerInstance;

    CMC_WATCHPOINT_CONTEXT_TYPE* pWatchPointContext;

    CIRCULAR_BUFFER_TYPE            eventBuffer;
    CIRCULAR_BUFFER_ELEMENT_TYPE    eventBufferElements[X2_COMMS_MAX_EVENT_BUFFER_ELEMENTS];


    uint32_t pendingRequestLength;
    uint8_t pendingRequestData[X2_COMMS_MAX_REQUEST_DATA_SIZE];

    CMC_X2_RESPONSE_PUSH_CALLBACK_TYPE responseCallback;
    void* pResponseContext;

    CMC_X2_RESET_CALLBACK_TYPE resetCallback;
    void* pResetContext;

    CMC_X2_VIRTUAL_REGISTER_SET_CONTEXT_TYPE    virtualRegisterSet;
   
    PERIPHERAL_X2_I2C_CONTEXT_TYPE*  pI2C;
    PERIPHERAL_X2_GPIO_CONTEXT_TYPE* pGPIO;

}CMC_X2_COMMS_CONTEXT;











bool X2_Comms_Thread_Schedule(void* pContext);

void X2_Comms_Thread_Initialize(CMC_X2_COMMS_CONTEXT* pContext, 
                                THREAD_TIMER_CONTEXT_TYPE* pThreadTimerContext,
                                CMC_WATCHPOINT_CONTEXT_TYPE* pWatchPointContext,
                                FSM_STATE_TRANSITION_LOGGER_CONTEXT_TYPE* pFSM_StateTransitionLoggerContext,
                                PERIPHERAL_X2_I2C_CONTEXT_TYPE* pI2CContext,
                                PERIPHERAL_X2_GPIO_CONTEXT_TYPE* pGPIOContext,
                                CMC_FIRMWARE_VERSION_CONTEXT_TYPE* pFirmwareVersionContext);




/* Call this function to bind a callback that will be called when a valid response is received from the X2 */
void CMC_X2_SetResponseCallback(CMC_X2_COMMS_CONTEXT* pContext, CMC_X2_RESPONSE_PUSH_CALLBACK_TYPE callback, void* pCallbackContext);


//* Call this function to bind a callback that will be called whenever the X2 MC sends a "reset" request */
void CMC_X2_SetResetCallback(CMC_X2_COMMS_CONTEXT* pContext, CMC_X2_RESET_CALLBACK_TYPE callback, void* pCallbackContext);


/* Call this function to pass a new request message */
void CMC_X2_NewUserRequest(CMC_X2_COMMS_CONTEXT* pContext, uint8_t* pRequestData, uint8_t requestDataLength);





void CMC_X2_FSM_Run(CMC_X2_COMMS_CONTEXT* pContext, CMC_X2_FSM_EVENT event);



void CMC_X2_FSM_ClearActions(CMC_X2_COMMS_CONTEXT* pContext);
void CMC_X2_FSM_AddAction(CMC_X2_COMMS_CONTEXT* pContext, CMC_X2_FSM_ACTION action);
void CMC_X2_FSM_PerformActions(CMC_X2_COMMS_CONTEXT* pContext);




void CMC_X2_FSM_AddEvent(CMC_X2_COMMS_CONTEXT* pContext, CMC_X2_FSM_EVENT event);



/* State Handlers */
void CMC_X2_IDLE_StateHandler(CMC_X2_COMMS_CONTEXT* pContext, CMC_X2_FSM_EVENT event);
void CMC_X2_WAITING_FOR_X2_READ_STATUS_StateHandler(CMC_X2_COMMS_CONTEXT* pContext, CMC_X2_FSM_EVENT event);
void CMC_X2_WAITING_FOR_X2_READ_REQUEST_LENGTH_StateHandler(CMC_X2_COMMS_CONTEXT* pContext, CMC_X2_FSM_EVENT event);
void CMC_X2_WAITING_FOR_X2_READ_REQUEST_DATA_StateHandler(CMC_X2_COMMS_CONTEXT* pContext, CMC_X2_FSM_EVENT event);
void CMC_X2_WAITING_FOR_X2_RESPONSE_LENGTH_StateHandler(CMC_X2_COMMS_CONTEXT* pContext, CMC_X2_FSM_EVENT event);
void CMC_X2_WAITING_FOR_X2_RESPONSE_DATA_StateHandler(CMC_X2_COMMS_CONTEXT* pContext, CMC_X2_FSM_EVENT event);




/* Timer Functions */
void CMC_X2_StartTimer(CMC_X2_COMMS_CONTEXT* pContext);
void CMC_X2_StopTimer(CMC_X2_COMMS_CONTEXT* pContext);
void CMC_X2_RestartTimer(CMC_X2_COMMS_CONTEXT* pContext);
void CMC_X2_TimerCallback(void* pCallbackContext, uint32_t timerInstance);


/* Interrupt Callbacks */
void CMC_X2_WriteRequestCallback(void* callbackContext, uint8_t* pData, uint32_t numBytes);
void CMC_X2_ReadRequestCallback(void* callbackContext, uint8_t* pBuffer, uint32_t bufferSize, uint32_t* pNumValidBytes);

void CMC_X2_RaiseWriteEvent(CMC_X2_COMMS_CONTEXT* pContext, uint8_t virtualRegisterAddress);
void CMC_X2_RaiseReadEvent(CMC_X2_COMMS_CONTEXT* pContext, uint8_t virtualRegisterAddress);


#ifdef __cplusplus
}
#endif





#endif /* CMC_THREAD_X2_COMMS_H */

