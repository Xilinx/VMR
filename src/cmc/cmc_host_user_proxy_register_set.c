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
 *  $Revision: #37 $
 *
 */






#include "cmc_host_user_proxy_thread.h"
#include "cmc_data_store_constants.h"
#include "cmc_data_store_board_info.h"
#include "cmc_local_sensor_supervisor_thread.h"
#include "cmc_peripheral_regmap_ram_controller.h"
#include "cmc_local_bootloader_thread.h"

extern LOCAL_SENSOR_SUPERVISOR_THREAD_CONTEXT_TYPE LocalSensorSupervisorThreadContext;

void HostUserProxyThread_UpdateResultErrorCodeRegister(HOST_USER_PROXY_THREAD_CONTEXT_TYPE* pContext, uint32_t AnyErrorCode)
{
    uint32_t Value;

    PERIPHERAL_REGMAP_RAM_CONTROLLER_Write(pContext->pPeripheralRegMapRamControllerContext, HOST_REGISTER_RESULT_ERROR_CODE_REGISTER, AnyErrorCode);
    /* Latch the host message error */
    if (RESULT_ERROR_CODE_CMC_HOST_MSG_NO_ERROR != AnyErrorCode)
    {
        PERIPHERAL_REGMAP_RAM_CONTROLLER_WriteWithMask(pContext->pPeripheralRegMapRamControllerContext, HOST_REGISTER_ERROR_REG, HOST_ERROR_REG_HOST_MSG_ERROR, HOST_ERROR_REG_HOST_MSG_ERROR);
    }

    Value = PERIPHERAL_REGMAP_RAM_CONTROLLER_Read(pContext->pPeripheralRegMapRamControllerContext, CMC_HOST_ERR_COVERAGE_REG + (4 * AnyErrorCode));
    PERIPHERAL_REGMAP_RAM_CONTROLLER_Write(pContext->pPeripheralRegMapRamControllerContext, CMC_HOST_ERR_COVERAGE_REG + (4 * AnyErrorCode), Value++);
}

void HostUserProxyThread_ClearCurrentControlRegisterRequest(HOST_USER_PROXY_THREAD_CONTEXT_TYPE *pContext)
{
    uint32_t ControlRegisterValue=PERIPHERAL_REGMAP_RAM_CONTROLLER_Read(pContext->pPeripheralRegMapRamControllerContext, HOST_REGISTER_CONTROL);

    ControlRegisterValue=ControlRegisterValue&~(HOST_REGISTER_PATTERN_ANY_REQUEST);

    PERIPHERAL_REGMAP_RAM_CONTROLLER_WriteWithMask(pContext->pPeripheralRegMapRamControllerContext, HOST_REGISTER_CONTROL, ControlRegisterValue, (uint32_t)HOST_REGISTER_PATTERN_ANY_REQUEST);
}

void HostUserProxyThread_ReadControlRegister(HOST_USER_PROXY_THREAD_CONTEXT_TYPE *pContext)
{
    uint32_t    ControlRegisterValue=PERIPHERAL_REGMAP_RAM_CONTROLLER_Read(pContext->pPeripheralRegMapRamControllerContext, HOST_REGISTER_CONTROL);
    uint32_t    StopConfirmRegisterValue;
    uint32_t    RemoteMessageRegisterValue;
    uint32_t    RemoteMessageHeader;
    bool        SearchingForRequest=true;

    uint32_t    HBMSupportedCMS=pContext->HBMSupportedCMS;
    uint32_t    CardSupportsScalingFactorCMS=pContext->CardSupportsScalingFactorCMS;
    uint32_t    CardSupportsLowSpeedQSFPFromGPIOCMS = pContext->CardSupportsLowSpeedQSFPFromGPIOCMS;


    if(SearchingForRequest&&(ControlRegisterValue & HOST_REGISTER_PATTERN_REBOOT_FIRMWARE))
    {
        HostUserProxyThread_CreateEvent_E_HP_LOCAL_OPERATION_REQUEST(pContext, HP_LOCAL_OPERATION_REBOOT_FIRMWARE);
        SearchingForRequest=false;
    }
    if(SearchingForRequest&&(ControlRegisterValue & HOST_REGISTER_PATTERN_STOP_FIRMWARE))
    {
        StopConfirmRegisterValue=PERIPHERAL_REGMAP_RAM_CONTROLLER_Read(pContext->pPeripheralRegMapRamControllerContext, HOST_REGISTER_STOP_CONFIRM);
        if(StopConfirmRegisterValue & HOST_REGISTER_PATTERN_STOP_CONFIRM)
        {
            HostUserProxyThread_CreateEvent_E_HP_LOCAL_OPERATION_REQUEST(pContext, HP_LOCAL_OPERATION_STOP_FIRMWARE);
            SearchingForRequest=false;
        }
    }
    if(SearchingForRequest&&(ControlRegisterValue & HOST_REGISTER_PATTERN_CLEAR_ERRORS))
    {
        HostUserProxyThread_CreateEvent_E_HP_LOCAL_OPERATION_REQUEST(pContext, HP_LOCAL_OPERATION_CLEAR_ERROR_REGISTER);
        SearchingForRequest=false;
    }
    if(SearchingForRequest&&(ControlRegisterValue & HOST_REGISTER_PATTERN_RESET_SENSORS))
    {
        HostUserProxyThread_CreateEvent_E_HP_LOCAL_OPERATION_REQUEST(pContext, HP_LOCAL_OPERATION_CLEAR_SENSOR_REGISTER);
        SearchingForRequest=false;
    }
    if (SearchingForRequest && (ControlRegisterValue & HOST_REGISTER_PATTERN_ENABLE_DEBUG_CAPTURE))
    {
        HostUserProxyThread_CreateEvent_E_HP_LOCAL_OPERATION_REQUEST(pContext, HP_LOCAL_OPERATION_ENABLE_DEBUG_CAPTURE);
        SearchingForRequest = false;
    }
    if (SearchingForRequest && (ControlRegisterValue & HOST_REGISTER_PATTERN_DEBUG_CAPTURE_MSG_ID))
    {
        pContext->DebugCaptureMsgID = (uint8_t)((ControlRegisterValue & HOST_REGISTER_PATTERN_DEBUG_CAPTURE_MSG_ID) >> 8);
        HostUserProxyThread_CreateEvent_E_HP_LOCAL_OPERATION_REQUEST(pContext, HP_LOCAL_OPERATION_DEBUG_CAPTURE_MSG_ID);
        SearchingForRequest = false;
    }
    if(SearchingForRequest&&(ControlRegisterValue & HOST_REGISTER_PATTERN_ENABLE_REMOTE_DEBUG_UART))
    {
        /* treat this as a local operation */
        HostUserProxyThread_CreateEvent_E_HP_LOCAL_OPERATION_REQUEST(pContext, HP_LOCAL_OPERATION_ENABLE_REMOTE_DEBUG_UART);
        SearchingForRequest=false;
    }
    if (SearchingForRequest && (HBMSupportedCMS != (ControlRegisterValue & HOST_REGISTER_PATTERN_HBM_SUPPORT_CMS)))
    {
        /* treat this as a local operation */
        HostUserProxyThread_CreateEvent_E_HP_LOCAL_OPERATION_REQUEST(pContext, HP_LOCAL_OPERATION_HBM_SUPPORT_CMS);
        SearchingForRequest = false;
        pContext->HBMSupportedCMS = (ControlRegisterValue & HOST_REGISTER_PATTERN_HBM_SUPPORT_CMS);
    }
    if (SearchingForRequest && (CardSupportsScalingFactorCMS != (ControlRegisterValue & HOST_REGISTER_PATTERN_CLOCK_SCALING_CMS)))
    {
        /* treat this as a local operation */
        HostUserProxyThread_CreateEvent_E_HP_LOCAL_OPERATION_REQUEST(pContext, HP_LOCAL_OPERATION_CLOCK_SCALING_CMS);
        SearchingForRequest = false;
        pContext->CardSupportsScalingFactorCMS = (ControlRegisterValue & HOST_REGISTER_PATTERN_CLOCK_SCALING_CMS);
    }
    if (SearchingForRequest && (CardSupportsLowSpeedQSFPFromGPIOCMS != (ControlRegisterValue & HOST_REGISTER_PATTERN_LOW_SPEED_QSFP_FROM_GPIO_CMS)))
    {
        /* treat this as a local operation */
        HostUserProxyThread_CreateEvent_E_HP_LOCAL_OPERATION_REQUEST(pContext, HP_LOCAL_OPERATION_LOW_SPEED_QSFP_FROM_GPIO_CMS);
        SearchingForRequest = false;
        pContext->CardSupportsLowSpeedQSFPFromGPIOCMS = (ControlRegisterValue & HOST_REGISTER_PATTERN_LOW_SPEED_QSFP_FROM_GPIO_CMS);
    }
    if(SearchingForRequest&&(ControlRegisterValue & HOST_REGISTER_PATTERN_REMOTE_COMMAND))
    {
        RemoteMessageRegisterValue=PERIPHERAL_REGMAP_RAM_CONTROLLER_Read(pContext->pPeripheralRegMapRamControllerContext, HOST_REGISTER_REMOTE_COMMAND_REGISTER);
        RemoteMessageHeader=(RemoteMessageRegisterValue&0xFF000000)>>24;
          
        if (REMOTE_COMMAND_REGISTER_PATTERN_QSFP_READ_SINGLE_BYTE == RemoteMessageHeader)
        {
            SearchingForRequest = false;
            if (!pContext->bQSFP_ReadSingleByteInProgress)
            {
                // TODO Need a location for host message coverage counts
                //cmcIncrementReg(pContext->pPeripheralRegMapRamControllerContext, CMC_HOST_MSG_COVERAGE_REG + (0xB << 2));
                pContext->bQSFP_ReadSingleByteInProgress = true;
                HostUserProxyThread_CreateEvent_E_HP_LOCAL_OPERATION_REQUEST(pContext, HP_LOCAL_OPERATION_QSFP_READ_SINGLE_BYTE_REQUEST);
            }
        }

        if (REMOTE_COMMAND_REGISTER_PATTERN_QSFP_WRITE_SINGLE_BYTE == RemoteMessageHeader)
        {
            SearchingForRequest = false;
            if (!pContext->bQSFP_WriteSingleByteInProgress)
            {
                // TODO Need a location for host message coverage counts
                //cmcIncrementReg(pContext->pPeripheralRegMapRamControllerContext, CMC_HOST_MSG_COVERAGE_REG + (0xC << 2));
                pContext->bQSFP_WriteSingleByteInProgress = true;
                HostUserProxyThread_CreateEvent_E_HP_LOCAL_OPERATION_REQUEST(pContext, HP_LOCAL_OPERATION_QSFP_WRITE_SINGLE_BYTE_REQUEST);
            }
        }

        if (REMOTE_COMMAND_REGISTER_PATTERN_READ_QSFP_DIAGNOSTICS == RemoteMessageHeader)
        {
            SearchingForRequest = false;
            if (!pContext->bQSFP_ReadDiagnosticsInProgress)
            {
                // TODO Need a location for host message coverage counts
                //cmcIncrementReg(pContext->pPeripheralRegMapRamControllerContext, CMC_HOST_MSG_COVERAGE_REG + (0xB << 2));
                pContext->bQSFP_ReadDiagnosticsInProgress = true;
                HostUserProxyThread_CreateEvent_E_HP_LOCAL_OPERATION_REQUEST(pContext, HP_LOCAL_OPERATION_READ_QSFP_DIAGNOSTICS);
            }
        }
        
        if (REMOTE_COMMAND_REGISTER_PATTERN_READ_QSFP_VALIDATE_LOW_SPEED_IO == RemoteMessageHeader)
        {
            SearchingForRequest = false;
            if (!pContext->bQSFP_ReadLowSpeedIOInProgress)
            {
                // TODO Need a location for host message coverage counts
                //cmcIncrementReg(pContext->pPeripheralRegMapRamControllerContext, CMC_HOST_MSG_COVERAGE_REG + (0xD << 2));
                pContext->bQSFP_ReadLowSpeedIOInProgress = true;
                HostUserProxyThread_CreateEvent_E_HP_LOCAL_OPERATION_REQUEST(pContext, HP_LOCAL_OPERATION_READ_QSFP_VALIDATE_LOW_SPEED_IO);
            }
        }
            
        if (REMOTE_COMMAND_REGISTER_PATTERN_WRITE_QSFP_VALIDATE_LOW_SPEED_IO == RemoteMessageHeader)
        {
            SearchingForRequest = false;
            if (!pContext->bQSFP_WriteLowSpeedIOInProgress)
            {
                // TODO Need a location for host message coverage counts
                //cmcIncrementReg(pContext->pPeripheralRegMapRamControllerContext, CMC_HOST_MSG_COVERAGE_REG + (0xE << 2));
                pContext->bQSFP_WriteLowSpeedIOInProgress = true;
                HostUserProxyThread_CreateEvent_E_HP_LOCAL_OPERATION_REQUEST(pContext, HP_LOCAL_OPERATION_WRITE_QSFP_VALIDATE_LOW_SPEED_IO);
            }
        }

        if (REMOTE_COMMAND_REGISTER_PATTERN_FIRST_CSDR_REQUEST == RemoteMessageHeader)
        {
            SearchingForRequest = false;
            if(!pContext->bCSDRInProgress)
            {   
                // TODO Need a location for host message coverage counts
                //cmcIncrementReg(pContext->pPeripheralRegMapRamControllerContext, CMC_HOST_MSG_COVERAGE_REG + (0x9 << 2));
                pContext->bCSDRInProgress = true;              
                HostUserProxyThread_CreateEvent_E_HP_LOCAL_OPERATION_REQUEST(pContext, HP_LOCAL_OPERATION_FIRST_CSDR_REQUEST);
            }
        }
        if (REMOTE_COMMAND_REGISTER_PATTERN_ADDITIONAL_CSDR_REQUEST == RemoteMessageHeader)
        {
            SearchingForRequest = false;
            if (!pContext->bCSDRInProgress)
            {
                // TODO Need a location for host message coverage counts
                //cmcIncrementReg(pContext->pPeripheralRegMapRamControllerContext, CMC_HOST_MSG_COVERAGE_REG + (0xA << 2));
                pContext->bCSDRInProgress = true;
                HostUserProxyThread_CreateEvent_E_HP_LOCAL_OPERATION_REQUEST(pContext, HP_LOCAL_OPERATION_ADDITIONAL_CSDR_REQUEST);
            }
        }

        if(REMOTE_COMMAND_REGISTER_PATTERN_BOARD_INFORMATION==RemoteMessageHeader)
        {
            SearchingForRequest=false;
            if (!pContext->bBoardInfoInProgress)
            {
                pContext->bBoardInfoInProgress = true;
                HostUserProxyThread_CreateEvent_E_HP_LOCAL_OPERATION_REQUEST(pContext, HP_LOCAL_OPERATION_BOARDINFO_REQUEST);
            
                /* Increment the message coverage counter */
                /* Ensure RemoteMessageHeader is never 0 as this would write over TEMP_THRESHOLD registers*/
                if ((RemoteMessageHeader > 0) && (RemoteMessageHeader < CMC_HOST_OP_MAX))
                {
                    cmcIncrementReg(pContext->pPeripheralRegMapRamControllerContext, CMC_HOST_MSG_COVERAGE_REG + (RemoteMessageHeader << 2));
                }
            }
        }    
    }

    
    if (SearchingForRequest && (ControlRegisterValue & HOST_REGISTER_PATTERN_REMOTE_COMMAND))
    {
        if (pContext->bCardSupportsSCUpgrade)
        {
            RemoteMessageRegisterValue = PERIPHERAL_REGMAP_RAM_CONTROLLER_Read(pContext->pPeripheralRegMapRamControllerContext, HOST_REGISTER_REMOTE_COMMAND_REGISTER);
            RemoteMessageHeader = (RemoteMessageRegisterValue & 0xFF000000) >> 24;

            if (RemoteMessageHeader == REMOTE_COMMAND_REGISTER_PATTERN_FIRST_DATA_SEGMENT ||
                RemoteMessageHeader == REMOTE_COMMAND_REGISTER_PATTERN_DATA_SEGMENT ||
                RemoteMessageHeader == REMOTE_COMMAND_REGISTER_PATTERN_JUMP_TO_RESET_VECTOR ||
                RemoteMessageHeader == REMOTE_COMMAND_REGISTER_PATTERN_ERASE_FIRMWARE)
            {
                switch (HostUserProxyThread_GetState(pContext))
                {
                case    S_HP_INITIAL:
                    //Watch_Inc(pContext->pWatchPointContext, W_FREE_2_0DC8);
                    Broker_StopCollectingSensorInformation(pContext->pBrokerContext);
                    Broker_ResetBootloader(pContext->pBrokerContext);
                    Broker_ActivateBootloader(pContext->pBrokerContext);
                    HostUserProxyThread_CreateEvent_E_HP_REQUESTED_LINK_USER_AS_BOOTLOADER(pContext);
                    Broker_RequestLinkUserIsBootloader(pContext->pBrokerContext);
                    break;

                case    S_HP_WAITING_FOR_BOOTLOADER:
                    /* Wait until the Link User is bootloader before handling the next request */
                    break;

                case    S_HP_BOOTLOADER:

                    switch (RemoteMessageHeader)
                    {
                    case    REMOTE_COMMAND_REGISTER_PATTERN_FIRST_DATA_SEGMENT:
                        HostUserProxyThread_CreateEvent_E_HP_REMOTE_OPERATION_REQUEST(pContext, HP_REMOTE_OPERATION_REMOTE_FIRST_DATA_SEGMENT);
                        break;

                    case    REMOTE_COMMAND_REGISTER_PATTERN_DATA_SEGMENT:
                        HostUserProxyThread_CreateEvent_E_HP_REMOTE_OPERATION_REQUEST(pContext, HP_REMOTE_OPERATION_REMOTE_DATA_SEGMENT);
                        break;

                    case    REMOTE_COMMAND_REGISTER_PATTERN_JUMP_TO_RESET_VECTOR:
                        HostUserProxyThread_CreateEvent_E_HP_REMOTE_OPERATION_REQUEST(pContext, HP_REMOTE_OPERATION_REMOTE_JUMP_TO_RESET_VECTOR);
                        break;

                    case    REMOTE_COMMAND_REGISTER_PATTERN_ERASE_FIRMWARE:
                        HostUserProxyThread_CreateEvent_E_HP_REMOTE_OPERATION_REQUEST(pContext, HP_REMOTE_OPERATION_ERASE_FIRMWARE);
                        break;

                    default:
                        HostUserProxyThread_UpdateResultErrorCodeRegister(pContext, RESULT_ERROR_CODE_CMC_HOST_MSG_BAD_OPCODE);
                        Watch_Inc(pContext->pWatchPointContext, W_HOST_PROXY_UNRECOGNISED_REMOTE_COMMAND);
                        break;
                    }

                    /* Increment the message coverage counter */
                    /* Ensure RemoteMessageHeader is never 0 as this would write over TEMP_THRESHOLD registers*/
                    if ((RemoteMessageHeader > 0) && (RemoteMessageHeader < CMC_HOST_OP_MAX))
                    {
                        cmcIncrementReg(pContext->pPeripheralRegMapRamControllerContext, CMC_HOST_MSG_COVERAGE_REG + (RemoteMessageHeader << 2));
                    }

                    break;

                case    S_HP_TRANSACTION_RUNNING:
                    /* Wait until the current transaction finishes before running the next one */
                    break;

                default:
                    Watch_Inc(pContext->pWatchPointContext, W_HOST_PROXY_FSM_ILLEGAL_STATE);
                    break;
                }

            }
        }
        else
        {
            HostUserProxyThread_UpdateResultErrorCodeRegister(pContext, RESULT_ERROR_CODE_CMC_HOST_MSG_SAT_FW_UPDATE_BLOCKED);
            HostUserProxyThread_ClearCurrentControlRegisterRequest(pContext);
        }
    }
}



           
            
           
          
         
