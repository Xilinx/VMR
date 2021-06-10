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
 *  $Change: 3137936 $
 *  $Date: 2021/03/03 $
 *  $Revision: #14 $
 *
 */




#ifndef _CMC_THREAD_LINKSTATE_EVENT_H_
#define _CMC_THREAD_LINKSTATE_EVENT_H_


#ifdef __cplusplus
extern "C"
{
#endif


#define E_LS_RESET_REQUEST											(0x01)
#define E_LS_T_RESPONSE_EXPIRY										(0x02)
#define E_LS_LINK_USER_IS_REMOTE_SENSOR_SUPERVISOR					(0x03)
#define E_LS_LINK_USER_IS_REMOTE_BOOTLOADER							(0x04)
#define E_LS_LINK_USER_TO_BOOTLOADER_REQUEST						(0x05)
#define E_LS_LINK_OFFERED_VERSION_ACKNOWLEDGE						(0x06)
#define E_LS_LINK_OFFERED_VERSION_DECLINED							(0x07)
#define E_LS_LINK_ACCEPTED_PROTOCOL_VERSION_RANGE_RESPONSE			(0x08)
#define E_LS_LINK_ACCEPTED_PROTOCOL_VERSION_RANGE_RESPONSE_FAILED   (0x09)
#define E_LS_T_BSL_EXPIRY											(0x0A)
#define E_LS_ENABLE_BSL_RESPONSE									(0x0B)
#define E_LS_ENABLE_BSL_RESPONSE_FAILED                             (0x0C)
#define E_LS_LINK_I2C_ACKNOWLEDGE                                   (0x0D)
#define E_LS_LINK_I2C_FAILED                                        (0x0E)

#ifdef __cplusplus
}
#endif


#endif /* _CMC_THREAD_LINKSTATE_EVENT_H_ */







