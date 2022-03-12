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
 *  $Change: 3163114 $
 *  $Date: 2021/03/29 $
 *  $Revision: #63 $
 *
 */





#ifndef _CMC_LOCAL_SENSOR_SUPERVISOR_THREAD_H_
#define _CMC_LOCAL_SENSOR_SUPERVISOR_THREAD_H_


#ifdef __cplusplus
extern "C"
{
#endif



#include "cmc_common.h"
#include "cmc_watchpoint.h"
#include "cmc_circular_buffer.h"
#include "cmc_thread_timer.h"
#include "cmc_fsm_state_transition_logger.h"
#include "cmc_peripheral_uart_lite_usb.h"
#include "cmc_peripheral_axi_uart_lite_satellite.h"
#include "cmc_peripheral_axi_gpio_qsfp.h"
#include "cmc_local_sensor_supervisor_thread_constants.h"
#include "cmc_local_sensor_supervisor_state.h"
#include "cmc_local_sensor_supervisor_event.h"
#include "cmc_local_sensor_supervisor_action.h"
#include "cmc_thread_communications_broker.h"
#include "cmc_local_bootloader_thread.h"
#include "cmc_thread_linkstate.h"
#include "cmc_host_user_proxy_thread.h"
#include "cmc_link_receive_thread.h"

#include "cmc_data_store.h"
#include "cmc_sensor_relay_buffer.h"
#include "cmc_version_support.h"
#include "cmc_feature_control.h"
#include "cmc_firmware_version.h"
#include "cmc_link_protocol_trace.h"
#include "cmc_entry_point_build_profile.h"
struct BROKER_CONTEXT_TYPE;

typedef enum CSDR_STATE_TYPE
{
    IDLE,
    READY_TO_SEND,
    WAITING_ON_RESPONSE,
    RESPONSE_RECEIVED

} CSDR_STATE_TYPE;


typedef enum SENSOR_SUPERVISOR_EXPECTED_MSG_TYPE
{
    SS_EXPECTED_MSG_NONE,
    SS_EXPECTED_MSG_ALERT_RESP,
    SS_EXPECTED_MSG_BOARD_INFO_RESP,
    SS_EXPECTED_MSG_VOLT_SNSR_RESP,
    SS_EXPECTED_MSG_POWER_SNSR_RESP,
    SS_EXPECTED_MSG_TEMP_SNSR_RESP,
    SS_EXPECTED_MSG_SNSR_STATE_RESP,
    SS_EXPECTED_MSG_SEND_OEM_CMD_RESP,
    SS_EXPECTED_MSG_POWER_THROTTLING_THRESHOLDS_RESP,
    SS_EXPECTED_MSG_TEMP_THROTTLING_THRESHOLDS_RESP,
    SS_EXPECTED_MSG_UART_EN_RESP,
    SS_EXPECTED_MSG_CSDR_RESP,
    SS_EXPECTED_MSG_READ_QSFP_DIAGNOSTICS_RESP,
    SS_EXPECTED_MSG_QSFP_VALIDATE_LOW_SPEED_READ_IO_RESP,
    SS_EXPECTED_MSG_INTERRUPT_STATUS_RESP,
    SS_EXPECTED_MSG_QSFP_READ_SINGLE_BYTE_RESP,
    SS_EXPECTED_MSG_GOOD,
    SS_EXPECTED_MSG_MAX

} SENSOR_SUPERVISOR_EXPECTED_MSG_TYPE;

typedef struct LOCAL_SENSOR_SUPERVISOR_THREAD_CONTEXT_TYPE
{
    CIRCULAR_BUFFER_TYPE                            EventSourceCircularBuffer;
    CIRCULAR_BUFFER_ELEMENT_TYPE                    CircularBuffer[MAX_SENSOR_SUPERVISOR_MESSAGES];
    CMC_WATCHPOINT_CONTEXT_TYPE *                   pWatchPointContext;
    struct BROKER_CONTEXT_TYPE *                    pBrokerContext;
    FSM_STATE_TRANSITION_LOGGER_CONTEXT_TYPE *      pFSM_StateTransitionLoggerContext;
    PERIPHERAL_AXI_UART_LITE_SATELLITE_CONTEXT_TYPE * pAXI_UART_LITE_Satellite_Context;
    THREAD_TIMER_CONTEXT_TYPE *                     pThreadTimerContext;
    PERIPHERAL_AXI_GPIO_QSFP_CONTEXT_TYPE *         pGPIO_QSFP_Context;
    uint32_t                                        CurrentTimerInstance;
    CMC_SENSOR_RELAY_BUFFER_CONTEXT_TYPE *          pSensorSupervisorRelayBufferContext;
	CMC_VERSION_SUPPORT_CONTEXT_TYPE *				pVersionSupportContext;
	CMC_FEATURE_CONTROL_CONTEXT_TYPE *				pFeatureControlContext;
	LINK_PROTOCOL_TRACE_CONTEXT_TYPE *				LinkProtocolTraceContext;
	CMC_BUILD_PROFILE_PROTOCOL_SENSOR_TYPE *		pProtocol;
	CMC_BUILD_PROFILE_TRANSPORT_SENSOR_TYPE *		pTransport;

    uint8_t                                         Event;
    uint32_t                                        Action;
    SENSOR_SUPERVISOR_STATE_TYPE                    State;
    SENSOR_SUPERVISOR_STATE_TYPE                    PreviousState;

    CMC_DATA_STORE_CONTEXT_TYPE                     DataStoreContext;
	uint8_t							                TxFramedMsgBuffer[MAX_SENSOR_MESSAGE_SIZE];
	uint8_t											TxFramedMsgBufferSize;
	uint8_t							                TxFormattedMsgBuffer[MAX_SENSOR_MESSAGE_SIZE];
	uint8_t											TxFormattedMsgBufferSize;
	uint32_t										MsgID;
	uint32_t										Sensor;
	uint32_t										Value;
	bool											HBMSupported;
	uint8_t											iQSFP;
	CMC_SENSOR_RELAY_ELEMENT_TYPE					LogElement;
    SENSOR_SUPERVISOR_EXPECTED_MSG_TYPE             ExpectedMessage;
    bool                                            UartEnabled;
    bool                                            EnableUartRequest;
    uint8_t                                         EnableUartCount;
    bool											PCIeECCSupported;
    USER_SUPPLIED_ENVIRONMENT_ECC_READ_TYPE         pFN_ECC_Read;
    USER_SUPPLIED_ENVIRONMENT_PCIE_READ_TYPE        pFN_PCIe_Read;
    bool                                            bKeepAliveSupported;
    USER_SUPPLIED_ENVIRONMENT_SHELLVERSION_TYPE	    pFN_ShellVersion_Read;
    USER_SUPPLIED_ENVIRONMENT_KEEPALIVE_TYPE	    pFN_KeepAlive_Read;
    USER_SUPPLIED_ENVIRONMENT_UUID_TYPE	            pFN_UUID_Read;
    void*                                           pUserContext;
    uint32_t                                        SensorSupervisorTimeout;
    uint16_t                                        CounterPCIeECC;
    bool                                            bCollectCSDR;
    uint8_t                                         CSDRInitialRecordNumber;
    CSDR_STATE_TYPE                                 WaitingOnCSDRResponse;
    bool                                            bQSFPReadSingleByte;
    bool                                            bQSFPWriteSingleByte;
    bool                                            bReadQSFPDiagnostics;
    bool                                            bReadQSFPValidateLowSpeedIO;
    bool                                            bWriteQSFPValidateLowSpeedIO;
    uint8_t                                         CSDRResponseWaitCount;
    bool                                            bQSFPRequestSent;
    uint16_t                                        QSFPResponseWaitCount;
    uint16_t                                        CounterKeepAlive;
    bool                                            bCardSupportsSCUpgrade;
    USER_SUPPLIED_ENVIRONMENT_GPIO_READ_TYPE        pFN_GPIO_Read;
    uint8_t                                         GPIO_read_previous;
    USER_SUPPLIED_ENVIRONMENT_STACK_CHECK_TYPE      pFN_Stack_Check;
    bool                                            bCMC_Heartbeat;
    bool                                            bFirstWrite;
    uint8_t                                         UUID[16];
    bool                                            bQSFPReadWriteGPIO;
    uint8_t                                         RenegotiateCommsCounter;
    //USER_SUPPLIED_ENVIRONMENT_PERIPHERAL_READ_TYPE  pFN_PeripheralRead;
    USER_SUPPLIED_ENVIRONMENT_QSFP_GPIO_READ_TYPE   pFN_QSFP_GPIO_Read;
    USER_SUPPLIED_ENVIRONMENT_QSFP_GPIO_WRITE_TYPE  pFN_QSFP_GPIO_Write;

} LOCAL_SENSOR_SUPERVISOR_THREAD_CONTEXT_TYPE;



void LocalSensorSupervisorThread_BindBroker( LOCAL_SENSOR_SUPERVISOR_THREAD_CONTEXT_TYPE * pContext, struct BROKER_CONTEXT_TYPE *    pBrokerContext);

bool LocalSensorSupervisorThread_Schedule(void *pContext);

void LocalSensorSupervisorThread_Initialize(    LOCAL_SENSOR_SUPERVISOR_THREAD_CONTEXT_TYPE *       pContext,
                                                THREAD_TIMER_CONTEXT_TYPE *                         pThreadTimerContext,
                                                CMC_WATCHPOINT_CONTEXT_TYPE *                       pWatchPointContext, 
                                                PERIPHERAL_REGMAP_RAM_CONTROLLER_CONTEXT_TYPE *     pHardwareRegisterSetContext,
                                                FSM_STATE_TRANSITION_LOGGER_CONTEXT_TYPE *          pFSM_StateTransitionLoggerContext,
                                                PERIPHERAL_AXI_UART_LITE_SATELLITE_CONTEXT_TYPE *   pAXI_UART_LITE_Satellite_Context,
                                                PERIPHERAL_AXI_GPIO_QSFP_CONTEXT_TYPE *             pGPIO_QSFP_Context,
                                                CMC_SENSOR_RELAY_BUFFER_CONTEXT_TYPE *              pSensorSupervisorRelayBufferContext,
												CMC_VERSION_SUPPORT_CONTEXT_TYPE *					pVersionSupportContext,
												CMC_FEATURE_CONTROL_CONTEXT_TYPE *					pFeatureControlContext,
												LINK_PROTOCOL_TRACE_CONTEXT_TYPE *					LinkProtocolTraceContext,
												CMC_BUILD_PROFILE_TYPE *							pProfile,
                                                CMC_FIRMWARE_VERSION_CONTEXT_TYPE *                 pFirmwareVersionContext);


void SensorSupervisor_FSM(LOCAL_SENSOR_SUPERVISOR_THREAD_CONTEXT_TYPE *pContext, uint8_t AnyEvent);


void LocalSensorSupervisor_SensorTimerCallback(void * pCallbackContext, uint32_t AnyInstance);
void LocalSensorSupervisor_SensorUartTimerCallback(void* pCallbackContext, uint32_t AnyInstance);

void SensorSupervisor_CLEAR_ACTION(LOCAL_SENSOR_SUPERVISOR_THREAD_CONTEXT_TYPE *pContext);

void SensorSupervisor_ACTION(LOCAL_SENSOR_SUPERVISOR_THREAD_CONTEXT_TYPE *pContext, uint32_t AnyAction);

void SensorSupervisor_HandleAction(LOCAL_SENSOR_SUPERVISOR_THREAD_CONTEXT_TYPE *pContext);


void SensorSupervisor_SeedAction(LOCAL_SENSOR_SUPERVISOR_THREAD_CONTEXT_TYPE *pContext);
void SensorSupervisor_CreateEvent_E_SS_T_SENSOR_EXPIRY(LOCAL_SENSOR_SUPERVISOR_THREAD_CONTEXT_TYPE *pContext);
void SensorSupervisor_CreateEvent_E_SS_LINK_IS_AVAILABLE(LOCAL_SENSOR_SUPERVISOR_THREAD_CONTEXT_TYPE *pContext);
void SensorSupervisor_CreateEvent_E_SS_LINK_IS_UNAVAILABLE(LOCAL_SENSOR_SUPERVISOR_THREAD_CONTEXT_TYPE *pContext);
void SensorSupervisor_CreateEvent_E_SS_CAGE_INFORMATION_ARRIVAL(LOCAL_SENSOR_SUPERVISOR_THREAD_CONTEXT_TYPE* pContext);
void SensorSupervisor_CreateEvent_E_SS_GPIO_INTERRUPT_ARRIVAL(LOCAL_SENSOR_SUPERVISOR_THREAD_CONTEXT_TYPE* pContext);
void SensorSupervisor_CreateEvent_E_SS_MESSAGE_ARRIVAL(LOCAL_SENSOR_SUPERVISOR_THREAD_CONTEXT_TYPE* pContext);
void SensorSupervisor_CreateEvent_E_SS_T_ENABLE_UART_NOT_REQUIRED(LOCAL_SENSOR_SUPERVISOR_THREAD_CONTEXT_TYPE* pContext);
void SensorSupervisor_CreateEvent_E_SS_DEBUG_UART_RESPONSE_ARRIVAL(LOCAL_SENSOR_SUPERVISOR_THREAD_CONTEXT_TYPE* pContext);

#ifdef __cplusplus
}
#endif


#endif /* _CMC_LOCAL_SENSOR_SUPERVISOR_THREAD_H_ */







