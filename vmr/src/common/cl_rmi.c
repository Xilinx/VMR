/******************************************************************************
 * * Copyright (C) 2024 AMD, Inc.    All rights reserved.
 * * SPDX-License-Identifier: MIT
 * *******************************************************************************/

#include "cl_config.h"

#ifdef BUILD_FOR_RMI

/* FreeRTOS includes */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "stream_buffer.h"

#include "xil_printf.h"
#include <stdio.h>
#include <stdlib.h>
#include "cl_mem.h"
#include "cl_rmi.h"
#include "RMI/rmi_api.h"
#include "RMI/rmi_sensors.h"
#include "../vmc/vmc_api.h"
#include "../vmc/vmc_sensors.h"
#include "../vmc/vmc_asdm.h"
#include "../vmc/rmi.h"

#define MAX_SENSOR_COUNT 100

static const u8 STREAM_BUF_MAX_ITEMS = 10;

static u8 is_rmi_ready = false;
static u32 cl_rmi_sensors_count = 0;
static u32 cl_rmi_dynamic_sensors_count = 0;
static u32 stream_buffer_item_size = 0;

StreamBufferHandle_t xStreamBuffer_rmi_sensor = NULL;
sensors_ds_t* cl_rmi_p_sensors = NULL;


static void cl_rmi_init_sensors(void);
static void cl_rmi_update_rmi_sensor_values(sensors_ds_t* sensor_val);
static s8 rmi_board_info_init(void);
static s8 rmi_sensor_init(void);

s8 rmi_init_all(void)
{

    if( rmi_board_info_init())
    {
        VMR_ERR(" RMI Board Info Init Failed \n\r");
        return -1;
    }

    if( rmi_sensor_init())
    {
        VMR_ERR(" RMI Sensor Init Failed \n\r");
        return -1;
    }

    return 0;
}

static s8 rmi_board_info_init(void)
{

    Versal_BoardInfo versal_board_info = {0};
    rmi_board_info_t rmi_board_info = {0};
    u32 temp_size = 0;
    s8 ret_val = 0;

    Cl_SecureMemset(&versal_board_info, 0, sizeof(versal_board_info));

    VMC_Get_BoardInfo(&versal_board_info);

    Cl_SecureMemset(&rmi_board_info, 0, sizeof(rmi_board_info));

    temp_size = sizeof(rmi_board_info.eeprom_version);
    Cl_SecureMemcpy(&rmi_board_info.eeprom_version, temp_size, &versal_board_info.eeprom_version, temp_size);

    temp_size = sizeof(rmi_board_info.board_act_pas);
    Cl_SecureMemcpy(&rmi_board_info.board_act_pas, temp_size, &versal_board_info.board_act_pas, temp_size);

    temp_size = sizeof(rmi_board_info.board_config_mode);
    Cl_SecureMemcpy(&rmi_board_info.board_config_mode, temp_size, &versal_board_info.board_config_mode, temp_size);

    temp_size = sizeof(rmi_board_info.board_mac);
    Cl_SecureMemcpy(&rmi_board_info.board_mac, temp_size, &versal_board_info.board_mac[0], temp_size);

    temp_size = sizeof(rmi_board_info.board_max_power_mode);
    Cl_SecureMemcpy(&rmi_board_info.board_max_power_mode, temp_size, &versal_board_info.board_max_power_mode, temp_size);

    temp_size = sizeof(rmi_board_info.board_mfg_date);
    Cl_SecureMemcpy(&rmi_board_info.board_mfg_date, temp_size, &versal_board_info.board_mfg_date, temp_size);
    
    temp_size = sizeof(rmi_board_info.board_part_num);
    Cl_SecureMemcpy(&rmi_board_info.board_part_num, temp_size, &versal_board_info.board_part_num, temp_size);

    temp_size = sizeof(rmi_board_info.board_pcie_info);
    Cl_SecureMemcpy(&rmi_board_info.board_pcie_info, temp_size, &versal_board_info.board_pcie_info, temp_size);

    temp_size = sizeof(rmi_board_info.board_rev);
    Cl_SecureMemcpy(&rmi_board_info.board_rev, temp_size, &versal_board_info.board_rev, temp_size);

    temp_size = sizeof(rmi_board_info.board_serial);
    Cl_SecureMemcpy(&rmi_board_info.board_serial, temp_size, &versal_board_info.board_serial, temp_size);

    temp_size = sizeof(rmi_board_info.board_uuid);
    Cl_SecureMemcpy(&rmi_board_info.board_uuid, temp_size, &versal_board_info.board_uuid, temp_size);

    temp_size = sizeof(rmi_board_info.capability_word);
    Cl_SecureMemcpy(&rmi_board_info.capability_word, temp_size, &versal_board_info.capability, temp_size);

    temp_size = sizeof(rmi_board_info.num_mac_ids);
    Cl_SecureMemcpy(&rmi_board_info.num_mac_ids, temp_size, &versal_board_info.Num_MAC_IDS, temp_size);

    temp_size = sizeof(rmi_board_info.oem_id);
    Cl_SecureMemcpy(&rmi_board_info.oem_id, temp_size, &versal_board_info.OEM_ID, temp_size);

    temp_size = sizeof(rmi_board_info.memory_size);
    Cl_SecureMemcpy(&rmi_board_info.memory_size, temp_size, &versal_board_info.Memory_size, temp_size);

    temp_size = sizeof(rmi_board_info.dimm_size);
    Cl_SecureMemcpy(&rmi_board_info.dimm_size, temp_size, &versal_board_info.DIMM_size, temp_size);

    temp_size = sizeof(rmi_board_info.product_name);
    Cl_SecureMemcpy(&rmi_board_info.product_name, temp_size, &versal_board_info.product_name, temp_size);

    if(eRMI_SUCCESS != lRmi_Write_Board_Info(&rmi_board_info, sizeof(rmi_board_info_t))){
        VMR_ERR(" RMI Write Board Info Failed! \n\r");
        ret_val = -1;
    }

    return ret_val;
}
/*
*  @brief Initializes and configures RMI sensors. It is called from vmc_main
*  @return 0 as success
*/
static s8 rmi_sensor_init(void)
{

    s8 ret_val = 0;

    cl_rmi_init_sensors();

    if(eRMI_SUCCESS != lRmi_Configure_Sensors(cl_rmi_p_sensors, cl_rmi_dynamic_sensors_count))
    {
        VMR_ERR(" RMI Sensor Configuration Failed! \n\r");
        ret_val = -1;
    }

    stream_buffer_item_size = sizeof(sensors_ds_t) * cl_rmi_dynamic_sensors_count;
    xStreamBuffer_rmi_sensor = xStreamBufferCreate( stream_buffer_item_size * STREAM_BUF_MAX_ITEMS, stream_buffer_item_size );
    configASSERT(NULL != xStreamBuffer_rmi_sensor);

    return ret_val;
}

/*
*  @brief Calls initializes rmi library. It is called from cl_main
*  @return
*/
int cl_rmi_init(void)
{
    rmi_config_t rmi_config = { .rmi_malloc_fptr = pvPortMalloc, .rmi_request_fptr = xRmi_Request_Handler, .rmi_free_fptr = vPortFree,
                                .rmi_memcpy_fptr = Cl_SecureMemcpy, .rmi_memset_fptr = Cl_SecureMemset, .rmi_memcmp_fptr = Cl_SecureMemcmp,
                                .rmi_memmove_fptr = Cl_SecureMemmove, .rmi_strncpy_fptr = Cl_SecureStrncpy, .rmi_strncmp_fptr = Cl_SecureStrncmp,
                                .task_delay = vTaskDelay };

    lRmi_Init(rmi_config);

    is_rmi_ready = true;

    VMR_LOG("RMI Init Done.");

    return 0;
}


/*
*  @brief Task function for RMI thread
*  @param task_args
*  @return
*/
void cl_rmi_func(void *task_args)
{
    
    sensors_ds_t* updated_sensors = NULL;
    updated_sensors = (sensors_ds_t*)pvPortMalloc(stream_buffer_item_size);
    u32 bytes_received = 0;

    VMR_LOG("RMI Started");
    
    while (1){

        Cl_SecureMemset(updated_sensors, 0, stream_buffer_item_size);

        bytes_received = 0;

        configASSERT(NULL != xStreamBuffer_rmi_sensor);
    
        bytes_received = xStreamBufferReceive(xStreamBuffer_rmi_sensor, updated_sensors, stream_buffer_item_size, portMAX_DELAY) ;

        if(bytes_received == stream_buffer_item_size){
            /*
             * cl_rmi_p_sensors is accessed inside interrupt callbacks so we need to disable interrupt before updating it.
             * */
            portENTER_CRITICAL();
            cl_rmi_update_rmi_sensor_values(updated_sensors);
            portEXIT_CRITICAL();

        }
        else {
            VMC_ERR("RMI sensor stream buffer copy failed bytes_received: %d item_size: %d", bytes_received, stream_buffer_item_size);
        }

        /* Call RMI Task here. */
        vRmi_Task_Func();

        /* every 100ms we should check hardware status */
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    
    if(NULL != updated_sensors)
        vPortFree(updated_sensors);
    
    updated_sensors = NULL;

}

/*
*  @brief Copies updated sensor values to RMI sensor memory space
*  @param p_vmc_sensors: pointer to updated sensors values received from stream buffer
*  @return void
*/
static void cl_rmi_update_rmi_sensor_values(sensors_ds_t* p_vmc_sensors)
{

    configASSERT( (NULL != p_vmc_sensors) && (NULL != cl_rmi_p_sensors));

    Cl_SecureMemcpy(cl_rmi_p_sensors, stream_buffer_item_size, p_vmc_sensors, stream_buffer_item_size);

}

/*
*  @brief Initializes and configures RMI Sensor Struct
*  @return void
*/
static void cl_rmi_init_sensors(void)
{

    sensors_ds_t* p_vmc_sensors = NULL;

    cl_rmi_sensors_count = Get_Asdm_Total_Sensor_Count();
    cl_rmi_dynamic_sensors_count = Get_Asdm_Dynamic_Sensor_Count();
    
    VMR_LOG("RMI dynamic sensor count: %d", cl_rmi_dynamic_sensors_count);

    p_vmc_sensors = Vmc_Init_RMI_Sensor_Buffer(cl_rmi_dynamic_sensors_count);
    cl_rmi_p_sensors = (sensors_ds_t*)pvPortMalloc(cl_rmi_dynamic_sensors_count * sizeof(sensors_ds_t));

    for(int idx = 0; idx < cl_rmi_dynamic_sensors_count; idx++){
            cl_rmi_p_sensors[idx].id[0] = idx + 1; 
            p_vmc_sensors[idx].id[0] = idx + 1; 
    }

}

/*
*  @brief De-initializes RMI
*  @return void
*/
void cl_rmi_exit(void)
{
    // TODO: release allocated memory and clean up

    if(NULL != cl_rmi_p_sensors)
        vPortFree(cl_rmi_p_sensors);

    cl_rmi_p_sensors = NULL;
    cl_rmi_sensors_count = 0;
    cl_rmi_dynamic_sensors_count = 0;
    stream_buffer_item_size = 0;
}

u8 cl_rmi_is_ready(void)
{
    return is_rmi_ready;
}


#endif
