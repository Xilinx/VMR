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
 *  $Revision: #31 $
 *
 */




#include "cmc_initialization.h"
#include "cmc_peripherals.h"
#include "cmc_ram.h"





void cmc_Initialize_Peripherals(CMC_BUILD_PROFILE_TYPE * pProfile)
{

    HardwarePlatformServices_Initialize(                        &HardwarePlatformContext,
                                                                &pProfile->HardwarePlatformServices);


    PERIPHERAL_X2_I2C_Initialize(                               &X2_I2C_Device_Context,
                                                                &pProfile->Peripherals.X2_I2C,
                                                                pProfile->UserSuppliedMemoryMap.Peripheral[PERIPHERAL_IDENTIFIER_X2_I2C_DEVICE].BaseAddress,
                                                                pProfile->UserSuppliedMemoryMap.Peripheral[PERIPHERAL_IDENTIFIER_X2_I2C_DEVICE].IsAvailable,
                                                                &RequiredEnvironmentContext);

    PERIPHERAL_X2_GPIO_Initialize(                              &X2_GPIO_Context,
                                                                &pProfile->Peripherals.X2_GPIO,
                                                                pProfile->UserSuppliedMemoryMap.Peripheral[PERIPHERAL_IDENTIFIER_X2_GPIO].BaseAddress,
                                                                pProfile->UserSuppliedMemoryMap.Peripheral[PERIPHERAL_IDENTIFIER_X2_GPIO].IsAvailable,
                                                                &RequiredEnvironmentContext);

    PERIPHERAL_AXI_GPIO_HBM_Initialize(                         &AXI_GPIO_HBM_Context,
                                                                pProfile->UserSuppliedMemoryMap.Peripheral[PERIPHERAL_IDENTIFIER_AXI_GPIO_HBM].BaseAddress,
                                                                pProfile->UserSuppliedMemoryMap.Peripheral[PERIPHERAL_IDENTIFIER_AXI_GPIO_HBM].IsAvailable,
                                                                &RequiredEnvironmentContext);

    PERIPHERAL_AXI_GPIO_QSFP_Initialize(                        &AXI_GPIO_QSFP_Context,
                                                                pProfile->UserSuppliedMemoryMap.Peripheral[PERIPHERAL_IDENTIFIER_AXI_GPIO_GPIO_QSFP].BaseAddress,
                                                                pProfile->UserSuppliedMemoryMap.Peripheral[PERIPHERAL_IDENTIFIER_AXI_GPIO_GPIO_QSFP].IsAvailable,
                                                                &RequiredEnvironmentContext,
																pProfile->NumberOfQSFPCages);

    PERIPHERAL_AXI_GPIO_MB_INTERRUPTS_Initialize(               &AXI_GPIO_MB_Interrupts_Context,
                                                                pProfile->UserSuppliedMemoryMap.Peripheral[PERIPHERAL_IDENTIFIER_AXI_GPIO_MB_INTERRUPTS].BaseAddress,
                                                                pProfile->UserSuppliedMemoryMap.Peripheral[PERIPHERAL_IDENTIFIER_AXI_GPIO_MB_INTERRUPTS].IsAvailable,
                                                                &RequiredEnvironmentContext);

    PERIPHERAL_AXI_INTC_Initialize(                             &AXI_INTC_Context,
                                                                &pProfile->Peripherals.Axi_IntC,
                                                                pProfile->UserSuppliedMemoryMap.Peripheral[PERIPHERAL_IDENTIFIER_AXI_INTC].BaseAddress,
                                                                pProfile->UserSuppliedMemoryMap.Peripheral[PERIPHERAL_IDENTIFIER_AXI_INTC].IsAvailable,
                                                                &RequiredEnvironmentContext);

    PERIPHERAL_AXI_TIMEBASE_WDT_Initialize(                     &AXI_Timebase_WDT_Context,
                                                                pProfile->UserSuppliedMemoryMap.Peripheral[PERIPHERAL_IDENTIFIER_AXI_TIMEBASE_WDT].BaseAddress,
                                                                pProfile->UserSuppliedMemoryMap.Peripheral[PERIPHERAL_IDENTIFIER_AXI_TIMEBASE_WDT].IsAvailable,
                                                                &RequiredEnvironmentContext,
                                                                &RegMap_RAM_Controller_Context);


 
    PERIPHERAL_REGMAP_RAM_CONTROLLER_Initialize(                &RegMap_RAM_Controller_Context,
                                                                &pProfile->Peripherals.RAM_Controller,
                                                                &WatchpointContext,
                                                                pProfile->UserSuppliedMemoryMap.Peripheral[PERIPHERAL_IDENTIFIER_REGMAP_RAM_CONTROLLER].BaseAddress,
                                                                pProfile->UserSuppliedMemoryMap.Peripheral[PERIPHERAL_IDENTIFIER_REGMAP_RAM_CONTROLLER].IsAvailable,
                                                                &RequiredEnvironmentContext,
                                                                &FirmwareVersionContext,
                                                                &CoreVersionContext,
                                                                pProfile->Name.ShortValue);

    PERIPHERAL_AXI_UART_LITE_USB_Initialize(                    &UART_LITE_USB_Context,
                                                                &pProfile->Peripherals.UART[UART_CATEGORY_DEBUG],
                                                                &WatchpointContext,
                                                                pProfile->UserSuppliedMemoryMap.Peripheral[PERIPHERAL_IDENTIFIER_UART_LITE_USB].BaseAddress,
                                                                pProfile->UserSuppliedMemoryMap.Peripheral[PERIPHERAL_IDENTIFIER_UART_LITE_USB].IsAvailable,
                                                                &RequiredEnvironmentContext);

    PERIPHERAL_AXI_UART_LITE_SATELLITE_Initialize(              &AXI_UART_LITE_Satellite_Context,
                                                                &pProfile->Peripherals.UART[UART_CATEGORY_SC_COMMUNICATIONS],
                                                                &WatchpointContext,
                                                                pProfile->UserSuppliedMemoryMap.Peripheral[PERIPHERAL_IDENTIFIER_AXI_UART_LITE_SATELLITE].BaseAddress,
                                                                pProfile->UserSuppliedMemoryMap.Peripheral[PERIPHERAL_IDENTIFIER_AXI_UART_LITE_SATELLITE].IsAvailable,
                                                                &RequiredEnvironmentContext);

    PERIPHERAL_AXI_GPIO_WDT_Initialize(                         &AXI_GPIO_WDT_Context,
                                                                pProfile->UserSuppliedMemoryMap.Peripheral[PERIPHERAL_IDENTIFIER_AXI_GPIO_GPIO_WDT].BaseAddress,
                                                                pProfile->UserSuppliedMemoryMap.Peripheral[PERIPHERAL_IDENTIFIER_AXI_GPIO_GPIO_WDT].IsAvailable,
                                                                &RequiredEnvironmentContext);

    PERIPHERAL_AXI_GPIO_MUTEX_CMC_Initialize(                   &AXI_GPIO_Mutex_CMC_Context,
                                                                pProfile->UserSuppliedMemoryMap.Peripheral[PERIPHERAL_IDENTIFIER_AXI_GPIO_MUTEX_CMC].BaseAddress,
                                                                pProfile->UserSuppliedMemoryMap.Peripheral[PERIPHERAL_IDENTIFIER_AXI_GPIO_MUTEX_CMC].IsAvailable,
                                                                &RequiredEnvironmentContext,
                                                                &WatchpointContext);

    PERIPHERAL_FREERUNNING_CLOCK_Initialize(                    &FreeRunningClock_Context,
                                                                &pProfile->Peripherals.CLOCK,
                                                                pProfile->UserSuppliedMemoryMap.Peripheral[PERIPHERAL_IDENTIFIER_FREE_RUNNING_CLOCK].BaseAddress,
                                                                pProfile->UserSuppliedMemoryMap.Peripheral[PERIPHERAL_IDENTIFIER_FREE_RUNNING_CLOCK].IsAvailable,
                                                                &RequiredEnvironmentContext);

    PERIPHERAL_ClockThrottling_Initialize(                      &ClockThrottlingPeripheral_Context,
                                                                pProfile->UserSuppliedMemoryMap.Peripheral[PERIPHERAL_IDENTIFIER_AXI_CLOCK_THROTTLING].BaseAddress,
                                                                pProfile->UserSuppliedMemoryMap.Peripheral[PERIPHERAL_IDENTIFIER_AXI_CLOCK_THROTTLING].IsAvailable,
                                                                &RequiredEnvironmentContext);

    PERIPHERAL_HW_BUILD_INFO_Initialize(                        &HWBuildInfoPeripheral_Context,
                                                                pProfile->UserSuppliedMemoryMap.Peripheral[PERIPHERAL_IDENTIFIER_HW_BUILD_INFO].BaseAddress,
                                                                pProfile->UserSuppliedMemoryMap.Peripheral[PERIPHERAL_IDENTIFIER_HW_BUILD_INFO].IsAvailable,
                                                                &RequiredEnvironmentContext,
                                                                &RegMap_RAM_Controller_Context);

    PERIPHERAL_Sysmon_Initialize(                               &SysMonPeripheral_Context,
                                                                pProfile->UserSuppliedMemoryMap.Peripheral[PERIPHERAL_IDENTIFIER_SYSMON].BaseAddress,
                                                                pProfile->UserSuppliedMemoryMap.Peripheral[PERIPHERAL_IDENTIFIER_SYSMON].IsAvailable,
                                                                &RequiredEnvironmentContext,
                                                                &RegMap_RAM_Controller_Context);

    PERIPHERAL_DNA_Initialize(                                  &DNAPeripheral_Context,
                                                                pProfile->UserSuppliedMemoryMap.Peripheral[PERIPHERAL_IDENTIFIER_DNA].BaseAddress,
                                                                pProfile->UserSuppliedMemoryMap.Peripheral[PERIPHERAL_IDENTIFIER_DNA].IsAvailable,
                                                                &RequiredEnvironmentContext,
                                                                &RegMap_RAM_Controller_Context);
}













