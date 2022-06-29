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

#include "platforms/vck5000.h"
#include "platforms/v70.h"

SemaphoreHandle_t vmc_sc_lock = NULL;
SemaphoreHandle_t sdr_lock = NULL;

uart_rtos_handle_t uart_vmcsc_log;
uart_rtos_handle_t uart_log;

static bool vmc_is_ready = false;
static ePlatformType current_platform = eV70;

Platform_t platform_names[eMax_Platforms]=
{
	{eVCK5000,"V350\0"},
	{eVCK5000,"VCK5000\0"},
	{eV70,"V70\0"},
};

Platform_Sensor_Handler_t platform_sensor_handlers[]=
{
	{eVCK5000,eTemperature_Sensor_Inlet,Vck5000_Temperature_Read_Inlet},
	{eVCK5000,eTemperature_Sensor_Outlet,Vck5000_Temperature_Read_Outlet},
	{eVCK5000,eTemperature_Sensor_Board,Vck5000_Temperature_Read_Board},
	{eVCK5000,eTemperature_Sensor_QSFP,Vck5000_Temperature_Read_QSFP},
	//{eVCK5000,eFAN_RPM_READ,Vck5000_Fan_RPM_Read},
};

Platform_Function_Handler_t platform_function_handlers[]=
{
	{eVCK5000,ePlatform_Init,Vck5000_Init},
	{eV70,ePlatform_Init,V70_Init},
};

Platform_Func_Ptr platform_init_ptr = NULL;

extern Versal_BoardInfo board_info;


static u8 Vmc_ConfigurePlatform(const char * product_name);

static u8 Init_Platform(void)
{
	return (*platform_init_ptr)();
}

int cl_vmc_is_ready()
{
	if (!vmc_is_ready)
		VMC_ERR("vmc main service is not ready");

	return cl_vmc_sysmon_is_ready() && (vmc_is_ready == true);
}

int cl_vmc_init()
{
	s8 status = 0;

#ifdef VMC_DEBUG
	/* Enable FreeRTOS Debug UART */
	if (UART_RTOS_Debug_Enable(&uart_log) != XST_SUCCESS) {
		return -ENODEV;
	}
	/* Demo Menu is already enabled by cl_main task handler */
#endif

	cl_I2CInit();

	/* Read the EEPROM */
	status = Versal_EEPROM_ReadBoardInfo();
	if(XST_FAILURE == status){
		vmc_is_ready = false;
		VMR_ERR("EEPROM Read Failed.");
		return -EINVAL;
	}

	status = Vmc_ConfigurePlatform((const char *)&board_info.product_name[0]);
	if(XST_FAILURE == status){
		vmc_is_ready = false;
		VMR_ERR("Platform is Unknown.");
		return -EINVAL;
	}

	status = Init_Platform();
	if (status != XST_SUCCESS) {
		VMR_ERR("Platform Initialization Failed.");
		return -EINVAL;
	}

	status = UART_VMC_SC_Enable(&uart_vmcsc_log);
	if (status != XST_SUCCESS) {
		VMR_ERR("UART VMC to SC init Failed.");
		return -EINVAL;
	}

//	/* Retry till fan controller is programmed */
//	while (max6639_init(1, 0x2E));  // only for vck5000

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
	VMR_LOG("Done. set vmc is ready.");
	return 0;
}


static sensorMonitorFunc Vmc_Find_Sensor_Handler(eSensor_Functions sensor_type)
{
	u16 j = 0;
	u16 platform_sensors_len = ARRAY_SIZE(platform_sensor_handlers);

	for(j = 0; j < platform_sensors_len; j++){
		if((current_platform == platform_sensor_handlers[j].product_type_id)
				&& sensor_type == platform_sensor_handlers[j].sensor_type){

			return platform_sensor_handlers[j].sensor_handler;
		}
	}

	return NULL;
}

static Platform_Func_Ptr Vmc_Find_Function_Handler(ePlatform_Functions function_type)
{
	u16 j = 0;
	u16 platform_functions_len = ARRAY_SIZE(platform_function_handlers);

	for(j = 0; j < platform_functions_len; j++){
		if((current_platform == platform_function_handlers[j].product_type_id)
				&& function_type == platform_function_handlers[j].func_type){

			return platform_function_handlers[j].func_handler;
		}
	}

	return NULL;
}

static u8 Vmc_ConfigurePlatform(const char * product_name)
{
	u8 i = 0;
	u8 status = XST_FAILURE;
	if(product_name == NULL){
		VMR_ERR("Platform type is unknown!");
		return status;
	}
	else
	{
		for(i = 0; i < eMax_Platforms; i++){
			if(strstr(product_name,&platform_names[i].product_type_name[0]) != NULL){
				current_platform = platform_names[i].product_type_id;
				status = XST_SUCCESS;
				VMR_LOG("Current Platform is %s",&platform_names[i].product_type_name[0]);
				break;
			}
		}
	}

	if(XST_SUCCESS != status)
		return status;

	for(i = 0; i < eMax_Platform_Functions; i++){
		switch (i){
			case ePlatform_Init:
				platform_init_ptr = Vmc_Find_Function_Handler(i);
				break;
		}
	}

	for(i = 0; i < eMax_Sensor_Functions; i++){
		switch (i){
			case eTemperature_Sensor_Inlet:
				Temperature_Read_Inlet_Ptr = Vmc_Find_Sensor_Handler(i);
				break;

			case eTemperature_Sensor_Outlet:
				Temperature_Read_Outlet_Ptr = Vmc_Find_Sensor_Handler(i);
				break;

			case eTemperature_Sensor_Board:
				Temperature_Read_Board_Ptr = Vmc_Find_Sensor_Handler(i);
				break;

			case eTemperature_Sensor_QSFP:
				Temperature_Read_QSFP_Ptr = Vmc_Find_Sensor_Handler(i);
				break;

//			case eFAN_RPM_READ:
//				Fan_RPM_Read_Ptr = Vmc_Find_Sensor_Handler(i);
//				break;

		}
	}


	return status;
}
