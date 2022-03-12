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
 *  $Change: 2787149 $
 *  $Date: 2020/02/21 $
 *  $Revision: #23 $
 *
 */





#include "cmc_peripheral_regmap_ram_controller.h"


bool PERIPHERAL_REGMAP_RAM_CONTROLLER_GetDataSegmentSubsequentRecord(PERIPHERAL_REGMAP_RAM_CONTROLLER_CONTEXT_TYPE *pContext, uint8_t ** ppDataSegment, uint32_t * pLength)
{
    bool Result=true;
    uint32_t RemoteMessageRegisterValue;
    
    if(pContext->IsAvailable)
    {
        RemoteMessageRegisterValue=PERIPHERAL_REGMAP_RAM_CONTROLLER_Read(pContext, HOST_REGISTER_REMOTE_COMMAND_REGISTER);
        *pLength=RemoteMessageRegisterValue&0x00000FFF;
	    *ppDataSegment = (uint8_t*)PERIPHERAL_REGMAP_RAM_CONTROLLER_GetRegisterAddress(pContext, HOST_REGISTER_REMOTE_COMMAND_START_OF_PAYLOAD_REGISTER_SUBSEQUENT_SEGMENT);
    }
    else
    {
        Result=false;
    }

    return Result;      
}

bool PERIPHERAL_REGMAP_RAM_CONTROLLER_GetDataSegmentFirstRecord(PERIPHERAL_REGMAP_RAM_CONTROLLER_CONTEXT_TYPE *pContext, uint8_t ** ppDataSegment, uint32_t * pLength)
{
    bool Result=true;
    uint32_t RemoteMessageRegisterValue;

    if(pContext->IsAvailable)
    {
        RemoteMessageRegisterValue=PERIPHERAL_REGMAP_RAM_CONTROLLER_Read(pContext, HOST_REGISTER_REMOTE_COMMAND_REGISTER);
        *pLength=RemoteMessageRegisterValue&0x00000FFF;

	    *ppDataSegment = (uint8_t*)PERIPHERAL_REGMAP_RAM_CONTROLLER_GetRegisterAddress(pContext, HOST_REGISTER_REMOTE_COMMAND_START_OF_PAYLOAD_REGISTER_FIRST_SEGMENT);
    }
    else
    {
        Result=false;
    }

    return Result;      
}

uint32_t PERIPHERAL_REGMAP_RAM_CONTROLLER_Read(PERIPHERAL_REGMAP_RAM_CONTROLLER_CONTEXT_TYPE *pContext, uint32_t RegisterOffset)
{
    uint32_t Result=0;

    if(pContext->IsAvailable)
    {
        if(PERIPHERAL_REGMAP_RAM_CONTROLLER_REGISTER_SET_SIZE>RegisterOffset)
        {
            Result=(*pContext->Profile.pFN_Read)(pContext->Profile.pUserContext, RegisterOffset);
        }
        else
        {
            Watch_Inc(pContext->pWatchPointContext, W_HOST_REGISTER_RANGE_VIOLATION);
        }
    }

    return Result;
}



void PERIPHERAL_REGMAP_RAM_CONTROLLER_Write(PERIPHERAL_REGMAP_RAM_CONTROLLER_CONTEXT_TYPE  *pContext, uint32_t RegisterOffset, uint32_t Value)
{
    if(pContext->IsAvailable)
    {
        if(PERIPHERAL_REGMAP_RAM_CONTROLLER_REGISTER_SET_SIZE>RegisterOffset)
        {
            (*pContext->Profile.pFN_Write)(pContext->Profile.pUserContext, RegisterOffset, Value);  
        }
        else
        {
            Watch_Inc(pContext->pWatchPointContext, W_HOST_REGISTER_RANGE_VIOLATION);
        }
    }
}



void PERIPHERAL_REGMAP_RAM_CONTROLLER_WriteWithMask(PERIPHERAL_REGMAP_RAM_CONTROLLER_CONTEXT_TYPE  *pContext, uint32_t RegisterOffset, uint32_t Value, uint32_t Mask)
{
    if(pContext->IsAvailable)
    {
        (*pContext->Profile.pFN_WriteWithMask)(pContext->Profile.pUserContext, RegisterOffset, Value, Mask);  
    }
}


volatile uint32_t* PERIPHERAL_REGMAP_RAM_CONTROLLER_GetRegisterAddress(PERIPHERAL_REGMAP_RAM_CONTROLLER_CONTEXT_TYPE* pContext, uint32_t RegisterOffset)
{
    volatile uint32_t* Result = NULL;
    if (pContext->IsAvailable)
    {
        Result = (*pContext->Profile.pFN_GetRegisterAddress)(pContext->Profile.pUserContext, RegisterOffset);
    }

    return Result;
}

uint32_t CMC_ProfileName_As_U32(char* pShortName)
{
    return (pShortName[0] << 24) | (pShortName[1] << 16) | (pShortName[2] << 8) | pShortName[3];
}


void PERIPHERAL_REGMAP_RAM_CONTROLLER_Initialize(   PERIPHERAL_REGMAP_RAM_CONTROLLER_CONTEXT_TYPE *pContext,
                                                    CMC_BUILD_PROFILE_PERIPHERAL_RAM_CONTROLLER_TYPE * pProfile,
                                                    CMC_WATCHPOINT_CONTEXT_TYPE * pWatchPointContext,
                                                    CMC_PERIPHERAL_BASE_ADDRESS_TYPE AnyBaseAddress, 
												    bool IsAvailable,
                                                    CMC_REQUIRED_ENVIRONMENT_CONTEXT_TYPE * pRequiredEnvironmentContext,
                                                    CMC_FIRMWARE_VERSION_CONTEXT_TYPE * pFirmwareVersionContext,
                                                    CMC_CORE_VERSION_CONTEXT_TYPE * pCoreVersionContext,
                                                    char * pShortName)
{
    
    pContext->Profile=*pProfile;
    pContext->BaseAddress=AnyBaseAddress;
    pContext->IsAvailable=IsAvailable;
    pContext->pWatchPointContext=pWatchPointContext;
    pContext->pRequiredEnvironmentContext=pRequiredEnvironmentContext;
    pContext->pFirmwareVersionContext=pFirmwareVersionContext;
    pContext->pCoreVersionContext = pCoreVersionContext;
    pContext->ShortNameU32 = CMC_ProfileName_As_U32(pShortName);

    if(pContext->IsAvailable)
    {
        if(pContext->Profile.pFN_Initialize)
        {
            (*pContext->Profile.pFN_Initialize)(pContext->Profile.pUserContext,pContext->BaseAddress);
        }
    }
}

