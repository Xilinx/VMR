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


#include "cmc_circular_buffer.h"




/*
 *
 *  RCS Keyword Metadata
 *
 *  $Change: 2680455 $
 *  $Date: 2019/10/01 $
 *  $Revision: #6 $
 *
 */




void CircularBuffer_Initialize(CIRCULAR_BUFFER_TYPE * pContext, CIRCULAR_BUFFER_ELEMENT_TYPE * pCircularBuffer, uint32_t MaxElements)
{
	uint32_t i;

    pContext->MaxElements=MaxElements;
    pContext->pCircularBuffer=pCircularBuffer;
 
	for (i = 0; i < (pContext->MaxElements); i++)
	{
		pContext->pCircularBuffer[i].IsOccupied = false;
		pContext->pCircularBuffer[i].Octet = 0x00;
	}
	pContext->iWrite = 0;
	pContext->iRead = 0;

}




uint32_t CircularBuffer_Inc(CIRCULAR_BUFFER_TYPE * pContext, uint32_t AnyIndex)
{
	uint32_t Result = AnyIndex;

	Result++;

	if ((pContext->MaxElements) <= Result)
	{
		Result = 0;
	}

	return Result;
}

bool CircularBuffer_SpaceIsAvailable(CIRCULAR_BUFFER_TYPE *pContext, uint32_t RequiredSpace)
{
    bool Result=true;
    uint32_t iWalk=pContext->iWrite;
    uint32_t i;

    for(i=0;i<RequiredSpace;i++)
    {
        if(pContext->pCircularBuffer[iWalk].IsOccupied)
        {
            Result=false;
            break;
        }
        else
        {
            iWalk = CircularBuffer_Inc(pContext, iWalk);
        }
    }

    return Result;
}

bool CircularBuffer_TryWriteArray(CIRCULAR_BUFFER_TYPE *pContext, char * pCharacters, uint32_t Length)
{
    bool Result=CircularBuffer_SpaceIsAvailable(pContext, Length);
    uint32_t i;

    if(Result)
    {
        for(i=0;i<Length;i++)
        {
            Result=CircularBuffer_TryWrite(pContext, pCharacters[i]);
            if(!Result)
            {
                break; // Internal Coding Error
            }
        }
    }

    return Result;
}

bool CircularBuffer_TryWrite(CIRCULAR_BUFFER_TYPE *pContext, char AnyCharacter)
{
	bool Result = false;

	if (!pContext->pCircularBuffer[pContext->iWrite].IsOccupied)
	{
		pContext->pCircularBuffer[pContext->iWrite].Octet = AnyCharacter;
		pContext->pCircularBuffer[pContext->iWrite].IsOccupied = true;
		pContext->iWrite = CircularBuffer_Inc(pContext, pContext->iWrite);
		Result = true;
	}
	return Result;
}




bool CircularBuffer_TryRead(CIRCULAR_BUFFER_TYPE *pContext, char * pAnyCharacter)
{
	bool Result = false;

	*pAnyCharacter = '\0';
	if (pContext->pCircularBuffer[pContext->iRead].IsOccupied)
	{
		*pAnyCharacter = pContext->pCircularBuffer[pContext->iRead].Octet;
		pContext->pCircularBuffer[pContext->iRead].IsOccupied = false;
		pContext->iRead = CircularBuffer_Inc(pContext, pContext->iRead);
		Result = true;
	}
	return Result;
}



void CircularBuffer_Flush(CIRCULAR_BUFFER_TYPE *pContext)
{
    char AnyCharacter;

    while(CircularBuffer_TryRead(pContext, &AnyCharacter)) {;}
}



bool CircularBuffer_Peek(CIRCULAR_BUFFER_TYPE *pContext, char * pAnyCharacter)
{
	bool Result = false;

	*pAnyCharacter = '\0';
	if (pContext->pCircularBuffer[pContext->iRead].IsOccupied)
	{
		*pAnyCharacter = pContext->pCircularBuffer[pContext->iRead].Octet;
		Result = true;
	}
	return Result;
}





bool CircularBuffer_Verify_AtLeast_Count_Unread(CIRCULAR_BUFFER_TYPE *pContext, uint32_t Count)
{
    bool Result=true;
    uint32_t iWalkingRead=pContext->iRead;
    uint32_t CountUp=0;
    
    while(CountUp!=Count)
    {
		if(pContext->pCircularBuffer)
		{
			if (pContext->pCircularBuffer[iWalkingRead].IsOccupied)
			{
				CountUp++;
				iWalkingRead = CircularBuffer_Inc(pContext, iWalkingRead);
			}
			else
			{
				Result = false;
				break; /* out of loop */
			}
		}
        
    }

    return Result;
}


