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
 *  $Change: 2680580 $
 *  $Date: 2019/10/01 $
 *  $Revision: #6 $
 *
 */


#ifndef _CMC_HOST_USER_PROXY_ACTION_H_
#define _CMC_HOST_USER_PROXY_ACTION_H_


#ifdef __cplusplus
extern "C"
{
#endif



#include "cmc_common.h"



#define A_HP_NO_ACTION                                              (0x00000000)
#define A_HP_T_TRANSACTION_START                                    (0x00000001)
#define A_HP_T_TRANSACTION_STOP                                     (0x00000002)
#define A_HP_PERFORM_LOCAL_ACTION                                   (0x00000004)
#define A_HP_START_REMOTE_ACTION                                    (0x00000008)
#define A_HP_CANCEL_REMOTE_OPERATION                                (0x00000010)
#define A_HP_ACKNOWLEDGE_REMOTE_OPERATION_FAILED                    (0x00000020)
#define A_HP_ACKNOWLEDGE_REMOTE_OPERATION_SUCCESS                   (0x00000040)
#define A_HP_ACTIVATE_BOOTLOADER                                    (0x00000080)


#define MAX_HP_ACTIONS                                              (8)




#ifdef __cplusplus
}
#endif


#endif /* _CMC_HOST_USER_PROXY_ACTION_H_ */








