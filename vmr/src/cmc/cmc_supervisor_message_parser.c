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




#include "cmc_supervisor_message_parser.h"
#include "cmc_receive_defines.h"

uint16_t calculate_checksum(uint8_t* msgBuffer, uint8_t length)
{
	uint8_t i;
	uint16_t checksum = 0;

	for (i = 0; i < length; ++i)
	{
		checksum += msgBuffer[i];
	}

	return checksum;
}

void cmc_sensor_supervisor_parser_initialize(   SENSOR_SUPERVISOR_PARSER_CONTEXT_TYPE* pContext,
                                                PERIPHERAL_REGMAP_RAM_CONTROLLER_CONTEXT_TYPE * pRegMap_RAM_Controller_Context)
{
	pContext->msgIDReceived = 0;
	pContext->payloadLength = 0;
    pContext->pPeripheralRegMapRamControllerContext=pRegMap_RAM_Controller_Context;
}

static int cmc_sensor_supervisor_parse_message(SENSOR_SUPERVISOR_PARSER_CONTEXT_TYPE* pContext, uint8_t* pMessage, uint8_t* msgID, uint8_t* payloadLength, bool CRCrequired)
{
	int ret;
	int checksum_pos;
	uint16_t receivedChecksum, calculatedChecksum;

	ret = 0;

	*msgID = pMessage[0];
	*payloadLength = pMessage[2];

	if (CRCrequired)
	{
		checksum_pos = *payloadLength + 3;

		receivedChecksum = (uint16_t)pMessage[checksum_pos] | (((uint16_t)pMessage[checksum_pos + 1]) << 8);
		calculatedChecksum = calculate_checksum(pMessage, *payloadLength + 3);
		if (receivedChecksum != calculatedChecksum)
		{
			ret = -1;
			PERIPHERAL_REGMAP_RAM_CONTROLLER_WriteWithMask(pContext->pPeripheralRegMapRamControllerContext, HOST_REGISTER_ERROR_REG, HOST_ERROR_REG_SAT_RX_ERROR_MASK, SAT_COMMS_CHKSUM_ERR);
			PERIPHERAL_REGMAP_RAM_CONTROLLER_WriteWithMask(pContext->pPeripheralRegMapRamControllerContext, HOST_REGISTER_ERROR_REG, HOST_ERROR_REG_SAT_COMMS_ERROR, HOST_ERROR_REG_SAT_COMMS_ERROR);
		}
	}
		
	return ret;
}


int cmc_sensor_supervisor_process_message(SENSOR_SUPERVISOR_PARSER_CONTEXT_TYPE *pContext, uint8_t* message, bool CRCrequired)
{
	uint8_t payloadLength, msgID;
	int ret;

	ret = cmc_sensor_supervisor_parse_message(pContext, message, &msgID, &payloadLength, CRCrequired);
	pContext->msgIDReceived = msgID;
	pContext->payloadLength = payloadLength;
	pContext->pMessageBody = &message[3];

	return ret;
}