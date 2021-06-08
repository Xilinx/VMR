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
 *  $Change: 2750637 $
 *  $Date: 2020/01/13 $
 *  $Revision: #3 $
 *
 */





#ifndef _CMC_ENTRY_POINT_BUILD_PROFILE_I2C_H_
#define _CMC_ENTRY_POINT_BUILD_PROFILE_I2C_H_





#ifdef __cplusplus
extern "C"
{
#endif

#include "cmc_common.h"



typedef void (*I2C_WRITE_REQUEST_CALLBACK_TYPE)(void* callbackContext, uint8_t* pData, uint32_t numBytes);
typedef void (*I2C_READ_REQUEST_CALLBACK_TYPE)(void* callbackContext, uint8_t* pBuffer, uint32_t bufferSize, uint32_t* pNumValidBytes);


typedef     void    (*I2C_SERVICES_INITIALIZE)   (void* pUserContext, CMC_PERIPHERAL_BASE_ADDRESS_TYPE baseAddress);
typedef     void    (*I2C_SERVICES_START) (void* pUserContext);
typedef     void    (*I2C_SERVICES_SET_WRITE_REQUEST_CALLBACK)(void* pUserContext, void* callbackContext, I2C_WRITE_REQUEST_CALLBACK_TYPE pFN_WriteRequestCallback);
typedef     void    (*I2C_SERVICES_SET_READ_REQUEST_CALLBACK)(void* pUserContext, void* callbackContext, I2C_READ_REQUEST_CALLBACK_TYPE pFN_ReadRequestCallback);
typedef     void	(*I2C_SERVICES_INTERRUPT) (void);


typedef struct CMC_BUILD_PROFILE_PERIPHERAL_I2C_TYPE
{
    void*                                   pUserContext;

   I2C_SERVICES_INITIALIZE                  pFN_Initialize;
   I2C_SERVICES_START                       pFN_Start;

   I2C_SERVICES_SET_WRITE_REQUEST_CALLBACK  pFN_SetWriteRequestCallback;
   I2C_SERVICES_SET_READ_REQUEST_CALLBACK   pFN_SetReadRequestCallback;

   I2C_SERVICES_INTERRUPT                   pFN_InterruptHandler;

}CMC_BUILD_PROFILE_PERIPHERAL_I2C_TYPE;



#ifdef __cplusplus
}
#endif





#endif /* _CMC_ENTRY_POINT_BUILD_PROFILE_I2C_H_ */





