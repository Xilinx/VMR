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
 *  $Revision: #39 $
 *
 */




#ifndef _CMC_DATA_STORE_H_
#define _CMC_DATA_STORE_H_


#ifdef __cplusplus
extern "C"
{
#endif

#include "cmc_common.h"
#include "cmc_peripherals.h"
#include "cmc_data_store_constants.h"
#include "cmc_data_store_board_info.h"
#include "cmc_data_store_sensor_identification.h"
#include "cmc_data_store_sensor_classification.h"

struct BROKER_CONTEXT_TYPE;
extern void Broker_RequestLinkUserIsSensorSupervisor(struct BROKER_CONTEXT_TYPE* pBrokerContext);

#define CMC_MAX_NUM_QSFP_CAGES  4


#define MAX_SAMPLES_PER_ELEMENT 5

#define SENSOR_IDS_3 96
#define SENSOR_IDS_2 64
#define SENSOR_IDS_1 32
#define SENSOR_IDS_0 0


typedef struct CMC_SENSOR_AVERAGE_ELEMENT_TYPE
{
	uint32_t    Sample[MAX_SAMPLES_PER_ELEMENT];

} CMC_SENSOR_AVERAGE_ELEMENT_TYPE;

typedef struct DATA_STORE_CAGE_INFO_TYPE
{
	bool		gpioPending;
	uint32_t	baseAddr;
} DATA_STORE_CAGE_INFO_TYPE;

typedef struct DATA_STORE_SENSOR_INFO_TYPE
{
	uint32_t						SensorValue[CMC_MAX_NUM_SNSRS];
	float							fSensorAverage[CMC_MAX_NUM_SNSRS];
	uint32_t						ProcessSensorCount;
	bool							IsValid;
	
} DATA_STORE_SENSOR_INFO_TYPE;


typedef struct CMC_DATA_STORE_CONTEXT_TYPE
{
    PERIPHERAL_REGMAP_RAM_CONTROLLER_CONTEXT_TYPE * pHardwareRegisterSetContext;
    CMC_FIRMWARE_VERSION_CONTEXT_TYPE *             pFirmwareVersionContext;
    CMC_WATCHPOINT_CONTEXT_TYPE *                   pWatchPointContext;
    struct BROKER_CONTEXT_TYPE *                    pBrokerContext;
    DATA_STORE_BOARD_INFO_TYPE                      BoardInfo;

    DATA_STORE_SENSOR_INFO_TYPE                     SensorInfo;
    DATA_STORE_CAGE_INFO_TYPE                       CageInfo;
    uint32_t                                        OEMID;
    bool                                            OEMID_Available;
    bool                                            bSensorsGatedByPowerGood;
    bool                                            bPowerGood;
    bool                                            bZync1Device;   // Used by U30
    uint64_t                                        CMCValidSensors_0_to_63;
    uint64_t                                        CMCValidSensors_64_to_127;
    bool                                            bCardSupportsScalingFactor;
    bool                                            bCardSupportsSCUpgrade;
    uint64_t                                        CMCHeartbeatSensors_0_to_63;
    uint64_t                                        CMCHeartbeatSensors_64_to_127;
    bool                                            bRenegotiateComms;

} CMC_DATA_STORE_CONTEXT_TYPE;


void DataStore_ResetSensors(CMC_DATA_STORE_CONTEXT_TYPE* pContext);
void DataStore_InitializeBoardInfo(DATA_STORE_BOARD_INFO_TYPE *pBoardInfo);
void DataStore_UpdateBoardInfo(CMC_DATA_STORE_CONTEXT_TYPE *pContext, uint8_t* payload, uint8_t length);
void DataStore_Initialize(CMC_DATA_STORE_CONTEXT_TYPE* pContext, PERIPHERAL_REGMAP_RAM_CONTROLLER_CONTEXT_TYPE* pHardwareRegisterSetContext, CMC_WATCHPOINT_CONTEXT_TYPE* pWatchPointContext, CMC_FIRMWARE_VERSION_CONTEXT_TYPE* pFirmwareVersionContext, bool bSensorsGatedByPowerGood, bool bZync1Device, uint64_t CMCValidSensors_0_to_63, uint64_t CMCValidSensors_64_to_127, bool bCardSupportsScalingFactor, bool bCardSupportsSCUpgrade);
void DataStore_BindBroker(CMC_DATA_STORE_CONTEXT_TYPE* pContext, struct BROKER_CONTEXT_TYPE* pBrokerContext);
void DataStore_UpdateSensors(CMC_DATA_STORE_CONTEXT_TYPE* pContext, char* payload, uint16_t length, uint8_t messageID, uint64_t CMCValidSensors_0_to_63, uint64_t CMCValidSensors_64_to_127, bool bCardSupportsScalingFactor, uint8_t CommsVersion);
void DataStore_UpdateAlertResponse(CMC_DATA_STORE_CONTEXT_TYPE* pContext, char* payload, uint16_t length);
void DataStore_UpdateSensorState(CMC_DATA_STORE_CONTEXT_TYPE* pContext, char* payload, uint16_t length);
void DataStore_UpdateOEM(CMC_DATA_STORE_CONTEXT_TYPE* pContext, char* payload, uint16_t length);
void DataStore_Update_Interrupt_Status(CMC_DATA_STORE_CONTEXT_TYPE* pContext, char* payload, uint16_t length);
void DataStore_UpdatePowerThresholdResponse(CMC_DATA_STORE_CONTEXT_TYPE* pContext, char* payload, uint16_t length, uint8_t CommsVersion);
void DataStore_UpdateTempThresholdResponse(CMC_DATA_STORE_CONTEXT_TYPE* pContext, char* payload, uint16_t length);

void cmcIncrementReg(PERIPHERAL_REGMAP_RAM_CONTROLLER_CONTEXT_TYPE* pHardwareRegisterSetContext, uint32_t regAddr);

void DataStoreWriteMACAddressToRegister(CMC_DATA_STORE_CONTEXT_TYPE* pContext, uint64_t input, uint32_t HostRegister);
uint64_t cmcU8ToU64(uint8_t* payload);
void cmcU8ToMacString(uint8_t* payload, char* macString);
void cmcMemClear(uint8_t* ptr, size_t length);

uint8_t cmcStrLength(const char* str);
void cmcStrCopy(char* destination, const char* source);
void cmcCopyBoardInfo(CMC_DATA_STORE_CONTEXT_TYPE* pContext, uint8_t* buffer);

#ifdef __cplusplus
}
#endif


#endif /* _CMC_DATA_STORE_H_ */





