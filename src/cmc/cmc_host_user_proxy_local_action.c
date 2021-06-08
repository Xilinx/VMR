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
 *  $Revision: #22 $
 *
 */





#include "cmc_host_user_proxy_thread.h"
#include "cmc_data_store.h"
#include "cmc_local_sensor_supervisor_thread.h"
extern LOCAL_SENSOR_SUPERVISOR_THREAD_CONTEXT_TYPE LocalSensorSupervisorThreadContext;

void HostUserProxyThread_HandleLocalAction_HP_LOCAL_OPERATION_REBOOT_FIRMWARE(HOST_USER_PROXY_THREAD_CONTEXT_TYPE *pContext)
{
    HardwarePlatformServices_DisableInterrupts(pContext->pHardwarePlatformContext);
    HardwarePlatformServices_JumpToResetVector(pContext->pHardwarePlatformContext);
}


void HostUserProxyThread_HandleLocalAction_HP_LOCAL_OPERATION_STOP_FIRMWARE(HOST_USER_PROXY_THREAD_CONTEXT_TYPE *pContext)
{
    pContext->FSM_Running=false;
}


void HostUserProxyThread_HandleLocalAction_HP_LOCAL_OPERATION_CLEAR_ERROR_REGISTER(HOST_USER_PROXY_THREAD_CONTEXT_TYPE *pContext)
{
    PERIPHERAL_REGMAP_RAM_CONTROLLER_Write(pContext->pPeripheralRegMapRamControllerContext, HOST_REGISTER_ERROR_REG, 0);
    PERIPHERAL_REGMAP_RAM_CONTROLLER_WriteWithMask(pContext->pPeripheralRegMapRamControllerContext, HOST_REGISTER_CONTROL, 0, HOST_REGISTER_PATTERN_CLEAR_ERRORS);
}


void HostUserProxyThread_HandleLocalAction_HP_LOCAL_OPERATION_CLEAR_SENSOR_REGISTER(HOST_USER_PROXY_THREAD_CONTEXT_TYPE *pContext)
{
    Broker_SensorSupervisor_Reset_Sensors(pContext->pBrokerContext);
    PERIPHERAL_REGMAP_RAM_CONTROLLER_WriteWithMask(pContext->pPeripheralRegMapRamControllerContext, HOST_REGISTER_CONTROL, 0, HOST_REGISTER_PATTERN_RESET_SENSORS);
}

void HostUserProxyThread_HandleLocalAction_HP_LOCAL_OPERATION_ENABLE_DEBUG_CAPTURE(HOST_USER_PROXY_THREAD_CONTEXT_TYPE* pContext)
{
    Broker_Enable_Debug_Capture(pContext->pBrokerContext);
    PERIPHERAL_REGMAP_RAM_CONTROLLER_WriteWithMask(pContext->pPeripheralRegMapRamControllerContext, HOST_REGISTER_CONTROL, 0, HOST_REGISTER_PATTERN_ENABLE_DEBUG_CAPTURE);
}

void HostUserProxyThread_HandleLocalAction_HP_LOCAL_OPERATION_DEBUG_CAPTURE_MSG_ID(HOST_USER_PROXY_THREAD_CONTEXT_TYPE* pContext)
{
    //uint32_t RemoteMessageRegisterValue = PERIPHERAL_REGMAP_RAM_CONTROLLER_Read(pContext->pPeripheralRegMapRamControllerContext, HOST_REGISTER_REMOTE_COMMAND_REGISTER);
    Broker_Store_Debug_Capture_MsgID(pContext->pBrokerContext, pContext->DebugCaptureMsgID);
    PERIPHERAL_REGMAP_RAM_CONTROLLER_WriteWithMask(pContext->pPeripheralRegMapRamControllerContext, HOST_REGISTER_CONTROL, 0, (uint32_t)HOST_REGISTER_PATTERN_DEBUG_CAPTURE_MSG_ID);
}

void HostUserProxyThread_HandleLocalAction_HP_LOCAL_OPERATION_ENABLE_REMOTE_DEBUG_UART(HOST_USER_PROXY_THREAD_CONTEXT_TYPE *pContext)
{
    pContext->pBrokerContext->pLocalSensorSupervisorThreadContext->EnableUartRequest = true;
    PERIPHERAL_REGMAP_RAM_CONTROLLER_WriteWithMask(pContext->pPeripheralRegMapRamControllerContext, HOST_REGISTER_CONTROL, 0, (uint32_t)HOST_REGISTER_PATTERN_ENABLE_REMOTE_DEBUG_UART);
}


void HostUserProxyThread_HandleLocalAction_HP_LOCAL_OPERATION_FIRST_CSDR_REQUEST(HOST_USER_PROXY_THREAD_CONTEXT_TYPE* pContext)
{
    pContext->pBrokerContext->pLocalSensorSupervisorThreadContext->bCollectCSDR = true;
    pContext->pBrokerContext->pLocalSensorSupervisorThreadContext->CSDRInitialRecordNumber = 0;
}

void HostUserProxyThread_HandleLocalAction_HP_LOCAL_OPERATION_ADDITIONAL_CSDR_REQUEST(HOST_USER_PROXY_THREAD_CONTEXT_TYPE* pContext)
{
    pContext->pBrokerContext->pLocalSensorSupervisorThreadContext->bCollectCSDR = true;
}


void HostUserProxyThread_HandleLocalAction_HP_LOCAL_OPERATION_CSDR_COMPLETE(HOST_USER_PROXY_THREAD_CONTEXT_TYPE* pContext)
{
    pContext->pBrokerContext->pLocalSensorSupervisorThreadContext->bCollectCSDR = false;
    pContext->bCSDRInProgress = false;
    HostUserProxyThread_UpdateResultErrorCodeRegister(pContext, RESULT_ERROR_CODE_CMC_HOST_MSG_NO_ERROR);
    PERIPHERAL_REGMAP_RAM_CONTROLLER_WriteWithMask(pContext->pPeripheralRegMapRamControllerContext, HOST_REGISTER_CONTROL, 0, (uint32_t)HOST_REGISTER_PATTERN_REMOTE_COMMAND);
}

void HostUserProxyThread_HandleLocalAction_HP_LOCAL_OPERATION_CSDR_FAILED(HOST_USER_PROXY_THREAD_CONTEXT_TYPE* pContext)
{
    pContext->pBrokerContext->pLocalSensorSupervisorThreadContext->bCollectCSDR = false;
    pContext->bCSDRInProgress = false;
    // Write to failure reg
    HostUserProxyThread_UpdateResultErrorCodeRegister(pContext, RESULT_ERROR_CODE_CMC_HOST_MSG_CSDR_FAIL);
    PERIPHERAL_REGMAP_RAM_CONTROLLER_WriteWithMask(pContext->pPeripheralRegMapRamControllerContext, HOST_REGISTER_CONTROL, 0, (uint32_t)HOST_REGISTER_PATTERN_REMOTE_COMMAND);
}

static void HostUserProxyThread_QSFP_Common(HOST_USER_PROXY_THREAD_CONTEXT_TYPE* pContext)
{
    pContext->pBrokerContext->pLocalSensorSupervisorThreadContext->QSFPResponseWaitCount = 0;
    pContext->pBrokerContext->pLocalSensorSupervisorThreadContext->bQSFPRequestSent = false;
}

void HostUserProxyThread_HandleLocalAction_HP_LOCAL_OPERATION_READ_QSFP_DIAGNOSTICS(HOST_USER_PROXY_THREAD_CONTEXT_TYPE* pContext)
{
    HostUserProxyThread_QSFP_Common(pContext);
    pContext->pBrokerContext->pLocalSensorSupervisorThreadContext->bReadQSFPDiagnostics = true; 
}

void HostUserProxyThread_HandleLocalAction_HP_LOCAL_OPERATION_BOARDINFO_REQUEST(HOST_USER_PROXY_THREAD_CONTEXT_TYPE* pContext)
{
    pContext->pBrokerContext->pLocalSensorSupervisorThreadContext->DataStoreContext.BoardInfo.IsValid = false;
}


void HostUserProxyThread_HandleLocalAction_HP_LOCAL_OPERATION_BOARDINFO_COMPLETE(HOST_USER_PROXY_THREAD_CONTEXT_TYPE* pContext)
{
    uint16_t    i;
    uint32_t    BoardInfo[BOARD_INFO_MAX_SIZE];
    uint32_t*    pBoardInfo = BoardInfo;
    uint32_t    AddressOffset;
    uint16_t    Length;
    uint32_t    RemoteMessageRegisterValue;
    
    for(i=0; i < BOARD_INFO_MAX_SIZE; i++)
    {
        BoardInfo[i] = 0;
    }
          
    cmcCopyBoardInfo(&(LocalSensorSupervisorThreadContext.DataStoreContext), (uint8_t*)BoardInfo);

    if (LocalSensorSupervisorThreadContext.DataStoreContext.BoardInfo.IsValid && LocalSensorSupervisorThreadContext.DataStoreContext.BoardInfo.Length)
    {
        RemoteMessageRegisterValue = PERIPHERAL_REGMAP_RAM_CONTROLLER_Read(pContext->pPeripheralRegMapRamControllerContext, HOST_REGISTER_REMOTE_COMMAND_REGISTER);
        RemoteMessageRegisterValue = RemoteMessageRegisterValue | LocalSensorSupervisorThreadContext.DataStoreContext.BoardInfo.Length;
        PERIPHERAL_REGMAP_RAM_CONTROLLER_Write(pContext->pPeripheralRegMapRamControllerContext, HOST_REGISTER_REMOTE_COMMAND_REGISTER, RemoteMessageRegisterValue);
              
        Length = LocalSensorSupervisorThreadContext.DataStoreContext.BoardInfo.Length / 4;

        if (LocalSensorSupervisorThreadContext.DataStoreContext.BoardInfo.Length % 4)
        {
            Length++;
        }

        AddressOffset = HOST_REGISTER_REMOTE_COMMAND_REGISTER;
        for (i = 0; i < Length; i++)
        {
            AddressOffset = AddressOffset + 4;
            PERIPHERAL_REGMAP_RAM_CONTROLLER_Write(pContext->pPeripheralRegMapRamControllerContext, AddressOffset, *pBoardInfo);
            pBoardInfo++;
        }
        HostUserProxyThread_UpdateResultErrorCodeRegister(pContext, RESULT_ERROR_CODE_CMC_HOST_MSG_NO_ERROR);
        HostUserProxyThread_ClearCurrentControlRegisterRequest(pContext);
    }
    else
    {
        HostUserProxyThread_UpdateResultErrorCodeRegister(pContext, RESULT_ERROR_CODE_CMC_HOST_MSG_BRD_INFO_MISSING);
        HostUserProxyThread_ClearCurrentControlRegisterRequest(pContext);
    } 

    pContext->bBoardInfoInProgress = false;
}

void HostUserProxyThread_HandleLocalAction_HP_LOCAL_OPERATION_READ_QSFP_VALIDATE_LOW_SPEED_IO(HOST_USER_PROXY_THREAD_CONTEXT_TYPE* pContext)
{

    HostUserProxyThread_QSFP_Common(pContext);
    pContext->pBrokerContext->pLocalSensorSupervisorThreadContext->bReadQSFPValidateLowSpeedIO = true;
}

void HostUserProxyThread_HandleLocalAction_HP_LOCAL_OPERATION_WRITE_QSFP_VALIDATE_LOW_SPEED_IO(HOST_USER_PROXY_THREAD_CONTEXT_TYPE* pContext)
{
    HostUserProxyThread_QSFP_Common(pContext);
    pContext->pBrokerContext->pLocalSensorSupervisorThreadContext->bWriteQSFPValidateLowSpeedIO = true;
}

void HostUserProxyThread_HandleLocalAction_HP_LOCAL_OPERATION_QSFP_READ_SINGLE_BYTE_REQUEST(HOST_USER_PROXY_THREAD_CONTEXT_TYPE* pContext)
{
    HostUserProxyThread_QSFP_Common(pContext);
    pContext->pBrokerContext->pLocalSensorSupervisorThreadContext->bQSFPReadSingleByte = true;
}

void HostUserProxyThread_HandleLocalAction_HP_LOCAL_OPERATION_QSFP_READ_SINGLE_BYTE_COMPLETE(HOST_USER_PROXY_THREAD_CONTEXT_TYPE* pContext)
{
    pContext->pBrokerContext->pLocalSensorSupervisorThreadContext->bQSFPReadSingleByte = false;
    pContext->bQSFP_ReadSingleByteInProgress = false;
    HostUserProxyThread_UpdateResultErrorCodeRegister(pContext, RESULT_ERROR_CODE_CMC_HOST_MSG_NO_ERROR);
    PERIPHERAL_REGMAP_RAM_CONTROLLER_WriteWithMask(pContext->pPeripheralRegMapRamControllerContext, HOST_REGISTER_CONTROL, 0, (uint32_t)HOST_REGISTER_PATTERN_REMOTE_COMMAND);
}

void HostUserProxyThread_HandleLocalAction_HP_LOCAL_OPERATION_QSFP_READ_SINGLE_BYTE_FAILED(HOST_USER_PROXY_THREAD_CONTEXT_TYPE* pContext)
{
    pContext->pBrokerContext->pLocalSensorSupervisorThreadContext->bQSFPReadSingleByte = false;
    pContext->bQSFP_ReadSingleByteInProgress = false;
    HostUserProxyThread_UpdateResultErrorCodeRegister(pContext, RESULT_ERROR_CODE_CMC_HOST_MSG_QSFP_FAIL);
    PERIPHERAL_REGMAP_RAM_CONTROLLER_WriteWithMask(pContext->pPeripheralRegMapRamControllerContext, HOST_REGISTER_CONTROL, 0, (uint32_t)HOST_REGISTER_PATTERN_REMOTE_COMMAND);
}

void HostUserProxyThread_HandleLocalAction_HP_LOCAL_OPERATION_QSFP_WRITE_SINGLE_BYTE_REQUEST(HOST_USER_PROXY_THREAD_CONTEXT_TYPE* pContext)
{
    HostUserProxyThread_QSFP_Common(pContext);
    pContext->pBrokerContext->pLocalSensorSupervisorThreadContext->bQSFPWriteSingleByte = true;
}
void HostUserProxyThread_HandleLocalAction_HP_LOCAL_OPERATION_QSFP_WRITE_SINGLE_BYTE_COMPLETE(HOST_USER_PROXY_THREAD_CONTEXT_TYPE* pContext)
{
    pContext->pBrokerContext->pLocalSensorSupervisorThreadContext->bQSFPWriteSingleByte = false;
    pContext->bQSFP_WriteSingleByteInProgress = false;
    HostUserProxyThread_UpdateResultErrorCodeRegister(pContext, RESULT_ERROR_CODE_CMC_HOST_MSG_NO_ERROR);
    PERIPHERAL_REGMAP_RAM_CONTROLLER_WriteWithMask(pContext->pPeripheralRegMapRamControllerContext, HOST_REGISTER_CONTROL, 0, (uint32_t)HOST_REGISTER_PATTERN_REMOTE_COMMAND);
}

void HostUserProxyThread_HandleLocalAction_HP_LOCAL_OPERATION_QSFP_WRITE_SINGLE_BYTE_FAILED(HOST_USER_PROXY_THREAD_CONTEXT_TYPE* pContext)
{
    pContext->pBrokerContext->pLocalSensorSupervisorThreadContext->bQSFPWriteSingleByte = false;
    pContext->bQSFP_WriteSingleByteInProgress = false;
    HostUserProxyThread_UpdateResultErrorCodeRegister(pContext, RESULT_ERROR_CODE_CMC_HOST_MSG_QSFP_FAIL);
    PERIPHERAL_REGMAP_RAM_CONTROLLER_WriteWithMask(pContext->pPeripheralRegMapRamControllerContext, HOST_REGISTER_CONTROL, 0, (uint32_t)HOST_REGISTER_PATTERN_REMOTE_COMMAND);
}

void HostUserProxyThread_HandleLocalAction_HP_LOCAL_OPERATION_READ_QSFP_DIAGNOSTICS_COMPLETE(HOST_USER_PROXY_THREAD_CONTEXT_TYPE* pContext)
{
    pContext->pBrokerContext->pLocalSensorSupervisorThreadContext->bReadQSFPDiagnostics = false;
    //HostUserProxyThread_QSFP_Common(pContext);
    pContext->bQSFP_ReadDiagnosticsInProgress = false;
    HostUserProxyThread_UpdateResultErrorCodeRegister(pContext, RESULT_ERROR_CODE_CMC_HOST_MSG_NO_ERROR);
    PERIPHERAL_REGMAP_RAM_CONTROLLER_WriteWithMask(pContext->pPeripheralRegMapRamControllerContext, HOST_REGISTER_CONTROL, 0, (uint32_t)HOST_REGISTER_PATTERN_REMOTE_COMMAND);
}

void HostUserProxyThread_HandleLocalAction_HP_LOCAL_OPERATION_READ_QSFP_VALIDATE_LOW_SPEED_IO_COMPLETE(HOST_USER_PROXY_THREAD_CONTEXT_TYPE* pContext)
{
    pContext->pBrokerContext->pLocalSensorSupervisorThreadContext->bReadQSFPValidateLowSpeedIO = false;
    //HostUserProxyThread_QSFP_Common(pContext);
    pContext->bQSFP_ReadLowSpeedIOInProgress = false;
    HostUserProxyThread_UpdateResultErrorCodeRegister(pContext, RESULT_ERROR_CODE_CMC_HOST_MSG_NO_ERROR);
    PERIPHERAL_REGMAP_RAM_CONTROLLER_WriteWithMask(pContext->pPeripheralRegMapRamControllerContext, HOST_REGISTER_CONTROL, 0, (uint32_t)HOST_REGISTER_PATTERN_REMOTE_COMMAND);
}

void HostUserProxyThread_HandleLocalAction_HP_LOCAL_OPERATION_WRITE_QSFP_VALIDATE_LOW_SPEED_IO_COMPLETE(HOST_USER_PROXY_THREAD_CONTEXT_TYPE* pContext)
{
    pContext->pBrokerContext->pLocalSensorSupervisorThreadContext->bWriteQSFPValidateLowSpeedIO = false;
    //HostUserProxyThread_QSFP_Common(pContext);
    pContext->bQSFP_WriteLowSpeedIOInProgress = false;
    HostUserProxyThread_UpdateResultErrorCodeRegister(pContext, RESULT_ERROR_CODE_CMC_HOST_MSG_NO_ERROR);
    PERIPHERAL_REGMAP_RAM_CONTROLLER_WriteWithMask(pContext->pPeripheralRegMapRamControllerContext, HOST_REGISTER_CONTROL, 0, (uint32_t)HOST_REGISTER_PATTERN_REMOTE_COMMAND);
}

void HostUserProxyThread_HandleLocalAction_HP_LOCAL_OPERATION_READ_QSFP_DIAGNOSTICS_FAILED(HOST_USER_PROXY_THREAD_CONTEXT_TYPE* pContext)
{
    pContext->pBrokerContext->pLocalSensorSupervisorThreadContext->bReadQSFPDiagnostics = false;
    //HostUserProxyThread_QSFP_Common(pContext);
    pContext->bQSFP_ReadDiagnosticsInProgress = false;
    HostUserProxyThread_UpdateResultErrorCodeRegister(pContext, RESULT_ERROR_CODE_CMC_HOST_MSG_QSFP_FAIL);
    PERIPHERAL_REGMAP_RAM_CONTROLLER_WriteWithMask(pContext->pPeripheralRegMapRamControllerContext, HOST_REGISTER_CONTROL, 0, (uint32_t)HOST_REGISTER_PATTERN_REMOTE_COMMAND);
}

void HostUserProxyThread_HandleLocalAction_HP_LOCAL_OPERATION_READ_QSFP_VALIDATE_LOW_SPEED_IO_FAILED(HOST_USER_PROXY_THREAD_CONTEXT_TYPE* pContext)
{
    pContext->pBrokerContext->pLocalSensorSupervisorThreadContext->bReadQSFPValidateLowSpeedIO = false;
    //HostUserProxyThread_QSFP_Common(pContext);
    pContext->bQSFP_ReadLowSpeedIOInProgress = false;
    HostUserProxyThread_UpdateResultErrorCodeRegister(pContext, RESULT_ERROR_CODE_CMC_HOST_MSG_QSFP_FAIL);
    PERIPHERAL_REGMAP_RAM_CONTROLLER_WriteWithMask(pContext->pPeripheralRegMapRamControllerContext, HOST_REGISTER_CONTROL, 0, (uint32_t)HOST_REGISTER_PATTERN_REMOTE_COMMAND);
}


void HostUserProxyThread_HandleLocalAction_HP_LOCAL_OPERATION_WRITE_QSFP_VALIDATE_LOW_SPEED_IO_FAILED(HOST_USER_PROXY_THREAD_CONTEXT_TYPE* pContext)
{
    pContext->pBrokerContext->pLocalSensorSupervisorThreadContext->bWriteQSFPValidateLowSpeedIO = false;
    //HostUserProxyThread_QSFP_Common(pContext);
    pContext->bQSFP_WriteLowSpeedIOInProgress = false;
    HostUserProxyThread_UpdateResultErrorCodeRegister(pContext, RESULT_ERROR_CODE_CMC_HOST_MSG_QSFP_FAIL);
    PERIPHERAL_REGMAP_RAM_CONTROLLER_WriteWithMask(pContext->pPeripheralRegMapRamControllerContext, HOST_REGISTER_CONTROL, 0, (uint32_t)HOST_REGISTER_PATTERN_REMOTE_COMMAND);
}

void HostUserProxyThread_HandleLocalAction_HP_LOCAL_OPERATION_HBM_SUPPORT_CMS(HOST_USER_PROXY_THREAD_CONTEXT_TYPE* pContext)
{
    uint32_t ControlRegisterValue = PERIPHERAL_REGMAP_RAM_CONTROLLER_Read(pContext->pPeripheralRegMapRamControllerContext, HOST_REGISTER_CONTROL);

    if (pContext->pBrokerContext->pLocalSensorSupervisorThreadContext->DataStoreContext.pFirmwareVersionContext->Version.Minor == 0x8) // Checking for CMS
    {
        if (ControlRegisterValue & HOST_REGISTER_PATTERN_HBM_SUPPORT_CMS)
        {
            pContext->pBrokerContext->pLocalSensorSupervisorThreadContext->HBMSupported = true;
        }
        else
        {
            pContext->pBrokerContext->pLocalSensorSupervisorThreadContext->HBMSupported = false;
        }
    }

}

void HostUserProxyThread_HandleLocalAction_HP_LOCAL_OPERATION_CLOCK_SCALING_CMS(HOST_USER_PROXY_THREAD_CONTEXT_TYPE* pContext)
{
    uint32_t ControlRegisterValue = PERIPHERAL_REGMAP_RAM_CONTROLLER_Read(pContext->pPeripheralRegMapRamControllerContext, HOST_REGISTER_CONTROL);

    if (pContext->pBrokerContext->pLocalSensorSupervisorThreadContext->DataStoreContext.pFirmwareVersionContext->Version.Minor == 0x8) // Checking for CMS
    {
        if (ControlRegisterValue & HOST_REGISTER_PATTERN_CLOCK_SCALING_CMS)
        {
            pContext->pBrokerContext->pLocalSensorSupervisorThreadContext->DataStoreContext.bCardSupportsScalingFactor = true;
        }
        else
        {
            pContext->pBrokerContext->pLocalSensorSupervisorThreadContext->DataStoreContext.bCardSupportsScalingFactor = false;
        }
    }

}

void HostUserProxyThread_HandleLocalAction_HP_LOCAL_OPERATION_LOW_SPEED_QSFP_FROM_GPIO_CMS(HOST_USER_PROXY_THREAD_CONTEXT_TYPE* pContext)
{
    uint32_t ControlRegisterValue = PERIPHERAL_REGMAP_RAM_CONTROLLER_Read(pContext->pPeripheralRegMapRamControllerContext, HOST_REGISTER_CONTROL);

    if (pContext->pBrokerContext->pLocalSensorSupervisorThreadContext->DataStoreContext.pFirmwareVersionContext->Version.Minor == 0x8) // Checking for CMS
    {
        if (ControlRegisterValue & HOST_REGISTER_PATTERN_LOW_SPEED_QSFP_FROM_GPIO_CMS)
        {
            pContext->pBrokerContext->pLocalSensorSupervisorThreadContext->bQSFPReadWriteGPIO = true;
        }
        else
        {
            pContext->pBrokerContext->pLocalSensorSupervisorThreadContext->bQSFPReadWriteGPIO = false;
        }
    }

}