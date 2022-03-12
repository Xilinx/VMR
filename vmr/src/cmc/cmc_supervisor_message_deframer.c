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
 *  $Revision: #7 $
 *
 */




#include "cmc_supervisor_message_deframer.h"


void cmc_sensor_supervisor_DeFramer_Initialize(SENSOR_SUPERVISOR_DEFRAMER_CONTEXT_TYPE* pContext)
{
	uint32_t i;
	pContext->HaveFrame = false;
	pContext->State = DEFRAMER_START_DEFRAMER_ESCAPE;
	pContext->iWrite = 0;

	for (i = 0; i < FRAMER_MAX_CHARACTERS; i++)
	{
		pContext->Element[i] = '\0';
	}

}

void cmc_sensor_supervisor_DeFramer_Reset(SENSOR_SUPERVISOR_DEFRAMER_CONTEXT_TYPE* pContext)
{
	cmc_sensor_supervisor_DeFramer_Initialize(pContext);
}

void cmc_sensor_supervisor_DeFramerAddCharacter(SENSOR_SUPERVISOR_DEFRAMER_CONTEXT_TYPE* pContext, char AnyCharacter)
{
	pContext->Element[(pContext->iWrite)++] = AnyCharacter;
}

bool cmc_sensor_supervisor_DeFramer_TryAdd(SENSOR_SUPERVISOR_DEFRAMER_CONTEXT_TYPE* pContext, char AnyCharacter)
{
	bool Result = false;

	if (pContext->HaveFrame)
	{
		cmc_sensor_supervisor_DeFramer_Reset(pContext);
	}

	if (FRAMER_MAX_CHARACTERS > (pContext->iWrite))
	{
		switch (pContext->State)
		{
		case DEFRAMER_START_DEFRAMER_ESCAPE:
			switch (AnyCharacter)
			{
			case	FRAMER_ESCAPE:
				pContext->State = DEFRAMER_START_OF_FRAME;
				break;

			default:
				cmc_sensor_supervisor_DeFramer_Reset(pContext);
				break;
			}
			break;
		case DEFRAMER_START_OF_FRAME:
			switch (AnyCharacter)
			{
			case	FRAMER_START_OF_FRAME:
				pContext->State = DEFRAMER_ESCAPE;
				break;

			default:
				cmc_sensor_supervisor_DeFramer_Reset(pContext);
				break;
			}
			break;

		case DEFRAMER_ESCAPE:
			switch (AnyCharacter)
			{
			case	FRAMER_ESCAPE:
				pContext->State = DEFRAMER_END_OF_FRAME;
				break;

			default:
				cmc_sensor_supervisor_DeFramerAddCharacter(pContext, AnyCharacter);
				break;
			}
			break;

		case DEFRAMER_END_OF_FRAME:
			switch (AnyCharacter)
			{
			case	FRAMER_ESCAPE:
				pContext->State = DEFRAMER_ESCAPE;
				cmc_sensor_supervisor_DeFramerAddCharacter(pContext, AnyCharacter);
				break;

			case FRAMER_END_OF_FRAME:
				if (0 == pContext->iWrite)
				{
					cmc_sensor_supervisor_DeFramer_Reset(pContext);
				}
				else
				{
					pContext->HaveFrame = true;
					Result = true;
				}
				break;

			default:
				cmc_sensor_supervisor_DeFramer_Reset(pContext);
				break;
			}
			break;

		default:
			cmc_sensor_supervisor_DeFramer_Reset(pContext);
			break;

		}
	}
	else
	{
		cmc_sensor_supervisor_DeFramer_Reset(pContext);
	}

	return Result;
}

bool cmc_sensor_supervisor_DeFramer_GetPayload(SENSOR_SUPERVISOR_DEFRAMER_CONTEXT_TYPE* pContext, char** ppPayload, uint32_t* pPayloadSize)
{
	bool Result = false;

	*ppPayload = NULL;
	*pPayloadSize = 0;


	if (pContext->HaveFrame)
	{
		*ppPayload = pContext->Element;
		*pPayloadSize = pContext->iWrite;
		Result = true;
	}

	return Result;
}
