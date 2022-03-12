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
 *  $Revision: #6 $
 *
 */


#ifndef CMC_THREAD_X2_COMMS_FSM_H
#define CMC_THREAD_X2_COMMS_FSM_H



#include <stdint.h>



#ifdef __cplusplus
extern "C"
{
#endif





typedef enum CMC_X2_FSM_STATE
{
    STATE_IDLE = 0,
    STATE_WAITING_FOR_X2_READ_STATUS,
    STATE_WAITING_FOR_X2_READ_REQUEST_LENGTH,
    STATE_WAITING_FOR_X2_READ_REQUEST_DATA,
    STATE_WAITING_FOR_X2_RESPONSE_LENGTH,
    STATE_WAITING_FOR_X2_RESPONSE_DATA,

    STATE_MAX_STATES

}CMC_X2_FSM_STATE;







typedef enum CMC_X2_FSM_EVENT
{
    EVENT_RESET_REQUEST,

    EVENT_TIMER_EXPIRY,

    EVENT_NEW_USER_REQUEST_ARRIVED,
    
    EVENT_X2_READ_STATUS,
    EVENT_X2_READ_REQUEST_LENGTH,
    EVENT_X2_READ_REQUEST_DATA,
    EVENT_X2_WROTE_RESPONSE_LENGTH,
    EVENT_X2_WROTE_RESPONSE_DATA,

    CMC_X2_MAX_EVENTS

}CMC_X2_FSM_EVENT;










typedef enum CMC_X2_FSM_ACTION
{
    ACTION_START_TIMER = 0,
    ACTION_STOP_TIMER,
    ACTION_RESTART_TIMER,

    ACTION_COPY_AND_SIGNAL_NEW_REQUEST,

    ACTION_DEASSERT_GPIO,
  
    ACTION_CLEAR_STATUS,

    ACTION_SEND_RESPONSE,

    ACTION_INCREMENT_RESPONSE_TIMEOUT_COUNT,
    ACTION_INCREMENT_REQUEST_ALREADY_IN_PROGRESS_COUNT,

    ACTION_RAISE_RESET_REQUEST,

    CMC_X2_MAX_ACTIONS

}CMC_X2_FSM_ACTION;
















#ifdef __cplusplus
}
#endif




#endif /* CMC_THREAD_X2_COMMS_FSM_H */


