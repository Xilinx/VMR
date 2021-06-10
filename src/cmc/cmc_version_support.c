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
 *  $Revision: #35 $
 *
 */




#include "cmc_version_support.h"

extern PERIPHERAL_REGMAP_RAM_CONTROLLER_CONTEXT_TYPE        RegMap_RAM_Controller_Context;


static bool CMC_VersionSupport_Check_If_BoardType_Supported(uint16_t boardType, uint16_t unsupportedBoardType)
{
	uint16_t mask = 1 << boardType;
	return ((unsupportedBoardType & mask) == 0);
}


void CMC_VersionSupport_Initialize(CMC_VERSION_SUPPORT_CONTEXT_TYPE* pContext, 
                                    CMC_WATCHPOINT_CONTEXT_TYPE* pWatchPointContext,
                                    uint8_t CMCOfferedInterfaceVersion,
                                    uint8_t boardType, 
                                    bool scUpgradeNotSupported)
{
	pContext->maxVersion = 0;

    if (boardType == 0x9) // Is this a U26 Cards with no SC will not negotiate a version so set commsVersion to the CMCOfferedInterfaceVersion instead.
    {
        pContext->commsVersion = CMCOfferedInterfaceVersion;
    }
    else
    {
        pContext->commsVersion = CMC_COMMS_MIN_VERSION;
    }
	
	pContext->boardType = boardType;
    pContext->CMCOfferedInterfaceVersion = CMCOfferedInterfaceVersion;
	uint8_t i;
	
    Watch_Set(pWatchPointContext, W_LINK_RECEIVE_NEGOTIATED_VERSION, pContext->commsVersion);


	CMC_SUPPORTED_MESSAGE_TYPE CMCSupportedMessageTable[CMC_SUPPORTED_MESSAGE_TABLE_SIZE] =
	{
		// msgid  version  unsupportedboardtypes
		{SAT_COMMS_NULL,								 1,		0},
		{SAT_COMMS_EN_BSL,								 1,		0},
		{SAT_COMMS_ALERT_REQ,							 1,		(1 << 4)},
		{SAT_COMMS_VERS_REQ,							 1,		0},
		{SAT_COMMS_SET_VERS,							 1,		0},
		{SAT_COMMS_BOARD_INFO_REQ,						 1,		0},
		{SAT_COMMS_VOLT_SNSR_REQ,						 1,		0},
		{SAT_COMMS_POWER_SNSR_REQ,						 1,		0},
		{SAT_COMMS_TEMP_SNSR_REQ,						 1,		0},
		{SAT_COMMS_DEBUG_UART_EN,						 1,		(1 << 4)},

		{SAT_COMMS_FPGA_I2C_BUS_ARB,					 2,		(1 << 4)},
		{SAT_COMMS_CAGE_IO_EVENT,						 2,		(1 << 4)},

		{SAT_COMMS_SNSR_PUSH,							 3,		(1 << 4)},
		{SAT_COMMS_SNSR_STATE_REQ,						 3,		(1 << 4)},

		{SAT_COMMS_SNSR_POWER_THROTTLING_THRESHOLDS_REQ, 4,		(1 << 4)},
		{SAT_COMMS_SNSR_TEMP_THROTTLING_THRESHOLDS_REQ,	 4,		(1 << 4)},
		{SAT_COMMS_SEND_OEM_CMD,						 4,		0},                             // Should not be disabled for VCK5000

		{SAT_COMMS_PCIE_ERROR_REPORT,					 5,		(uint16_t)(~(1 << 4))},         // U30 only
		{SAT_COMMS_ECC_ERROR_REPORT,					 5,		(uint16_t)(~(1 << 4))},         // U30 only
		{SAT_COMMS_KEEPALIVE,							 5,		(uint16_t)(~(1 << 4))},         // U30 only
        {SAT_COMMS_INTERRUPT_STATUS_REQ,				 7,		(uint16_t)(~(1 << 4))},         // U30 only
        
        {SAT_COMMS_CSDR_REQ,                             6,     (1 << 4)}, // | (1 << 7)},
        {SAT_COMMS_READ_QSFP_DIAGNOSTICS_REQ,            6,     (1 << 4)}, // | (1 << 7)},

        {SAT_COMMS_QSFP_VALIDATE_LOW_SPEED_WRITE_IO_REQ, 6,     (1 << 4)}, // | (1 << 7)},
        {SAT_COMMS_QSFP_VALIDATE_LOW_SPEED_READ_IO_REQ,  6,     (1 << 4)}, // | (1 << 7)},

        {SAT_COMMS_QSFP_WRITE_SINGLE_BYTE_REQ,           9,     (1 << 4)}, // | (1 << 7)},
        {SAT_COMMS_QSFP_READ_SINGLE_BYTE_REQ,            9,     (1 << 4)}, // | (1 << 7)},

	};

	
	for (i = 0; i < CMC_SUPPORTED_MESSAGE_TABLE_SIZE; i++)
	{
		// when SC upgrade is not supported then disable BSL Enable message
		if (scUpgradeNotSupported && (CMCSupportedMessageTable[i].msgId == SAT_COMMS_EN_BSL))
		{
			CMCSupportedMessageTable[i].unsupportedBoardType |= (1 << 4);
		}

		//SAT_COMMS_EN_BSL
		pContext->CMCSupportedMessageTable[i] = CMCSupportedMessageTable[i];
	}

}


bool CMC_VersionSupport_CheckMessageSupport(CMC_VERSION_SUPPORT_CONTEXT_TYPE* pContext, uint8_t msgId)
{
	bool result;
	uint8_t minVersion = 0;
	uint8_t i;

	for (i = 0; i < sizeof(pContext->CMCSupportedMessageTable)/sizeof(CMC_SUPPORTED_MESSAGE_TYPE); i++)
	{
		if (msgId == pContext->CMCSupportedMessageTable[i].msgId)
		{
			if (CMC_VersionSupport_Check_If_BoardType_Supported(pContext->boardType, pContext->CMCSupportedMessageTable[i].unsupportedBoardType) == true)
			{
				minVersion = pContext->CMCSupportedMessageTable[i].minVersion;

				// additional check for U30 ZYNQ 
				if (pContext->boardType == 4)
				{
					uint32_t OEMID = PERIPHERAL_REGMAP_RAM_CONTROLLER_Read(&RegMap_RAM_Controller_Context, HOST_REGISTER_OEM_ID);
					if (OEMID == 0x12EB)
					{
						// U30 ZYNQ AWS mode
						if (msgId == SAT_COMMS_VOLT_SNSR_REQ ||
							msgId == SAT_COMMS_POWER_SNSR_REQ ||
							msgId == SAT_COMMS_TEMP_SNSR_REQ ||
							msgId == SAT_COMMS_EN_BSL)
						{
							minVersion = 0;
						}
					}
                    else
                    {
                        if (msgId == SAT_COMMS_INTERRUPT_STATUS_REQ)
                        {
                            minVersion = 0;
                        }
                    }
				}
				break;
			}
		}
	}
	
	if ((0 != minVersion) && (pContext->commsVersion >= minVersion))
	{
		result = true;
	}
	else
	{
		result = false;
	}

	return result;
}