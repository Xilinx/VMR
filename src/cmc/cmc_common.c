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
 *  $Revision: #13 $
 *
 */





#include "cmc_common.h"

void cmcU32ToU8(uint32_t input, uint8_t* output)
{
	*output++ = (uint8_t)(input & 0xFF);
	*output++ = (uint8_t)((input & 0xFF00) >> 8);
	*output++ = (uint8_t)((input & 0xFF0000) >> 16);
	*output = (uint8_t)((input & 0xFF000000) >> 24);
}

void cmcU16ToU8(uint16_t input, uint8_t* output)
{
	*output++ = (uint8_t)(input & 0xFF);
	*output++ = (uint8_t)((input & 0xFF00) >> 8);
}

uint16_t cmcU8ToU16(uint8_t* payload) 
{
	return (uint16_t)payload[0] | (((uint16_t)payload[1]) << 8);
}

uint32_t cmcU8ToU32(uint8_t* payload)
{
	return (uint32_t)payload[0] | (((uint32_t)payload[1]) << 8) | (((uint32_t)payload[2]) << 16) | (((uint32_t)payload[3]) << 24);
}

uint16_t calculateChecksum(uint8_t* msgBuffer, uint16_t length)
{
	uint16_t i;
	uint16_t checksum = 0;

	for (i = 0; i < length; ++i)
	{
		checksum += msgBuffer[i];
	}

	return checksum;
}

void cmcMemCopy(char* destination, const char* source, size_t length)
{
	size_t i;

	if (source != NULL && destination != NULL)
	{
		for (i = 0; i < length; i++)
		{
			destination[i] = source[i];
		}
	}
}


void* cmcMemSet(void* s, int c, size_t length)
{
	unsigned char* p = s;

	while (length--)
	{
		*p++ = (unsigned char)c;
	}

	return s;
}

