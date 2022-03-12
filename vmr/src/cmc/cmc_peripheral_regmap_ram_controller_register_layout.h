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
 *  $Change: 3136694 $
 *  $Date: 2021/03/02 $
 *  $Revision: #47 $
 *
 */





#ifndef _CMC_PERIPHERAL_REGMAP_RAM_CONTROLLER_REGISTER_LAYOUT_H_
#define _CMC_PERIPHERAL_REGMAP_RAM_CONTROLLER_REGISTER_LAYOUT_H_


#ifdef __cplusplus
extern "C"
{
#endif



#define HOST_REGISTER_MAGIC                                                         (0x00000000)
#define HOST_REGISTER_VERSION                                                       (0x00000004)
#define HOST_REGISTER_STATUS                                                        (0x00000008)
#define HOST_REGISTER_ERROR_REG                                                     (0x0000000C)
#define HOST_REGISTER_FEATURE                                                       (0x00000010)
#define HOST_REGISTER_PROFILE_NAME                                                  (0x00000014)
#define HOST_REGISTER_CONTROL                                                       (0x00000018)
#define HOST_REGISTER_STOP_CONFIRM                                                  (0x0000001C)

////sensor registers, max/avg/instant values stored per sensor
#define HOST_REGISTER_SNSR_BASE_REG                                                 (0x00000020)
#define HOST_REGISTER_SNSR_MAX_REG_OFFSET                                           (0x00000020)
#define HOST_REGISTER_SNSR_AVG_REG_OFFSET                                           (0x00000024)
#define HOST_REGISTER_SNSR_INS_REG_OFFSET                                           (0x00000028)

//// host messages
#define CMC_HEARTBEAT                                                               (0x000002FC)
#define HOST_REGISTER_LOCATION_OF_HOST_MESSAGE_REGISTER                             (0x00000300)
#define HOST_REGISTER_RESULT_ERROR_CODE_REGISTER                                    (0x00000304)

#define HOST_REGISTER_STATUS2                                                       (0x0000030C)
#define CMC_HEARTBEAT_ERR_CODE                                                      (0x00000310)

//// watchpoint counters
#define CMC_WATCHPOINT_BASE_REG                                                     (0x000004D0)    // 0x310 to max of 0x57C
#define CMC_WATCHPOINT2_BASE_REG                                                    (0x00000C94)
#define CMC_WATCHPOINT3_BASE_REG                                                    (0x00001C00)

//// coverage


#define CMC_SC_MSG_COVERAGE_REG                                                     (0x00000580)
#define CMC_SC_SNSR_COVERAGE_REG                                                    (0x000009C0)




//// New Feature Support
//
// 32 registers have been reserved in the Host / CMC shared memory for XRT to pass information
// for new features to the CMC.
// The CMC will use the 'reach-out' master AXI interface to access these features when enabled 
// by the host/CMC Mutex Handshake function.
//
//// New Feature Support
#define HOST_REGISTER_LOCATION_NEW_FEATURE_REGISTER_00                              (0x00000B20)  // Clock Throttling Feature
#define HOST_REGISTER_LOCATION_NEW_FEATURE_REGISTER_01                              (0x00000B24)
#define HOST_REGISTER_LOCATION_NEW_FEATURE_REGISTER_02                              (0x00000B28)
#define HOST_REGISTER_LOCATION_NEW_FEATURE_REGISTER_03                              (0x00000B2C)
#define HOST_REGISTER_LOCATION_NEW_FEATURE_REGISTER_04                              (0x00000B30)
#define HOST_REGISTER_LOCATION_NEW_FEATURE_REGISTER_05                              (0x00000B34)
#define HOST_REGISTER_LOCATION_NEW_FEATURE_REGISTER_06                              (0x00000B38)
#define HOST_REGISTER_LOCATION_NEW_FEATURE_REGISTER_07                              (0x00000B3C)
#define HOST_REGISTER_LOCATION_NEW_FEATURE_REGISTER_08                              (0x00000B40)
#define HOST_REGISTER_LOCATION_NEW_FEATURE_REGISTER_09                              (0x00000B44)
#define HOST_REGISTER_LOCATION_NEW_FEATURE_REGISTER_10                              (0x00000B48)
#define HOST_REGISTER_LOCATION_NEW_FEATURE_REGISTER_11                              (0x00000B4C)
#define HOST_REGISTER_LOCATION_NEW_FEATURE_REGISTER_12                              (0x00000B50)
#define HOST_REGISTER_LOCATION_NEW_FEATURE_REGISTER_13                              (0x00000B54)
#define HOST_REGISTER_LOCATION_NEW_FEATURE_REGISTER_14                              (0x00000B58)
#define HOST_REGISTER_LOCATION_NEW_FEATURE_REGISTER_15                              (0x00000B5C)
#define HOST_REGISTER_LOCATION_NEW_FEATURE_REGISTER_16                              (0x00000B60)
#define HOST_REGISTER_LOCATION_NEW_FEATURE_REGISTER_17                              (0x00000B64)
#define HOST_REGISTER_LOCATION_NEW_FEATURE_REGISTER_18                              (0x00000B68)
#define HOST_REGISTER_LOCATION_NEW_FEATURE_REGISTER_19                              (0x00000B6C)
#define HOST_REGISTER_LOCATION_NEW_FEATURE_REGISTER_20                              (0x00000B70)
#define HOST_REGISTER_LOCATION_NEW_FEATURE_REGISTER_21                              (0x00000B74)
#define HOST_REGISTER_LOCATION_NEW_FEATURE_REGISTER_22                              (0x00000B78)
#define HOST_REGISTER_LOCATION_NEW_FEATURE_REGISTER_23                              (0x00000B7C)
#define HOST_REGISTER_LOCATION_NEW_FEATURE_REGISTER_24                              (0x00000B80)
#define HOST_REGISTER_LOCATION_NEW_FEATURE_REGISTER_25                              (0x00000B84)
#define HOST_REGISTER_LOCATION_NEW_FEATURE_REGISTER_26                              (0x00000B88)
#define HOST_REGISTER_LOCATION_NEW_FEATURE_REGISTER_27                              (0x00000B8C)
#define HOST_REGISTER_LOCATION_NEW_FEATURE_REGISTER_28                              (0x00000B90)
#define HOST_REGISTER_LOCATION_NEW_FEATURE_REGISTER_29                              (0x00000B94)
#define HOST_REGISTER_LOCATION_NEW_FEATURE_REGISTER_30                              (0x00000B98)
#define HOST_REGISTER_LOCATION_NEW_FEATURE_REGISTER_31                              (0x00000B9C)
#define MAX_NEW_FEATURE_REGISTERS                                                   (32)


#define HOST_REGISTER_ALERT_RESP_WARNING_REG                                        (0x00000BA0)
#define HOST_REGISTER_ALERT_RESP_CRITICAL_REG                                       (0x00000BA4)
#define HOST_REGISTER_SNSR_STATE_BASE_REG                                           (0x00000BA8)    // 0xBA8 to 0xBE8 inclusive

// Used to debug Clock Throttling Temperture Code
#define HOST_REGISTER_CT_KP_FPGA                                                    (0x00000BEC)
#define HOST_REGISTER_CT_KI                                                         (0x00000BF0)
#define HOST_REGISTER_CT_KAW                                                        (0x00000BF4)
#define HOST_REGISTER_CT_KP_VCCINT                                                  (0x00000BF8)
#define HOST_REGISTER_CT_MUTEX                                                      (0x00000BFC)
#define HOST_REGISTER_CT_DEBUG                                                      (0x00000C00) 


#define HOST_REGISTER_ZYNC_UART_STATUS                                              (0x00000C44)
#define HOST_REGISTER_QSPI_STATUS                                                   (0x00000C48)
#define HOST_REGISTER_CORE_VERSION                                                  (0x00000C4C)
//// OEM ID
#define HOST_REGISTER_OEM_ID                                                        (0x00000C50)//(0x000002C0)

#define HOST_MAC0                                                                   (0x000001A0)
#define HOST_MAC1                                                                   (0x000001A8)
#define HOST_MAC2                                                                   (0x000001B0)
#define HOST_MAC3                                                                   (0x000001B8)
#define HOST_MAC4                                                                   (0x000001C0)
#define HOST_MAC5                                                                   (0x000001C8)
#define HOST_MAC6                                                                   (0x000001D0)
#define HOST_MAC7                                                                   (0x000001D8)




//#define CMC_CSDR_EXTENDED_BASE_REG                                                  (0x00000C08) // 0x0C08 - 0x0C4C
//#define CMC_CSDR_BASE_REG                                                           (0x00000C54) // 0x0C54 - 0x0C90

#define HOST_REGISTER_ECC_UE_ERROR_COUNT                                            (0x00000E50)
#define HOST_REGISTER_ECC_CE_ERROR_COUNT                                            (0x00000E54)
#define HOST_REGISTER_PCIE_SURPRISE_DOWN_ERROR_COUNT                                (0x00000E58)
#define HOST_REGISTER_PCIE_UNSUPPORTED_REQUEST_COUNT                                (0x00000E5C)
#define HOST_REGISTER_PCIE_RECEIVER_ERROR_COUNT                                     (0x00000E60)
#define HOST_REGISTER_PCIE_REPLAY_TIMER_TIMEOUT_COUNT                               (0x00000E64)

#define HOST_REGISTER_POWER_THRESHOLD_BASE_REG                                      (0x00000E68)    // 0xE68 to 0xE8C inclusive
#define HOST_REGISTER_TEMP_THRESHOLD_BASE_REG                                       (0x00000E90)    // 0xE90 to 0xEB4 inclusive


//// hw build info
#define SHELL_TOOLS_VERSION                                                         (0x00000EB8)//(0x000002C4) // Vivado Version 31:8 & Subsystem Unique ID 7:0
#define SHELL_BUILD_VERSION                                                         (0x00000EBC)//(0x000002C8) // Major Version 31:16 & Minor Version 15:0
#define SHELL_SS_PATCH_CORE_REVISION                                                (0x00000EC0)//(0x000002CC) // Patch Revision 31:16 (Reserved for future) & Core Revision 15:0
#define SHELL_PERFORCE_CL                                                           (0x00000EC4)//(0x000002D0) // Perforce Changelist
#define SHELL_RESERVED_TAG                                                          (0x00000EC8)//(0x000002D4) // Reserved Tag for designer build use

                                                                                                    // Setting base address to 0xEC8 even though it is not used for coverage 
#define CMC_HOST_MSG_COVERAGE_REG                                                   (0x00000EC8)    // 0xECC to 0xEDC inclusive (only 5 messages max)
                                                                                                    // NOTE: Host messages are 1-5 inclusive

#define CMC_HOST_ERR_COVERAGE_REG                                                   (0x00000EE0) // 0x0EE0 - 0x0EFC




// debug buffer14
#define CMC_DEBUG_BUFFER_REG                                                        (0x00001900)    // reserve 0x0FF, SAT_COMMS_MAX_MSG_SIZE=0x20E

#define HOST_REGISTER_REMOTE_COMMAND_REGISTER                                       (0x00001000)
#define HOST_REGISTER_REMOTE_COMMAND_ADDRESS_REGISTER_FIRST_SEGMENT                 (0x00001004)
#define HOST_REGISTER_REMOTE_COMMAND_COMPLETE_RECORD_LENGTH_REGISTER_FIRST_SEGMENT  (0x00001008)
#define HOST_REGISTER_REMOTE_COMMAND_START_OF_PAYLOAD_REGISTER_FIRST_SEGMENT        (0x0000100C)

#define HOST_REGISTER_REMOTE_COMMAND_START_OF_PAYLOAD_REGISTER_SUBSEQUENT_SEGMENT   (0x00001004)


#define HOST_REGISTER_REMOTE_COMMAND_FIRST_CSDR_TOTAL_RECORD_SIZE_RESPONSE          (0x00001004)
#define HOST_REGISTER_REMOTE_COMMAND_FIRST_CSDR_THIS_RECORD_SIZE_RESPONSE           (0x00001008)
#define HOST_REGISTER_REMOTE_COMMAND_FIRST_CSDR_RECORD_RESPONSE                     (0x0000100C)

#define HOST_REGISTER_REMOTE_COMMAND_ADDITIONAL_CSDR_THIS_RECORD_SIZE_RESPONSE      (0x00001004)
#define HOST_REGISTER_REMOTE_COMMAND_ADDITIONAL_CSDR_RECORD_RESPONSE                (0x00001008)

#define HOST_REGISTER_REMOTE_COMMAND_READWRITE_QSFP_WHICH_QSFP                      (0x00001004)
#define HOST_REGISTER_REMOTE_COMMAND_READWRITE_QSFP_WHICH_PAGE                      (0x00001008)
#define HOST_REGISTER_REMOTE_COMMAND_READWRITE_QSFP_LOWER_OR_UPPER                  (0x0000100C)
#define HOST_REGISTER_REMOTE_COMMAND_READWRITE_QSFP_BYTE_OFFSET                     (0x00001010)
#define HOST_REGISTER_REMOTE_COMMAND_READWRITE_QSFP_BYTE_VALUE                      (0x00001014)

#define HOST_REGISTER_REMOTE_COMMAND_READWRITE_QSFP_DIAGNOSTICS_LENGTH              (0x00001010)
#define HOST_REGISTER_REMOTE_COMMAND_READWRITE_QSFP_DIAGNOSTICS_DATA                (0x00001014)
#define HOST_REGISTER_REMOTE_COMMAND_READWRITE_QSFP_VALIDATE_LOW_SPEED_IO_DATA      (0x00001008)

#define HOST_REGISTER_REMOTE_COMMAND_WRITE_QSFP_CONTROL_LENGTH                      (0x00001008)
#define HOST_REGISTER_REMOTE_COMMAND_WRITE_QSFP_CONTROL_DATA                        (0x0000100C)



//#define HOST_REGISTER_REMOTE_COMMAND_ADDITIONAL_CSDR_RECORD_RESPONSE                (0x00001008)


// FW load is 0x3FF


//// message trace
#define CMC_SAT_TX_MSG_TRACE_REG                                                    (0x00001E00)



//                                                                            MAX is 0x00002000


#ifdef __cplusplus
}
#endif


#endif /* _CMC_PERIPHERAL_REGMAP_RAM_CONTROLLER_REGISTER_LAYOUT_H_ */



















