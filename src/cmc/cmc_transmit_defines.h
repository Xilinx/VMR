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
 *  $Change: 3165967 $
 *  $Date: 2021/03/31 $
 *  $Revision: #14 $
 *
 */




#ifndef _CMC_TRANSMIT_DEFINES_H_
#define _CMC_TRANSMIT_DEFINES_H_


#ifdef __cplusplus
extern "C"
{
#endif


	typedef enum CMC_REQUEST_MESSAGE_TYPE
	{
		SAT_COMMS_NULL = 0x00,
		SAT_COMMS_EN_BSL,
		SAT_COMMS_ALERT_REQ,
		SAT_COMMS_VERS_REQ,
		SAT_COMMS_SET_VERS,
		SAT_COMMS_BOARD_INFO_REQ,
		SAT_COMMS_VOLT_SNSR_REQ,
		SAT_COMMS_POWER_SNSR_REQ,
		SAT_COMMS_TEMP_SNSR_REQ,
		SAT_COMMS_DEBUG_UART_EN,
		SAT_COMMS_FPGA_I2C_BUS_ARB,
		SAT_COMMS_CAGE_IO_EVENT,
		//SAT_COMMS_QSFP_INFO,
		SAT_COMMS_SNSR_PUSH = 0x0D,
		//SAT_COMMS_GET_MCTP_MSG,
		//SAT_COMMS_SEND_MCTP_RESP,
		//SAT_COMMS_SEND_VDM_MSG,
		//SAT_COMMS_GET_VDM_RESP,
		//SAT_COMMS_FEATURE_REQ,
		//SAT_COMMS_SET_FEATURE,
		SAT_COMMS_SNSR_STATE_REQ = 0x14,
		//SAT_COMMS_SNSR_POLL_FREQ_REQ,
		//SAT_COMMS_SC_FW_SECTOR_INFO_REQ,
        SAT_COMMS_SNSR_POWER_THROTTLING_THRESHOLDS_REQ = 0x18,
        SAT_COMMS_SNSR_TEMP_THROTTLING_THRESHOLDS_REQ = 0x20,
        SAT_COMMS_CSDR_REQ = 0x22,
        //SAT_COMMS_EXTENDED_CSDR_REQ = 0x24,
        SAT_COMMS_ECC_ERROR_REPORT = 0x26,
        SAT_COMMS_PCIE_ERROR_REPORT = 0x27,
		SAT_COMMS_KEEPALIVE = 0x28,
        SAT_COMMS_READ_QSFP_DIAGNOSTICS_REQ = 0x29,

        SAT_COMMS_QSFP_VALIDATE_LOW_SPEED_WRITE_IO_REQ = 0x2C,
        SAT_COMMS_QSFP_VALIDATE_LOW_SPEED_READ_IO_REQ = 0x2D,
        SAT_COMMS_INTERRUPT_STATUS_REQ = 0x2F,

        SAT_COMMS_QSFP_WRITE_SINGLE_BYTE_REQ = 0x31,
        SAT_COMMS_QSFP_READ_SINGLE_BYTE_REQ = 0x32,

		//
		SAT_COMMS_SEND_OEM_CMD = 0x6F,

	} CMC_REQUEST_MESSAGE_TYPE;





#ifdef __cplusplus
}
#endif


#endif /* _CMC_TRANSMIT_DEFINES_H_ */









