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
 *  $Change: 2766318 $
 *  $Date: 2020/01/30 $
 *  $Revision: #16 $
 *
 */




#ifndef _CMC_PERIPHERAL_AXI_TIMEBASE_WDT_H_
#define _CMC_PERIPHERAL_AXI_TIMEBASE_WDT_H_


#ifdef __cplusplus
extern "C"
{
#endif

#include "cmc_common.h"
#include "cmc_peripherals_access.h"
#include "cmc_required_environment.h"
#include "cmc_peripheral_regmap_ram_controller.h"




#define PERIPHERAL_AXI_TIMEBASE_WDT_XWT_TWCSR0_OFFSET   0x00U   /**< Control/Status Register 0 Offset */
#define PERIPHERAL_AXI_TIMEBASE_WDT_XWT_TWCSR1_OFFSET   0x04U   /**< Control/Status Register 1 Offset */
#define PERIPHERAL_AXI_TIMEBASE_WDT_XWT_TBR_OFFSET      0x08U   /**< Timebase Register Offset */
#define PERIPHERAL_AXI_TIMEBASE_WDT_XWT_MWR_OFFSET      0x0CU   /**< Master Write Control Register Offset */
#define PERIPHERAL_AXI_TIMEBASE_WDT_XWT_CSR0_WRS_MASK   0x00000008U /**< Reset status Mask */
#define PERIPHERAL_AXI_TIMEBASE_WDT_XWT_CSR0_WDS_MASK   0x00000004U /**< Timer state Mask */
#define PERIPHERAL_AXI_TIMEBASE_WDT_XWT_CSR0_EWDT1_MASK 0x00000002U /**< Enable bit 1 Mask */
#define PERIPHERAL_AXI_TIMEBASE_WDT_XWT_CSRX_EWDT2_MASK 0x00000001U /**< Enable bit 2 Mask */
#define PERIPHERAL_AXI_TIMEBASE_WDT_XWT_MWR_WDW_MASK    0x0000001BU /**< Width Control */




#define PERIPHERAL_AXI_TIMEBASE_WDT_REGISTER_SET_SIZE   (0x10000)



typedef struct PERIPHERAL_AXI_TIMEBASE_WDT_CONTEXT_TYPE
{
    CMC_PERIPHERAL_BASE_ADDRESS_TYPE BaseAddress;
    bool IsAvailable;
    CMC_REQUIRED_ENVIRONMENT_CONTEXT_TYPE       * pRequiredEnvironmentContext;
    PERIPHERAL_REGMAP_RAM_CONTROLLER_CONTEXT_TYPE    * pPeripheralRegMapRamControllerContext;

} PERIPHERAL_AXI_TIMEBASE_WDT_CONTEXT_TYPE;

void PERIPHERAL_AXI_TIMEBASE_WDT_WatchdogService(PERIPHERAL_AXI_TIMEBASE_WDT_CONTEXT_TYPE *pContext);
void PERIPHERAL_AXI_TIMEBASE_WDT_Initialize(PERIPHERAL_AXI_TIMEBASE_WDT_CONTEXT_TYPE *pContext, CMC_PERIPHERAL_BASE_ADDRESS_TYPE AnyBaseAddress, bool IsAvailable, CMC_REQUIRED_ENVIRONMENT_CONTEXT_TYPE * pRequiredEnvironmentContext, PERIPHERAL_REGMAP_RAM_CONTROLLER_CONTEXT_TYPE* pPeripheralRegMapRamControllerContext);
void PERIPHERAL_AXI_TIMEBASE_WDT_Start(PERIPHERAL_AXI_TIMEBASE_WDT_CONTEXT_TYPE *pContext);




#ifdef __cplusplus
}
#endif


#endif /* _CMC_PERIPHERAL_AXI_TIMEBASE_WDT_H_ */


















