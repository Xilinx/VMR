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
 *  $Revision: #11 $
 *
 */




#ifndef _CMC_USER_SUPPLIED_MEMORY_MAP_PERIPHERAL_LIST_H_
#define _CMC_USER_SUPPLIED_MEMORY_MAP_PERIPHERAL_LIST_H_


#ifdef __cplusplus
extern "C"
{
#endif

#include "cmc_common.h"


typedef enum USER_SUPPLIED_MEMORY_MAP_PERIPHERAL_LIST_TYPE
{
    PERIPHERAL_IDENTIFIER_AXI_GPIO_HBM=0,
    PERIPHERAL_IDENTIFIER_AXI_CLOCK_THROTTLING,
    PERIPHERAL_IDENTIFIER_AXI_GPIO_MB_INTERRUPTS,
    PERIPHERAL_IDENTIFIER_AXI_GPIO_MUTEX_CMC,
    PERIPHERAL_IDENTIFIER_AXI_GPIO_GPIO_QSFP,
    PERIPHERAL_IDENTIFIER_AXI_GPIO_GPIO_WDT,

    PERIPHERAL_IDENTIFIER_AXI_INTC,
    PERIPHERAL_IDENTIFIER_AXI_TIMEBASE_WDT,
    PERIPHERAL_IDENTIFIER_AXI_UART_LITE_SATELLITE,
    PERIPHERAL_IDENTIFIER_FREE_RUNNING_CLOCK,
    PERIPHERAL_IDENTIFIER_REGMAP_RAM_CONTROLLER,
    PERIPHERAL_IDENTIFIER_UART_LITE_USB,
    PERIPHERAL_IDENTIFIER_HW_BUILD_INFO,
    PERIPHERAL_IDENTIFIER_RESET_VECTOR,

    PERIPHERAL_IDENTIFIER_X2_GPIO,
    PERIPHERAL_IDENTIFIER_X2_I2C_DEVICE,

    PERIPHERAL_IDENTIFIER_SYSMON,
    PERIPHERAL_IDENTIFIER_DNA,

    PERIPHERAL_MAX_PERIPHERALS
    
} USER_SUPPLIED_MEMORY_MAP_PERIPHERAL_LIST_TYPE;




#ifdef __cplusplus
}
#endif


#endif /* _CMC_USER_SUPPLIED_MEMORY_MAP_PERIPHERAL_LIST_H_ */

















