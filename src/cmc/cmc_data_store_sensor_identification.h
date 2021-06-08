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
 *  $Change: 3229550 $
 *  $Date: 2021/05/25 $
 *  $Revision: #15 $
 *
 */




#ifndef _CMC_DATA_STORE_SENSOR_IDENTIFICATION_H_
#define _CMC_DATA_STORE_SENSOR_IDENTIFICATION_H_


#ifdef __cplusplus
extern "C"
{
#endif


enum // satellite controller sensor and info ids
{
	SNSR_ID_12V_PEX                         = 0x0,
	SNSR_ID_3V3_PEX,
	SNSR_ID_3V3_AUX,
	SNSR_ID_12V_AUX0,
	SNSR_ID_DDR4_VPP_BTM,
	SNSR_ID_SYS_5V5,
	SNSR_ID_VCC1V2_TOP,
	SNSR_ID_VCC1V8,
	SNSR_ID_VCC0V85,
	SNSR_ID_DDR4_VPP_TOP,
	SNSR_ID_MGT0V9AVCC,
	SNSR_ID_12V_SW,
	SNSR_ID_MGTAVTT,
	SNSR_ID_VCC1V2_BTM,
	SNSR_ID_12VPEX_I_IN,
	SNSR_ID_12V_AUX_I_IN,
	SNSR_ID_VCCINT,
	SNSR_ID_VCCINT_I,
	SNSR_ID_FPGA_TEMP,
	SNSR_ID_FAN_TEMP,
	SNSR_ID_DIMM_TEMP0,
	SNSR_ID_DIMM_TEMP1,
	SNSR_ID_DIMM_TEMP2,
	SNSR_ID_DIMM_TEMP3,
	SNSR_ID_SE98_TEMP0,
	SNSR_ID_SE98_TEMP1,
	SNSR_ID_SE98_TEMP2,
	SNSR_ID_FAN_SPEED,
	SNSR_ID_CAGE_TEMP0,
	SNSR_ID_CAGE_TEMP1,
	SNSR_ID_CAGE_TEMP2,
	SNSR_ID_CAGE_TEMP3,
	SNSR_ID_BOARD_SN                        = 0x21,
	SNSR_ID_MAC_ADDRESS0,
	SNSR_ID_MAC_ADDRESS1,
	SNSR_ID_MAC_ADDRESS2,
	SNSR_ID_MAC_ADDRESS3,
	SNSR_ID_BOARD_REV,
	SNSR_ID_BOARD_NAME,
	SNSR_ID_SAT_VERSION,
	SNSR_ID_TOTAL_POWER_AVAIL,
	SNSR_ID_FAN_PRESENCE,
	SNSR_ID_CONFIG_MODE,
    SNSR_ID_MAC_ADDRESS4                    = 0x2C,
    SNSR_ID_MAC_ADDRESS5                    = 0x2D,
    SNSR_ID_MAC_ADDRESS6                    = 0x2E,
    SNSR_ID_MAC_ADDRESS7                    = 0x2F,
	SNSR_ID_HBM_TEMP1                       = 0x30,
	SNSR_ID_VCC3V3,
	SNSR_ID_3V3PEX_I_IN,
	SNSR_ID_VCC0V85_I,
	SNSR_ID_HBM_1V2,
	SNSR_ID_VPP2V5,
	SNSR_ID_VCCINT_BRAM,
	SNSR_ID_HBM_TEMP2                       = 0x37,
    SNSR_ID_12V_AUX1                        = 0x38,
    SNSR_ID_VCCINT_TEMP                     = 0x39,
    SNSR_ID_PEX_12V_POWER                   = 0x3A,
    SNSR_ID_PEX_3V3_POWER                   = 0x3B,
	SNSR_ID_AUX_3V3_POWER                   = 0x3C,
    SNSR_ID_RESERVED_0                      = 0x3D,      // These 2 IDs are not to be sent by SC
    SNSR_ID_RESERVED_1                      = 0x3E,      // Reserved to jump over CMC_HOST_MSG_OFFSET_REG etc in reg map
	SNSR_ID_VCC1V2_I                        = 0x3F,
	SNSR_ID_V12_IN_I                        = 0x40,
	SNSR_ID_V12_IN_AUX0_I                   = 0x41,
	SNSR_ID_V12_IN_AUX1_I                   = 0x42,
	SNSR_ID_VCCAUX                          = 0x43,
	SNSR_ID_VCCAUX_PMC                      = 0x44,
	SNSR_ID_VCCRAM                          = 0x45,
    SNSR_ID_POWER_GOOD                      = 0x46,
	SNSR_ID_VCCINT_POWER					= 0x47,
	SNSR_ID_VCCINT_VCU_0V9					= 0x48,
    SNSR_ID_1V2_VCCIO                       = 0x49,
    SNSR_ID_GTAVCC                          = 0x4A,
    SNSR_ID_NEW_MAC_SCHEME                  = 0x4B,     // TODO can we move this back
    SNSR_ID_VCCSOC                          = 0x4C,
    SNSR_ID_VCC_5V0                         = 0x4D,
    SNSR_ID_2V5_VPP23                       = 0x4E,
    SNSR_ID_GTVCC_AUX                       = 0x4F,

	CMC_MAX_NUM_SNSRS
};


enum // satellite controller error reporting ids
{
	CMC_REPORTING_ID_ECC_UE_ERROR_COUNT,
	CMC_REPORTING_ID_ECC_CE_ERROR_COUNT,
	CMC_REPORTING_ID_PCIE_SURPRISE_DOWN_ERROR_COUNT,
	CMC_REPORTING_ID_PCIE_UNSUPPORTED_REQUEST_COUNT,
	CMC_REPORTING_ID_PCIE_RECEIVER_ERROR_COUNT,
	CMC_REPORTING_ID_PCIE_REPLAY_TIMER_TIMEOUT_COUNT,
	CMC_REPORTING_ID_MAX
};


enum // satellite controller error reporting ids
{
	CMC_SHELL_VERSION,
	CMC_KEEPALIVE_COUNTER,
    CMC_UUID,
};

#ifdef __cplusplus
}
#endif


#endif /* _CMC_DATA_STORE_SENSOR_IDENTIFICATION_H_ */








