/******************************************************************************
 * Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
 * SPDX-License-Identifier: MIT
 *******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "xparameters.h"
#include "xgpiops.h"
#include "cl_gpio.h"
#include "cl_log.h"


#define LPD_GPIO_BASE XPAR_BLP_CIPS_PSPMC_0_PSV_GPIO_2_BASEADDR
#define PMC_GPIO_BASE XPAR_BLP_CIPS_PSPMC_0_PSV_PMC_GPIO_0_BASEADDR


#define LPD_GPIO_DEVICE_ID 	XPAR_BLP_CIPS_PSPMC_0_PSV_GPIO_2_DEVICE_ID
#define PMC_GPIO_DEVICE_ID	XPAR_BLP_CIPS_PSPMC_0_PSV_PMC_GPIO_0_DEVICE_ID

XGpioPs_Config gpioConfig[] = {
		{LPD_GPIO_DEVICE_ID,LPD_GPIO_BASE},
		{PMC_GPIO_DEVICE_ID,PMC_GPIO_BASE}
};

XGpioPs Gpio[2];	/* The driver instance for GPIO Device. */


u8 GPIOInit(void)
{

	//XGpioPs *ConfigPtr = XGpioPs_LookupConfig(GPIO_DEVICE_ID);

	/* Access PMC GPIO by setting to TRUE */
	Gpio[PMC_GPIO_PORT].PmcGpio =  TRUE;
	int Status = XGpioPs_CfgInitialize(&Gpio[PMC_GPIO_PORT], &gpioConfig[PMC_GPIO_PORT],
			gpioConfig[PMC_GPIO_PORT].BaseAddr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	CL_LOG(APP_MAIN,"GPIO0 max pins %u, banks %u\n\r", Gpio[0].MaxPinNum,Gpio[0].MaxBanks);

	Gpio[LPD_GPIO_PORT].PmcGpio =  FALSE;
	Status = XGpioPs_CfgInitialize(&Gpio[LPD_GPIO_PORT], &gpioConfig[LPD_GPIO_PORT],
			gpioConfig[LPD_GPIO_PORT].BaseAddr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	CL_LOG(APP_MAIN,"GPIO1 max pins %u, banks %u\n\r", Gpio[1].MaxPinNum,Gpio[1].MaxBanks);

	/* Config pins */
	CL_LOG(APP_MAIN,"Setting pin polarities and initial state.\n\r");

	GPIOWriteOutput(LPD_GPIO_PORT,GPIO_PIN_FRU_EEPROM_WP,DEASSERTED);

	GPIOWriteOutput(LPD_GPIO_PORT,GPIO_PIN_VPD_EEPROM_WP,ASSERTED);

	/* SMBus EN. 0 for VPD EEPROM in master, 1 for ADK in Slave */
	GPIOWriteOutput(LPD_GPIO_PORT,GPIO_PIN_VPD_EEPROM_SMBUS_SELECT,ASSERTED);

	GPIOWriteOutput(LPD_GPIO_PORT,GPIO_PIN_SPI_FLASH_CS,DEASSERTED);


	return XST_SUCCESS;
}


u8 GPIOReadInput(uint32_t port, uint32_t theBit, uint32_t *value)
{
	if (Gpio[port].MaxPinNum <= theBit)
		return XST_FAILURE;

	XGpioPs_SetDirectionPin(&Gpio[port], theBit, INPUT_PIN);
	u32 Data = XGpioPs_ReadPin(&Gpio[port], theBit);
	*value = Data;
	return XST_SUCCESS;
}
u8 GPIOWriteOutput(uint32_t port, uint32_t theBit, uint32_t value)
{
	if (Gpio[port].MaxPinNum <= theBit)
		return XST_FAILURE;

	XGpioPs_SetDirectionPin(&Gpio[port], theBit, OUTPUT_PIN);
	XGpioPs_WritePin(&Gpio[port], theBit, value);

	return XST_SUCCESS;
}

