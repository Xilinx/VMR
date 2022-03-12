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
 *  $Change: 2754926 $
 *  $Date: 2020/01/17 $
 *  $Revision: #16 $
 *
 */



/*
 *  Mutex Handshake Function
 *  
 *  For v2.0 subsystems all peripherals accessed by the CMC Microblaze will reside locally within the CMC subsystem and be addressed
 *  using the static addresses defined in the Microblaze Address Map.
 *  
 *  To support future requirements for the CMC to access peripherals in the PLP or ULP, a master AXI-Lite 'reach-out' interface is
 *  provided (e.g for clock throttling the CMC will access the User Clocking Subsystem in the ULP).
 *  
 *  The 'reach-out' interface will be merged with the AXI-Lite control interface output from the BLP to the level 0 Isolation Interface
 *  (See Shell Subsystem - Level 0 Specification for more detail).
 *  
 *  A Mutex handshake function will be provided to enable XRT to manage CMC access to the PLP/ULP during partial reconfiguration.
 *  
 *  The handshake is a simple 2-pin function.
 *  
 *  
 *  
 *  Grant
 *  
 *  The 'Grant' pin should be asserted by XRT to grant CMC access to the master AXI-Lite 'reach-out' interface.
 *  For 2019.2_pu1 release,  the 'Grant' pin need only be asserted when a ULP is present and the 'New Feature'
 *  table has been initialized.
 *  
 *  Prior to PR of the PLP or ULP the 'Gran't pin should be de-asserted and XRT should wait for the CMC to confirm
 *  access to the reach-out port has ceased ('Ack' = 0)
 *  
 *  
 *  Ack
 *
 *  The 'Acknowledge' pin should be asserted by the CMC to confirm access to the master AXI-Lite 'reach-out' interface is active.
 *  The 'Acknowledge' pin should be de-asserted by the CMC to confirm access to the master AXI-Lite 'reach-out' interface has ceased.
 *  
 */

  





#ifndef _CMC_PERIPHERAL_AXI_GPIO_MUTEX_CMC_H_
#define _CMC_PERIPHERAL_AXI_GPIO_MUTEX_CMC_H_


#ifdef __cplusplus
extern "C"
{
#endif

#include "cmc_common.h"
#include "cmc_peripherals_access.h"
#include "cmc_required_environment.h"
#include "cmc_peripheral_axi_gpio_mutex_cmc_constants.h"
#include "cmc_watchpoint.h"


#define PERIPHERAL_AXI_GPIO_MUTEX_CMC_REGISTER_SET_SIZE (2)


typedef struct PERIPHERAL_AXI_GPIO_MUTEX_CMC_CONTEXT_TYPE
{
    CMC_PERIPHERAL_BASE_ADDRESS_TYPE BaseAddress;
    bool IsAvailable;
    CMC_REQUIRED_ENVIRONMENT_CONTEXT_TYPE       * pRequiredEnvironmentContext;
    CMC_WATCHPOINT_CONTEXT_TYPE                 * pWatchpointContext;


} PERIPHERAL_AXI_GPIO_MUTEX_CMC_CONTEXT_TYPE;



bool PERIPHERAL_AXI_GPIO_MUTEX_CMC_ReachOutInterfaceAccessIsGranted(PERIPHERAL_AXI_GPIO_MUTEX_CMC_CONTEXT_TYPE *pContext);
void PERIPHERAL_AXI_GPIO_MUTEX_CMC_AcknowledgeCMCIsAccessingReachOutInterface(PERIPHERAL_AXI_GPIO_MUTEX_CMC_CONTEXT_TYPE *pContext);
void PERIPHERAL_AXI_GPIO_MUTEX_CMC_CMCIsNoLongerAccessingReachOutInterface(PERIPHERAL_AXI_GPIO_MUTEX_CMC_CONTEXT_TYPE *pContext);
void PERIPHERAL_AXI_GPIO_MUTEX_CMC_RegMapIsReady(PERIPHERAL_AXI_GPIO_MUTEX_CMC_CONTEXT_TYPE* pContext);
void PERIPHERAL_AXI_GPIO_MUTEX_CMC_RegMapIsNotReady(PERIPHERAL_AXI_GPIO_MUTEX_CMC_CONTEXT_TYPE* pContext);

void PERIPHERAL_AXI_GPIO_MUTEX_CMC_Initialize(PERIPHERAL_AXI_GPIO_MUTEX_CMC_CONTEXT_TYPE *pContext, CMC_PERIPHERAL_BASE_ADDRESS_TYPE AnyBaseAddress, bool IsAvailable, CMC_REQUIRED_ENVIRONMENT_CONTEXT_TYPE * pRequiredEnvironmentContext, CMC_WATCHPOINT_CONTEXT_TYPE* pWatchpointContext);
void PERIPHERAL_AXI_GPIO_MUTEX_CMC_Start(PERIPHERAL_AXI_GPIO_MUTEX_CMC_CONTEXT_TYPE *pContext);




#ifdef __cplusplus
}
#endif


#endif /* _CMC_PERIPHERAL_AXI_GPIO_MUTEX_CMC_H_ */













