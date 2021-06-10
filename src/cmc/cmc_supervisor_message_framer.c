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
 *  $Change: 2680491 $
 *  $Date: 2019/10/01 $
 *  $Revision: #9 $
 *
 */




#include "cmc_supervisor_message_framer.h"

static void add_header(uint8_t* pInputMessage, uint8_t inputMessageLength, uint8_t* pOutputFormattedMessage, uint8_t* pOutputFormattedMessageLength)
{
	uint8_t SOP[] = { ESC_CHAR, SOP_CHAR };
	uint8_t EOP[] = { ESC_CHAR, EOP_CHAR };


	*pOutputFormattedMessageLength = 0;

	/*
	 * Header
	 */

	 /* Start Of Message */
	cmcMemCopy((char*)pOutputFormattedMessage, (char*)SOP, sizeof(SOP));
	pOutputFormattedMessage += sizeof(SOP);
	*pOutputFormattedMessageLength += sizeof(SOP);

	/* Add the formatted message */
	cmcMemCopy((char*)pOutputFormattedMessage, (char*)pInputMessage, inputMessageLength);
	pOutputFormattedMessage += inputMessageLength;
	*pOutputFormattedMessageLength += inputMessageLength;

	/* End Of Message */
	cmcMemCopy((char*)pOutputFormattedMessage, (char*)EOP, sizeof(EOP));
	pOutputFormattedMessage += sizeof(EOP);
	*pOutputFormattedMessageLength += sizeof(EOP);
}

static void process_escape_char(uint8_t* pMessage, uint8_t* pMessageLength)
{
	uint8_t i, j, esc_found;
	esc_found = 0;

	for (i = 0; i < *pMessageLength; i++)
	{
		if (pMessage[i] == ESC_CHAR)
		{
			for (j = *pMessageLength; j >= i; j--)
			{
				pMessage[j + 1] = pMessage[j];
			}
			pMessage[++i] = ESC_CHAR;
			esc_found++;

		}
	}

	*pMessageLength += esc_found;
}


void cmc_frame_formatted_message(uint8_t* pMessage, uint8_t* pMessageLength, uint8_t* pFormattedMessage, uint8_t* pFormattedMessageLength)
{
	process_escape_char(pMessage, pMessageLength);
	add_header(pMessage, *pMessageLength, pFormattedMessage, pFormattedMessageLength);
}