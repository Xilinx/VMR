/*----------------------------------------------------------------------------------------------*/
/*                                                                                              */
/* � Copyright 2019 Xilinx, Inc. All rights reserved.                                           */
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
 *  $Change: 3163114 $
 *  $Date: 2021/03/29 $
 *  $Revision: #48 $
 *
 */




#include "cmc_local_sensor_supervisor_thread.h"






void LocalSensorSupervisorThread_BindBroker( LOCAL_SENSOR_SUPERVISOR_THREAD_CONTEXT_TYPE * pContext, struct BROKER_CONTEXT_TYPE *    pBrokerContext)
{
    pContext->pBrokerContext=pBrokerContext;

    SensorSupervisor_SeedAction(pContext); /* FSM requires timer to be running in initial state */
}




void LocalSensorSupervisorThread_Initialize(    LOCAL_SENSOR_SUPERVISOR_THREAD_CONTEXT_TYPE *           pContext,
                                                THREAD_TIMER_CONTEXT_TYPE *                             pThreadTimerContext,
                                                CMC_WATCHPOINT_CONTEXT_TYPE *                           pWatchPointContext, 
                                                PERIPHERAL_REGMAP_RAM_CONTROLLER_CONTEXT_TYPE *         pHardwareRegisterSetContext,
                                                FSM_STATE_TRANSITION_LOGGER_CONTEXT_TYPE *              pFSM_StateTransitionLoggerContext,
                                                PERIPHERAL_AXI_UART_LITE_SATELLITE_CONTEXT_TYPE *       pAXI_UART_LITE_Satellite_Context,
                                                PERIPHERAL_AXI_GPIO_QSFP_CONTEXT_TYPE *                 pGPIO_QSFP_Context,
                                                CMC_SENSOR_RELAY_BUFFER_CONTEXT_TYPE *                  pSensorSupervisorRelayBufferContext,
												CMC_VERSION_SUPPORT_CONTEXT_TYPE *						pVersionSupportContext,
												CMC_FEATURE_CONTROL_CONTEXT_TYPE *						pFeatureControlContext,
												LINK_PROTOCOL_TRACE_CONTEXT_TYPE *						pLinkProtocolTraceContext,
												CMC_BUILD_PROFILE_TYPE *								pProfile,
												CMC_FIRMWARE_VERSION_CONTEXT_TYPE *                     pFirmwareVersionContext)
{
	uint8_t i;

    pContext->Event=0;
    pContext->CurrentTimerInstance=0;

    pContext->pThreadTimerContext=pThreadTimerContext;
    pContext->pWatchPointContext=pWatchPointContext;
    pContext->pFSM_StateTransitionLoggerContext=pFSM_StateTransitionLoggerContext;
    pContext->pAXI_UART_LITE_Satellite_Context=pAXI_UART_LITE_Satellite_Context;
    pContext->pGPIO_QSFP_Context=pGPIO_QSFP_Context;
    pContext->pSensorSupervisorRelayBufferContext=pSensorSupervisorRelayBufferContext;
	pContext->pVersionSupportContext = pVersionSupportContext;
	pContext->pFeatureControlContext = pFeatureControlContext;
	pContext->LinkProtocolTraceContext = pLinkProtocolTraceContext;
	pContext->pProtocol = &pProfile->Protocols.SENSOR;
	pContext->pTransport = &pProfile->Transports.SENSOR;

    pContext->pBrokerContext=NULL;

    pContext->State=S_SS_INITIAL;
    pContext->PreviousState=S_SS_INITIAL;
    pContext->HBMSupported = pProfile->UserSuppliedEnvironment.bCardSupportsHBM;
    pContext->UartEnabled = false;
    pContext->EnableUartRequest = false;
    pContext->PCIeECCSupported = pProfile->UserSuppliedEnvironment.bPCIeECCReportingSupported;
    pContext->pFN_ECC_Read = pProfile->UserSuppliedEnvironment.pFN_ECC_Read;
    pContext->pFN_PCIe_Read = pProfile->UserSuppliedEnvironment.pFN_PCIe_Read;
    pContext->bKeepAliveSupported = pProfile->UserSuppliedEnvironment.bKeepAliveSupported;
    pContext->pFN_ShellVersion_Read = pProfile->UserSuppliedEnvironment.pFN_ShellVersion_Read;
    pContext->pFN_KeepAlive_Read = pProfile->UserSuppliedEnvironment.pFN_KeepAlive_Read;
    pContext->pUserContext = pProfile->UserSuppliedEnvironment.pUserContext;
    pContext->SensorSupervisorTimeout = pProfile->UserSuppliedEnvironment.SensorSupervisorTimeout;
    pContext->CounterPCIeECC = 0;
    pContext->WaitingOnCSDRResponse = IDLE;
    pContext->bReadQSFPDiagnostics = false;
    pContext->bReadQSFPValidateLowSpeedIO = false;
    pContext->bWriteQSFPValidateLowSpeedIO = false;
    pContext->CSDRResponseWaitCount = 0;
    pContext->bQSFPRequestSent = false;
    pContext->QSFPResponseWaitCount = 0;
    pContext->CounterKeepAlive = 0;
    pContext->pFN_GPIO_Read = pProfile->UserSuppliedEnvironment.pFN_GPIO_Read;
    pContext->GPIO_read_previous = 0;
    pContext->pFN_Stack_Check = pProfile->UserSuppliedEnvironment.pFN_Stack_Check;
    pContext->bCMC_Heartbeat = false;
    pContext->bFirstWrite = true;
    pContext->pFN_UUID_Read = pProfile->UserSuppliedEnvironment.pFN_UUID_Read;
    pContext->bQSFPReadWriteGPIO = false;
    pContext->RenegotiateCommsCounter = 0;

    pContext->pFN_QSFP_GPIO_Read = pProfile->UserSuppliedEnvironment.pFN_QSFP_GPIO_Read;
    pContext->pFN_QSFP_GPIO_Write = pProfile->UserSuppliedEnvironment.pFN_QSFP_GPIO_Write;
    //pContext->pFN_PeripheralRead = pProfile->UserSuppliedEnvironment.pFN_PeripheralRead;


    for (i = 0; i < 16; i++)
    {
        pContext->UUID[i] = 0;
    }
	
    CircularBuffer_Initialize(&(pContext->EventSourceCircularBuffer), pContext->CircularBuffer, MAX_SENSOR_SUPERVISOR_MESSAGES);

    DataStore_Initialize(&(pContext->DataStoreContext), pHardwareRegisterSetContext, pWatchPointContext, pFirmwareVersionContext, pProfile->UserSuppliedEnvironment.bSensorsGatedByPowerGood, 
        pProfile->UserSuppliedEnvironment.bU30_Zync1_Device, pProfile->UserSuppliedEnvironment.CMCValidSensors_0_to_63, pProfile->UserSuppliedEnvironment.CMCValidSensors_64_to_127, pProfile->UserSuppliedEnvironment.bCardSupportsScalingFactor, pProfile->UserSuppliedEnvironment.bCardSupportsSCUpgrade);
    
}




