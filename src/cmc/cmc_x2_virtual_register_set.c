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
 *  $Change: 2751721 $
 *  $Date: 2020/01/14 $
 *  $Revision: #7 $
 *
 */





#include "cmc_x2_virtual_register_set.h"







#define CASE_REG_DETAILS(REG_ADDRESS, REG_FIELD_NAME)           case(REG_ADDRESS):                              \
                                                                {                                               \
                                                                    *ppRegister = (uint8_t*) &(REG_FIELD_NAME); \
                                                                    *pRegisterSize = sizeof(REG_FIELD_NAME);    \
                                                                    break;                                      \
                                                                }



void CMC_X2_VirtualRegisterSet_GetRegister(CMC_X2_VIRTUAL_REGISTER_SET_CONTEXT_TYPE* pContext, uint32_t registerAddress, uint8_t** ppRegister, uint32_t* pRegisterSize)
{
    switch (registerAddress)
    {

        CASE_REG_DETAILS(CMC_X2_VIRTUAL_REG_VERSION,            pContext->Version)
        CASE_REG_DETAILS(CMC_X2_VIRTUAL_REG_STATUS,             pContext->Status)
        CASE_REG_DETAILS(CMC_X2_VIRTUAL_REG_REQUEST_RESET,      pContext->Request_Reset)
        CASE_REG_DETAILS(CMC_X2_VIRTUAL_REG_REQUEST_LENGTH,     pContext->Request_Length)
        CASE_REG_DETAILS(CMC_X2_VIRTUAL_REG_REQUEST_DATA,       pContext->Request_Data)
        CASE_REG_DETAILS(CMC_X2_VIRTUAL_REG_RESPONSE_LENGTH,    pContext->Response_Length)
        CASE_REG_DETAILS(CMC_X2_VIRTUAL_REG_RESPONSE_DATA,      pContext->Response_Data)

        default:
        {
            *ppRegister = NULL;
            *pRegisterSize = 0;
            break;
        }
    }
}











void CMC_X2_VirtualRegisterSet_Initialize(CMC_X2_VIRTUAL_REGISTER_SET_CONTEXT_TYPE* pContext)
{
    uint32_t i;

    for (i = 0; i < CMC_X2_MAX_VERSION_REGISTERS; i++)
    {
        pContext->Version[i] = 0;
    }

    pContext->Status = 0;
    pContext->Request_Reset = 0;
    pContext->Request_Length = 0;

    for (i = 0; i < CMC_X2_MAX_REQUEST_DATA_REGISTERS; i++)
    {
        pContext->Request_Data[i] = 0;
    }

    pContext->Response_Length = 0;

    for (i = 0; i < CMC_X2_MAX_RESPONSE_DATA_REGISTERS; i++)
    {
        pContext->Response_Data[i] = 0;
    }


    pContext->SelectedRegisterAddress = 0;


}



















void CMC_X2_VirtualRegisterSet_Write(CMC_X2_VIRTUAL_REGISTER_SET_CONTEXT_TYPE* pContext, uint32_t registerAddress, uint8_t* pData, uint32_t dataLength, uint32_t* pNumBytesWritten)
{
    uint8_t* pRegister = NULL;
    uint32_t registerSizeInBytes = 0;
    uint32_t numBytesToWrite = 0;
    uint32_t i;

    CMC_X2_VirtualRegisterSet_GetRegister(pContext, registerAddress, &pRegister, &registerSizeInBytes);

    if (pRegister != NULL)
    {

        if (registerSizeInBytes < dataLength)
        {
            numBytesToWrite = registerSizeInBytes;
        }
        else
        {
            numBytesToWrite = dataLength;
        }


        for (i = 0; i < numBytesToWrite; i++)
        {
            pRegister[i] = pData[i];
        }
    }


    *pNumBytesWritten = numBytesToWrite;

}












void CMC_X2_VirtualRegisterSet_Read(CMC_X2_VIRTUAL_REGISTER_SET_CONTEXT_TYPE* pContext, uint32_t registerAddress, uint8_t* pBuffer, uint32_t bufferLength, uint32_t* pNumBytesRead)
{
    uint8_t* pRegister = NULL;
    uint32_t registerSizeInBytes = 0;
    uint32_t numBytesToRead = 0;
    uint32_t i;

    CMC_X2_VirtualRegisterSet_GetRegister(pContext, registerAddress, &pRegister, &registerSizeInBytes);

    if (pRegister != NULL)
    {

        if (registerSizeInBytes < bufferLength)
        {
            numBytesToRead = registerSizeInBytes;
        }
        else
        {
            numBytesToRead = bufferLength;
        }


        for (i = 0; i < numBytesToRead; i++)
        {
            pBuffer[i] = pRegister[i];
        }
    }


    *pNumBytesRead = numBytesToRead;
}



void CMC_X2_VirtualRegisterSet_SetSelectedRegisterAddress(CMC_X2_VIRTUAL_REGISTER_SET_CONTEXT_TYPE* pContext, uint32_t registerAddress)
{
    pContext->SelectedRegisterAddress = registerAddress;
}



void CMC_X2_VirtualRegisterSet_GetSelectedRegisterAddress(CMC_X2_VIRTUAL_REGISTER_SET_CONTEXT_TYPE* pContext, uint32_t* pRegisterAddress)
{
    *pRegisterAddress = pContext->SelectedRegisterAddress;
}




void CMC_X2_VirtualRegisterSet_SetVersionInfo(CMC_X2_VIRTUAL_REGISTER_SET_CONTEXT_TYPE* pContext, uint8_t major, uint8_t minor, uint8_t patch, uint8_t build)
{
    pContext->Version[0] = major;
    pContext->Version[1] = minor;
    pContext->Version[2] = patch;
    pContext->Version[3] = build;
}