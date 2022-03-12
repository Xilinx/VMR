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


/* PROFILE_LINUX_VERSAL_VCK5000_R5 */ 
/* LINUX_DIST */




/*
 *
 *  RCS Keyword Metadata
 *
 *  $Change: 3095780 $
 *  $Date: 2021/01/13 $
 *  $Revision: #16 $
 *
 */






#ifndef _CMC_PROFILEVERSAL_VCK5000_R5_INTERNAL_H_
#define _CMC_PROFILEVERSAL_VCK5000_R5_INTERNAL_H_


#ifdef __cplusplus
extern "C"
{
#endif

#include "cmc_entry_point.h"
#include "cmc_profile_versal_VCK5000_R5_internal.h"
#include "xil_io.h"
#include "xil_cache.h"

#define IO_SYNC_READ32(addr) ({ 					\
		Xil_DCacheFlushRange(addr, sizeof(u32));	\
		Xil_In32(addr); })

#define IO_SYNC_WRITE32(val, addr) ({ 				\
		Xil_Out32(addr, val);						\
		Xil_DCacheFlushRange(addr, sizeof(u32)); })

#define CMC_VALIDSENSORS_0_to_63 ((uint64_t)1 << SNSR_ID_12V_PEX) |                          \
                                 ((uint64_t)1 << SNSR_ID_3V3_PEX) |                          \
                                 ((uint64_t)1 << SNSR_ID_3V3_AUX) |                          \
                                 ((uint64_t)1 << SNSR_ID_12V_AUX0) |                         \
                                 ((uint64_t)1 << SNSR_ID_SYS_5V5) |                          \
                                 ((uint64_t)1 << SNSR_ID_VCC1V2_TOP) |                       \
                                 ((uint64_t)1 << SNSR_ID_VCC1V8) |                           \
                                 ((uint64_t)1 << SNSR_ID_DDR4_VPP_TOP) |                     \
                                 ((uint64_t)1 << SNSR_ID_MGT0V9AVCC) |                       \
                                 ((uint64_t)1 << SNSR_ID_MGTAVTT) |                          \
                                 ((uint64_t)1 << SNSR_ID_VCCINT) |                           \
                                 ((uint64_t)1 << SNSR_ID_VCCINT_I) |                         \
                                 ((uint64_t)1 << SNSR_ID_FPGA_TEMP) |                        \
                                 ((uint64_t)1 << SNSR_ID_FAN_TEMP) |                         \
                                 ((uint64_t)1 << SNSR_ID_SE98_TEMP0) |                       \
                                 ((uint64_t)1 << SNSR_ID_SE98_TEMP1) |                       \
                                 ((uint64_t)1 << SNSR_ID_FAN_SPEED) |                        \
                                 ((uint64_t)1 << SNSR_ID_BOARD_SN) |                         \
                                 ((uint64_t)1 << SNSR_ID_MAC_ADDRESS0) |                     \
                                 ((uint64_t)1 << SNSR_ID_MAC_ADDRESS1) |                     \
                                 ((uint64_t)1 << SNSR_ID_MAC_ADDRESS2) |                     \
                                 ((uint64_t)1 << SNSR_ID_MAC_ADDRESS3) |                     \
                                 ((uint64_t)1 << SNSR_ID_BOARD_REV) |                        \
                                 ((uint64_t)1 << SNSR_ID_BOARD_NAME) |                       \
                                 ((uint64_t)1 << SNSR_ID_SAT_VERSION) |                      \
                                 ((uint64_t)1 << SNSR_ID_TOTAL_POWER_AVAIL) |                \
                                 ((uint64_t)1 << SNSR_ID_FAN_PRESENCE) |                     \
                                 ((uint64_t)1 << SNSR_ID_CONFIG_MODE) |                      \
                                 ((uint64_t)1 << SNSR_ID_VCC3V3) |                           \
                                 ((uint64_t)1 << SNSR_ID_12V_AUX1) |                         \
                                 ((uint64_t)1 << SNSR_ID_VCC1V2_I)

#define CMC_VALIDSENSORS_64_to_127 ((uint64_t)1 << (SNSR_ID_V12_IN_I - 64)) |               \
                                   ((uint64_t)1 << (SNSR_ID_V12_IN_AUX0_I - 64)) |          \
                                   ((uint64_t)1 << (SNSR_ID_V12_IN_AUX1_I - 64)) |          \
                                   ((uint64_t)1 << (SNSR_ID_VCCAUX - 64)) |                 \
                                   ((uint64_t)1 << (SNSR_ID_VCCAUX_PMC - 64)) |             \
                                   ((uint64_t)1 << (SNSR_ID_VCCRAM - 64)) |                 \
                                   ((uint64_t)1 << (SNSR_ID_POWER_GOOD - 64))

typedef struct CMC_BUILDPROFILE_VERSAL_VCK5000_R5_USER_ENVIRONMENT_TYPE
{
    void* pDeframerContext;
    void* pUartContext;
    void* pParserContext;
    void* pBootloaderParserContext;
}CMC_BUILDPROFILE_VERSAL_VCK5000_R5_USER_ENVIRONMENT_TYPE;


void CMC_BuildProfile_Versal_VCK5000_R5(CMC_BUILD_PROFILE_TYPE * pProfile);

void *CMC_BuildProfile_Versal_VCK5000_R5_InterruptHandler(void *ptr);



void CMC_BuildProfile_Versal_VCK5000_R5_feature_is_enabled(CMC_BUILD_PROFILE_TYPE * pProfile);
void CMC_BuildProfile_Versal_VCK5000_R5_profile_name(CMC_BUILD_PROFILE_TYPE * pProfile);
void CMC_BuildProfile_Versal_VCK5000_R5_user_supplied_environment(CMC_BUILD_PROFILE_TYPE * pProfile);
void CMC_BuildProfile_Versal_VCK5000_R5_user_supplied_memory_map(CMC_BUILD_PROFILE_TYPE * pProfile);
void CMC_BuildProfile_Versal_VCK5000_R5_set_max_cages(CMC_BUILD_PROFILE_TYPE* pProfile);
void CMC_BuildProfile_Versal_VCK5000_R5_set_cmc_version(CMC_BUILD_PROFILE_TYPE* pProfile);
void CMC_BuildProfile_Versal_VCK5000_R5_clock_throttling(CMC_BUILD_PROFILE_TYPE * pProfile);
void CMC_BuildProfile_Versal_VCK5000_R5_hardware_platform(CMC_BUILD_PROFILE_TYPE * pProfile);
void CMC_BuildProfile_Versal_VCK5000_R5_set_SC_InterfaceVersion(CMC_BUILD_PROFILE_TYPE* pProfile);

void CMC_BuildProfile_Versal_VCK5000_R5_Transport_Sensor(CMC_BUILD_PROFILE_TYPE* pProfile);
void CMC_BuildProfile_Versal_VCK5000_R5_Transports(CMC_BUILD_PROFILE_TYPE* pProfile);

void CMC_BuildProfile_Versal_VCK5000_R5_Protocol_Bootloader(CMC_BUILD_PROFILE_TYPE* pProfile);
void CMC_BuildProfile_Versal_VCK5000_R5_Protocol_Sensor(CMC_BUILD_PROFILE_TYPE* pProfile);
void CMC_BuildProfile_Versal_VCK5000_R5_Protocols(CMC_BUILD_PROFILE_TYPE* pProfile);

void CMC_BuildProfile_Versal_VCK5000_R5_Peripherals(CMC_BUILD_PROFILE_TYPE * pProfile);
void CMC_BuildProfile_Versal_VCK5000_R5_Peripheral_uart_debug(CMC_BUILD_PROFILE_TYPE * pProfile);
void CMC_BuildProfile_Versal_VCK5000_R5_Peripheral_uart_sc_communications(CMC_BUILD_PROFILE_TYPE * pProfile);
void CMC_BuildProfile_Versal_VCK5000_R5_Peripheral_Clock(CMC_BUILD_PROFILE_TYPE * pProfile);
void CMC_BuildProfile_Versal_VCK5000_R5_Peripheral_RegMap_RAM_Controller(CMC_BUILD_PROFILE_TYPE * pProfile);
//void CMC_BuildProfile_Versal_VCK5000_R5_Peripheral_IntC(CMC_BUILD_PROFILE_TYPE * pProfile);
void CMC_BuildProfile_Versal_VCK5000_R5_Peripheral_uart_sc_communications_UART_SERVICES_INITIALIZE_FREERTOS(void);

extern CMC_BUILD_PROFILE_TYPE CMC_BuildProfile_Versal_VCK5000_R5_Profile;



#ifdef __cplusplus
}
#endif


#endif /* _CMC_PROFILEVERSAL_VCK5000_R5_INTERNAL_H_ */


















