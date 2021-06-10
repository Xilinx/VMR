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
 *  $Change: 3136694 $
 *  $Date: 2021/03/02 $
 *  $Revision: #12 $
 *
 */




#ifndef _CMC_RECEIVE_DEFINES_H_
#define _CMC_RECEIVE_DEFINES_H_


#ifdef __cplusplus
extern "C"
{
#endif

	typedef enum sat_comms_resp
	{
		SAT_COMMS_SC_FW_SECTOR_INFO_RESP = 0x17,
        SAT_COMMS_SNSR_POWER_THROTTLING_THRESHOLDS_RESP =0x19,
        SAT_COMMS_SNSR_TEMP_THROTTLING_THRESHOLDS_RESP = 0x21,
        SAT_COMMS_CSDR_RESP = 0x23,
        SAT_COMMS_READ_QSFP_DIAGNOSTICS_RESP = 0x2A,
        SAT_COMMS_QSFP_VALIDATE_LOW_SPEED_READ_IO_RESP = 0x2E,
        SAT_COMMS_INTERRUPT_STATUS_RESP = 0x30,
        SAT_COMMS_QSFP_READ_SINGLE_BYTE_RESP = 0x33,

		SAT_COMMS_ALERT_RESP = 0x82,
		SAT_COMMS_VERS_RESP,
		SAT_COMMS_BOARD_INFO_RESP = 0x85,
		SAT_COMMS_VOLT_SNSR_RESP,
		SAT_COMMS_POWER_SNSR_RESP,
		SAT_COMMS_TEMP_SNSR_RESP,
		SAT_COMMS_DEBUG_UART_EN_RESP,
		//
		SAT_COMMS_GET_MCTP_MSG_RESP = 0x8E,
		//
		SAT_COMMS_SNSR_STATE_RESP = 0x90,
		SAT_COMMS_GET_VDM_RESP_RESP = 0x91,
		SAT_COMMS_FEATURE_RESP = 0x92,
		SAT_COMMS_SNSR_POLL_FREQ_RESP = 0x93,
		
		//
		SAT_COMMS_SEND_OEM_CMD_RESP = 0xEF,
		//
		SAT_COMMS_MSG_GOOD = 0xFE,
		SAT_COMMS_MSG_ERR
	} sat_comms_resp;

	// satellite controller normal message error codes
	typedef enum SAT_COMMS_ERROR_CODE 
	{
		SAT_COMMS_CHKSUM_ERR = 0x01,
		SAT_COMMS_EOP_ERR,
		SAT_COMMS_SOP_ERR,
		SAT_COMMS_ESQ_SEQ_ERR,
		SAT_COMMS_BAD_MSG_ID,
		SAT_COMMS_BAD_VERSION,
		SAT_COMMS_BAD_CHAN_STATE,
		SAT_COMMS_MAX_ERR
	} SAT_COMMS_ERROR_CODE;



#ifdef __cplusplus
}
#endif


#endif /* _CMC_RECEIVE_DEFINES_H_ */









