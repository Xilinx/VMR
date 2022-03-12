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
 *  $Change: 3205276 $
 *  $Date: 2021/05/04 $
 *  $Revision: #60 $
 *
 */




#ifndef _CMC_WATCHPOINT_LIST_H_
#define _CMC_WATCHPOINT_LIST_H_


#ifdef __cplusplus
extern "C"
{
#endif

#include "cmc_common.h"



typedef enum CMC_WATCHPOINT_TYPE
{
    W_LINK_STATE_LAST_EVENT = 0,                                //0x04D0,  CMC_WATCHPOINT_BASE_REG
    W_LINK_STATE_CURRENT,                                       //0x04D4,
    W_LINK_STATE_PREVIOUS,                                      //0x04D8,
    W_LINK_STATE_FSM_ILLEGAL_STATE,                             //0x04DC,
    W_LINK_STATE_UNEXPECTED_EVENT,                              //0x04E0,
    W_LINK_STATE_UNEXPECTED_ACTION,                             //0x04E4,
    W_LINK_STATE_FSM_FAILED_TO_ADD_EVENT,                       //0x04E8,
    W_BOOTLOADER_LAST_EVENT,                                    //0x04EC,
    W_BOOTLOADER_CURRENT,                                       //0x04F0,
    W_BOOTLOADER_PREVIOUS,                                      //0x04F4,
    W_BOOTLOADER_FSM_ILLEGAL_STATE,                             //0x04F8,
    W_BOOTLOADER_UNEXPECTED_EVENT,                              //0x04FC,
    W_BOOTLOADER_UNEXPECTED_ACTION,                             //0x0500,
    W_BOOTLOADER_FSM_FAILED_TO_ADD_EVENT,                       //0x0504,
    W_THREAD_LOOP_ACTIVITY,                                     //0x0508,
    W_THREAD_TIMER,                                             //0x050C,
    W_THREAD_WATCHDOG,                                          //0x0510,
    W_THREAD_HOST_USER_PROXY,                                   //0x0514,
    W_THREAD_LOCAL_BOOTLOADER,                                  //0x0518,
    W_THREAD_LOCAL_LINK_SUPERVISOR,                             //0x051C,
    W_THREAD_LOCAL_SENSOR_SUPERVISOR,                           //0x0520,
    W_THREAD_LINK_RECEIVE,                                      //0x0524,
    W_THREAD_LINK_PROTOCOL_LOGGER,                              //0x0528,
    W_THREAD_MUTEX_OBSERVER,                                    //0x052C,
    W_THREAD_X2_COMMS,                                          //0x0530,	
    W_SENSOR_SUPERVISOR_LAST_EVENT,                             //0x0534,
    W_SENSOR_SUPERVISOR_CURRENT,                                //0x0538,
    W_SENSOR_SUPERVISOR_PREVIOUS,                               //0x053C,
    W_SENSOR_SUPERVISOR_FSM_ILLEGAL_STATE,                      //0x0540,
    W_SENSOR_SUPERVISOR_UNEXPECTED_EVENT,                       //0x0544,
    W_SENSOR_SUPERVISOR_UNEXPECTED_ACTION,                      //0x0548,
    W_SENSOR_SUPERVISOR_FSM_FAILED_TO_ADD_EVENT,                //0x054C,
    W_SENSOR_SUPERVISOR_MESSAGE_VERSION_UNSUPPORTED,            //0x0550,
    W_HOST_PROXY_LAST_EVENT,                                    //0x0554,
    W_HOST_PROXY_CURRENT,                                       //0x0558,
    W_HOST_PROXY_PREVIOUS,                                      //0x055C,
    W_HOST_PROXY_FSM_ILLEGAL_STATE,                             //0x0560,
    W_HOST_PROXY_UNEXPECTED_EVENT,                              //0x0564,
    W_HOST_PROXY_UNEXPECTED_ACTION,                             //0x0568,
    W_HOST_PROXY_FSM_FAILED_TO_ADD_EVENT,                       //0x056C,
    W_HOST_PROXY_UNRECOGNISED_REMOTE_COMMAND,                   //0x0570,
    W_HOST_PROXY_UNEXPECTED_LOCAL_ACTION,                       //0x0574,
    W_HOST_PROXY_UNEXPECTED_REMOTE_ACTION,                      //0x0578,
    W_LINK_RECEIVE_NO_ROOM_FOR_OCTET,                           //0x057C,

    // 2nd RAM section
    W_INTERRUPT_ENTER,                                          //0x0C94,       CMC_WATCHPOINT2_BASE_REG
    W_INTERRUPT_EXIT,                                           //0x0C98,
    W_INTERRUPT_SOURCE_UART_LITE_SATELLITE,                     //0x0C9C,
    W_INTERRUPT_SOURCE_CAGE_IO,                                 //0x0CA0,
    W_INTERRUPT_SOURCE_WDT,                                     //0x0CA4,
    W_INTERRUPT_SOURCE_GPIO_SATELLITE,                          //0x0CA8,
    W_HEARTBEAT_SINGLE_SNSR_ERR_CNT,                            //0x0CAC,
    W_INTERRUPT_IGNORED_SOURCES,                                //0x0CB0,
    W_SENSOR_RELAY_TRY_ADD_FAIL,                                //0x0CB4,
    W_SENSOR_RELAY_TRY_REMOVE_FAIL,                             //0x0CB8,
    W_SENSOR_ERROR_COUNT_REG,                                   //0x0CBC,
    W_BOARDINFO_SIZE_ERROR_COUNT_REG,                           //0x0CC0,
    W_LINK_RECEIVE_GOOD_MSG_UNHANDLED_MSG_ID,                   //0x0CC4,
    W_LINK_RECEIVE_ERR_MSG_UNHANDLED_MSG_ID,                    //0x0CC8,
    W_LINK_RECEIVE_MSG_RECEIVED_UNKNOWN_MSG_ID,                 //0x0CCC,
    W_LINK_RECEIVE_NEGOTIATED_VERSION,                          //0x0CD0,
    W_HBM_SUPPORTED,                                            //0x0CD4,
    W_HBM_PAYLOAD,                                              //0x0CD8,
    W_HEARTBEAT_MULTIPLE_SNSR_ERR_CNT,                          //0x0CDC,
    W_SCHEDULER_ENTER,                                          //0x0CE0,
    W_SCHEDULER_EXIT,                                           //0x0CE4,
    W_HOST_REGISTER_RANGE_VIOLATION,                            //0x0CE8,
    W_RUNTIME_THREAD_LOOP_ACTIVITY,                             //0x0CEC,
    W_RUNTIME_THREAD_TIMER,                                     //0x0CF0,
    W_RUNTIME_THREAD_WATCHDOG,                                  //0x0CF4,
    W_RUNTIME_THREAD_HOST_USER_PROXY,                           //0x0CF8,
    W_RUNTIME_THREAD_LOCAL_BOOTLOADER,                          //0x0CFC,
    W_RUNTIME_THREAD_LOCAL_LINK_SUPERVISOR,                     //0x0D00,
    W_RUNTIME_THREAD_LOCAL_SENSOR_SUPERVISOR,                   //0x0D04,
    W_RUNTIME_THREAD_LINK_RECEIVE,                              //0x0D08,
    W_RUNTIME_THREAD_LINK_PROTOCOL_LOGGER,                      //0x0D0C,
    W_RUNTIME_THREAD_MUTEX_OBSERVER,                            //0x0D10,
    W_RUNTIME_THREAD_X2_COMMS,                                  //0x0D14,
    W_MAX_RUNTIME_THREAD_LOOP_ACTIVITY,                         //0x0D18,
    W_MAX_RUNTIME_THREAD_TIMER,                                 //0x0D1C,
    W_MAX_RUNTIME_THREAD_WATCHDOG,                              //0x0D20,
    W_MAX_RUNTIME_THREAD_HOST_USER_PROXY,                       //0x0D24,
    W_MAX_RUNTIME_THREAD_LOCAL_BOOTLOADER,                      //0x0D28,
    W_MAX_RUNTIME_THREAD_LOCAL_LINK_SUPERVISOR,                 //0x0D2C,           
    W_MAX_RUNTIME_THREAD_LOCAL_SENSOR_SUPERVISOR,               //0x0D30,
    W_MAX_RUNTIME_THREAD_LINK_RECEIVE,                          //0x0D34,
    W_MAX_RUNTIME_THREAD_LINK_PROTOCOL_LOGGER,                  //0x0D38,
    W_MAX_RUNTIME_THREAD_MUTEX_OBSERVER,                        //0x0D3C,
    W_MAX_RUNTIME_THREAD_X2_COMMS,                              //0x0D40,         
    W_ARBITRARY_TIME_DIFFERENCE_CURRENT,                        //0x0D44,    
    W_ARBITRARY_TIME_DIFFERENCE_MAX,                            //0x0D48,
    W_SENSOR_SUPERVISOR_LATE_MESSAGE_ARRIVAL,                   //0x0D4C,
    W_SENSOR_SUPERVISOR_MESSAGE_DIDNT_ARRIVE,                   //0x0D50,
    W_LINK_RECEIVE_DEBUG_CAPTURE_POSITION,                      //0x0D54,
    W_MUTEX_REACHOUT_GRANTED,                                   //0x0D58,
    W_MUTEX_ACCESSING_REACHOUT_INTERFACE,                       //0x0D5C,
    W_X2_COMMS_FSM_LAST_EVENT,                                  //0x0D60,
    W_X2_COMMS_FSM_CURRENT_STATE,                               //0x0D64,
    W_X2_COMMS_FSM_PREVIOUS_STATE,                              //0x0D68,
    W_X2_COMMS_FSM_ILLEGAL_STATE,                               //0x0D6C,
    W_X2_COMMS_FSM_UNEXPECTED_EVENT,                            //0x0D70,
    W_X2_COMMS_FSM_UNEXPECTED_ACTION,                           //0x0D74,
    W_X2_COMMS_FSM_FAILED_TO_ADD_EVENT,                         //0x0D78,
    W_X2_COMMS_REQUEST_ALREADY_IN_PROGRESS,                     //0x0D7C,
    W_X2_COMMS_RESPONSE_SEQUENCE_TIMEOUT,                       //0x0D80,
    W_INTERRUPT_SOURCE_I2C,                                     //0x0D84,
    W_I2C_STATUS_HANDLER_COUNT,                                 //0x0D88,
    W_I2C_MASTER_WRITE_TO_SLAVE_COUNT,                          //0x0D8C,
    W_I2C_MASTER_READ_FROM_SLAVE_COUNT,                         //0x0D90,
    W_INVALID_SENSOR_ID_RECEIVED,                               //0x0D94
    W_INVALID_SENSOR_ID_FOR_PROFILE_0_31_RECEIVED,              //0x0D98
    W_INVALID_SENSOR_ID_FOR_PROFILE_32_63_RECEIVED,             //0x0D9C
    W_INVALID_SENSOR_ID_FOR_PROFILE_64_95_RECEIVED,             //0x0DA0
    W_INVALID_SENSOR_ID_FOR_PROFILE_96_127_RECEIVED,            //0x0DA4
    W_CSDR_EXPECTED_MSG_DIDNT_ARRIVE,                           //0x0DA8
    W_CSDR_ERROR_IN_MESSAGE,                                    //0x0DAC
    W_SUC_TRANSPORT_TX_INVALID_STATE,                           //0x0DB0
    W_SUC_TRANSPORT_RX_INVALID_START_BYTE,                      //0x0DB4
    W_SUC_TRANSPORT_RX_FAILED_VALIDATION,                       //0x0DB8
    W_SUC_TRANSPORT_RX_HEADER_CRC_FAILED,                       //0x0DBC
    W_SUC_TRANSPORT_RX_PAYLOAD_CRC_FAILED,                      //0x0DC0
    W_SUC_TRANSPORT_RX_INVALID_STATE,                           //0x0DC4
    W_INVALID_SENSOR_SIZE,                                      //0x0DC8
    W_STACK_FILL_PERCENTAGE,                                    //0x0DCC
    W_SUC_BYTES_RX,                                             //0x0DD0

    // 3rd RAM section      
    W_CT_MODE,                                                  //0x1C00,       CMC_WATCHPOINT3_BASE_REG
    W_CT_BOARDMEASUREDPOWER,                                    //0x1C04,
    W_CT_USERNORMALIZEDPOWER,                                   //0x1C08,
    W_CT_XRTSUPPLIEDBOARDTHROTTLINGTHRESHOLDPOWER,              //0x1C0C,
    W_CT_BOARDNORMALIZEDPOWER,                                  //0x1C10,
    W_CT_KERNELPOWER,                                           //0x1C14,
    W_CT_KERNELTARGET,                                          //0x1C18,
    W_CT_IDLEPOWER,                                             //0x1C1C,
    W_CT_ACTIVITY,                                              //0x1C20,          
    W_CT_RATECURRENT,                                           //0x1C24,
    W_CT_RATELINEAR,                                            //0x1C28,
    W_CT_0_VOLTAGESENSORID,                                     //0x1C2C,
    W_CT_0_CURRENTSENSORID,                                     //0x1C30,
    W_CT_0_DUPLICATEREADINGCURRENT,                             //0x1C34,
    W_CT_0_LATESTREADINGCURRENT,                                //0x1C38,
    W_CT_0_LATESTREADINGVOLTAGE,                                //0x1C3C,
    W_CT_0_LATESTREADINGTHRESHCURRENT,                          //0x1C40,
    W_CT_0_NOMINALVOLTAGE,                                      //0x1C44,
    W_CT_0_MEASUREDPOWER,                                       //0x1C48,
    W_CT_0_THROTTLEDTHRESHOLDPOWER,                             //0x1C4C,
    W_CT_0_CONTRIBUTESTOBOARDTHROTTLINGPOWER,                   //0x1C50,
    W_CT_0_RAILNORMALIZEDPOWER,                                 //0x1C54,
    W_CT_1_VOLTAGESENSORID,                                     //0x1C58,
    W_CT_1_CURRENTSENSORID,                                     //0x1C5C,
    W_CT_1_DUPLICATEREADINGCURRENT,                             //0x1C60,
    W_CT_1_LATESTREADINGCURRENT,                                //0x1C64,
    W_CT_1_LATESTREADINGVOLTAGE,                                //0x1C68,
    W_CT_1_LATESTREADINGTHRESHCURRENT,                          //0x1C6C,
    W_CT_1_NOMINALVOLTAGE,                                      //0x1C70,
    W_CT_1_MEASUREDPOWER,                                       //0x1C74,
    W_CT_1_THROTTLEDTHRESHOLDPOWER,                             //0x1C78,
    W_CT_1_CONTRIBUTESTOBOARDTHROTTLINGPOWER,                   //0x1C7C,
    W_CT_1_RAILNORMALIZEDPOWER,                                 //0x1C80,
    W_CT_2_VOLTAGESENSORID,                                     //0x1C84,
    W_CT_2_CURRENTSENSORID,                                     //0x1C88,
    W_CT_2_DUPLICATEREADINGCURRENT,                             //0x1C8C,
    W_CT_2_LATESTREADINGCURRENT,                                //0x1C90,
    W_CT_2_LATESTREADINGVOLTAGE,                                //0x1C94,                                                               
    W_CT_2_LATESTREADINGTHRESHCURRENT,                          //0x1C98,
    W_CT_2_NOMINALVOLTAGE,                                      //0x1C9C,
    W_CT_2_MEASUREDPOWER,                                       //0x1CA0,  
    W_CT_2_THROTTLEDTHRESHOLDPOWER,                             //0x1CA4,
    W_CT_2_CONTRIBUTESTOBOARDTHROTTLINGPOWER,                   //0x1CA8,
    W_CT_2_RAILNORMALIZEDPOWER,                                 //0x1CAC,
    W_CT_FPGAMEASUREDTEMP,                                      //0x1CB0,
    W_CT_FPGATHROTTLINGTEMPLIMIT,                               //0x1CB4,
    W_CT_BVCCINTTHERMALTHROTTLINGENABLED,                       //0x1CB8,
    W_CT_VCCINTMEASUREDTEMP,                                    //0x1CBC,
    W_CT_VCCINTTHROTTLINGTEMPLIMIT,                             //0x1CC0,
    W_CT_BUSERTHROTTLINGTEMPLIMITENABLED,                       //0x1CC4,
    W_CT_XRTSUPPLIEDUSERTHROTTLINGTEMPLIMIT,                    //0x1CC8,
    W_CT_THERMALTHROTTLINGTHRESHOLDPOWER,                       //0x1CCC,
    W_CT_THERMALNORMALISEDPOWER,                                //0x1CD0,
    W_CT_VCCINTTHERMALNORMALISEDPOWER,                          //0x1CD4,
    W_CT_THERMALTHROTTLINGLOOPJUSTENABLED,                      //0x1CD8,
    W_CT_INTEGTAIONSUM,                                         //0x1CDC,
    W_CT_TEMPGAINKPFPGA,                                        //0x1CE0,
    W_CT_TEMPGAINKI,                                            //0x1CE4,
    W_CT_TEMPGAINKPVCCINT,                                      //0x1CE8,
    W_CT_TEMPGAINKAW,                                           //0x1CEC,




    W_MAX_WATCHPOINTS           

} CMC_WATCHPOINT_TYPE;



#ifdef __cplusplus
}
#endif


#endif /* _CMC_WATCHPOINT_LIST_H_ */




