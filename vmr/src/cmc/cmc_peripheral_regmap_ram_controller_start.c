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
 *  $Change: 2783110 $
 *  $Date: 2020/02/18 $
 *  $Revision: #7 $
 *
 */




#include "cmc_peripheral_regmap_ram_controller.h"


void PERIPHERAL_REGMAP_RAM_CONTROLLER_Start(PERIPHERAL_REGMAP_RAM_CONTROLLER_CONTEXT_TYPE *pContext)
{
    uint32_t iRegister;
    uint32_t Version=CMC_FirmwareVersion_As_U32(pContext->pFirmwareVersionContext);
    uint32_t CoreVersion = CMC_CoreVersion_As_U32(pContext->pCoreVersionContext);

   if(pContext->IsAvailable)
   {
        for(iRegister=0;iRegister<PERIPHERAL_REGMAP_RAM_CONTROLLER_REGISTER_SET_SIZE;iRegister+=4)
        {
            PERIPHERAL_REGMAP_RAM_CONTROLLER_Write(pContext, iRegister, 0x00000000);
        }

        PERIPHERAL_REGMAP_RAM_CONTROLLER_Write(pContext, HOST_REGISTER_MAGIC,									HOST_REGISTER_PATTERN_MAGIC);
        PERIPHERAL_REGMAP_RAM_CONTROLLER_Write(pContext, HOST_REGISTER_VERSION,									Version);
        PERIPHERAL_REGMAP_RAM_CONTROLLER_Write(pContext, HOST_REGISTER_CORE_VERSION,                            CoreVersion);
        PERIPHERAL_REGMAP_RAM_CONTROLLER_Write(pContext, HOST_REGISTER_LOCATION_OF_HOST_MESSAGE_REGISTER,		HOST_REGISTER_REMOTE_COMMAND_REGISTER);
        PERIPHERAL_REGMAP_RAM_CONTROLLER_Write(pContext, HOST_REGISTER_PROFILE_NAME,                            pContext->ShortNameU32);
    }
}

