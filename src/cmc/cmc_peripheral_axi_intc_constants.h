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
 *  $Change: 2685649 $
 *  $Date: 2019/10/08 $
 *  $Revision: #4 $
 *
 */




#ifndef _CMC_PERIPHERAL_AXI_INTC_CONSTANTS_H_
#define _CMC_PERIPHERAL_AXI_INTC_CONSTANTS_H_


#ifdef __cplusplus
extern "C"
{
#endif

#include "cmc_common.h"


/*
 *
 * NB - use byte ofset, not word offset 
 *
 */

#define PERIPHERAL_AXI_INTC_XIN_ISR_OFFSET      (0*4)	/* Interrupt Status Register */
#define PERIPHERAL_AXI_INTC_XIN_IPR_OFFSET      (1*4)	/* Interrupt Pending Register */
#define PERIPHERAL_AXI_INTC_XIN_IER_OFFSET      (2*4)	/* Interrupt Enable Register */
#define PERIPHERAL_AXI_INTC_XIN_IAR_OFFSET      (3*4)	/* Interrupt Acknowledge Register */
#define PERIPHERAL_AXI_INTC_XIN_SIE_OFFSET      (4*4)	/* Set Interrupt Enable Register */
#define PERIPHERAL_AXI_INTC_XIN_CIE_OFFSET      (5*4)	/* Clear Interrupt Enable Register */
#define PERIPHERAL_AXI_INTC_XIN_IVR_OFFSET      (6*4)	/* Interrupt Vector Register */
#define PERIPHERAL_AXI_INTC_XIN_MER_OFFSET      (7*4)	/* Master Enable Register */
#define PERIPHERAL_AXI_INTC_XIN_IMR_OFFSET      (8*4)	/* Interrupt Mode Register , this is present
				 *  only for Fast Interrupt */
#define PERIPHERAL_AXI_INTC_XIN_IVAR_OFFSET     (64*4)  /* Interrupt Vector Address Register
				 * Interrupt 0 Offest, this is present
				 * only for Fast Interrupt */


#define PERIPHERAL_AXI_INTC_XIN_INT_MASTER_ENABLE_MASK      0x1UL
#define PERIPHERAL_AXI_INTC_XIN_INT_HARDWARE_ENABLE_MASK    0x2UL	/* once set cannot be cleared */









#ifdef __cplusplus
}
#endif


#endif /* _CMC_PERIPHERAL_AXI_INTC_CONSTANTS_H_ */






