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
 *  $Change: 3217619 $
 *  $Date: 2021/05/13 $
 *  $Revision: #42 $
 *
 */




#include "cmc_thread_communications_broker.h"
#include "cmc_local_sensor_supervisor_thread.h"
#include "cmc_local_bootloader_thread.h"
#include "cmc_thread_linkstate.h"
#include "cmc_link_receive_thread.h"
#include "cmc_host_user_proxy_thread.h"


void Broker_ForwardUserRequestToBootloader(BROKER_CONTEXT_TYPE* pBrokerContext, uint32_t AnyRequest)
{
    switch((HOST_PROXY_REMOTE_OPERATION_TYPE)AnyRequest)
    {
        case    HP_REMOTE_OPERATION_ERASE_FIRMWARE:
                Bootloader_CreateEvent_E_LB_COMMAND_ERASE_FIRMWARE_SEQUENCE(pBrokerContext->pBootloaderThreadContext);
                break;

        case    HP_REMOTE_OPERATION_REMOTE_JUMP_TO_RESET_VECTOR:
                Bootloader_CreateEvent_E_LB_COMMAND_RESTART_FIRMWARE(pBrokerContext->pBootloaderThreadContext);
                break;

        case    HP_REMOTE_OPERATION_REMOTE_DATA_SEGMENT:
                Bootloader_CreateEvent_E_LB_COMMAND_UPDATE_FIRMWARE(pBrokerContext->pBootloaderThreadContext);
                break;

        case    HP_REMOTE_OPERATION_REMOTE_FIRST_DATA_SEGMENT:
                Bootloader_CreateEvent_E_LB_COMMAND_UPDATE_FIRMWARE(pBrokerContext->pBootloaderThreadContext);
                break;

        default:
                /* Ignore */
                break;
    }
}


void Broker_ResetBootloader(BROKER_CONTEXT_TYPE* pBrokerContext)
{
    Bootloader_CreateEvent_E_LB_RESET_BOOTLOADER(pBrokerContext->pBootloaderThreadContext);
}


void Broker_ActivateBootloader(BROKER_CONTEXT_TYPE* pBrokerContext)
{
    Bootloader_CreateEvent_E_LB_ACTIVATE_BOOTLOADER(pBrokerContext->pBootloaderThreadContext);
}


void Broker_SensorSupervisorRequestsLinkState(BROKER_CONTEXT_TYPE* pBrokerContext)
{
    if(LinkState_CanSensorSupervisorUseLink(pBrokerContext->pLinkStateThreadContext))
    {
        SensorSupervisor_CreateEvent_E_SS_LINK_IS_AVAILABLE(pBrokerContext->pLocalSensorSupervisorThreadContext);
    }
    else
    {
        SensorSupervisor_CreateEvent_E_SS_LINK_IS_UNAVAILABLE(pBrokerContext->pLocalSensorSupervisorThreadContext);
    }
}


void Broker_AnnounceLinkUserIsSensorSupervisor(BROKER_CONTEXT_TYPE* pBrokerContext)
{
    Bootloader_CreateEvent_E_LB_LINK_USER_IS_SENSOR_SUPERVISOR(pBrokerContext->pBootloaderThreadContext);
    //SensorSupervisor_CreateEvent_E_SS_LINK_IS_AVAILABLE(pBrokerContext->pLocalSensorSupervisorThreadContext);
    HostUserProxyThread_CreateEvent_E_HP_LINK_USER_IS_SENSOR_SUPERVISOR(pBrokerContext->pHostUserProxyThreadContext);
}

void Broker_AnnounceLinkUserIsSensorSupervisorSUC(BROKER_CONTEXT_TYPE* pBrokerContext)
{
    HostUserProxyThread_CreateEvent_E_HP_LINK_USER_IS_SENSOR_SUPERVISOR_SUC(pBrokerContext->pHostUserProxyThreadContext);
}

void Broker_AnnounceLinkUserIsSensorSupervisorLinkOperating(BROKER_CONTEXT_TYPE* pBrokerContext)
{
	SensorSupervisor_CreateEvent_E_SS_LINK_IS_AVAILABLE(pBrokerContext->pLocalSensorSupervisorThreadContext);
}

void Broker_AnnounceLinkUserIsBootloader(BROKER_CONTEXT_TYPE* pBrokerContext)
{
    Bootloader_CreateEvent_E_LB_LINK_USER_IS_BOOTLOADER(pBrokerContext->pBootloaderThreadContext);
    SensorSupervisor_CreateEvent_E_SS_LINK_IS_UNAVAILABLE(pBrokerContext->pLocalSensorSupervisorThreadContext);
    HostUserProxyThread_CreateEvent_E_HP_LINK_USER_IS_BOOTLOADER(pBrokerContext->pHostUserProxyThreadContext);

}

void Broker_StopCollectingSensorInformation(BROKER_CONTEXT_TYPE* pBrokerContext)
{
    SensorSupervisor_CreateEvent_E_SS_LINK_IS_UNAVAILABLE(pBrokerContext->pLocalSensorSupervisorThreadContext);
}

void Broker_SensorSupervisor_GPIO_Interrupt_Arrival(BROKER_CONTEXT_TYPE* pBrokerContext)
{
	SensorSupervisor_CreateEvent_E_SS_GPIO_INTERRUPT_ARRIVAL(pBrokerContext->pLocalSensorSupervisorThreadContext);
}

void Broker_SensorSupervisor_Message_Arrival(BROKER_CONTEXT_TYPE* pBrokerContext)
{
	SensorSupervisor_CreateEvent_E_SS_MESSAGE_ARRIVAL(pBrokerContext->pLocalSensorSupervisorThreadContext);
}

void Broker_SensorSupervisor_Cage_Info_Arrival(BROKER_CONTEXT_TYPE* pBrokerContext)
{
	SensorSupervisor_CreateEvent_E_SS_CAGE_INFORMATION_ARRIVAL(pBrokerContext->pLocalSensorSupervisorThreadContext);
}

void Broker_SensorSupervisor_Reset_Sensors(BROKER_CONTEXT_TYPE* pBrokerContext)
{
	DataStore_ResetSensors(&(pBrokerContext->pLocalSensorSupervisorThreadContext->DataStoreContext));
}

void Broker_Enable_Debug_Capture(BROKER_CONTEXT_TYPE* pBrokerContext)
{
	LinkReceiveThread_Packet_Capture_Enable(pBrokerContext->pLinkReceiveThreadContext);
}

void Broker_Store_Debug_Capture_MsgID(BROKER_CONTEXT_TYPE* pBrokerContext, uint8_t MsgID)
{
	LinkReceiveThread_Store_Debug_Capture_MsgID(pBrokerContext->pLinkReceiveThreadContext, MsgID);
}

void Broker_SensorSupervisor_Debug_Uart_Response_Arrival(BROKER_CONTEXT_TYPE* pBrokerContext)
{
    SensorSupervisor_CreateEvent_E_SS_DEBUG_UART_RESPONSE_ARRIVAL(pBrokerContext->pLocalSensorSupervisorThreadContext);
}

void Broker_DetermineLinkState(BROKER_CONTEXT_TYPE* pBrokerContext)
{
    LinkState_CreateEvent_E_LS_RESET_REQUEST(pBrokerContext->pLinkStateThreadContext);
}

void Broker_RequestLinkUserIsSensorSupervisor(BROKER_CONTEXT_TYPE* pBrokerContext)
{
    LinkState_CreateEvent_E_LS_LINK_USER_IS_REMOTE_SENSOR_SUPERVISOR(pBrokerContext->pLinkStateThreadContext);
}

void Broker_RequestLinkUserIsBootloader(BROKER_CONTEXT_TYPE* pBrokerContext)
{
    LinkState_CreateEvent_E_LS_LINK_USER_TO_BOOTLOADER_REQUEST(pBrokerContext->pLinkStateThreadContext);
}

void Broker_Announce_LinkstateVersionResponse(BROKER_CONTEXT_TYPE* pBrokerContext)
{
	LinkState_CreateEvent_E_LS_LINK_ACCEPTED_PROTOCOL_VERSION_RANGE_RESPONSE(pBrokerContext->pLinkStateThreadContext);
}


void Broker_Announce_LinkstateVersionResponseFailed(BROKER_CONTEXT_TYPE* pBrokerContext)
{
	LinkState_CreateEvent_E_LS_LINK_ACCEPTED_PROTOCOL_VERSION_RANGE_RESPONSE_FAILED(pBrokerContext->pLinkStateThreadContext);
}

void Broker_Announce_LinkstateVersionAcknowledge(BROKER_CONTEXT_TYPE* pBrokerContext)
{
	LinkState_CreateEvent_E_LS_LINK_OFFERED_VERSION_ACKNOWLEDGE(pBrokerContext->pLinkStateThreadContext);
}

void Broker_Announce_LinkstateI2CResponseFailed(BROKER_CONTEXT_TYPE* pBrokerContext)
{
    LinkState_CreateEvent_E_LS_LINK_I2C_FAILED(pBrokerContext->pLinkStateThreadContext);
}

void Broker_Announce_LinkstateI2CAcknowledge(BROKER_CONTEXT_TYPE* pBrokerContext)
{
    LinkState_CreateEvent_E_LS_LINK_I2C_ACKNOWLEDGE(pBrokerContext->pLinkStateThreadContext);
}

void Broker_Announce_LinkstateVersionDeclined(BROKER_CONTEXT_TYPE* pBrokerContext)
{
	LinkState_CreateEvent_E_LS_LINK_OFFERED_VERSION_DECLINED(pBrokerContext->pLinkStateThreadContext);
}

void Broker_Announce_RemoteBootloaderOperationSuccess(BROKER_CONTEXT_TYPE* pBrokerContext)
{
	Bootloader_CreateEvent_E_LB_REMOTE_ACK(pBrokerContext->pBootloaderThreadContext);
}

void Broker_Announce_UserProxyRemoteBootloaderOperationSuccess(BROKER_CONTEXT_TYPE* pBrokerContext)
{
	HostUserProxyThread_CreateEvent_E_HP_REMOTE_OPERATION_SUCCESS(pBrokerContext->pHostUserProxyThreadContext);
}


void Broker_Announce_RemoteBootloaderOperationFailed(BROKER_CONTEXT_TYPE* pBrokerContext)
{
	Bootloader_CreateEvent_E_LB_REMOTE_NAK(pBrokerContext->pBootloaderThreadContext);
}

void Broker_Announce_UserProxyRemoteBootloaderOperationFailed(BROKER_CONTEXT_TYPE* pBrokerContext)
{
	HostUserProxyThread_CreateEvent_E_HP_REMOTE_OPERATION_FAILED(pBrokerContext->pHostUserProxyThreadContext);
}

void Broker_Announce_UserProxyCSDRComplete(BROKER_CONTEXT_TYPE* pBrokerContext)
{
    HostUserProxyThread_CreateEvent_E_HP_LOCAL_OPERATION_REQUEST(pBrokerContext->pHostUserProxyThreadContext, HP_LOCAL_OPERATION_CSDR_COMPLETE);
}

void Broker_Announce_UserProxyCSDRFailed(BROKER_CONTEXT_TYPE* pBrokerContext)
{
    HostUserProxyThread_CreateEvent_E_HP_LOCAL_OPERATION_REQUEST(pBrokerContext->pHostUserProxyThreadContext, HP_LOCAL_OPERATION_CSDR_FAILED);
}


void Broker_Announce_SensorSupervisor_ReadQSFPDiagnosticsComplete(BROKER_CONTEXT_TYPE* pBrokerContext)
{
    HostUserProxyThread_CreateEvent_E_HP_LOCAL_OPERATION_REQUEST(pBrokerContext->pHostUserProxyThreadContext, HP_LOCAL_OPERATION_READ_QSFP_DIAGNOSTICS_COMPLETE);
}

void Broker_Announce_SensorSupervisor_WriteQSFPValidateLowSpeedIOComplete(BROKER_CONTEXT_TYPE* pBrokerContext)
{
    HostUserProxyThread_CreateEvent_E_HP_LOCAL_OPERATION_REQUEST(pBrokerContext->pHostUserProxyThreadContext, HP_LOCAL_OPERATION_WRITE_QSFP_VALIDATE_LOW_SPEED_IO_COMPLETE);
}

void Broker_Announce_SensorSupervisor_ReadQSFPValidateLowSpeedIOComplete(BROKER_CONTEXT_TYPE* pBrokerContext)
{
    HostUserProxyThread_CreateEvent_E_HP_LOCAL_OPERATION_REQUEST(pBrokerContext->pHostUserProxyThreadContext, HP_LOCAL_OPERATION_READ_QSFP_VALIDATE_LOW_SPEED_IO_COMPLETE);
}

void Broker_Announce_SensorSupervisor_ReadQSFPSingleByteComplete(BROKER_CONTEXT_TYPE* pBrokerContext)
{
    HostUserProxyThread_CreateEvent_E_HP_LOCAL_OPERATION_REQUEST(pBrokerContext->pHostUserProxyThreadContext, HP_LOCAL_OPERATION_QSFP_READ_SINGLE_BYTE_COMPLETE);
}

void Broker_Announce_SensorSupervisor_WriteQSFPSingleByteComplete(BROKER_CONTEXT_TYPE* pBrokerContext)
{
    HostUserProxyThread_CreateEvent_E_HP_LOCAL_OPERATION_REQUEST(pBrokerContext->pHostUserProxyThreadContext, HP_LOCAL_OPERATION_QSFP_WRITE_SINGLE_BYTE_COMPLETE);
}

void Broker_Announce_SensorSupervisor_ReadQSFPSingleByteFailed(BROKER_CONTEXT_TYPE* pBrokerContext)
{
    HostUserProxyThread_CreateEvent_E_HP_LOCAL_OPERATION_REQUEST(pBrokerContext->pHostUserProxyThreadContext, HP_LOCAL_OPERATION_QSFP_READ_SINGLE_BYTE_FAILED);
}

void Broker_Announce_SensorSupervisor_WriteQSFPSingleByteFailed(BROKER_CONTEXT_TYPE* pBrokerContext)
{
    HostUserProxyThread_CreateEvent_E_HP_LOCAL_OPERATION_REQUEST(pBrokerContext->pHostUserProxyThreadContext, HP_LOCAL_OPERATION_QSFP_WRITE_SINGLE_BYTE_FAILED);
}

void Broker_Announce_SensorSupervisor_BoardInfoComplete(BROKER_CONTEXT_TYPE* pBrokerContext)
{
    HostUserProxyThread_CreateEvent_E_HP_LOCAL_OPERATION_REQUEST(pBrokerContext->pHostUserProxyThreadContext, HP_LOCAL_OPERATION_READ_BOARDINFO_COMPLETE);
}

void Broker_Announce_SensorSupervisor_BoardInfoFailed(BROKER_CONTEXT_TYPE* pBrokerContext)
{
    HostUserProxyThread_CreateEvent_E_HP_LOCAL_OPERATION_REQUEST(pBrokerContext->pHostUserProxyThreadContext, HP_LOCAL_OPERATION_READ_BOARDINFO_FAILED);
}



void Broker_Announce_SensorSupervisor_ReadQSFPDiagnosticsFailed(BROKER_CONTEXT_TYPE* pBrokerContext)
{
    HostUserProxyThread_CreateEvent_E_HP_LOCAL_OPERATION_REQUEST(pBrokerContext->pHostUserProxyThreadContext, HP_LOCAL_OPERATION_READ_QSFP_DIAGNOSTICS_FAILED);
}

void Broker_Announce_SensorSupervisor_ReadQSFPValidateLowSpeedIOFailed(BROKER_CONTEXT_TYPE* pBrokerContext)
{
    HostUserProxyThread_CreateEvent_E_HP_LOCAL_OPERATION_REQUEST(pBrokerContext->pHostUserProxyThreadContext, HP_LOCAL_OPERATION_READ_QSFP_VALIDATE_LOW_SPEED_IO_FAILED);
}

void Broker_Announce_SensorSupervisor_WriteQSFPValidateLowSpeedIOFailed(BROKER_CONTEXT_TYPE* pBrokerContext)
{
    HostUserProxyThread_CreateEvent_E_HP_LOCAL_OPERATION_REQUEST(pBrokerContext->pHostUserProxyThreadContext, HP_LOCAL_OPERATION_WRITE_QSFP_VALIDATE_LOW_SPEED_IO_FAILED);
}


void Broker_Initialize( BROKER_CONTEXT_TYPE*                                    pBrokerContext,
                        struct LOCAL_SENSOR_SUPERVISOR_THREAD_CONTEXT_TYPE *    pLocalSensorSupervisorThreadContext,
                        struct LOCAL_BOOTLOADER_THREAD_CONTEXT_TYPE *           pBootloaderThreadContext,
                        struct LINK_STATE_CONTEXT_TYPE *                        pLinkStateThreadContext,
                        struct HOST_USER_PROXY_THREAD_CONTEXT_TYPE *            pHostUserProxyThreadContext,
						struct LINK_RECEIVE_THREAD_CONTEXT_TYPE*				pLinkReceiveThreadContext)
{
    pBrokerContext->pLocalSensorSupervisorThreadContext=pLocalSensorSupervisorThreadContext;
    pBrokerContext->pBootloaderThreadContext=pBootloaderThreadContext;
    pBrokerContext->pLinkStateThreadContext=pLinkStateThreadContext;
    pBrokerContext->pHostUserProxyThreadContext=pHostUserProxyThreadContext;
	pBrokerContext->pLinkReceiveThreadContext = pLinkReceiveThreadContext;
}










