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
 *  $Change: 3217619 $
 *  $Date: 2021/05/13 $
 *  $Revision: #17 $
 *
 */




#ifndef _CMC_THREAD_LINKSTATE_ACTION_H_
#define _CMC_THREAD_LINKSTATE_ACTION_H_


#ifdef __cplusplus
extern "C"
{
#endif


#define A_LS_NO_ACTION                                              (0x00000000)
#define A_LS_T_RESPONSE_START                                       (0x00000001)
#define A_LS_T_RESPONSE_STOP                                        (0x00000002)
#define A_LS_T_RESPONSE_RESTART                                     (0x00000004)
#define A_LS_SEND_LINK_OFFERED_PROTOCOL_VERSION                     (0x00000008)
#define A_LS_SEND_LINK_USER_QUERY                                   (0x00000010)
#define A_LS_SEND_LINK_USER_TO_BOOTLOADER_COMMAND                   (0x00000020)
#define A_LS_SEND_LINK_ACCEPTED_PROTOCOL_VERSION_RANGE_QUERY        (0x00000040)
#define A_LS_CACHE_LINK_ACCEPTED_PROTOCOL_VERSION_RANGE             (0x00000080)
#define A_LS_ANNOUNCE_LINK_USER_IS_SENSOR_SUPERVISOR                (0x00000100)
#define A_LS_ANNOUNCE_LINK_USER_IS_BOOTLOADER                       (0x00000200)
#define A_LS_ANNOUNCE_LINK_USER_IS_SENSOR_SUPERVISOR_LINK_OPERATING	(0x00000400)
#define A_LS_T_BSL_START                                            (0x00000800)
#define A_LS_SEND_LINK_FPGA_I2C_BUS_ARB                             (0x00001000)
#define A_LS_ANNOUNCE_COMMAND_FAILED                                (0x00002000)
#define A_LS_ANNOUNCE_LINK_USER_IS_SENSOR_SUPERVISOR_SUC            (0x00004000)

#define MAX_ACTIONS                                                 (16)


#ifdef __cplusplus
}
#endif


#endif /* _CMC_THREAD_LINKSTATE_ACTION_H_ */






