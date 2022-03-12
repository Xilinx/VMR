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
 *  $Change: 2784362 $
 *  $Date: 2020/02/19 $
 *  $Revision: #20 $
 *
 */




#ifndef _CMC_PERIPHERAL_REGMAP_RAM_CONTROLLER_H_
#define _CMC_PERIPHERAL_REGMAP_RAM_CONTROLLER_H_


#ifdef __cplusplus
extern "C"
{
#endif

#include "cmc_common.h"
#include "cmc_watchpoint.h"
#include "cmc_peripherals_access.h"
#include "cmc_peripheral_regmap_ram_controller_register_layout.h"
#include "cmc_peripheral_regmap_ram_controller_register_contents.h"
#include "cmc_peripheral_regmap_ram_controller_constants.h"
#include "cmc_required_environment.h"
#include "cmc_firmware_version.h"
#include "cmc_entry_point_build_profile_ram_controller.h"
#include "cmc_core_version.h"




typedef struct PERIPHERAL_REGMAP_RAM_CONTROLLER_CONTEXT_TYPE
{
    CMC_PERIPHERAL_BASE_ADDRESS_TYPE                            BaseAddress;
    bool                                                        IsAvailable;
    CMC_WATCHPOINT_CONTEXT_TYPE *                               pWatchPointContext;
    CMC_REQUIRED_ENVIRONMENT_CONTEXT_TYPE *                     pRequiredEnvironmentContext;
    CMC_FIRMWARE_VERSION_CONTEXT_TYPE *                         pFirmwareVersionContext;
    CMC_CORE_VERSION_CONTEXT_TYPE *                             pCoreVersionContext;
    uint32_t                                                    ShortNameU32;
    CMC_BUILD_PROFILE_PERIPHERAL_RAM_CONTROLLER_TYPE            Profile;

} PERIPHERAL_REGMAP_RAM_CONTROLLER_CONTEXT_TYPE;




bool PERIPHERAL_REGMAP_RAM_CONTROLLER_GetDataSegmentSubsequentRecord(PERIPHERAL_REGMAP_RAM_CONTROLLER_CONTEXT_TYPE *pContext, uint8_t ** ppDataSegment, uint32_t * pLength);
bool PERIPHERAL_REGMAP_RAM_CONTROLLER_GetDataSegmentFirstRecord(PERIPHERAL_REGMAP_RAM_CONTROLLER_CONTEXT_TYPE *pContext, uint8_t ** pDataSegment, uint32_t * pLength);
uint32_t PERIPHERAL_REGMAP_RAM_CONTROLLER_Read(PERIPHERAL_REGMAP_RAM_CONTROLLER_CONTEXT_TYPE *pContext, uint32_t RegisterOffset);
void PERIPHERAL_REGMAP_RAM_CONTROLLER_Write(PERIPHERAL_REGMAP_RAM_CONTROLLER_CONTEXT_TYPE  *pContext, uint32_t RegisterOffset, uint32_t Value);
void PERIPHERAL_REGMAP_RAM_CONTROLLER_WriteWithMask(PERIPHERAL_REGMAP_RAM_CONTROLLER_CONTEXT_TYPE  *pContext, uint32_t RegisterOffset, uint32_t Value, uint32_t Mask);
void PERIPHERAL_REGMAP_RAM_CONTROLLER_Initialize(PERIPHERAL_REGMAP_RAM_CONTROLLER_CONTEXT_TYPE *pContext, CMC_BUILD_PROFILE_PERIPHERAL_RAM_CONTROLLER_TYPE * pProfile, CMC_WATCHPOINT_CONTEXT_TYPE * pWatchPointContext, CMC_PERIPHERAL_BASE_ADDRESS_TYPE AnyBaseAddress, bool IsAvailable, CMC_REQUIRED_ENVIRONMENT_CONTEXT_TYPE * pRequiredEnvironmentContext, CMC_FIRMWARE_VERSION_CONTEXT_TYPE * pFirmwareVersionContext, CMC_CORE_VERSION_CONTEXT_TYPE* pCoreVersionContext, char* pShortName);
void PERIPHERAL_REGMAP_RAM_CONTROLLER_Start(PERIPHERAL_REGMAP_RAM_CONTROLLER_CONTEXT_TYPE *pContext);
volatile uint32_t* PERIPHERAL_REGMAP_RAM_CONTROLLER_GetRegisterAddress(PERIPHERAL_REGMAP_RAM_CONTROLLER_CONTEXT_TYPE* pContext, uint32_t RegisterOffset);



#ifdef __cplusplus
}
#endif


#endif /* _CMC_PERIPHERAL_REGMAP_RAM_CONTROLLER_H_ */



















