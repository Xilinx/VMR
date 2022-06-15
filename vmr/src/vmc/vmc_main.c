/******************************************************************************
 * Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
 * SPDX-License-Identifier: MIT
 *******************************************************************************/

#include "FreeRTOS.h"
#include "task.h"

#include "cl_vmc.h"
#include "cl_log.h"
#include "cl_i2c.h"
#include "cl_uart_rtos.h"
#include "vmc_api.h"
#include "vmc_sensors.h"
#include "sensors/inc/m24c128.h"
#include "sensors/inc/max6639.h"
#include "vmc_asdm.h"
#include "vmr_common.h"

#include "xsysmonpsv.h"
#include "vmc_sc_comms.h"
#include "vmc_update_sc.h"

SemaphoreHandle_t vmc_sc_lock = NULL;
SemaphoreHandle_t sdr_lock = NULL;

uart_rtos_handle_t uart_vmcsc_log;
uart_rtos_handle_t uart_log;

static bool vmc_is_ready = false;

int cl_vmc_is_ready()
{
	if (!vmc_is_ready)
		VMC_ERR("vmc main service is not ready");

	return cl_vmc_sysmon_is_ready() && (vmc_is_ready == true);
}

int cl_vmc_init()
{
	s8 Status = 0;

	cl_I2CInit();

	Status = UART_VMC_SC_Enable(&uart_vmcsc_log);
	if (Status != XST_SUCCESS) {
		VMR_ERR("UART VMC to SC init failed");
		return -EINVAL;
	}

#ifdef VMC_DEBUG
	/* Enable FreeRTOS Debug UART */
	if (UART_RTOS_Debug_Enable(&uart_log) != XST_SUCCESS) {
		return -ENODEV;
	}
	/* Demo Menu is already enabled by cl_main task handler */
#endif

	/* Read the EEPROM */
	Versal_EEPROM_ReadBoardInfo();

	/* Retry till fan controller is programmed */
	while (max6639_init(1, 0x2E));  // only for vck5000

	/* sdr_lock */
	sdr_lock = xSemaphoreCreateMutex();
	configASSERT(sdr_lock != NULL);

	/* vmc_sc_lock */
	vmc_sc_lock = xSemaphoreCreateMutex();
	configASSERT(vmc_sc_lock != NULL);

	if (Init_Asdm()) {
		VMC_ERR(" ASDM Init Failed \n\r"); 
		return -EINVAL;
	}

	vmc_is_ready = true;
	VMC_LOG("Done. set vmc is ready.");
	return 0;
}
