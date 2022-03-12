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
 *  $Change: 3095780 $
 *  $Date: 2021/01/13 $
 *  $Revision: #18 $
 *
 */




#ifndef _CMC_ENTRY_POINT_BUILD_PROFILE_H_
#define _CMC_ENTRY_POINT_BUILD_PROFILE_H_


#ifdef __cplusplus
extern "C"
{
#endif

//#include "cmc_common.h"
//#include "cmc_scheduler.h"
#include "cmc_user_supplied_environment.h"
#include "cmc_user_supplied_memory_map.h"
#include "cmc_user_supplied_build_profile_name.h"
#include "cmc_entry_point_build_profile_clock_throttling.h"
#include "cmc_entry_point_build_profile_peripherals.h"
#include "cmc_entry_point_build_profile_protocols.h"
#include "cmc_entry_point_build_profile_transports.h"
#include "cmc_firmware_version.h"
#include "cmc_hardware_platform.h"
#include "cmc_feature_control_feature_list.h"




typedef struct CMC_BUILD_PROFILE_TYPE
{
    CMC_BUILD_PROFILE_NAME_TYPE                         Name;
    CMC_USER_SUPPLIED_ENVIRONMENT_TYPE                  UserSuppliedEnvironment;
    CMC_USER_SUPPLIED_MEMORY_MAP_TYPE                   UserSuppliedMemoryMap;
    bool                                                FeatureIsEnabled[FC_MAX_FEATURES];
	uint8_t												NumberOfQSFPCages;
    CMC_BUILD_PROFILE_CLOCK_THROTTLING_TYPE             ClockThrottling;

    HARDWARE_PLATFORM_SERVICES_TYPE                     HardwarePlatformServices;
    CMC_BUILD_PROFILE_PERIPHERALS_TYPE                  Peripherals;
	CMC_BUILD_PROFILE_PROTOCOLS_TYPE                    Protocols;
	CMC_BUILD_PROFILE_TRANSPORTS_TYPE                   Transports;

	CMC_USER_SUPPLIED_VERSION_TYPE						Version;
    uint8_t                                             CMCOfferedInterfaceVersion;

} CMC_BUILD_PROFILE_TYPE;

#ifdef __cplusplus
}
#endif


#endif /* _CMC_ENTRY_POINT_BUILD_PROFILE_H_ */















