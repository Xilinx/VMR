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
 *  $Change: 3164421 $
 *  $Date: 2021/03/30 $
 *  $Revision: #27 $
 *
 */





#include "cmc_data_store.h"
#include "string.h"
extern CMC_FIRMWARE_VERSION_CONTEXT_TYPE                    FirmwareVersionContext;
struct BROKER_CONTEXT_TYPE;

void DataStore_InitializeBoardInfo(DATA_STORE_BOARD_INFO_TYPE *pBoardInfo)
{
    uint32_t i;

    for(i=0;i<(DATA_STORE_BOARD_NAME_SIZE+1);i++)
    {
        pBoardInfo->Name[i]='\0';
    }
    for(i=0;i<(DATA_STORE_BOARD_REVISION_SIZE+1);i++)
    {
        pBoardInfo->Revision[i]='\0';
    }
    for(i=0;i<(DATA_STORE_BOARD_SERIAL_NUMBER_SIZE+1);i++)
    {
        pBoardInfo->SerialNumber[i]='\0';
    }
    for(i=0;i<(DATA_STORE_MAC_ADDRESS_SIZE+1);i++)
    {
        pBoardInfo->MAC0[i]='\0';
        pBoardInfo->MAC1[i]='\0';
        pBoardInfo->MAC2[i]='\0';
        pBoardInfo->MAC3[i]='\0';
    }

    for(i=0;i<(DATA_STORE_SATELLITE_CONTROLLER_FW_REVISION_SIZE+1);i++)
    {
        pBoardInfo->SatelliteController_FirmwareRevision[i]='\0';
    }
    pBoardInfo->FanIsPresent=0;
    pBoardInfo->ConfigMode=0;
    pBoardInfo->AvailablePower=0;
    pBoardInfo->Length=0;
    pBoardInfo->IsValid=false;
    pBoardInfo->MAC0_hex=0;
    pBoardInfo->MAC1_hex=0;
    pBoardInfo->MAC2_hex=0;
    pBoardInfo->MAC3_hex=0;

    pBoardInfo->FirstNewMAC[0] = 0x00;
    pBoardInfo->FirstNewMAC[1] = 0x00;
    pBoardInfo->FirstNewMAC[2] = 0x00;
    pBoardInfo->FirstNewMAC[3] = 0x00;
    pBoardInfo->FirstNewMAC[4] = 0x00;
    pBoardInfo->FirstNewMAC[5] = 0x00;
    pBoardInfo->NumberOfMACs = 0;
}


void DataStore_UpdateBoardInfo(CMC_DATA_STORE_CONTEXT_TYPE* pContext, uint8_t* payload, uint8_t length)
{
	uint8_t i,j,k;

    DataStore_InitializeBoardInfo(&(pContext->BoardInfo));
	pContext->BoardInfo.IsValid = true;


	for (i = 0; i < length;)
	{

		// coverage counters (with boundary check to prevent memory corruption)
		if (payload[i] < CMC_MAX_NUM_SNSRS)
		{
			cmcIncrementReg(pContext->pHardwareRegisterSetContext, CMC_SC_SNSR_COVERAGE_REG + (payload[i] << 2));
		}

		switch (payload[i])
		{
		case SNSR_ID_BOARD_SN:
			if ((cmcStrLength((char*)(payload + i + 1)) > DATA_STORE_BOARD_SERIAL_NUMBER_SIZE ) || (cmcStrLength((char*)(payload + i + 1)) == 0))
			{
				Watch_Inc(pContext->pWatchPointContext, W_BOARDINFO_SIZE_ERROR_COUNT_REG);
				goto exit;
			}
			else
			{
				cmcStrCopy(pContext->BoardInfo.SerialNumber, (char*)(payload + i + 1));
				i += (1 + cmcStrLength((char*)(payload + i + 1)) + 1);
			}
			break;
		case SNSR_ID_MAC_ADDRESS0:
			cmcU8ToMacString(payload + i + 1, pContext->BoardInfo.MAC0);
            pContext->BoardInfo.MAC0_hex = cmcU8ToU64(payload + i + 1);
            DataStoreWriteMACAddressToRegister(pContext, pContext->BoardInfo.MAC0_hex, HOST_MAC0);
			i += 7;
			break;
		case SNSR_ID_MAC_ADDRESS1:
			cmcU8ToMacString(payload + i + 1, pContext->BoardInfo.MAC1);
            pContext->BoardInfo.MAC1_hex = cmcU8ToU64(payload + i + 1);
            DataStoreWriteMACAddressToRegister(pContext, pContext->BoardInfo.MAC1_hex, HOST_MAC1);
			i += 7;
			break;
		case SNSR_ID_MAC_ADDRESS2:
			cmcU8ToMacString(payload + i + 1, pContext->BoardInfo.MAC2);
            pContext->BoardInfo.MAC2_hex = cmcU8ToU64(payload + i + 1);
            DataStoreWriteMACAddressToRegister(pContext, pContext->BoardInfo.MAC2_hex, HOST_MAC2);
			i += 7;
			break;
		case SNSR_ID_MAC_ADDRESS3:
			cmcU8ToMacString(payload + i + 1, pContext->BoardInfo.MAC3);
            pContext->BoardInfo.MAC3_hex = cmcU8ToU64(payload + i + 1);
            DataStoreWriteMACAddressToRegister(pContext, pContext->BoardInfo.MAC3_hex, HOST_MAC3);
			i += 7;
			break;
		case SNSR_ID_BOARD_REV:
			if ((cmcStrLength((char*)(payload + i + 1)) > DATA_STORE_BOARD_REVISION_SIZE) || (cmcStrLength((char*)(payload + i + 1)) == 0))
			{
				Watch_Inc(pContext->pWatchPointContext, W_BOARDINFO_SIZE_ERROR_COUNT_REG);
				goto exit;
			}
			else
			{
				cmcStrCopy(pContext->BoardInfo.Revision, (char*)(payload + i + 1));
				i += (1 + cmcStrLength((char*)(payload + i + 1)) + 1);
			}
			break;
		case SNSR_ID_BOARD_NAME:
			if ((cmcStrLength((char*)(payload + i + 1)) > DATA_STORE_BOARD_NAME_SIZE) || (cmcStrLength((char*)(payload + i + 1)) == 0))
			{
				Watch_Inc(pContext->pWatchPointContext, W_BOARDINFO_SIZE_ERROR_COUNT_REG);
				goto exit;
			}
			else
			{
				cmcStrCopy(pContext->BoardInfo.Name, (char*)(payload + i + 1));
				i += (1 + cmcStrLength((char*)(payload + i + 1)) + 1);
			}
			break;
		case SNSR_ID_SAT_VERSION:
			if ((cmcStrLength((char*)(payload + i + 1)) > DATA_STORE_SATELLITE_CONTROLLER_FW_REVISION_SIZE) || (cmcStrLength((char*)(payload + i + 1)) == 0))
			{
				Watch_Inc(pContext->pWatchPointContext, W_BOARDINFO_SIZE_ERROR_COUNT_REG);
				goto exit;
			}
			else
			{
				cmcStrCopy(pContext->BoardInfo.SatelliteController_FirmwareRevision, (char*)(payload + i + 1));
				
                // If the SC version has changed
                if (strcmp(pContext->BoardInfo.SatelliteController_FirmwareRevision, pContext->BoardInfo.SatelliteController_FirmwarePreviousRevision) != 0)
                {
                    // If previous isn't different due to a restart set the watchpoint and/or trigger a re-negotiate of comms on U30 only
                    if (pContext->BoardInfo.SatelliteController_FirmwarePreviousRevision[0] != '\0')
                    {
                        //If U30 Zync1 then we need to kick the CMC to renegotiate COMMS since the SC has been changed via the other CMC
                        if (pContext->bZync1Device)
                        {
                            pContext->bRenegotiateComms = true;
                        }                     
                    }
                    cmcStrCopy(pContext->BoardInfo.SatelliteController_FirmwarePreviousRevision, (char*)(payload + i + 1));
                }
				i += (1 + cmcStrLength((char*)(payload + i + 1)) + 1);
			}
			
			break;
		case SNSR_ID_TOTAL_POWER_AVAIL:
			pContext->BoardInfo.AvailablePower = payload[i + 1];

            PERIPHERAL_REGMAP_RAM_CONTROLLER_WriteWithMask(pContext->pHardwareRegisterSetContext, HOST_REGISTER_STATUS, 
													pContext->BoardInfo.AvailablePower << HOST_REGISTER_PATTERN_POWER_MODE_OFFSET, HOST_REGISTER_PATTERN_POWER_MODE_MASK);

			i += 2;
			break;
		case SNSR_ID_FAN_PRESENCE:
			pContext->BoardInfo.FanIsPresent = payload[i + 1];
			i += 2;
			break;
		case SNSR_ID_CONFIG_MODE:
			pContext->BoardInfo.ConfigMode = payload[i + 1];
			i += 2;
			break;

        case SNSR_ID_NEW_MAC_SCHEME:
            pContext->BoardInfo.NumberOfMACs = cmcU8ToU16(&payload[i + 1]);
            if(pContext->BoardInfo.NumberOfMACs > 0)
            { 
                pContext->BoardInfo.MAC0_hex = cmcU8ToU64(payload + i + 3);
            }
            
            for (j = 0; j < 6; j++)
            {
                pContext->BoardInfo.FirstNewMAC[j] = payload[i + 3 + j];
            }

            for(k = 0; k < pContext->BoardInfo.NumberOfMACs; k++)
            { 
                if (k < 8 )
                {
                    DataStoreWriteMACAddressToRegister(pContext, pContext->BoardInfo.MAC0_hex + k, HOST_MAC0 + (k * 4));
                }
            }
            
            i += 9;
            break;

		default:
			Watch_Inc(pContext->pWatchPointContext, W_SENSOR_ERROR_COUNT_REG);
			goto exit;
		}
	}

exit:
	pContext->BoardInfo.Length = i;

    if (pContext->pFirmwareVersionContext->Version.Minor == 0x4) // Is this a U30
    {
        if((pContext->bZync1Device) || (pContext->OEMID == 0x12EB)) //Zync1 or U30 AWS card
        { 
            // TODO make this change when XRT are ready to support
            PERIPHERAL_REGMAP_RAM_CONTROLLER_WriteWithMask(pContext->pHardwareRegisterSetContext, HOST_REGISTER_STATUS, (uint32_t)(HOST_STATUS_REGISTER_SC_MODE_NORMAL_MODE_SC_NOT_UPGRADABLE << HOST_REGISTER_PATTERN_SAT_MODE_OFFSET), (uint32_t)HOST_REGISTER_PATTERN_SAT_MODE_MASK);
            //PERIPHERAL_REGMAP_RAM_CONTROLLER_WriteWithMask(pContext->pHardwareRegisterSetContext, HOST_REGISTER_STATUS, (uint32_t)(HOST_STATUS_REGISTER_SC_MODE_NORMAL_MODE << HOST_REGISTER_PATTERN_SAT_MODE_OFFSET), (uint32_t)HOST_REGISTER_PATTERN_SAT_MODE_MASK);
        }
        else //Zync2
        { 
            PERIPHERAL_REGMAP_RAM_CONTROLLER_WriteWithMask(pContext->pHardwareRegisterSetContext, HOST_REGISTER_STATUS, (uint32_t)(HOST_STATUS_REGISTER_SC_MODE_NORMAL_MODE << HOST_REGISTER_PATTERN_SAT_MODE_OFFSET), (uint32_t)HOST_REGISTER_PATTERN_SAT_MODE_MASK);
        }  
    }
    else
    {
        if (pContext->bCardSupportsSCUpgrade)
        {
            PERIPHERAL_REGMAP_RAM_CONTROLLER_WriteWithMask(pContext->pHardwareRegisterSetContext, HOST_REGISTER_STATUS, (uint32_t)(HOST_STATUS_REGISTER_SC_MODE_NORMAL_MODE << HOST_REGISTER_PATTERN_SAT_MODE_OFFSET), (uint32_t)HOST_REGISTER_PATTERN_SAT_MODE_MASK);

        }
        else
        {
            PERIPHERAL_REGMAP_RAM_CONTROLLER_WriteWithMask(pContext->pHardwareRegisterSetContext, HOST_REGISTER_STATUS, (uint32_t)(HOST_STATUS_REGISTER_SC_MODE_NORMAL_MODE_SC_NOT_UPGRADABLE << HOST_REGISTER_PATTERN_SAT_MODE_OFFSET), (uint32_t)HOST_REGISTER_PATTERN_SAT_MODE_MASK);
        }
    }
    

	return;
}

void cmcCopyBoardInfo(CMC_DATA_STORE_CONTEXT_TYPE* pContext, uint8_t* buffer)
{
	uint16_t i = 0;
    uint8_t j = 0;
    uint8_t MACStrLength = 0;

	buffer[i++] = SNSR_ID_BOARD_NAME;
	buffer[i++] = cmcStrLength(pContext->BoardInfo.Name) + 1;
	cmcStrCopy((char*)buffer + i, pContext->BoardInfo.Name);
	i += cmcStrLength(pContext->BoardInfo.Name) + 1;

	buffer[i++] = SNSR_ID_BOARD_REV;
	buffer[i++] = cmcStrLength(pContext->BoardInfo.Revision) + 1;
	cmcStrCopy((char*)buffer + i, pContext->BoardInfo.Revision);
	i += cmcStrLength(pContext->BoardInfo.Revision) + 1;

	buffer[i++] = SNSR_ID_BOARD_SN;
	buffer[i++] = cmcStrLength(pContext->BoardInfo.SerialNumber) + 1;
	cmcStrCopy((char*)buffer + i, pContext->BoardInfo.SerialNumber);
	i += cmcStrLength(pContext->BoardInfo.SerialNumber) + 1;

    MACStrLength = cmcStrLength(pContext->BoardInfo.MAC0);
    if (MACStrLength)
    {
        buffer[i++] = SNSR_ID_MAC_ADDRESS0;
        buffer[i++] = MACStrLength + 1;
        cmcStrCopy((char*)buffer + i, pContext->BoardInfo.MAC0);
        i += cmcStrLength(pContext->BoardInfo.MAC0) + 1;
    }

    MACStrLength = cmcStrLength(pContext->BoardInfo.MAC1);
    if (MACStrLength)
    {
        buffer[i++] = SNSR_ID_MAC_ADDRESS1;
        buffer[i++] = MACStrLength + 1;
        cmcStrCopy((char*)buffer + i, pContext->BoardInfo.MAC1);
        i += cmcStrLength(pContext->BoardInfo.MAC1) + 1;
    }

    MACStrLength = cmcStrLength(pContext->BoardInfo.MAC2);
    if (MACStrLength)
    {
        buffer[i++] = SNSR_ID_MAC_ADDRESS2;
        buffer[i++] = MACStrLength + 1;
        cmcStrCopy((char*)buffer + i, pContext->BoardInfo.MAC2);
        i += cmcStrLength(pContext->BoardInfo.MAC2) + 1;
    }

    MACStrLength = cmcStrLength(pContext->BoardInfo.MAC3);
    if (MACStrLength)
    {
        buffer[i++] = SNSR_ID_MAC_ADDRESS3;
        buffer[i++] = MACStrLength + 1;
        cmcStrCopy((char*)buffer + i, pContext->BoardInfo.MAC3);
        i += cmcStrLength(pContext->BoardInfo.MAC3) + 1;
    }

	buffer[i++] = SNSR_ID_FAN_PRESENCE;
	buffer[i++] = 1;
	buffer[i++] = pContext->BoardInfo.FanIsPresent;

	buffer[i++] = SNSR_ID_CONFIG_MODE;
	buffer[i++] = 1;
	buffer[i++] = pContext->BoardInfo.ConfigMode;

	buffer[i++] = SNSR_ID_TOTAL_POWER_AVAIL;
	buffer[i++] = 1;
	buffer[i++] = pContext->BoardInfo.AvailablePower;

	buffer[i++] = SNSR_ID_SAT_VERSION;
	buffer[i++] = cmcStrLength(pContext->BoardInfo.SatelliteController_FirmwareRevision) + 1;
	cmcStrCopy((char*)buffer + i, pContext->BoardInfo.SatelliteController_FirmwareRevision);
	i += cmcStrLength(pContext->BoardInfo.SatelliteController_FirmwareRevision) + 1;

    if (pContext->BoardInfo.NumberOfMACs)
    {
        buffer[i++] = SNSR_ID_NEW_MAC_SCHEME;
        buffer[i++] = 8;
        cmcU16ToU8(pContext->BoardInfo.NumberOfMACs, &buffer[i]);
        i++;
        i++;
        for (j = 0; j < 6; j++)
        {
            buffer[i++] = pContext->BoardInfo.FirstNewMAC[j];
        }
    }

	pContext->BoardInfo.Length = i;

	return;
}