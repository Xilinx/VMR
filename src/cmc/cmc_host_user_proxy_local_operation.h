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
 *  $Revision: #14 $
 *
 */




#ifndef _CMC_HOST_USER_PROXY_LOCAL_OPERATION_H_
#define _CMC_HOST_USER_PROXY_LOCAL_OPERATION_H_


#ifdef __cplusplus
extern "C"
{
#endif



#include "cmc_common.h"




typedef enum HOST_PROXY_LOCAL_OPERATION_TYPE
{
    HP_LOCAL_OPERATION_NONE=0,
    HP_LOCAL_OPERATION_REBOOT_FIRMWARE,
    HP_LOCAL_OPERATION_STOP_FIRMWARE,
    HP_LOCAL_OPERATION_CLEAR_ERROR_REGISTER,
    HP_LOCAL_OPERATION_CLEAR_SENSOR_REGISTER,
    HP_LOCAL_OPERATION_ENABLE_DEBUG_CAPTURE,
    HP_LOCAL_OPERATION_ENABLE_REMOTE_DEBUG_UART,
    HP_LOCAL_OPERATION_DEBUG_CAPTURE_MSG_ID,
    HP_LOCAL_OPERATION_FIRST_CSDR_REQUEST,
    HP_LOCAL_OPERATION_ADDITIONAL_CSDR_REQUEST,
    HP_LOCAL_OPERATION_CSDR_COMPLETE,
    HP_LOCAL_OPERATION_CSDR_FAILED,
    HP_LOCAL_OPERATION_READ_QSFP_DIAGNOSTICS,
    HP_LOCAL_OPERATION_READ_QSFP_VALIDATE_LOW_SPEED_IO,
    HP_LOCAL_OPERATION_WRITE_QSFP_VALIDATE_LOW_SPEED_IO,
    HP_LOCAL_OPERATION_READ_QSFP_DIAGNOSTICS_COMPLETE,
    HP_LOCAL_OPERATION_READ_QSFP_VALIDATE_LOW_SPEED_IO_COMPLETE,
    HP_LOCAL_OPERATION_WRITE_QSFP_VALIDATE_LOW_SPEED_IO_COMPLETE,
    HP_LOCAL_OPERATION_READ_QSFP_DIAGNOSTICS_FAILED,
    HP_LOCAL_OPERATION_READ_QSFP_VALIDATE_LOW_SPEED_IO_FAILED,
    HP_LOCAL_OPERATION_WRITE_QSFP_VALIDATE_LOW_SPEED_IO_FAILED,
    HP_LOCAL_OPERATION_HBM_SUPPORT_CMS,
    HP_LOCAL_OPERATION_CLOCK_SCALING_CMS,
    HP_LOCAL_OPERATION_BOARDINFO_REQUEST,
    HP_LOCAL_OPERATION_READ_BOARDINFO_COMPLETE,
    HP_LOCAL_OPERATION_READ_BOARDINFO_FAILED,
    HP_LOCAL_OPERATION_QSFP_READ_SINGLE_BYTE_REQUEST,
    HP_LOCAL_OPERATION_QSFP_READ_SINGLE_BYTE_COMPLETE,
    HP_LOCAL_OPERATION_QSFP_READ_SINGLE_BYTE_FAILED,
    HP_LOCAL_OPERATION_QSFP_WRITE_SINGLE_BYTE_REQUEST,
    HP_LOCAL_OPERATION_QSFP_WRITE_SINGLE_BYTE_COMPLETE,
    HP_LOCAL_OPERATION_QSFP_WRITE_SINGLE_BYTE_FAILED,
    HP_LOCAL_OPERATION_LOW_SPEED_QSFP_FROM_GPIO_CMS,

    S_HP_MAX_LOCAL_OPERATIONS
    
} HOST_PROXY_LOCAL_OPERATION_TYPE;






#ifdef __cplusplus
}
#endif


#endif /* _CMC_HOST_USER_PROXY_LOCAL_OPERATION_H_ */










