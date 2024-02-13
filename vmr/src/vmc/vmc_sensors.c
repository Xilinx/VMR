/******************************************************************************
 * Copyright (C) 2020-2022 Xilinx, Inc.  All rights reserved.
 * SPDX-License-Identifier: MIT
 *******************************************************************************/

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "stream_buffer.h"

#include "cl_vmc.h"
#include "cl_mem.h"
#include "cl_uart_rtos.h"
#include "cl_i2c.h"
#include "cl_log.h"
#include "cl_msg.h"
#include "vmc_api.h"
#include "sensors/inc/se98a.h"
#include "sensors/inc/max6639.h"
#include "sensors/inc/qsfp.h"
#include "vmc_sensors.h"
#include "vmc_asdm.h"
#include "xsysmonpsv.h"
#include "vmc_sc_comms.h"
#include "vmc_update_sc.h"
#include "vmr_common.h"
#include "vmc_main.h"
#include "clock_throttling.h"

#include "platforms/vck5000.h"
#include "platforms/v70.h"
#include "platforms/v80.h"

#include "cl_config.h"

#ifdef BUILD_FOR_RMI
#include "cl_rmi.h"
#endif

#define BITMASK_TO_CLEAR    0xFFF0000F
#define ENABLE_FORCE_SHUTDOWN   0x000DB190

#define WATTS_TO_MICROWATTS (1000000)
static int vmc_sysmon_is_ready = 0;
/* TODO: init those to a certain value */
static XSysMonPsv InstancePtr;
static XScuGic IntcInst;

/* TODO: make those static if not sharing with other files */
sensorMonitorFunc Temperature_Read_Inlet_Ptr;
sensorMonitorFunc Temperature_Read_Outlet_Ptr;
sensorMonitorFunc Temperature_Read_Board_Ptr;
sensorMonitorFunc Temperature_Read_QSFP_Ptr;
sensorMonitorFunc Temperature_Read_VCCINT_Ptr;
sensorMonitorFunc Power_Read_Ptr;

snsrNameFunc Temperature_Read_Ptr;
snsrNameFunc Voltage_Read_Ptr;
snsrNameFunc Current_Read_Ptr;
snsrNameFunc QSFP_Read_Ptr;

/* VCK5000 Specific sensors */
#define VCK5000_TEMPERATURE_SENSORS_INSTANCES   ( 0 )
#define VCK5000_CURRENT_SENSORS_INSTANCES       ( 3 )
#define VCK5000_VOLTAGE_SENSORS_INSTANCES       ( 5 )

/* V70 Specific sensors */
#define V70_TEMPERATURE_SENSORS_INSTANCES   ( 0 )
#define V70_CURRENT_SENSORS_INSTANCES       ( 3 )
#define V70_VOLTAGE_SENSORS_INSTANCES       ( 3 )

/* V80 Specific sensors */
#define V80_TEMPERATURE_SENSORS_INSTANCES   ( 3 )
#define V80_CURRENT_SENSORS_INSTANCES       ( 10 )
#define V80_VOLTAGE_SENSORS_INSTANCES       ( 10 )

#define MIN_TEMPERATURE_SENSORS_INSTANCES   ( 0 )
#define MIN_CURRENT_SENSORS_INSTANCES       ( 3 )
#define MIN_VOLTAGE_SENSORS_INSTANCES       ( 3 )

platform_sensors_monitor_ptr Monitor_Sensors;

extern SemaphoreHandle_t vmc_sc_lock;

extern Clock_Throttling_Handle_t  clock_throttling_std_algorithm;
extern SC_VMC_Data sc_vmc_data;
extern SDR_t *sdrInfo;

Vmc_Sensors_Gl_t sensor_glvr = {
    .logging_level = VMC_LOG_LEVEL_NONE,
    .clk_throttling_enabled = 0,
};

clk_throttling_params_t g_clk_throttling_params;

#ifdef BUILD_FOR_RMI
sensors_ds_t* p_vmc_rmi_sensors = NULL;
u32 rmi_sensor_count = 0;
static void Vmc_Push_Sensors_To_RMI_Q(StreamBufferHandle_t *q_handle);
extern StreamBufferHandle_t xStreamBuffer_rmi_sensor;
#endif

int cl_vmc_clk_throttling_enable()
{
    VMC_LOG("set enabled to 1");
    sensor_glvr.clk_throttling_enabled = 1;
    return 0;
}


int cl_vmc_clk_throttling_disable()
{
    u32 ep_gapping = IO_SYNC_READ32(VMR_EP_GAPPING_DEMAND);

    /* write 0 on gapping demand */
    IO_SYNC_WRITE32(ep_gapping & ~MASK_GAPPING_DEMAND_CONTROL, VMR_EP_GAPPING_DEMAND);

    VMC_LOG("set enabled to 0");
    sensor_glvr.clk_throttling_enabled = 0;
    return 0;
}
/*
 * To avoid PCIE link error, Clock shutdown implimentation is changed.
 * Instead of complete clock shutdown , now we are reducing clock speed to 5%
 * by updating lower clocking speed to gapping registers.
 *
 * Step1: Write bit[20] to 0x1 in the Gapping Demand Control register at offset 0x0000
 *    This will enable the feature to throttle the clock on a clock shutdown event to a max of 25%, rather than 
 *    stopping the clock(25% is bit higher, so decide to run at 5%) 
 * Step2: Write magic number(0xDB190) to 0x80031008 .
 * Step3: After writing magic number , shutdown request latched status 
 *    (Gapping Demand Status register bit[0], offset 0x0008) will be set.
 * Step4: when shutdown request latched status set ,clock shutdown message will be pushed to dmessage.
 * Step5: Hot reset by XRT.
 *
 */
void ucs_clock_shutdown()
{
#ifdef DISABLE_LIMITS_FOR_TEST
    VMR_ERR("Clock shutdown is disabled for testing.\n\r");
    return;
#else
    u32 shutdown_status = 0 ;

    u32 trigger_value = 0;

    u32 gapping_rate = REDUCE_GAPPING_DEMAND_RATE_TO_FIVE_PERCENTAGE; // Changing to 5% of 128(Max gapping rate)

    u32 originalValue = IO_SYNC_READ32(VMR_EP_GAPPING_DEMAND);
    IO_SYNC_WRITE32(originalValue| MASK_CLOCKTHROTTLING_ENABLE_THROTTLING, VMR_EP_GAPPING_DEMAND);

    // offset for clock shutdown
    originalValue = IO_SYNC_READ32(VMR_EP_UCS_SHUTDOWN);

    // clear 19:4 bits
    trigger_value = originalValue & BITMASK_TO_CLEAR;

    //to trigger clock shutdown write 0xDB190 at [19:4] bits
    trigger_value = trigger_value | ENABLE_FORCE_SHUTDOWN;

    //the bits can be immediately cleared back to 0, as the shutdown state is latched by the hardware
    IO_SYNC_WRITE32(trigger_value, VMR_EP_UCS_SHUTDOWN) ;
    IO_SYNC_WRITE32(originalValue, VMR_EP_UCS_SHUTDOWN) ;

    //offset to read shutdown status
    shutdown_status = IO_SYNC_READ32(VMR_EP_GAPPING_DEMAND + VMR_EP_UCS_CHANNEL_2);
    if(shutdown_status & SHUTDOWN_LATCHED_STATUS)
    {
        originalValue = IO_SYNC_READ32(VMR_EP_GAPPING_DEMAND);
        originalValue = originalValue & (~0xFF);
        originalValue = originalValue | gapping_rate;
        IO_SYNC_WRITE32(originalValue, VMR_EP_GAPPING_DEMAND);
        VMC_ERR("Clock shutdown due to power or temperature value reaching Critical threshold \n\r");
    }
    return;
#endif
}

s8 Temperature_Read_ACAP_Device_Sysmon(snsrRead_t *snsrData)
{
    s8 status = XST_FAILURE;
    float TempReading = 0.0;

    status = XSysMonPsv_ReadTempProcessed(&InstancePtr, XSYSMONPSV_TEMP, &TempReading);
    if (status == XST_SUCCESS)
    {
        u16 roundedOffVal = (TempReading > 0) ? TempReading : 0;
        Cl_SecureMemcpy(&snsrData->snsrValue[0],sizeof(roundedOffVal),&roundedOffVal,sizeof(roundedOffVal));
        snsrData->sensorValueSize = sizeof(roundedOffVal);
        snsrData->snsrSatus = Vmc_Snsr_State_Normal;
        sensor_glvr.sensor_readings.sysmon_max_temp = TempReading;
    }
    else
    {
        snsrData->snsrSatus = Vmc_Snsr_State_Comms_failure;
        VMC_DBG("Failed to read Sysmon : %d \n\r",XSYSMONPSV_TEMP);
    }

    if (TempReading >= TEMP_FPGA_CRITICAL_THRESHOLD)
    {
        ucs_clock_shutdown();
    }
    return status;
}

s8 VCCINT_Read_ACAP_Device_Sysmon(snsrRead_t *snsrData)
{
    s8 status = XST_FAILURE;
    float VCCINT_reading = 0.0;
    u32 VCCINT_mv = 0;

    status = XSysMonPsv_ReadSupplyProcessed(&InstancePtr, VCCINT, &VCCINT_reading);
    VCCINT_mv = VCCINT_reading *1000;

    if(VCCINT_mv != 0)
    {
        sensor_glvr.sensor_readings.voltage[eVCCINT] = VCCINT_mv;
        Cl_SecureMemcpy(&snsrData->snsrValue[0], (sizeof(u8)*4), &VCCINT_mv,sizeof(VCCINT_mv));
        snsrData->sensorValueSize = sizeof(VCCINT_mv);
        snsrData->snsrSatus = Vmc_Snsr_State_Normal;
    }
    else
    {
        sensor_glvr.sensor_readings.voltage[eVCCINT] = 0;
        Cl_SecureMemcpy(&snsrData->snsrValue[0], (sizeof(u8)*4), &VCCINT_mv,sizeof(VCCINT_mv));
        snsrData->snsrSatus = Vmc_Snsr_State_Comms_failure;
        VMC_DBG("MSP Sensor Id : %d Data read failed \n\r",snsrData->mspSensorIndex);
    }

    return status;
}

s8 PMBUS_SC_Sensor_Read(snsrRead_t *snsrData)
{
    s8 status = XST_FAILURE;
    u16 sensorReading = 0;

    if (xSemaphoreTake(vmc_sc_lock, portMAX_DELAY))
    {
        sensorReading = sc_vmc_data.sensor_values[snsrData->mspSensorIndex];
        xSemaphoreGive(vmc_sc_lock);
    }
    else
    {
        VMC_ERR("vmc_sc_lock lock failed \r\n");
    }

    if(sensorReading != 0)
    {
        Cl_SecureMemcpy(&snsrData->snsrValue[0],sizeof(sensorReading),&sensorReading,sizeof(sensorReading));
        snsrData->sensorValueSize = sizeof(sensorReading);
        snsrData->snsrSatus = Vmc_Snsr_State_Normal;
    }
    else
    {
        Cl_SecureMemcpy(&snsrData->snsrValue[0],sizeof(sensorReading),&sensorReading,sizeof(sensorReading));
        snsrData->snsrSatus = Vmc_Snsr_State_Comms_failure;
        VMC_DBG("MSP Sensor Id : %d Data read failed \n\r",snsrData->mspSensorIndex);
    }

    return status;
}

s8 PMBUS_SC_Vccint_Read(snsrRead_t *snsrData)
{
    s8 status = XST_FAILURE;
    u32 vccint_i_sensorReading = 0;


    if (xSemaphoreTake(vmc_sc_lock, portMAX_DELAY))
    {
        vccint_i_sensorReading = sc_vmc_data.VCCINT_sensor_value;
        xSemaphoreGive(vmc_sc_lock);
    }
    else
    {
        VMC_ERR("vmc_sc_lock lock failed \r\n");
    }

    if(vccint_i_sensorReading != 0)
    {
        memcpy(&snsrData->snsrValue[0],&vccint_i_sensorReading,sizeof(vccint_i_sensorReading));
        snsrData->sensorValueSize = sizeof(vccint_i_sensorReading);
        snsrData->snsrSatus = Vmc_Snsr_State_Normal;
    }
    else
    {
        memcpy(&snsrData->snsrValue[0],&vccint_i_sensorReading,sizeof(vccint_i_sensorReading));
        snsrData->snsrSatus = Vmc_Snsr_State_Comms_failure;
        VMC_DBG("MSP Sensor Id : %d Data read failed \n\r",snsrData->mspSensorIndex);
    }


    return status;
}

void sysmon_monitor(void)
{

    float TempReading = 0.0;
    if (XSysMonPsv_ReadTempProcessed(&InstancePtr, XSYSMONPSV_TEMP, &TempReading))
    {
        CL_LOG(APP_VMC, "Failed to read sysmon temperature \n\r");
        sensor_glvr.sensor_readings.sysmon_max_temp = -1.0;
        return;
    }

    sensor_glvr.sensor_readings.sysmon_max_temp = TempReading;
    return;
}

void Monitor_Thresholds()
{
    u16 sensorReading = sc_vmc_data.sensor_values[eSC_VCCINT_TEMP];

    if (sensorReading >= TEMP_VCCINT_CRITICAL_THRESHOLD)
    {
        ucs_clock_shutdown();
    }
}

#define SENSOR_RESP_BUFFER_SIZE 512
static int validate_sensor_payload(struct xgq_vmr_sensor_payload *payload)
{
    int ret = -EINVAL;
    u32 address = RPU_SHARED_MEMORY_ADDR(payload->address);

    if ((address + SENSOR_RESP_BUFFER_SIZE) >= VMR_EP_RPU_SHARED_MEMORY_END) {
        VMC_ERR("address overflow 0x%x", address);
        return ret;
    }

    return 0;
}

int cl_vmc_sensor_request(cl_msg_t *msg)
{
    u32 address = RPU_SHARED_MEMORY_ADDR(msg->sensor_payload.address);
    u32 size = msg->sensor_payload.size;
    u8 reqBuffer[3] = {0};
    u8 respBuffer[SENSOR_RESP_BUFFER_SIZE] = {0};
    u16 respSize = 0;
    s32 ret = 0;

    reqBuffer[0] = msg->sensor_payload.aid;
    reqBuffer[1] = msg->sensor_payload.sid;
    reqBuffer[2] = msg->sensor_payload.sensor_id;

    if(reqBuffer[0] == ASDM_CMD_GET_SIZE)
    {
        Asdm_Get_SDR_Size(&reqBuffer[0], &respBuffer[0], &respSize);
        msg->sensor_payload.size = respSize;
        return ret;
    }

    ret = validate_sensor_payload(&msg->sensor_payload);
    if (ret)
        goto done;

    /* Read All Sensors from cached data */
    if(Asdm_Process_Sensor_Request(&reqBuffer[0], &respBuffer[0], &respSize)) {
        VMC_ERR("ERROR: Failed to Process Sensor Request %d 0x%x",
            msg->sensor_payload.aid, msg->sensor_payload.sid);
        ret = -EINVAL;
    } else {
        if(size < respSize) {
            VMC_ERR("ERROR: Expected Size %d Actual Size: %d", size, respSize);
            ret = -EINVAL;
        } else {
            cl_memcpy_toio8(address, &respBuffer[0], respSize);
        }
    }

done:

    VMC_DBG("msg cid%d, ret %d", msg->hdr.cid, ret);
    return ret;
}

void cl_vmc_get_clk_throttling_params(clk_throttling_params_t *pParams)
{
    Cl_SecureMemcpy(pParams, sizeof(clk_throttling_params_t),
                &g_clk_throttling_params, sizeof(clk_throttling_params_t));
    return;
}

int validate_clk_scaling_payload(struct xgq_vmr_clk_scaling_payload *payload)
{
    int ret = -EINVAL;


    if (payload->aid == 0) {
        VMC_ERR("Aid %d is 0", payload->aid);
        return ret;
    }

    if((payload->scaling_en != false) && (payload->scaling_en != true)) {
        VMC_ERR("invalid Scaling Enable %d", payload->scaling_en);
        return ret;
    }

    if (payload->pwr_scaling_ovrd_limit > clock_throttling_std_algorithm.PowerThrottlingLimit) {
        VMC_ERR("invalid Power Limit %d", payload->pwr_scaling_ovrd_limit);
        return ret;
    }

    if (payload->temp_scaling_ovrd_limit > TEMP_FPGA_CRITICAL_THRESHOLD) {
        VMC_ERR("invalid Temp %d", payload->temp_scaling_ovrd_limit);
        return ret;
    }

    return 0;


}

int vmc_get_clk_throttling_status(cl_msg_t *msg)
{
    VMR_LOG("Fetching CLK throttling Status");

    return 0;
}

int vmc_set_clk_throttling_override(cl_msg_t *msg)
{
    VMR_ERR("Scaling En:%d Pwr Ord lim:%d Temp Ord lim:%d reset:%d Pwr Ord En:%d temp Ord En:%d",
            msg->clk_scaling_payload.scaling_en,
            msg->clk_scaling_payload.pwr_scaling_ovrd_limit,
            msg->clk_scaling_payload.temp_scaling_ovrd_limit,
            msg->clk_scaling_payload.reset,
            msg->clk_scaling_payload.pwr_ovrd_en,
            msg->clk_scaling_payload.temp_ovrd_en);

    g_clk_throttling_params.limits_update_req = (u8) true;
    g_clk_throttling_params.clk_scaling_enable = msg->clk_scaling_payload.scaling_en;

    /*
     * Apply limits only if Scaling is enabled.
     * If limits are sent 0 from XRT, apply default limits
     */
    if (!msg->clk_scaling_payload.reset)
    {
        if(g_clk_throttling_params.clk_scaling_enable) {

            if((msg->clk_scaling_payload.temp_ovrd_en) && (msg->clk_scaling_payload.temp_scaling_ovrd_limit > 0)) {

                g_clk_throttling_params.limits.throttle_limit_temp =
                        msg->clk_scaling_payload.temp_scaling_ovrd_limit;
                g_clk_throttling_params.temp_throttling_enabled = true;
            }

            if((msg->clk_scaling_payload.pwr_ovrd_en) && (msg->clk_scaling_payload.pwr_scaling_ovrd_limit > 0)) {

                g_clk_throttling_params.limits.throttle_limit_pwr =
                        msg->clk_scaling_payload.pwr_scaling_ovrd_limit;
                g_clk_throttling_params.power_throttling_enabled = true;
            }
        }
         else  {
                /* Clock Throttling Disabled*/
                g_clk_throttling_params.temp_throttling_enabled = false;
                g_clk_throttling_params.power_throttling_enabled = false;
                /* Set the default value*/
                g_clk_throttling_params.limits.throttle_limit_temp  = clock_throttling_std_algorithm.TempThrottlingLimit;
                g_clk_throttling_params.limits.throttle_limit_pwr = clock_throttling_std_algorithm.PowerThrottlingLimit;
            }
    } else  {

        g_clk_throttling_params.clk_scaling_enable = false;
        /* Clock Throttling Disabled*/
        g_clk_throttling_params.temp_throttling_enabled = false;
        g_clk_throttling_params.power_throttling_enabled = false;
        /* Set the default value*/
        g_clk_throttling_params.limits.throttle_limit_temp  = clock_throttling_std_algorithm.TempThrottlingLimit;
        g_clk_throttling_params.limits.throttle_limit_pwr = clock_throttling_std_algorithm.PowerThrottlingLimit;
    }

    return 0;
}

int cl_vmc_clk_scaling(cl_msg_t *msg)
{
    int ret = 0;

    ret = validate_clk_scaling_payload(&msg->clk_scaling_payload);
    if (ret)
        goto done;

    switch (msg->clk_scaling_payload.aid) {
    case CL_CLK_SCALING_READ:
        ret = vmc_get_clk_throttling_status(msg);
        break;
    case CL_CLK_SCALING_SET:
        ret = vmc_set_clk_throttling_override(msg);
        break;
    default:
        VMR_ERR("ERROR: unknown req_type %d",
            msg->clk_scaling_payload.aid);
        ret = -EINVAL;
        break;
    }
done:
    VMR_DBG("complete msg id %d, ret %d", msg->hdr.cid, ret);
    return ret;
}

s8 Temperature_Read_Inlet(snsrRead_t *snsrData)
{
    if (Temperature_Read_Inlet_Ptr == NULL)
        return XST_SUCCESS;

    return (*Temperature_Read_Inlet_Ptr)(snsrData);
}

s8 Temperature_Read_Outlet(snsrRead_t *snsrData)
{
    if (Temperature_Read_Outlet_Ptr == NULL)
        return XST_SUCCESS;

    return (*Temperature_Read_Outlet_Ptr)(snsrData);
}

s8 Temperature_Read_Board(snsrRead_t *snsrData)
{
    if (Temperature_Read_Board_Ptr == NULL)
        return XST_SUCCESS;

    return (*Temperature_Read_Board_Ptr)(snsrData);
}

s8 Temperature_Read_QSFP(snsrRead_t *snsrData)
{
    if (Temperature_Read_QSFP_Ptr == NULL)
        return XST_SUCCESS;

    return (*Temperature_Read_QSFP_Ptr)(snsrData);
}

s8 Temperature_Read_VCCINT(snsrRead_t *snsrData)
{
    if (Temperature_Read_VCCINT_Ptr == NULL)
        return XST_SUCCESS;

    return (*Temperature_Read_VCCINT_Ptr)(snsrData);
}

s8 Asdm_Read_Power(snsrRead_t *snsrData)
{
    if (NULL == Power_Read_Ptr)
        return XST_SUCCESS;

    return (*Power_Read_Ptr)(snsrData);
}

s8 getVoltagesName(u8 index, char8* snsrName, u8 *sensorId,sensorMonitorFunc *sensor_handler)
{
    if (NULL == Voltage_Read_Ptr)
        return XST_SUCCESS;

    return (*Voltage_Read_Ptr)(index,snsrName,sensorId,sensor_handler);
}

s8 getCurrentNames(u8 index, char8* snsrName, u8 *sensorId,sensorMonitorFunc *sensor_handler)
{
    if (NULL == Current_Read_Ptr)
        return XST_SUCCESS;

    return (*Current_Read_Ptr)(index,snsrName,sensorId,sensor_handler);
}

s8 getQSFPName(u8 index, char8* snsrName, u8 *sensorId,sensorMonitorFunc *sensor_handler)
{
    if (NULL == QSFP_Read_Ptr)
        return XST_SUCCESS;

    return (*QSFP_Read_Ptr)(index,snsrName,sensorId,sensor_handler);
}

s8 scGetTemperatureName(u8 ucIndex, char8* pucSnsrName, u8 *pucSensorId, sensorMonitorFunc *pxSensorHandler)
{
    if ( NULL == Temperature_Read_Ptr )
        return XST_SUCCESS;

    return ( *Temperature_Read_Ptr )( ucIndex, pucSnsrName, pucSensorId, pxSensorHandler );
}

u8 getVoltageSensorNum()
{
    if ( Vmc_Get_PlatformType() == eVCK5000 )
    {
        return VCK5000_VOLTAGE_SENSORS_INSTANCES;
    }
    else if( Vmc_Get_PlatformType() == eV70 )
    {
        return V70_VOLTAGE_SENSORS_INSTANCES;
    }
    else if( Vmc_Get_PlatformType() == eV80 )
    {
      return V80_VOLTAGE_SENSORS_INSTANCES;
    }

    return MIN_VOLTAGE_SENSORS_INSTANCES;
}

u8 ucGetTemperatureSensorNum()
{
    if( Vmc_Get_PlatformType() == eVCK5000 )
    {
        return VCK5000_TEMPERATURE_SENSORS_INSTANCES;
    }
    else if( Vmc_Get_PlatformType() == eV70 )
    {
        return V70_TEMPERATURE_SENSORS_INSTANCES;
    }
    else if( Vmc_Get_PlatformType() == eV80 )
    {
        return V80_TEMPERATURE_SENSORS_INSTANCES;
    }

    return MIN_TEMPERATURE_SENSORS_INSTANCES;
}


u8 getCurrentSensorNum()
{
    if( Vmc_Get_PlatformType() == eVCK5000 )
    {
        return VCK5000_CURRENT_SENSORS_INSTANCES;
    }
    else if( Vmc_Get_PlatformType() == eV70 )
    {
        return V70_CURRENT_SENSORS_INSTANCES;
    }
    else if( Vmc_Get_PlatformType() == eV80 )
    {
        return V80_CURRENT_SENSORS_INSTANCES;
    }

    return MIN_CURRENT_SENSORS_INSTANCES;
}

void Clock_throttling()
{
#ifdef DISABLE_LIMITS_FOR_TEST
    VMR_ERR("Clock throttling is disabled for testing.\n\r");
    return;
#else
    /* Check if we have received update request from XRT */
    if(g_clk_throttling_params.limits_update_req) {
        clock_throttling_std_algorithm.FeatureEnabled = 
                        g_clk_throttling_params.clk_scaling_enable;

        /* Update override status */
        clock_throttling_std_algorithm.bUserThrottlingTempLimitEnabled = 
                    g_clk_throttling_params.temp_throttling_enabled;
        clock_throttling_std_algorithm.PowerOverRideEnabled = 
                    g_clk_throttling_params.power_throttling_enabled;
        
        /* Update the Override limits */
        clock_throttling_std_algorithm.XRTSuppliedUserThrottlingTempLimit = 
                    g_clk_throttling_params.limits.throttle_limit_temp;
        clock_throttling_std_algorithm.XRTSuppliedBoardThrottlingThresholdPower = 
                    (g_clk_throttling_params.limits.throttle_limit_pwr * WATTS_TO_MICROWATTS);

        /* Update done, clear the flag */
        g_clk_throttling_params.limits_update_req = false;
    }

    clock_throttling_algorithm_power(&clock_throttling_std_algorithm);
    clock_throttling_algorithm_temperature(&clock_throttling_std_algorithm);
#endif
}

void cl_vmc_monitor_sensors()
{
    if(NULL != Monitor_Sensors) {
        Monitor_Sensors();
    }
        Monitor_Thresholds();

        Asdm_Update_Sensors();
        
#ifdef BUILD_FOR_RMI
        Vmc_Push_Sensors_To_RMI_Q(&xStreamBuffer_rmi_sensor);
#endif

    if (sensor_glvr.clk_throttling_enabled)
        Clock_throttling();

#ifdef VMC_TEST
    se98a_monitor();
    max6639_monitor();
    sysmon_monitor();
    qsfp_monitor ();
#endif
#ifdef VMC_TEST_V70
    sysmon_monitor();
#endif
}

int cl_vmc_sysmon_is_ready()
{
    if (!vmc_sysmon_is_ready)
        VMC_ERR("vmc sysmon is not ready");

    return vmc_sysmon_is_ready;
}

int cl_vmc_sysmon_init()
{
    vmc_sysmon_is_ready = (XSysMonPsv_Init(&InstancePtr, &IntcInst) == XST_SUCCESS) ? 1 : 0;

    return cl_vmc_sysmon_is_ready();
}

u8 cl_clk_throttling_enabled_or_disabled (){

    return g_clk_throttling_params.clk_scaling_enable;
}

#ifdef BUILD_FOR_RMI

static void Vmc_Push_Sensors_To_RMI_Q(StreamBufferHandle_t *q_handle){

    configASSERT(NULL != q_handle );

    u32 bytes_sent = 0;
    u32 buffer_item_size = rmi_sensor_count * sizeof(sensors_ds_t);


    bytes_sent = xStreamBufferSend( *q_handle, p_vmc_rmi_sensors, buffer_item_size, pdMS_TO_TICKS(25) );
    if(bytes_sent != buffer_item_size){
        VMC_ERR("RMI sensor queue update failed! bytes_sent: %d size: %d", bytes_sent, buffer_item_size);
    }

}

sensors_ds_t* Vmc_Init_RMI_Sensor_Buffer(u32 counts){

    if(NULL != p_vmc_rmi_sensors)
        return p_vmc_rmi_sensors;

    p_vmc_rmi_sensors = (sensors_ds_t*)pvPortMalloc(counts * sizeof(sensors_ds_t));
    configASSERT(NULL != p_vmc_rmi_sensors);

    rmi_sensor_count = counts;

    return p_vmc_rmi_sensors;
}

#endif


