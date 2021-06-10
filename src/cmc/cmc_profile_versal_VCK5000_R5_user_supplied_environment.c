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


/* PROFILE_LINUX_VERSAL_VCK5000_R5 */ 
/* LINUX_DIST */





/*
 *
 *  RCS Keyword Metadata
 *
 *  $Change: 3095780 $
 *  $Date: 2021/01/13 $
 *  $Revision: #20 $
 *
 */





#include <signal.h>
#include <stdio.h>
#include "cmc_profile_versal_VCK5000_R5.h"
#include "cmc_ram.h"

CMC_BUILDPROFILE_VERSAL_VCK5000_R5_USER_ENVIRONMENT_TYPE CMC_BuildProfile_Versal_VCK5000_R5_SuppliedEnvironment_UserContext;


void CMC_BuildProfile_Versal_VCK5000_R5_SuppliedEnvironment_PreAmble(void * pUserContext)
{
    UNUSED(pUserContext);
}



void CMC_BuildProfile_Versal_VCK5000_R5_SuppliedEnvironment_PostAmble(void * pUserContext)
{
    UNUSED(pUserContext);
}



uint32_t CMC_BuildProfile_Versal_VCK5000_R5_SuppliedEnvironment_Read(void * pUserContext, CMC_PERIPHERAL_BASE_ADDRESS_TYPE PeripheralBaseAddress, uint32_t RegisterOffset)
{
    volatile uint32_t  Address=(uint32_t)(PeripheralBaseAddress+RegisterOffset);

    UNUSED(pUserContext);

    return Xil_In32(Address);
}


void CMC_BuildProfile_Versal_VCK5000_R5_SuppliedEnvironment_Write(void * pUserContext, CMC_PERIPHERAL_BASE_ADDRESS_TYPE PeripheralBaseAddress, uint32_t RegisterOffset, uint32_t Value)
{
    volatile uint32_t Address=(uint32_t)(PeripheralBaseAddress+RegisterOffset);

    UNUSED(pUserContext);

    Xil_Out32(Address,Value);
}



void CMC_BuildProfile_Versal_VCK5000_R5_SuppliedEnvironment_WriteWithMask(void * pUserContext, CMC_PERIPHERAL_BASE_ADDRESS_TYPE PeripheralBaseAddress, uint32_t RegisterOffset, uint32_t Value, uint32_t Mask)
{
    uint32_t RegisterValue=CMC_BuildProfile_Versal_VCK5000_R5_SuppliedEnvironment_Read(pUserContext, PeripheralBaseAddress, RegisterOffset);
    uint32_t MaskedValue;

    RegisterValue=RegisterValue&~Mask;
    MaskedValue=Value&Mask;
    RegisterValue|=MaskedValue;
    CMC_BuildProfile_Versal_VCK5000_R5_SuppliedEnvironment_Write(pUserContext, PeripheralBaseAddress, RegisterOffset, RegisterValue);

}



uint16_t CMC_BuildProfile_Versal_VCK5000_R5_SuppliedEnvironment_CALCULATE_CRC16_CCITT(void * pUserContext, uint16_t SeedValue, char *pBuffer, uint32_t BufferLength)
{
	uint32_t i, j;
	uint16_t crc = SeedValue; /* Assign CRC initial value */
	uint16_t CRC_POLY = 0x1021;

	UNUSED(pUserContext);

	for (i = 0; i < BufferLength; i++)
	{
		/* XOR least significant byte of the crc */
		crc = crc ^ pBuffer[i] << 8;

		/* Loop over the bits */
		for (j = 0; j < 8; j++)
		{
			if (crc & 0x8000)  /* Is bit 15 == 1? */
			{
				crc = crc * 2; /* CRC = CRC * 2 */
				crc ^= CRC_POLY; /* XOR with polynomial */
			}
			else /* bit 15 == 0 */
			{
				crc = crc * 2;
			}
		}
	}
	return crc;
}


uint32_t CMC_BuildProfile_Versal_VCK5000_R5_StackFillPercentage(void* pUserContext)
{
    UNUSED(pUserContext);

    return 0;
}

void CMC_BuildProfile_Versal_VCK5000_R5_SuppliedEnvironment_QSFP_GPIO_Read(void* pUserContext, uint8_t iQSFP, uint8_t* QSFP_Read_Value)
{
    uint32_t MIO;
    uint32_t QSFP_bits;
    uint8_t MODSEL_L;
    uint8_t RESET_L;
    uint8_t MODPRS_L;
    uint8_t INT_L;
    uint8_t LPMODE;
    uint8_t QSFP_output;

    // If VCK5000 R5 use a different read function
    MIO = CMC_BuildProfile_Versal_VCK5000_R5_SuppliedEnvironment_Read(pUserContext, (CMC_PERIPHERAL_BASE_ADDRESS_TYPE)0xF1020064, 0x0);
    // QSFP 0 is bits 14- 18, QSFP 1 is bits 19 - 23
    QSFP_bits = (MIO >> (14 + (iQSFP * 5))) & 0x1F;

    /* Rearrange into correct order for SC CAGE_IO message
       Read bits are
       bit 0 MODSEL_L
       bit 1 RESET_L
       bit 2 MODPRS_L
       bit 3 INT_L
       bit 4 LPMODE

       change this to

       bit 0 LPMODE
       bit 1 RESET_L
       bit 2 MODSEL_L
       bit 3 MODPRS_L
       bit 4 INT_L
       */

    MODSEL_L   = (uint8_t)(QSFP_bits & 0x1);
    RESET_L    = (uint8_t)((QSFP_bits & 0x2 ) >> 1);
    MODPRS_L   = (uint8_t)((QSFP_bits & 0x4 ) >> 2);
    INT_L      = (uint8_t)((QSFP_bits & 0x8 ) >> 3);
    LPMODE     = (uint8_t)((QSFP_bits & 0x10) >> 4);
    
    QSFP_output = LPMODE | (RESET_L << 1) | (MODSEL_L << 2) | (MODPRS_L << 3) | (INT_L << 4);

    *QSFP_Read_Value = (uint8_t)QSFP_output;
}

void CMC_BuildProfile_Versal_VCK5000_R5_SuppliedEnvironment_QSFP_GPIO_Write(void* pUserContext, uint8_t iQSFP, uint8_t QSFP_Write_Value)
{
    uint32_t MIO;
    uint8_t LPMODE;
    uint8_t RESET_L;
    uint8_t MODSEL_L;
    uint8_t MODPRS_L;
    uint8_t INT_L;
    uint8_t QSFP_output;

    /* Rearrange into correct order

       bit 0 LPMODE
       bit 1 RESET_L
       bit 2 MODSEL_L
       bit 3 MODPRS_L
       bit 4 INT_L

       change this to

       bit 0 MODSEL_L
       bit 1 RESET_L
       bit 2 MODPRS_L
       bit 3 INT_L
       bit 4 LPMODE
       */
    LPMODE      = (uint8_t)(QSFP_Write_Value & 0x1);
    RESET_L     = (uint8_t)((QSFP_Write_Value & 0x2) >> 1);
    MODSEL_L    = (uint8_t)((QSFP_Write_Value & 0x4) >> 2);
    MODPRS_L    = (uint8_t)((QSFP_Write_Value & 0x8) >> 3);
    INT_L       = (uint8_t)((QSFP_Write_Value & 0x10) >> 4);

    QSFP_output = MODSEL_L | (RESET_L << 1) | (MODPRS_L << 2) | (INT_L << 3) | (LPMODE << 4);

    MIO = CMC_BuildProfile_Versal_VCK5000_R5_SuppliedEnvironment_Read(pUserContext, (CMC_PERIPHERAL_BASE_ADDRESS_TYPE)0xF1020064, 0x0);
    MIO = MIO & ~(0x1F << (14 + (iQSFP * 5)));
    MIO = MIO | (QSFP_output << (14 + (iQSFP * 5)));

    // If VCK5000 R5 use a different read function
    //xil_printf("%s:%d MIO is 0x%x\r\n",__FILE__, __LINE__, MIO);
    CMC_BuildProfile_Versal_VCK5000_R5_SuppliedEnvironment_Write(pUserContext, (CMC_PERIPHERAL_BASE_ADDRESS_TYPE)0xF1020044, 0x0, MIO);
    // QSFP 0 is bits 14- 18, QSFP 1 is bits 19 - 23

}

void CMC_BuildProfile_Versal_VCK5000_R5_user_supplied_environment(CMC_BUILD_PROFILE_TYPE * pProfile)
{
	CMC_BuildProfile_Versal_VCK5000_R5_SuppliedEnvironment_UserContext.pDeframerContext = &SensorSupervisorDeFramerContext;
	CMC_BuildProfile_Versal_VCK5000_R5_SuppliedEnvironment_UserContext.pUartContext = &AXI_UART_LITE_Satellite_Context;
	CMC_BuildProfile_Versal_VCK5000_R5_SuppliedEnvironment_UserContext.pParserContext = &SensorSupervisorParserContext;
	CMC_BuildProfile_Versal_VCK5000_R5_SuppliedEnvironment_UserContext.pBootloaderParserContext = &BootloaderParserContext;

	CMC_USER_AddEnvironmentBinding_UserContext(&pProfile->UserSuppliedEnvironment, &CMC_BuildProfile_Versal_VCK5000_R5_SuppliedEnvironment_UserContext);
    CMC_USER_AddEnvironmentBinding_StartScheduling(&pProfile->UserSuppliedEnvironment, true);
    CMC_USER_AddEnvironmentBinding_PreAmble(&pProfile->UserSuppliedEnvironment, CMC_BuildProfile_Versal_VCK5000_R5_SuppliedEnvironment_PreAmble);
    CMC_USER_AddEnvironmentBinding_PostAmble(&pProfile->UserSuppliedEnvironment, CMC_BuildProfile_Versal_VCK5000_R5_SuppliedEnvironment_PostAmble);
    CMC_USER_AddEnvironmentBinding_PeripheralRead(&pProfile->UserSuppliedEnvironment, CMC_BuildProfile_Versal_VCK5000_R5_SuppliedEnvironment_Read);
    CMC_USER_AddEnvironmentBinding_PeripheralWrite(&pProfile->UserSuppliedEnvironment, CMC_BuildProfile_Versal_VCK5000_R5_SuppliedEnvironment_Write);
    CMC_USER_AddEnvironmentBinding_PeripheralWriteWithMask(&pProfile->UserSuppliedEnvironment, CMC_BuildProfile_Versal_VCK5000_R5_SuppliedEnvironment_WriteWithMask);
    CMC_USER_AddEnvironmentBinding_Calculate_CRC16_CCITT(&pProfile->UserSuppliedEnvironment, CMC_BuildProfile_Versal_VCK5000_R5_SuppliedEnvironment_CALCULATE_CRC16_CCITT);
    CMC_USER_AddEnvironmentBinding_SensorsGatedByPowerGood(&pProfile->UserSuppliedEnvironment, false); // TODO change this false to a true if SC is sending POWER_GOOD
    CMC_USER_AddEnvironmentBinding_PCIeECCReportingSupported(&pProfile->UserSuppliedEnvironment, false);
	CMC_USER_AddEnvironmentBinding_KeepAliveSupported(&pProfile->UserSuppliedEnvironment, false);
    CMC_USER_AddEnvironmentBinding_CardSupportsSCUpgrade(&pProfile->UserSuppliedEnvironment, true);
    CMC_USER_AddEnvironmentBinding_SensorSupervisorTimeout(&pProfile->UserSuppliedEnvironment, 16);
    CMC_USER_AddEnvironmentBinding_Valid_Sensors(&pProfile->UserSuppliedEnvironment, (uint64_t)CMC_VALIDSENSORS_0_to_63, (uint64_t)CMC_VALIDSENSORS_64_to_127);
    CMC_USER_AddEnvironmentBinding_CardSupportsScalingFactor(&pProfile->UserSuppliedEnvironment, true);
    CMC_USER_AddEnvironmentBinding_CardSupportsHBM(&pProfile->UserSuppliedEnvironment, false);
	CMC_USER_AddEnvironmentBinding_CardSupportsSC(&pProfile->UserSuppliedEnvironment, true);
    CMC_USER_AddEnvironmentBinding_StackCheck(&pProfile->UserSuppliedEnvironment, CMC_BuildProfile_Versal_VCK5000_R5_StackFillPercentage);
	CMC_USER_AddEnvironmentBinding_CardSupportsSUCUpgrade(&pProfile->UserSuppliedEnvironment, false);
    CMC_USER_AddEnvironmentBinding_QSFP_GPIORead(&pProfile->UserSuppliedEnvironment, CMC_BuildProfile_Versal_VCK5000_R5_SuppliedEnvironment_QSFP_GPIO_Read);
    CMC_USER_AddEnvironmentBinding_QSFP_GPIOWrite(&pProfile->UserSuppliedEnvironment, CMC_BuildProfile_Versal_VCK5000_R5_SuppliedEnvironment_QSFP_GPIO_Write);
}

