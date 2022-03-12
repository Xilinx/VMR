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
 *  $Change: 3143817 $
 *  $Date: 2021/03/09 $
 *  $Revision: #26 $
 *
 */




#ifndef _CMC_PERIPHERAL_REGMAP_RAM_CONTROLLER_REGISTER_CONTENTS_H_
#define _CMC_PERIPHERAL_REGMAP_RAM_CONTROLLER_REGISTER_CONTENTS_H_


#ifdef __cplusplus
extern "C"
{
#endif




/*
 *
 * HOST_REGISTER_MAGIC
 *
 */

#define HOST_REGISTER_PATTERN_MAGIC     (0x74736574)




/*
 *
 * HOST_REGISTER_VERSION
 *
 */

#define HOST_REGISTER_PATTERN_VERSION   (CMC_VERSION)







/*
 *
 * HOST_REGISTER_STATUS
 *
 */


#define HOST_REGISTER_PATTERN_INIT_DONE                                 (1<<0)
#define HOST_REGISTER_PATTERN_STOPPED                                   (1<<1)
#define HOST_REGISTER_PATTERN_WDT_RESET_STATUS            (1<<3)
#define HOST_REGISTER_PATTERN_SAT_FW_STATUS_OFFSET        (4)
#define HOST_REGISTER_PATTERN_SAT_FW_STATUS_MASK          (0b111111<<HOST_REGISTER_PATTERN_SAT_FW_STATUS_OFFSET)
#define HOST_REGISTER_PATTERN_POWER_MODE_OFFSET           (10)
#define HOST_REGISTER_PATTERN_POWER_MODE_MASK             (0b11<<HOST_REGISTER_PATTERN_POWER_MODE_OFFSET)
#define HOST_REGISTER_PATTERN_SAT_VER_OFFSET              (24)
#define HOST_REGISTER_PATTERN_SAT_VER_MASK                (0b1111<<HOST_REGISTER_PATTERN_SAT_VER_OFFSET)
#define HOST_REGISTER_PATTERN_SAT_MODE_OFFSET             (28)
#define HOST_REGISTER_PATTERN_SAT_MODE_MASK               (0b1111<<HOST_REGISTER_PATTERN_SAT_MODE_OFFSET)


#define HOST_STATUS_REGISTER_SC_MODE_UNKNOWN_MODE                       (0)
#define HOST_STATUS_REGISTER_SC_MODE_NORMAL_MODE                        (1)
#define HOST_STATUS_REGISTER_SC_MODE_BSL_MODE_UNSYNCED                  (2)
#define HOST_STATUS_REGISTER_SC_MODE_BSL_MODE_SYNCED                    (3)

#define HOST_STATUS_REGISTER_SC_MODE_BSL_MODE_SYNCED_SC_NOT_UPGRADABLE  (4)
#define HOST_STATUS_REGISTER_SC_MODE_NORMAL_MODE_SC_NOT_UPGRADABLE      (5)

 /*
  *
  * HOST_REGISTER_STATUS2
  *
  */
#define HOST_STATUS2_SENSOR_DATA_AVAILABLE                 (1<<0)



/*  error and warnings register
    warnings will self clear on read, errors will remain until the reset is set to clear */
#define HOST_ERROR_REG_SAT_RX_ERROR_MASK           (0x03F00000)
#define HOST_ERROR_REG_SAT_RX_ERROR_OFFSET         (20)
#define HOST_ERROR_REG_HOST_MSG_ERROR              (1<<26)
#define HOST_ERROR_REG_SAT_COMMS_ERROR             (1<<27)




/*
 *
 * HOST_REGISTER_CONTROL
 *
 */

#define HOST_REGISTER_PATTERN_RESET_SENSORS                             (1<<0)
#define HOST_REGISTER_PATTERN_CLEAR_ERRORS                              (1<<1)
#define HOST_REGISTER_PATTERN_STOP_FIRMWARE                             (1<<3)
#define HOST_REGISTER_PATTERN_REMOTE_COMMAND                            (1<<5)
#define HOST_REGISTER_PATTERN_REBOOT_FIRMWARE                           (1<<6)
#define HOST_REGISTER_PATTERN_DEBUG_CAPTURE_MSG_ID                      (0xFF<<8)
#define HOST_REGISTER_PATTERN_LOW_SPEED_QSFP_FROM_GPIO_CMS              (1<<26)
#define HOST_REGISTER_PATTERN_HBM_SUPPORT_CMS                           (1<<27)
#define HOST_REGISTER_PATTERN_CLOCK_SCALING_CMS                         (1<<28)
#define HOST_REGISTER_PATTERN_ENABLE_DEBUG_CAPTURE                      (1<<30)
#define HOST_REGISTER_PATTERN_ENABLE_REMOTE_DEBUG_UART                  (1<<31)

#define HOST_REGISTER_PATTERN_ANY_REQUEST                               (   HOST_REGISTER_PATTERN_RESET_SENSORS                 |    \
                                                                            HOST_REGISTER_PATTERN_CLEAR_ERRORS                  |    \
                                                                            HOST_REGISTER_PATTERN_STOP_FIRMWARE                 |    \
                                                                            HOST_REGISTER_PATTERN_REMOTE_COMMAND                |    \
                                                                            HOST_REGISTER_PATTERN_REBOOT_FIRMWARE               |    \
                                                                            HOST_REGISTER_PATTERN_LOW_SPEED_QSFP_FROM_GPIO_CMS  |    \
                                                                            HOST_REGISTER_PATTERN_HBM_SUPPORT_CMS               |    \
                                                                            HOST_REGISTER_PATTERN_CLOCK_SCALING_CMS             |    \
                                                                            HOST_REGISTER_PATTERN_ENABLE_DEBUG_CAPTURE          |    \
                                                                            HOST_REGISTER_PATTERN_ENABLE_REMOTE_DEBUG_UART  )



/*
 *
 * HOST_REGISTER_STOP_CONFIRM
 *
 */

#define HOST_REGISTER_PATTERN_STOP_CONFIRM                              (1<<0)



/*
 *
 * HOST_REGISTER_RESULT_ERROR_CODE_REGISTER
 *
 */

#define RESULT_ERROR_CODE_CMC_HOST_MSG_NO_ERROR                         (0x00000000)
#define RESULT_ERROR_CODE_CMC_HOST_MSG_BAD_OPCODE                       (0x00000001)
#define RESULT_ERROR_CODE_CMC_HOST_MSG_BRD_INFO_MISSING                 (0x00000002)
#define RESULT_ERROR_CODE_CMC_HOST_MSG_LENGTH                           (0x00000003)
#define RESULT_ERROR_CODE_CMC_HOST_MSG_SAT_FW_WRITE_FAIL                (0x00000004)
#define RESULT_ERROR_CODE_CMC_HOST_MSG_SAT_FW_UPDATE_FAIL               (0x00000005)
#define RESULT_ERROR_CODE_CMC_HOST_MSG_SAT_FW_LOAD_FAIL                 (0x00000006)
#define RESULT_ERROR_CODE_CMC_HOST_MSG_SAT_FW_ERASE_FAIL                (0x00000007)
#define RESULT_ERROR_CODE_CMC_HOST_MSG_SAT_FW_UPDATE_BLOCKED            (0x00000008)
#define RESULT_ERROR_CODE_CMC_HOST_MSG_CSDR_FAIL                        (0x00000009)
#define RESULT_ERROR_CODE_CMC_HOST_MSG_QSFP_FAIL                        (0x0000000A)

/*
 *
 * HOST_REGISTER_REMOTE_COMMAND_REGISTER
 *
 */

#define REMOTE_COMMAND_REGISTER_PATTERN_FIRST_DATA_SEGMENT                  (0x00000001)
#define REMOTE_COMMAND_REGISTER_PATTERN_DATA_SEGMENT                        (0x00000002)
#define REMOTE_COMMAND_REGISTER_PATTERN_JUMP_TO_RESET_VECTOR                (0x00000003)
#define REMOTE_COMMAND_REGISTER_PATTERN_BOARD_INFORMATION                   (0x00000004)
#define REMOTE_COMMAND_REGISTER_PATTERN_ERASE_FIRMWARE                      (0x00000005)
#define REMOTE_COMMAND_REGISTER_PATTERN_FIRST_CSDR_REQUEST                  (0x00000009)
#define REMOTE_COMMAND_REGISTER_PATTERN_ADDITIONAL_CSDR_REQUEST             (0x0000000A)
#define REMOTE_COMMAND_REGISTER_PATTERN_READ_QSFP_DIAGNOSTICS               (0x0000000B)
//#define REMOTE_COMMAND_REGISTER_PATTERN_WRITE_QSFP_CONTROL                  (0x0000000C)    Deprecated
#define REMOTE_COMMAND_REGISTER_PATTERN_READ_QSFP_VALIDATE_LOW_SPEED_IO     (0x0000000D)
#define REMOTE_COMMAND_REGISTER_PATTERN_WRITE_QSFP_VALIDATE_LOW_SPEED_IO    (0x0000000E)
#define REMOTE_COMMAND_REGISTER_PATTERN_QSFP_READ_SINGLE_BYTE               (0x0000000F)
#define REMOTE_COMMAND_REGISTER_PATTERN_QSFP_WRITE_SINGLE_BYTE              (0x00000010)



/*
 *
 * HOST_REGISTER_LOCATION_NEW_FEATURE_REGISTER_00 - Clock Throttling Feature
 *
 */

#define FEATURE_CLOCK_THROTTLING_FEATURE_PRESENT                        (1<<29)
#define FEATURE_CLOCK_THROTTLING_FEATURE_ENABLED                        (1<<28)
#define FEATURE_CLOCK_THROTTLING_BASE_ADDRESS_MASK                      (0x01FFFFFF)


 /*
  *
  * CMC_HEARTBEAT_ERR_CODE_REGISTER
  *
  */

#define SINGLE_EXPECTED_SENSOR_NOT_RECEIVED                             (1<<0)
#define MULTIPLE_EXPECTED_SENSORS_NOT_RECEIVED                          (1<<1)
#define HEARTBEAT_ERROR_CODE_MASK                                       (0x0000FFFF)
#define LAST_SENSOR_NOT_RECEIVED_MASK                                   (0x00FF0000)
#define SENSOR_NOT_RECEIVED_CNT_MASK                                    (0xFF000000)


#ifdef __cplusplus
}
#endif


#endif /* _CMC_PERIPHERAL_REGMAP_RAM_CONTROLLER_REGISTER_CONTENTS_H_ */
