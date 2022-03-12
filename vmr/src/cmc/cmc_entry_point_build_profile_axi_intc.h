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
 *  $Change: 3097075 $
 *  $Date: 2021/01/14 $
 *  $Revision: #2 $
 *
 */


#ifndef _CMC_ENTRY_POINT_BUILD_PROFILE_AXI_INTC_H_
#define _CMC_ENTRY_POINT_BUILD_PROFILE_AXI_INTC_H_





#ifdef __cplusplus
extern "C"
{
#endif

#include "cmc_common.h"




	typedef     void(*AXI_INTC_SERVICES_ENABLE_INTERRUPT)   (void *pUserContext, uint32_t InterruptSource);
	typedef     void(*AXI_INTC_SERVICES_DISABLE_INTERRUPT)   (void *pUserContext, uint32_t InterruptSource);
	typedef     void(*AXI_INTC_SERVICES_INTERRUPT_HANDLER)   (void);
	typedef     void(*AXI_INTC_SERVICES_INITIALIZE)   (void *pUserContext, CMC_PERIPHERAL_BASE_ADDRESS_TYPE BaseAddress);
	typedef     void(*AXI_INTC_SERVICES_START)   (void *pUserContext);



typedef struct CMC_BUILD_PROFILE_PERIPHERAL_AXI_INTC_TYPE
{
    void *                                                pUserContext;
	AXI_INTC_SERVICES_INITIALIZE                          pFN_Initialize;
	AXI_INTC_SERVICES_ENABLE_INTERRUPT                    pFN_EnableInterrupt;
	AXI_INTC_SERVICES_DISABLE_INTERRUPT                   pFN_DisableInterrupt;
	AXI_INTC_SERVICES_INTERRUPT_HANDLER                   pFN_InterruptHandler;
	AXI_INTC_SERVICES_START                               pFN_Start;

}CMC_BUILD_PROFILE_PERIPHERAL_AXI_INTC_TYPE;







#ifdef __cplusplus
}
#endif





#endif /* _CMC_ENTRY_POINT_BUILD_PROFILE_AXI_INTC_H_ */







