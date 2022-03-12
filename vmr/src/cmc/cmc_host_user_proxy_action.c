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
 *  $Revision: #24 $
 *
 */





#include "cmc_host_user_proxy_thread.h"


 


void HostUserProxyThread_CLEAR_ACTION(HOST_USER_PROXY_THREAD_CONTEXT_TYPE *pContext)
{
    pContext->Action=0;    
}


void HostUserProxyThread_ACTION(HOST_USER_PROXY_THREAD_CONTEXT_TYPE *pContext, uint32_t AnyAction)
{
    pContext->Action|=AnyAction;
}


                           
                 
                  
                
                 
             

void HostUserProxyThread_HandleAction_A_HP_T_TRANSACTION_START(HOST_USER_PROXY_THREAD_CONTEXT_TYPE *pContext)
{
    pContext->CurrentHostProxyTransactionTimerInstance=TimerThread_Start(   
                                                            pContext->pThreadTimerContext,
                                                            T_HOST_PROXY_TRANSACTION_TIMER,
                                                            HOST_PROXY_TRANSACTION_TIMER_MS,
                                                            pContext,
                                                            HostUserProxyThread_TransactionTimerCallback);
}
 


   
void HostUserProxyThread_HandleAction_A_HP_T_TRANSACTION_STOP(HOST_USER_PROXY_THREAD_CONTEXT_TYPE *pContext)
{
    TimerThread_Stop(pContext->pThreadTimerContext, T_HOST_PROXY_TRANSACTION_TIMER);
}
 


   






void HostUserProxyThread_HandleAction_A_HP_PERFORM_LOCAL_ACTION(HOST_USER_PROXY_THREAD_CONTEXT_TYPE *pContext)
{
    switch(pContext->AdditionalInformationLocal)
    {
        case    HP_LOCAL_OPERATION_NONE:
                /* No additional work to do */
                break;

        case    HP_LOCAL_OPERATION_REBOOT_FIRMWARE:
                HostUserProxyThread_HandleLocalAction_HP_LOCAL_OPERATION_REBOOT_FIRMWARE(pContext);
                break;

        case    HP_LOCAL_OPERATION_STOP_FIRMWARE:
                HostUserProxyThread_HandleLocalAction_HP_LOCAL_OPERATION_STOP_FIRMWARE(pContext);
                break;

        case    HP_LOCAL_OPERATION_CLEAR_ERROR_REGISTER:
                HostUserProxyThread_HandleLocalAction_HP_LOCAL_OPERATION_CLEAR_ERROR_REGISTER(pContext);
                break;

        case    HP_LOCAL_OPERATION_CLEAR_SENSOR_REGISTER:
                HostUserProxyThread_HandleLocalAction_HP_LOCAL_OPERATION_CLEAR_SENSOR_REGISTER(pContext);
                break;

		case    HP_LOCAL_OPERATION_ENABLE_DEBUG_CAPTURE:
				HostUserProxyThread_HandleLocalAction_HP_LOCAL_OPERATION_ENABLE_DEBUG_CAPTURE(pContext);
				break;

		case    HP_LOCAL_OPERATION_DEBUG_CAPTURE_MSG_ID:
				HostUserProxyThread_HandleLocalAction_HP_LOCAL_OPERATION_DEBUG_CAPTURE_MSG_ID(pContext);
				break;

				
		
        case    HP_LOCAL_OPERATION_ENABLE_REMOTE_DEBUG_UART:
                HostUserProxyThread_HandleLocalAction_HP_LOCAL_OPERATION_ENABLE_REMOTE_DEBUG_UART(pContext);
                break;

        case    HP_LOCAL_OPERATION_FIRST_CSDR_REQUEST:
                HostUserProxyThread_HandleLocalAction_HP_LOCAL_OPERATION_FIRST_CSDR_REQUEST(pContext);
                break;

        case    HP_LOCAL_OPERATION_ADDITIONAL_CSDR_REQUEST:
                HostUserProxyThread_HandleLocalAction_HP_LOCAL_OPERATION_ADDITIONAL_CSDR_REQUEST(pContext);
                break;

        case    HP_LOCAL_OPERATION_CSDR_COMPLETE:
                HostUserProxyThread_HandleLocalAction_HP_LOCAL_OPERATION_CSDR_COMPLETE(pContext);
                break;

        case    HP_LOCAL_OPERATION_CSDR_FAILED:        
                HostUserProxyThread_HandleLocalAction_HP_LOCAL_OPERATION_CSDR_FAILED(pContext);
                break;

        case    HP_LOCAL_OPERATION_QSFP_READ_SINGLE_BYTE_REQUEST:
                HostUserProxyThread_HandleLocalAction_HP_LOCAL_OPERATION_QSFP_READ_SINGLE_BYTE_REQUEST(pContext);
                break;

        case    HP_LOCAL_OPERATION_QSFP_READ_SINGLE_BYTE_COMPLETE:
                HostUserProxyThread_HandleLocalAction_HP_LOCAL_OPERATION_QSFP_READ_SINGLE_BYTE_COMPLETE(pContext);
                break;

        case    HP_LOCAL_OPERATION_QSFP_READ_SINGLE_BYTE_FAILED:
                HostUserProxyThread_HandleLocalAction_HP_LOCAL_OPERATION_QSFP_READ_SINGLE_BYTE_FAILED(pContext);
                break;

        case    HP_LOCAL_OPERATION_QSFP_WRITE_SINGLE_BYTE_REQUEST:
                HostUserProxyThread_HandleLocalAction_HP_LOCAL_OPERATION_QSFP_WRITE_SINGLE_BYTE_REQUEST(pContext);
                break;

        case    HP_LOCAL_OPERATION_QSFP_WRITE_SINGLE_BYTE_COMPLETE:
                HostUserProxyThread_HandleLocalAction_HP_LOCAL_OPERATION_QSFP_WRITE_SINGLE_BYTE_COMPLETE(pContext);
                break;

        case    HP_LOCAL_OPERATION_QSFP_WRITE_SINGLE_BYTE_FAILED:
                HostUserProxyThread_HandleLocalAction_HP_LOCAL_OPERATION_QSFP_WRITE_SINGLE_BYTE_FAILED(pContext);
                break;

        case    HP_LOCAL_OPERATION_READ_QSFP_DIAGNOSTICS:
                HostUserProxyThread_HandleLocalAction_HP_LOCAL_OPERATION_READ_QSFP_DIAGNOSTICS(pContext);
                break;

        case    HP_LOCAL_OPERATION_READ_QSFP_VALIDATE_LOW_SPEED_IO:
                HostUserProxyThread_HandleLocalAction_HP_LOCAL_OPERATION_READ_QSFP_VALIDATE_LOW_SPEED_IO(pContext);
                break;

        case    HP_LOCAL_OPERATION_WRITE_QSFP_VALIDATE_LOW_SPEED_IO:
                HostUserProxyThread_HandleLocalAction_HP_LOCAL_OPERATION_WRITE_QSFP_VALIDATE_LOW_SPEED_IO(pContext);
                break;

        case    HP_LOCAL_OPERATION_READ_QSFP_DIAGNOSTICS_COMPLETE:
                HostUserProxyThread_HandleLocalAction_HP_LOCAL_OPERATION_READ_QSFP_DIAGNOSTICS_COMPLETE(pContext);
                break;

        case    HP_LOCAL_OPERATION_READ_QSFP_VALIDATE_LOW_SPEED_IO_COMPLETE:
                HostUserProxyThread_HandleLocalAction_HP_LOCAL_OPERATION_READ_QSFP_VALIDATE_LOW_SPEED_IO_COMPLETE(pContext);
                break;

        case    HP_LOCAL_OPERATION_WRITE_QSFP_VALIDATE_LOW_SPEED_IO_COMPLETE:
                HostUserProxyThread_HandleLocalAction_HP_LOCAL_OPERATION_WRITE_QSFP_VALIDATE_LOW_SPEED_IO_COMPLETE(pContext);
                break;

        case    HP_LOCAL_OPERATION_READ_QSFP_DIAGNOSTICS_FAILED:
                HostUserProxyThread_HandleLocalAction_HP_LOCAL_OPERATION_READ_QSFP_DIAGNOSTICS_FAILED(pContext);
                break;

        case    HP_LOCAL_OPERATION_READ_QSFP_VALIDATE_LOW_SPEED_IO_FAILED:
                HostUserProxyThread_HandleLocalAction_HP_LOCAL_OPERATION_READ_QSFP_VALIDATE_LOW_SPEED_IO_FAILED(pContext);
                break;

        case    HP_LOCAL_OPERATION_WRITE_QSFP_VALIDATE_LOW_SPEED_IO_FAILED:
                HostUserProxyThread_HandleLocalAction_HP_LOCAL_OPERATION_WRITE_QSFP_VALIDATE_LOW_SPEED_IO_FAILED(pContext);
                break;

        case    HP_LOCAL_OPERATION_HBM_SUPPORT_CMS:
                HostUserProxyThread_HandleLocalAction_HP_LOCAL_OPERATION_HBM_SUPPORT_CMS(pContext);
                break;

        case    HP_LOCAL_OPERATION_CLOCK_SCALING_CMS:
                HostUserProxyThread_HandleLocalAction_HP_LOCAL_OPERATION_CLOCK_SCALING_CMS(pContext);
                break;

        case    HP_LOCAL_OPERATION_BOARDINFO_REQUEST:
                HostUserProxyThread_HandleLocalAction_HP_LOCAL_OPERATION_BOARDINFO_REQUEST(pContext);
                break;

        case    HP_LOCAL_OPERATION_READ_BOARDINFO_COMPLETE:
                HostUserProxyThread_HandleLocalAction_HP_LOCAL_OPERATION_BOARDINFO_COMPLETE(pContext);
                break;

        case    HP_LOCAL_OPERATION_LOW_SPEED_QSFP_FROM_GPIO_CMS:
                HostUserProxyThread_HandleLocalAction_HP_LOCAL_OPERATION_LOW_SPEED_QSFP_FROM_GPIO_CMS(pContext);
                break;

        default:
                Watch_Inc(pContext->pWatchPointContext, W_HOST_PROXY_UNEXPECTED_LOCAL_ACTION);
                break;
    }
}
 


   
void HostUserProxyThread_HandleAction_A_HP_START_REMOTE_ACTION(HOST_USER_PROXY_THREAD_CONTEXT_TYPE *pContext)   
{
    switch(pContext->AdditionalInformationRemote)
    {
        case    HP_REMOTE_OPERATION_NONE:
                /* No additional work to do */
                break;

        case    HP_REMOTE_OPERATION_ERASE_FIRMWARE:
        case    HP_REMOTE_OPERATION_REMOTE_JUMP_TO_RESET_VECTOR:
        case    HP_REMOTE_OPERATION_REMOTE_DATA_SEGMENT:
        case    HP_REMOTE_OPERATION_REMOTE_FIRST_DATA_SEGMENT:
                HostUserProxyThread_UpdateResultErrorCodeRegister(pContext, RESULT_ERROR_CODE_CMC_HOST_MSG_NO_ERROR);
                Broker_ForwardUserRequestToBootloader(pContext->pBrokerContext, (uint32_t)pContext->AdditionalInformationRemote);
                break;

        case    HP_REMOTE_OPERATION_REMOTE_BOARD_INFORMATION:
                /* Nothing to do  */
                break;

        default:
                Watch_Inc(pContext->pWatchPointContext, W_HOST_PROXY_UNEXPECTED_REMOTE_ACTION);
                break;
    }
}
 


void HostUserProxyThread_HandleAction_Helper_UpdateResultErrorCodeRegisterOnFail(HOST_USER_PROXY_THREAD_CONTEXT_TYPE *pContext)   
{
    switch(pContext->AdditionalInformationRemote)
    {
        case    HP_REMOTE_OPERATION_NONE:
                /* No additional work to do */
                break;

        case    HP_REMOTE_OPERATION_ERASE_FIRMWARE:
                HostUserProxyThread_UpdateResultErrorCodeRegister(pContext, RESULT_ERROR_CODE_CMC_HOST_MSG_SAT_FW_ERASE_FAIL);
                break;

        case    HP_REMOTE_OPERATION_REMOTE_JUMP_TO_RESET_VECTOR:
                HostUserProxyThread_UpdateResultErrorCodeRegister(pContext, RESULT_ERROR_CODE_CMC_HOST_MSG_SAT_FW_LOAD_FAIL);
                break;

        case    HP_REMOTE_OPERATION_REMOTE_DATA_SEGMENT:
                HostUserProxyThread_UpdateResultErrorCodeRegister(pContext, RESULT_ERROR_CODE_CMC_HOST_MSG_SAT_FW_UPDATE_FAIL);
                break;

        case    HP_REMOTE_OPERATION_REMOTE_FIRST_DATA_SEGMENT:
                HostUserProxyThread_UpdateResultErrorCodeRegister(pContext, RESULT_ERROR_CODE_CMC_HOST_MSG_SAT_FW_WRITE_FAIL);
                break;

        case    HP_REMOTE_OPERATION_REMOTE_BOARD_INFORMATION:
                /* Nothing to do */
                break;

        default:
                HostUserProxyThread_UpdateResultErrorCodeRegister(pContext, RESULT_ERROR_CODE_CMC_HOST_MSG_NO_ERROR);
                Watch_Inc(pContext->pWatchPointContext, W_HOST_PROXY_UNEXPECTED_REMOTE_ACTION);
                break;
    }
}

   
void HostUserProxyThread_HandleAction_A_HP_CANCEL_REMOTE_OPERATION(HOST_USER_PROXY_THREAD_CONTEXT_TYPE *pContext)
{
    HostUserProxyThread_UpdateResultErrorCodeRegister(pContext, RESULT_ERROR_CODE_CMC_HOST_MSG_NO_ERROR);
    HostUserProxyThread_ClearCurrentControlRegisterRequest(pContext);
}
 


   
void HostUserProxyThread_HandleAction_A_HP_ACKNOWLEDGE_REMOTE_OPERATION_FAILED(HOST_USER_PROXY_THREAD_CONTEXT_TYPE *pContext)
{
    HostUserProxyThread_HandleAction_Helper_UpdateResultErrorCodeRegisterOnFail(pContext);
    HostUserProxyThread_ClearCurrentControlRegisterRequest(pContext);
}
 


   
void HostUserProxyThread_HandleAction_A_HP_ACKNOWLEDGE_REMOTE_OPERATION_SUCCESS(HOST_USER_PROXY_THREAD_CONTEXT_TYPE *pContext)
{
    HostUserProxyThread_UpdateResultErrorCodeRegister(pContext, RESULT_ERROR_CODE_CMC_HOST_MSG_NO_ERROR);
    HostUserProxyThread_ClearCurrentControlRegisterRequest(pContext);
}
 

void HostUserProxyThread_HandleAction_A_HP_ACTIVATE_BOOTLOADER(HOST_USER_PROXY_THREAD_CONTEXT_TYPE *pContext)
{
    Broker_ActivateBootloader(pContext->pBrokerContext);
}
                           

void HostUserProxyThread_HandleAction(HOST_USER_PROXY_THREAD_CONTEXT_TYPE *pContext)
{
    uint32_t i;

    for(i=0;i<MAX_HP_ACTIONS; i++)
    {
        switch((1<<i)&(pContext->Action))
        {
            case    A_HP_NO_ACTION:
                    /* Ignore */
                    break;

            case    A_HP_T_TRANSACTION_START:
                    HostUserProxyThread_HandleAction_A_HP_T_TRANSACTION_START(pContext);
                    break;
                               
            case    A_HP_T_TRANSACTION_STOP: 
                    HostUserProxyThread_HandleAction_A_HP_T_TRANSACTION_STOP(pContext);
                    break;
                                                         
            case    A_HP_PERFORM_LOCAL_ACTION:
                    HostUserProxyThread_HandleAction_A_HP_PERFORM_LOCAL_ACTION(pContext);
                    break;
                                                       
            case    A_HP_START_REMOTE_ACTION:
                    HostUserProxyThread_HandleAction_A_HP_START_REMOTE_ACTION(pContext);
                    break;
                                       
            case    A_HP_CANCEL_REMOTE_OPERATION:      
                    HostUserProxyThread_HandleAction_A_HP_CANCEL_REMOTE_OPERATION(pContext);
                    break;
                                               
            case    A_HP_ACKNOWLEDGE_REMOTE_OPERATION_FAILED:  
                    HostUserProxyThread_HandleAction_A_HP_ACKNOWLEDGE_REMOTE_OPERATION_FAILED(pContext);
                    break;
                                   
            case    A_HP_ACKNOWLEDGE_REMOTE_OPERATION_SUCCESS:
                    HostUserProxyThread_HandleAction_A_HP_ACKNOWLEDGE_REMOTE_OPERATION_SUCCESS(pContext);
                    break;
                          
            case    A_HP_ACTIVATE_BOOTLOADER:
                    HostUserProxyThread_HandleAction_A_HP_ACTIVATE_BOOTLOADER(pContext);
                    break;

            default:
                    Watch_Inc(pContext->pWatchPointContext, W_LINK_STATE_UNEXPECTED_ACTION);
                    break;
        }
    }
        

}



