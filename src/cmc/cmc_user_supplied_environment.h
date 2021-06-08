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
 *  $Revision: #24 $
 *
 */




#ifndef _CMC_USER_SUPPLIED_ENVIRONMENT_H_
#define _CMC_USER_SUPPLIED_ENVIRONMENT_H_


#ifdef __cplusplus
extern "C"
{
#endif

#include "cmc_common.h"
#include "cmc_peripherals_address.h"




typedef     uint16_t    (*USER_SUPPLIED_ENVIRONMENT_CALCULATE_CRC16_CCITT_TYPE)(void *pUserContext, uint16_t SeedValue, char *pBuffer, uint32_t BufferLength);
typedef     void        (*USER_SUPPLIED_ENVIRONMENT_PREAMBLE_TYPE)(void *pUserContext);
typedef     void        (*USER_SUPPLIED_ENVIRONMENT_POSTAMBLE_TYPE)(void *pUserContext); 
typedef     uint32_t    (*USER_SUPPLIED_ENVIRONMENT_PERIPHERAL_READ_TYPE)(void *pUserContext, CMC_PERIPHERAL_BASE_ADDRESS_TYPE PeripheralBaseAddress, uint32_t RegisterOffset);
typedef     void        (*USER_SUPPLIED_ENVIRONMENT_PERIPHERAL_WRITE_TYPE)(void *pUserContext, CMC_PERIPHERAL_BASE_ADDRESS_TYPE PeripheralBaseAddress, uint32_t RegisterOffset, uint32_t Value);
typedef     void        (*USER_SUPPLIED_ENVIRONMENT_PERIPHERAL_WRITE_WITH_MASK_TYPE)(void *pUserContext, CMC_PERIPHERAL_BASE_ADDRESS_TYPE PeripheralBaseAddress, uint32_t RegisterOffset, uint32_t Value, uint32_t Mask);

typedef     uint32_t    (*USER_SUPPLIED_ENVIRONMENT_ECC_READ_TYPE)(void* pUserContext, PCIE_ECC_ERROR_REGISTER_TYPE Register);
typedef     uint32_t    (*USER_SUPPLIED_ENVIRONMENT_PCIE_READ_TYPE)(void* pUserContext, PCIE_ECC_ERROR_REGISTER_TYPE Register);
typedef     uint8_t     (*USER_SUPPLIED_ENVIRONMENT_GPIO_READ_TYPE)(void* pUserContext);
typedef     uint32_t    (*USER_SUPPLIED_ENVIRONMENT_STACK_CHECK_TYPE)(void* pUserContext);

typedef     uint16_t    (*USER_SUPPLIED_ENVIRONMENT_SHELLVERSION_TYPE)(void* pUserContext);
typedef     void        (*USER_SUPPLIED_ENVIRONMENT_UUID_TYPE)(void* pUserContext, uint8_t* UUID);
typedef     uint8_t     (*USER_SUPPLIED_ENVIRONMENT_KEEPALIVE_TYPE)(void* pUserContext);


typedef     uint64_t    (*USER_SUPPLIED_ENVIRONMENT_SENSORFILTER_TYPE)(void* pUserContext);
typedef     void        (*USER_SUPPLIED_ENVIRONMENT_QSFP_GPIO_READ_TYPE)(void* pUserContext, uint8_t QSFP, uint8_t* QSFP_Read_Value);
typedef     void        (*USER_SUPPLIED_ENVIRONMENT_QSFP_GPIO_WRITE_TYPE)(void* pUserContext, uint8_t QSFP, uint8_t QSFP_Write_Value);



typedef struct CMC_USER_SUPPLIED_ENVIRONMENT_TYPE
{
    bool StartScheduling;

    void *                                                      pUserContext;
    USER_SUPPLIED_ENVIRONMENT_PREAMBLE_TYPE                     pFN_PreAmble;
    USER_SUPPLIED_ENVIRONMENT_POSTAMBLE_TYPE                    pFN_PostAmble;
    USER_SUPPLIED_ENVIRONMENT_PERIPHERAL_READ_TYPE              pFN_PeripheralRead;
    USER_SUPPLIED_ENVIRONMENT_PERIPHERAL_WRITE_TYPE             pFN_PeripheralWrite;
    USER_SUPPLIED_ENVIRONMENT_PERIPHERAL_WRITE_WITH_MASK_TYPE   pFN_PeripheralWriteWithMask;

    USER_SUPPLIED_ENVIRONMENT_CALCULATE_CRC16_CCITT_TYPE        pFN_Calculate_CRC16_CCITT;
    bool                                                        bSensorsGatedByPowerGood;
    bool                                                        bPCIeECCReportingSupported;

    USER_SUPPLIED_ENVIRONMENT_ECC_READ_TYPE                     pFN_ECC_Read;
    USER_SUPPLIED_ENVIRONMENT_PCIE_READ_TYPE                    pFN_PCIe_Read;

    bool                                                        bU30_Zync1_Device;
    
    bool                                                        bKeepAliveSupported;
    USER_SUPPLIED_ENVIRONMENT_SHELLVERSION_TYPE                 pFN_ShellVersion_Read;
    USER_SUPPLIED_ENVIRONMENT_UUID_TYPE                         pFN_UUID_Read;
    USER_SUPPLIED_ENVIRONMENT_KEEPALIVE_TYPE                    pFN_KeepAlive_Read;
    bool                                                        bCardSupportsSCUpgrade;
    uint32_t                                                    SensorSupervisorTimeout;
    uint64_t                                                    CMCValidSensors_0_to_63;
    uint64_t                                                    CMCValidSensors_64_to_127;
    bool                                                        bCardSupportsScalingFactor;
    bool                                                        bCardSupportsHBM;
	bool                                                        bCardSupportsSC;
    USER_SUPPLIED_ENVIRONMENT_GPIO_READ_TYPE                    pFN_GPIO_Read;
    USER_SUPPLIED_ENVIRONMENT_STACK_CHECK_TYPE                  pFN_Stack_Check;
    bool                                                        bCardSupportsSUCUpgrade;
    USER_SUPPLIED_ENVIRONMENT_QSFP_GPIO_READ_TYPE               pFN_QSFP_GPIO_Read;
    USER_SUPPLIED_ENVIRONMENT_QSFP_GPIO_WRITE_TYPE              pFN_QSFP_GPIO_Write;
} CMC_USER_SUPPLIED_ENVIRONMENT_TYPE;


void CMC_USER_AddEnvironmentBinding_UserContext(CMC_USER_SUPPLIED_ENVIRONMENT_TYPE * pUserSuppliedEnvironment, void * pUserContext);
void CMC_USER_AddEnvironmentBinding_StartScheduling(CMC_USER_SUPPLIED_ENVIRONMENT_TYPE * pUserSuppliedEnvironment, bool StartScheduling);
void CMC_USER_AddEnvironmentBinding_PreAmble(CMC_USER_SUPPLIED_ENVIRONMENT_TYPE * pUserSuppliedEnvironment, USER_SUPPLIED_ENVIRONMENT_PREAMBLE_TYPE pFN_PreAmble);
void CMC_USER_AddEnvironmentBinding_PostAmble(CMC_USER_SUPPLIED_ENVIRONMENT_TYPE * pUserSuppliedEnvironment, USER_SUPPLIED_ENVIRONMENT_POSTAMBLE_TYPE pFN_PostAmble);
void CMC_USER_AddEnvironmentBinding_PeripheralRead(CMC_USER_SUPPLIED_ENVIRONMENT_TYPE * pUserSuppliedEnvironment, USER_SUPPLIED_ENVIRONMENT_PERIPHERAL_READ_TYPE pFN_PeripheralRead);
void CMC_USER_AddEnvironmentBinding_PeripheralWrite(CMC_USER_SUPPLIED_ENVIRONMENT_TYPE * pUserSuppliedEnvironment, USER_SUPPLIED_ENVIRONMENT_PERIPHERAL_WRITE_TYPE pFN_PeripheralWrite);
void CMC_USER_AddEnvironmentBinding_PeripheralWriteWithMask(CMC_USER_SUPPLIED_ENVIRONMENT_TYPE * pUserSuppliedEnvironment, USER_SUPPLIED_ENVIRONMENT_PERIPHERAL_WRITE_WITH_MASK_TYPE pFN_PeripheralWriteWithMask);
void CMC_USER_AddEnvironmentBinding_Calculate_CRC16_CCITT(CMC_USER_SUPPLIED_ENVIRONMENT_TYPE * pUserSuppliedEnvironment, USER_SUPPLIED_ENVIRONMENT_CALCULATE_CRC16_CCITT_TYPE pFN_Calculate_CRC16_CCITT);
void CMC_USER_AddEnvironmentBinding_SensorsGatedByPowerGood(CMC_USER_SUPPLIED_ENVIRONMENT_TYPE* pUserSuppliedEnvironment, bool bSensorsGatedByPowerGood);
void CMC_USER_AddEnvironmentBinding_PCIeECCReportingSupported(CMC_USER_SUPPLIED_ENVIRONMENT_TYPE* pUserSuppliedEnvironment, bool bPCIeECCReportingSupported);
void CMC_USER_AddEnvironmentBinding_ECCRead(CMC_USER_SUPPLIED_ENVIRONMENT_TYPE* pUserSuppliedEnvironment, USER_SUPPLIED_ENVIRONMENT_ECC_READ_TYPE pFN_ECC_Read);
void CMC_USER_AddEnvironmentBinding_PCIeRead(CMC_USER_SUPPLIED_ENVIRONMENT_TYPE* pUserSuppliedEnvironment, USER_SUPPLIED_ENVIRONMENT_PCIE_READ_TYPE pFN_PCIe_Read);
void CMC_USER_AddEnvironmentBinding_U30_Zync1_Device(CMC_USER_SUPPLIED_ENVIRONMENT_TYPE* pUserSuppliedEnvironment, bool bU30_Zync1_Device);
void CMC_USER_AddEnvironmentBinding_KeepAliveSupported(CMC_USER_SUPPLIED_ENVIRONMENT_TYPE* pUserSuppliedEnvironment, bool bKeepAliveSupported);
void CMC_USER_AddEnvironmentBinding_ShellVersionRead(CMC_USER_SUPPLIED_ENVIRONMENT_TYPE* pUserSuppliedEnvironment, USER_SUPPLIED_ENVIRONMENT_SHELLVERSION_TYPE pFN_ShellVersion_Read);
void CMC_USER_AddEnvironmentBinding_UUIDRead(CMC_USER_SUPPLIED_ENVIRONMENT_TYPE* pUserSuppliedEnvironment, USER_SUPPLIED_ENVIRONMENT_UUID_TYPE pFN_UUID_Read);
void CMC_USER_AddEnvironmentBinding_KeepAliveRead(CMC_USER_SUPPLIED_ENVIRONMENT_TYPE* pUserSuppliedEnvironment, USER_SUPPLIED_ENVIRONMENT_KEEPALIVE_TYPE pFN_KeepAlive_Read);
void CMC_USER_AddEnvironmentBinding_CardSupportsSCUpgrade(CMC_USER_SUPPLIED_ENVIRONMENT_TYPE* pUserSuppliedEnvironment, bool bCardSupportsSCUpgrade);
void CMC_USER_AddEnvironmentBinding_SensorSupervisorTimeout(CMC_USER_SUPPLIED_ENVIRONMENT_TYPE* pUserSuppliedEnvironment, uint32_t SensorSupervisorTimeout);
void CMC_USER_AddEnvironmentBinding_Valid_Sensors(CMC_USER_SUPPLIED_ENVIRONMENT_TYPE* pUserSuppliedEnvironment, uint64_t CMCValidSensors_0_to_63, uint64_t CMCValidSensors_64_to_127);
void CMC_USER_AddEnvironmentBinding_CardSupportsScalingFactor(CMC_USER_SUPPLIED_ENVIRONMENT_TYPE* pUserSuppliedEnvironment, bool bCardSupportsScalingFactor);
void CMC_USER_AddEnvironmentBinding_CardSupportsHBM(CMC_USER_SUPPLIED_ENVIRONMENT_TYPE* pUserSuppliedEnvironment, bool bCardSupportsHBM);
void CMC_USER_AddEnvironmentBinding_CardSupportsSC(CMC_USER_SUPPLIED_ENVIRONMENT_TYPE* pUserSuppliedEnvironment, bool bCardSupportsSC);
void CMC_USER_AddEnvironmentBinding_GPIORead(CMC_USER_SUPPLIED_ENVIRONMENT_TYPE* pUserSuppliedEnvironment, USER_SUPPLIED_ENVIRONMENT_GPIO_READ_TYPE pFN_GPIO_Read);
void CMC_USER_AddEnvironmentBinding_StackCheck(CMC_USER_SUPPLIED_ENVIRONMENT_TYPE* pUserSuppliedEnvironment, USER_SUPPLIED_ENVIRONMENT_STACK_CHECK_TYPE pFN_Stack_Check);
void CMC_USER_AddEnvironmentBinding_CardSupportsSUCUpgrade(CMC_USER_SUPPLIED_ENVIRONMENT_TYPE* pUserSuppliedEnvironment, bool bCardSupportsSUCUpgrade);
void CMC_USER_AddEnvironmentBinding_QSFP_GPIORead(CMC_USER_SUPPLIED_ENVIRONMENT_TYPE* pUserSuppliedEnvironment, USER_SUPPLIED_ENVIRONMENT_QSFP_GPIO_READ_TYPE pFN_QSFP_GPIO_Read);
void CMC_USER_AddEnvironmentBinding_QSFP_GPIOWrite(CMC_USER_SUPPLIED_ENVIRONMENT_TYPE* pUserSuppliedEnvironment, USER_SUPPLIED_ENVIRONMENT_QSFP_GPIO_WRITE_TYPE pFN_QSFP_GPIO_Write);


#ifdef __cplusplus
}
#endif


#endif /* _CMC_USER_SUPPLIED_ENVIRONMENT_H_ */















