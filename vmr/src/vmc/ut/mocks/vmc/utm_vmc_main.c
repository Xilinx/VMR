
/******************************************************************************
* Copyright (C) 2024 Advanced Micro Devices, Inc.    All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <cmocka.h>

#include "FreeRTOS.h"
#include "task.h"
#include "cl_vmc.h"
#include "cl_log.h"
#include "vmc_api.h"
#include "vmc_sensors.h"
#include "vmc_asdm.h"
#include "vmr_common.h"
#include "vmc_sc_comms.h"
#include "vmc_update_sc.h"
#include "platforms/v70.h"

SemaphoreHandle_t sdr_lock = NULL;
static ePlatformType current_platform = eV70;

extern s8 V70_Asdm_Read_Temp_Vccint(snsrRead_t *snsrData);

/*****************************Internal functions*******************************/
void ConfigureV70Platform()
{
	Temperature_Read_Board_Ptr  = V70_Temperature_Read_Board;
	Temperature_Read_QSFP_Ptr   = NULL;
	Temperature_Read_VCCINT_Ptr = V70_Asdm_Read_Temp_Vccint;
	Power_Read_Ptr 	            = V70_Asdm_Read_Power;

	Voltage_Read_Ptr            = V70_Get_Voltage_Names;
	Current_Read_Ptr            = V70_Get_Current_Names;
}

/*****************************Real function definitions*******************************/
/*This definition is same as real implementation.
 * It does not need mock features, since it is very simple definition*/
ePlatformType Vmc_Get_PlatformType(void)
{
        return current_platform;
}
