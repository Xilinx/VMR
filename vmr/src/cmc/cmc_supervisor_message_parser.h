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
 *  $Revision: #16 $
 *
 */




#ifndef _CMC_SUPERVISOR_PARSER_H_
#define _CMC_SUPERVISOR_PARSER_H_


#ifdef __cplusplus
extern "C"
{
#endif

#include "cmc_common.h"
#include "cmc_peripherals.h"

	/* size is SOP(2)+ID(1*2)+FLAGS(1*2)+PAYLOAD_LENGTH(1*2)+PAYLOAD(256*2)+CHKSUM(2*2)+EOP(2)
	need to double message sections where an '\' can be used */
#define SAT_COMMS_MAX_MSG_SIZE  (2+2+2+2+512+4+2)
#define SAT_COMMS_MIN_MSG_SIZE  9

typedef struct SENSOR_SUPERVISOR_PARSER_CONTEXT_TYPE
{
	PERIPHERAL_REGMAP_RAM_CONTROLLER_CONTEXT_TYPE* pPeripheralRegMapRamControllerContext;
	uint8_t msgIDReceived;
	uint8_t payloadLength;
	uint8_t* pMessageBody;

} SENSOR_SUPERVISOR_PARSER_CONTEXT_TYPE;

void cmc_sensor_supervisor_parser_initialize(   SENSOR_SUPERVISOR_PARSER_CONTEXT_TYPE* pContext,
                                                PERIPHERAL_REGMAP_RAM_CONTROLLER_CONTEXT_TYPE * pRegMap_RAM_Controller_Context);
int cmc_sensor_supervisor_process_message(SENSOR_SUPERVISOR_PARSER_CONTEXT_TYPE* pContext, uint8_t* message, bool CRCrequired);

#ifdef __cplusplus
}
#endif


#endif /* _CMC_SUPERVISOR_PARSER_H_ */











