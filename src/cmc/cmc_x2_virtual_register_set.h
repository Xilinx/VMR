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
 *  $Revision: #6 $
 *
 */


#ifndef _CMC_X2_VIRTUAL_REGISTER_SET_H_
#define _CMC_X2_VIRTUAL_REGISTER_SET_H_



#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif



#define CMC_X2_MAX_VERSION_REGISTERS                    (4)
#define CMC_X2_MAX_REQUEST_DATA_REGISTERS               (256)
#define CMC_X2_MAX_RESPONSE_DATA_REGISTERS              (256)



/* Virtual Register Indexes */
#define CMC_X2_VIRTUAL_REG_VERSION                      (0)
#define CMC_X2_VIRTUAL_REG_STATUS                       (1)
#define CMC_X2_VIRTUAL_REG_REQUEST_RESET                (2)
#define CMC_X2_VIRTUAL_REG_REQUEST_LENGTH               (3)
#define CMC_X2_VIRTUAL_REG_REQUEST_DATA                 (4)
#define CMC_X2_VIRTUAL_REG_RESPONSE_LENGTH              (5)
#define CMC_X2_VIRTUAL_REG_RESPONSE_DATA                (6)



typedef struct CMC_X2_VIRTUAL_REGISTER_SET_CONTEXT_TYPE
{
    uint8_t Version[CMC_X2_MAX_VERSION_REGISTERS];

    uint8_t Status;

    uint8_t Request_Reset;

    uint8_t Request_Length;
    uint8_t Request_Data[CMC_X2_MAX_REQUEST_DATA_REGISTERS];

    uint8_t Response_Length;
    uint8_t Response_Data[CMC_X2_MAX_RESPONSE_DATA_REGISTERS];



    uint32_t SelectedRegisterAddress;

} CMC_X2_VIRTUAL_REGISTER_SET_CONTEXT_TYPE;



void CMC_X2_VirtualRegisterSet_Initialize(CMC_X2_VIRTUAL_REGISTER_SET_CONTEXT_TYPE* pContext);

void CMC_X2_VirtualRegisterSet_Write(CMC_X2_VIRTUAL_REGISTER_SET_CONTEXT_TYPE* pContext, uint32_t registerAddress, uint8_t* pData, uint32_t dataLength, uint32_t* pNumBytesWritten);

void CMC_X2_VirtualRegisterSet_Read(CMC_X2_VIRTUAL_REGISTER_SET_CONTEXT_TYPE* pContext, uint32_t registerAddress, uint8_t* pBuffer, uint32_t bufferSize, uint32_t* pNumBytesRead);

void CMC_X2_VirtualRegisterSet_SetSelectedRegisterAddress(CMC_X2_VIRTUAL_REGISTER_SET_CONTEXT_TYPE* pContext, uint32_t registerAddress);
void CMC_X2_VirtualRegisterSet_GetSelectedRegisterAddress(CMC_X2_VIRTUAL_REGISTER_SET_CONTEXT_TYPE* pContext, uint32_t* pRegisterAddress);


void CMC_X2_VirtualRegisterSet_SetVersionInfo(CMC_X2_VIRTUAL_REGISTER_SET_CONTEXT_TYPE* pContext, uint8_t major, uint8_t minor, uint8_t patch, uint8_t build);

#ifdef __cplusplus
}
#endif


#endif /* _CMC_X2_VIRTUAL_REGISTER_SET_H_ */
