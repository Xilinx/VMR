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
 *  $Revision: #40 $
 *
 */




#include "cmc_thread_linkstate.h"
#include "cmc_transmit_defines.h"
#include "cmc_protocol_sensor.h"
#include "cmc_protocol_bootloader.h"
#include "cmc_transport_sensor.h"


void CLEAR_ACTION(LINK_STATE_CONTEXT_TYPE *pContext)
{
    pContext->Action=0;    
}


void ACTION(LINK_STATE_CONTEXT_TYPE *pContext, uint32_t AnyAction)
{
    pContext->Action|=AnyAction;
}



void LinkState_HandleAction_A_LS_T_RESPONSE_START(LINK_STATE_CONTEXT_TYPE *pContext)
{
    pContext->CurrentTimerInstance=TimerThread_Start(   pContext->pThreadTimerContext,
                                                        T_LINK_STATE_TIMER,
                                                        LINK_STATE_TIMER_DURATION_MS,
                                                        pContext,
                                                        LinkState_TimerCallback);
}

void LinkState_HandleAction_A_LS_T_BSL_START(LINK_STATE_CONTEXT_TYPE* pContext)
{
	pContext->CurrentTimerInstance = TimerThread_Start(pContext->pThreadTimerContext,
		T_BSL_LOAD_TIMER,
		LINK_STATE_BSL_DURATION_MS,
		pContext,
		LinkState_BSLTimerCallback);
}

void LinkState_HandleAction_A_LS_T_RESPONSE_STOP(LINK_STATE_CONTEXT_TYPE *pContext)
{
    TimerThread_Stop(pContext->pThreadTimerContext, T_LINK_STATE_TIMER);
}


void LinkState_HandleAction_A_LS_T_RESPONSE_RESTART(LINK_STATE_CONTEXT_TYPE *pContext)
{
    pContext->CurrentTimerInstance=TimerThread_Restart(pContext->pThreadTimerContext, T_LINK_STATE_TIMER);
}


void LinkState_HandleAction_A_LS_SEND_LINK_OFFERED_PROTOCOL_VERSION(LINK_STATE_CONTEXT_TYPE *pContext)
{
	/* Send a Version Request Message */
    uint8_t payload = pContext->pVersionSupportContext->CMCOfferedInterfaceVersion;
	uint8_t payloadLength = 1;
	PROTOCOL_SENSOR_FormatSETMessage(pContext->pProtocol, (uint8_t*)pContext->TxMsgBuffer, (uint8_t*)&(pContext->TxMsgBufferSize), SAT_COMMS_SET_VERS, &payload, payloadLength);
	TRANSPORT_SENSOR_FrameMessage(pContext->pTransport, (uint8_t*)pContext->TxMsgBuffer, (uint8_t*)&(pContext->TxMsgBufferSize), (uint8_t*)pContext->TxFramedMsgBuffer, (uint8_t*) & (pContext->TxFramedMsgBufferSize));
	TRANSPORT_SENSOR_SendByteSequenceBlocking(pContext->pTransport, pContext->TxFramedMsgBuffer, pContext->TxFramedMsgBufferSize);
	/* Increment the message coverage counter */
	cmcIncrementReg(pContext->pBrokerContext->pLocalSensorSupervisorThreadContext->DataStoreContext.pHardwareRegisterSetContext, CMC_SC_MSG_COVERAGE_REG + (SAT_COMMS_SET_VERS << 2));

}

     
void LinkState_HandleAction_A_LS_SEND_LINK_USER_QUERY(LINK_STATE_CONTEXT_TYPE *pContext)
{
    PROTOCOL_BOOTLOADER_FormatMessage(pContext->pProtocolBootloader, SAT_COMMS_BSL_SYNC, pContext->TxMsgBuffer, &(pContext->TxMsgBufferSize), 0, NULL, 0);

	TRANSPORT_SENSOR_SendByteSequenceBlocking(pContext->pTransport, pContext->TxMsgBuffer, pContext->TxMsgBufferSize);
	
	/* Increment the message coverage counter */
	cmcIncrementReg(pContext->pBrokerContext->pLocalSensorSupervisorThreadContext->DataStoreContext.pHardwareRegisterSetContext, CMC_SC_MSG_COVERAGE_REG + (SAT_COMMS_BSL_SYNC << 2));

	pContext->pBrokerContext->pBootloaderThreadContext->lastCommandSent = BSL_SYNC_ID;
}

static void LinkState_SendMessage(LINK_STATE_CONTEXT_TYPE* pContext, uint8_t MsgID)
{
	PROTOCOL_SENSOR_FormatREQMessage(pContext->pProtocol, (uint8_t*)pContext->TxMsgBuffer, (uint8_t*) & (pContext->TxMsgBufferSize), MsgID);
	TRANSPORT_SENSOR_FrameMessage(pContext->pTransport, (uint8_t*)pContext->TxMsgBuffer, (uint8_t*) & (pContext->TxMsgBufferSize), (uint8_t*)pContext->TxFramedMsgBuffer, (uint8_t*) & (pContext->TxFramedMsgBufferSize));
	TRANSPORT_SENSOR_SendByteSequenceBlocking(pContext->pTransport, pContext->TxFramedMsgBuffer, pContext->TxFramedMsgBufferSize);

	/* Increment the message coverage counter */
	cmcIncrementReg(pContext->pBrokerContext->pLocalSensorSupervisorThreadContext->DataStoreContext.pHardwareRegisterSetContext, CMC_SC_MSG_COVERAGE_REG + (MsgID << 2));
}


void LinkState_HandleAction_A_LS_SEND_LINK_USER_TO_BOOTLOADER_COMMAND(LINK_STATE_CONTEXT_TYPE *pContext)
{
	LinkState_SendMessage(pContext, SAT_COMMS_EN_BSL);
}


void LinkState_HandleAction_A_LS_SEND_LINK_ACCEPTED_PROTOCOL_VERSION_RANGE_QUERY(LINK_STATE_CONTEXT_TYPE* pContext)
{
	LinkState_SendMessage(pContext, SAT_COMMS_VERS_REQ);
}


void LinkState_HandleAction_A_LS_CACHE_LINK_ACCEPTED_PROTOCOL_VERSION_RANGE(LINK_STATE_CONTEXT_TYPE *pContext)
{
	UNUSED(pContext);
	/* Record the negotiated protocol versions */
	/* Are they needed within link state or are the better saved elsewhere ? */

}

#define SAT_COMMS_FPGA_I2C_BUS_SAT_MASTER   1

void LinkState_HandleAction_A_LS_SEND_LINK_FPGA_I2C_BUS_ARB(LINK_STATE_CONTEXT_TYPE* pContext)
{
    uint8_t payload = SAT_COMMS_FPGA_I2C_BUS_SAT_MASTER;
    uint8_t payloadLength = 1;
    
    if (CMC_VersionSupport_CheckMessageSupport(pContext->pVersionSupportContext, SAT_COMMS_FPGA_I2C_BUS_ARB))
    {
		PROTOCOL_SENSOR_FormatSETMessage(pContext->pProtocol, (uint8_t*)pContext->TxMsgBuffer, (uint8_t*)&(pContext->TxMsgBufferSize), SAT_COMMS_FPGA_I2C_BUS_ARB, &payload, payloadLength);
		TRANSPORT_SENSOR_FrameMessage(pContext->pTransport, (uint8_t*)pContext->TxMsgBuffer, (uint8_t*)&(pContext->TxMsgBufferSize), (uint8_t*)pContext->TxFramedMsgBuffer, (uint8_t*) & (pContext->TxFramedMsgBufferSize));
		TRANSPORT_SENSOR_SendByteSequenceBlocking(pContext->pTransport, pContext->TxFramedMsgBuffer, pContext->TxFramedMsgBufferSize);
        /* Increment the message coverage counter */
        cmcIncrementReg(pContext->pBrokerContext->pLocalSensorSupervisorThreadContext->DataStoreContext.pHardwareRegisterSetContext, CMC_SC_MSG_COVERAGE_REG + (SAT_COMMS_FPGA_I2C_BUS_ARB << 2));
   }
    else
    {
        Watch_Inc(pContext->pWatchPointContext, W_SENSOR_SUPERVISOR_MESSAGE_VERSION_UNSUPPORTED);

        // If it's not supported just assume success
        LinkState_CreateEvent_E_LS_LINK_I2C_ACKNOWLEDGE(pContext);
    }

}



void LinkState_HandleAction_A_LS_ANNOUNCE_LINK_USER_IS_SENSOR_SUPERVISOR(LINK_STATE_CONTEXT_TYPE* pContext)
{
    if (NULL != (pContext->pBrokerContext))
    {
        Broker_AnnounceLinkUserIsSensorSupervisor(pContext->pBrokerContext);

        /* Make sure board info gets updated if we've finished downloading a new SC load */
        pContext->pBrokerContext->pLocalSensorSupervisorThreadContext->DataStoreContext.BoardInfo.IsValid = false;
        pContext->pBrokerContext->pLocalSensorSupervisorThreadContext->DataStoreContext.OEMID_Available = false;
        pContext->pBrokerContext->pLocalSensorSupervisorThreadContext->UartEnabled = false;

       // PERIPHERAL_REGMAP_RAM_CONTROLLER_WriteWithMask(pContext->pBrokerContext->pLocalSensorSupervisorThreadContext->DataStoreContext.pHardwareRegisterSetContext, HOST_REGISTER_STATUS, (uint32_t)(HOST_STATUS_REGISTER_SC_MODE_NORMAL_MODE << HOST_REGISTER_PATTERN_SAT_MODE_OFFSET), (uint32_t)HOST_REGISTER_PATTERN_SAT_MODE_MASK);
    }
}

void LinkState_HandleAction_A_LS_ANNOUNCE_LINK_USER_IS_SENSOR_SUPERVISOR_SUC(LINK_STATE_CONTEXT_TYPE* pContext)
{
    if (NULL != (pContext->pBrokerContext))
    {
        Broker_AnnounceLinkUserIsSensorSupervisorSUC(pContext->pBrokerContext);

        /* Make sure board info gets updated if we've finished downloading a new SUC load */
        pContext->pBrokerContext->pLocalSensorSupervisorThreadContext->DataStoreContext.BoardInfo.IsValid = false;
        pContext->pBrokerContext->pLocalSensorSupervisorThreadContext->DataStoreContext.OEMID_Available = false;
        pContext->pBrokerContext->pLocalSensorSupervisorThreadContext->UartEnabled = false;

    }
}

void LinkState_HandleAction_A_LS_ANNOUNCE_LINK_USER_IS_SENSOR_SUPERVISOR_LINK_OPERATING(LINK_STATE_CONTEXT_TYPE* pContext)
{
	if (NULL != (pContext->pBrokerContext))
	{
		Broker_AnnounceLinkUserIsSensorSupervisorLinkOperating(pContext->pBrokerContext);
        //PERIPHERAL_REGMAP_RAM_CONTROLLER_WriteWithMask(pContext->pBrokerContext->pLocalSensorSupervisorThreadContext->DataStoreContext.pHardwareRegisterSetContext, HOST_REGISTER_STATUS, (uint32_t)(HOST_STATUS_REGISTER_SC_MODE_NORMAL_MODE<< HOST_REGISTER_PATTERN_SAT_MODE_OFFSET), (uint32_t)HOST_REGISTER_PATTERN_SAT_MODE_MASK);
	}
}



void LinkState_HandleAction_A_LS_ANNOUNCE_LINK_USER_IS_BOOTLOADER(LINK_STATE_CONTEXT_TYPE *pContext)
{
    if(NULL!=(pContext->pBrokerContext))
    {
        Broker_AnnounceLinkUserIsBootloader(pContext->pBrokerContext);


        if (pContext->pFirmwareVersionContext->Version.Minor == 0x4) // Is this a U30
        {
            if (pContext->bZync1Device) //Zync1
            {
                // TODO make this change when XRT are ready to support
                PERIPHERAL_REGMAP_RAM_CONTROLLER_WriteWithMask(pContext->pBrokerContext->pLocalSensorSupervisorThreadContext->DataStoreContext.pHardwareRegisterSetContext, HOST_REGISTER_STATUS, (uint32_t)(HOST_STATUS_REGISTER_SC_MODE_BSL_MODE_SYNCED_SC_NOT_UPGRADABLE << HOST_REGISTER_PATTERN_SAT_MODE_OFFSET), (uint32_t)HOST_REGISTER_PATTERN_SAT_MODE_MASK);
                //PERIPHERAL_REGMAP_RAM_CONTROLLER_WriteWithMask(pContext->pBrokerContext->pLocalSensorSupervisorThreadContext->DataStoreContext.pHardwareRegisterSetContext, HOST_REGISTER_STATUS, (uint32_t)(HOST_STATUS_REGISTER_SC_MODE_BSL_MODE_SYNCED << HOST_REGISTER_PATTERN_SAT_MODE_OFFSET), (uint32_t)HOST_REGISTER_PATTERN_SAT_MODE_MASK);
            }
            else //Zync2
            {
                PERIPHERAL_REGMAP_RAM_CONTROLLER_WriteWithMask(pContext->pBrokerContext->pLocalSensorSupervisorThreadContext->DataStoreContext.pHardwareRegisterSetContext, HOST_REGISTER_STATUS, (uint32_t)(HOST_STATUS_REGISTER_SC_MODE_BSL_MODE_SYNCED << HOST_REGISTER_PATTERN_SAT_MODE_OFFSET), (uint32_t)HOST_REGISTER_PATTERN_SAT_MODE_MASK);
            }
        }
        else
        {
            PERIPHERAL_REGMAP_RAM_CONTROLLER_WriteWithMask(pContext->pBrokerContext->pLocalSensorSupervisorThreadContext->DataStoreContext.pHardwareRegisterSetContext, HOST_REGISTER_STATUS, (uint32_t)(HOST_STATUS_REGISTER_SC_MODE_BSL_MODE_SYNCED << HOST_REGISTER_PATTERN_SAT_MODE_OFFSET), (uint32_t)HOST_REGISTER_PATTERN_SAT_MODE_MASK);
        }
    }
}

void LinkState_HandleAction_A_LS_ANNOUNCE_COMMAND_FAILED(LINK_STATE_CONTEXT_TYPE* pContext)
{
    Broker_Announce_UserProxyRemoteBootloaderOperationFailed(pContext->pBrokerContext);
}

void LinkState_HandleAction(LINK_STATE_CONTEXT_TYPE *pContext)
{
    uint32_t i;

    for(i=0;i<MAX_ACTIONS; i++)
    {
        switch((1<<i)&(pContext->Action))
        {
            case    A_LS_NO_ACTION:
                    /* Ignore */
                    break;

            case    A_LS_T_RESPONSE_START:
                    LinkState_HandleAction_A_LS_T_RESPONSE_START(pContext);
                    break;
                               
            case    A_LS_T_RESPONSE_STOP: 
                    LinkState_HandleAction_A_LS_T_RESPONSE_STOP(pContext);
                    break;
                                                         
            case    A_LS_T_RESPONSE_RESTART:
                    LinkState_HandleAction_A_LS_T_RESPONSE_RESTART(pContext);
                    break;
                                                       
            case    A_LS_SEND_LINK_OFFERED_PROTOCOL_VERSION:
                    LinkState_HandleAction_A_LS_SEND_LINK_OFFERED_PROTOCOL_VERSION(pContext);
                    break;
                                       
            case    A_LS_SEND_LINK_USER_QUERY:      
                    LinkState_HandleAction_A_LS_SEND_LINK_USER_QUERY(pContext);
                    break;
                                               
            case    A_LS_SEND_LINK_USER_TO_BOOTLOADER_COMMAND:  
                    LinkState_HandleAction_A_LS_SEND_LINK_USER_TO_BOOTLOADER_COMMAND(pContext);
                    break;
                                   
            case    A_LS_SEND_LINK_ACCEPTED_PROTOCOL_VERSION_RANGE_QUERY:
                    LinkState_HandleAction_A_LS_SEND_LINK_ACCEPTED_PROTOCOL_VERSION_RANGE_QUERY(pContext);
                    break;
                          
            case    A_LS_CACHE_LINK_ACCEPTED_PROTOCOL_VERSION_RANGE:
                    LinkState_HandleAction_A_LS_CACHE_LINK_ACCEPTED_PROTOCOL_VERSION_RANGE(pContext);
                    break;
           
            case    A_LS_ANNOUNCE_LINK_USER_IS_SENSOR_SUPERVISOR:
                    LinkState_HandleAction_A_LS_ANNOUNCE_LINK_USER_IS_SENSOR_SUPERVISOR(pContext);
                    break;

            case    A_LS_ANNOUNCE_LINK_USER_IS_SENSOR_SUPERVISOR_SUC:
                    LinkState_HandleAction_A_LS_ANNOUNCE_LINK_USER_IS_SENSOR_SUPERVISOR_SUC(pContext);
                    break;
 
            case    A_LS_ANNOUNCE_LINK_USER_IS_BOOTLOADER:
                    LinkState_HandleAction_A_LS_ANNOUNCE_LINK_USER_IS_BOOTLOADER(pContext);
                    break;

			case    A_LS_ANNOUNCE_LINK_USER_IS_SENSOR_SUPERVISOR_LINK_OPERATING:
					LinkState_HandleAction_A_LS_ANNOUNCE_LINK_USER_IS_SENSOR_SUPERVISOR_LINK_OPERATING(pContext);
					break;

			case    A_LS_T_BSL_START:
					LinkState_HandleAction_A_LS_T_BSL_START(pContext);
					break;		

            case    A_LS_SEND_LINK_FPGA_I2C_BUS_ARB:
                    LinkState_HandleAction_A_LS_SEND_LINK_FPGA_I2C_BUS_ARB(pContext);
                    break;

            case    A_LS_ANNOUNCE_COMMAND_FAILED:
                    LinkState_HandleAction_A_LS_ANNOUNCE_COMMAND_FAILED(pContext);
                    break;

            default:
                    Watch_Inc(pContext->pWatchPointContext, W_LINK_STATE_UNEXPECTED_ACTION);
                    break;
        }
    }
        

}



