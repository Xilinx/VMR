/******************************************************************************
 * Copyright (C) 2020-2022 Xilinx, Inc.  All rights reserved.
 * SPDX-License-Identifier: MIT
 *******************************************************************************/

#include "FreeRTOS.h"
#include "task.h"

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

#define BITMASK_TO_CLEAR	0xFF00000F
#define ENABLE_FORCE_SHUTDOWN	0x001B6320

static int vmc_sysmon_is_ready = 0;
/* TODO: init those to a certain value */
static XSysMonPsv InstancePtr;
static XScuGic IntcInst;

/* TODO: make those static if not sharing with other files */
sensorMonitorFunc Temperature_Read_Inlet_Ptr;
sensorMonitorFunc Temperature_Read_Outlet_Ptr;
sensorMonitorFunc Temperature_Read_Board_Ptr;
sensorMonitorFunc Temperature_Read_QSFP_Ptr;
sensorMonitorFunc Power_Read_Ptr;

platform_sensors_monitor_ptr Monitor_Sensors;

extern SemaphoreHandle_t vmc_sc_lock;

extern Clock_Throttling_Algorithm  clock_throttling_std_algorithm;
extern SC_VMC_Data sc_vmc_data;

Vmc_Sensors_Gl_t sensor_glvr = {
	.logging_level = VMC_LOG_LEVEL_NONE,
};

clk_throttling_params_t g_clk_trottling_params;

void clear_clock_shutdown_status()
{
    /* shutdown state can be cleared by writing a ‘1’ followed by a ‘0’ to bit 16
     * But this requires a hardware change which hasn't been applied to the hardware yet
     * The only way to clear shutdown state bit is by reloading the FPGA (Hot reset or cold reset)
     */

    u32 shutdownSatus = IO_SYNC_READ32(VMR_EP_GAPPING_DEMAND);
    shutdownSatus = (shutdownSatus | (1 << 0xF));
    IO_SYNC_WRITE32(shutdownSatus, VMR_EP_GAPPING_DEMAND);
    shutdownSatus = (shutdownSatus &(~ (1 << 0xF)));
    IO_SYNC_WRITE32(shutdownSatus, VMR_EP_GAPPING_DEMAND);
    return;
}

void ucs_clock_shutdown()
{
    u32 shutdown_status = 0 ;

    // offset for clock shutdown
    u32 originalValue = IO_SYNC_READ32(VMR_EP_UCS_SHUTDOWN);

    // clear 23:4 bits
    u32 triggerValue = originalValue & BITMASK_TO_CLEAR;

    //to trigger clock shutdown write 0x1B632 at [23:4] bits
    triggerValue = triggerValue | ENABLE_FORCE_SHUTDOWN;

     //the bits can be immediately cleared back to 0, as the shutdown state is latched by the hardware
    IO_SYNC_WRITE32(triggerValue, VMR_EP_UCS_SHUTDOWN) ;
    IO_SYNC_WRITE32(originalValue, VMR_EP_UCS_SHUTDOWN) ;

    //offset to read shutdown status
    shutdown_status = IO_SYNC_READ32(VMR_EP_UCS_CONTROL_STATUS_BASEADDR);

    if(shutdown_status & 0x01)
    	VMC_ERR("Clock shutdown due to power or temperature value reaching Critical threshold \n\r");

   return;
}

s8 Temperature_Read_ACAP_Device_Sysmon(snsrRead_t *snsrData)
{
	s8 status = XST_FAILURE;
	float TempReading = 0.0;

	static bool is_sysmon_critical_threshold_reached = false;

	status = XSysMonPsv_ReadTempProcessed(&InstancePtr, XSYSMONPSV_TEMP_MAX, &TempReading);
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
		VMC_DBG("Failed to read Sysmon : %d \n\r",XSYSMONPSV_TEMP_MAX);
	}

	if (TempReading >= TEMP_FPGA_CRITICAL_THRESHOLD)
	{
		ucs_clock_shutdown();
		is_sysmon_critical_threshold_reached = true;

	}
	if((is_sysmon_critical_threshold_reached == true) && 
                    (TempReading <  TEMP_FPGA_CRITICAL_THRESHOLD ))
	{
		clear_clock_shutdown_status();
		is_sysmon_critical_threshold_reached = false;
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
		Cl_SecureMemcpy(&snsrData->snsrValue[0], (sizeof(u8)*4), &VCCINT_mv,sizeof(VCCINT_mv));
		snsrData->sensorValueSize = sizeof(VCCINT_mv);
		snsrData->snsrSatus = Vmc_Snsr_State_Normal;
	}
	else
	{
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

s8 Vck5000_Asdm_Read_Power(snsrRead_t *snsrData)
{
    s8 status = XST_SUCCESS;
    u8 powerMode = 0;
    float totalPower = 0;
    float pexPower = 0;
    float aux0Power = 0;
    float aux1Power = 0;

    static u8 count_12vpex = 1;
    static u8 count_12vaux_2x4_2x3 = 1;
    static u8 count_12vaux_2x4 = 1;

    static bool is_12vpex_critical_threshold_reached = false;
    static bool is_12v_aux_2x3_2x4_critical_threshold_reached = false;
    static bool is_12v_aux_2x4_critical_threshold_reached = false;

    if (xSemaphoreTake(vmc_sc_lock, portMAX_DELAY))
    {
    	powerMode =  sc_vmc_data.powerMode;
    	pexPower = ((sc_vmc_data.sensor_values[eSC_PEX_12V]/1000.0) * (sc_vmc_data.sensor_values[eSC_PEX_12V_I_IN])/1000.0);
    	aux0Power = ((sc_vmc_data.sensor_values[eSC_AUX_12V]/1000.0) * (sc_vmc_data.sensor_values[eSC_V12_IN_AUX0_I])/1000.0); //2x4 AUX
    	aux1Power = ((sc_vmc_data.sensor_values[eSC_AUX1_12V]/1000.0) * (sc_vmc_data.sensor_values[eSC_V12_IN_AUX1_I])/1000.0); //2x3 AUX
    	totalPower = (pexPower + aux0Power +aux1Power);
    	xSemaphoreGive(vmc_sc_lock);
    }
    else
    {
    	VMC_ERR("vmc_sc_lock lock failed \r\n");
    }

    if(totalPower != 0)
    {
    	/* Adding 0.5 to round off the totalPower to nearest integer.*/
        u16 roundedOffVal = (totalPower + 0.5);
        Cl_SecureMemcpy(&snsrData->snsrValue[0],sizeof(roundedOffVal),&roundedOffVal,sizeof(roundedOffVal));
        snsrData->sensorValueSize = sizeof(roundedOffVal);
        snsrData->snsrSatus = Vmc_Snsr_State_Normal;
    }
    else
    {
        snsrData->snsrSatus = Vmc_Snsr_State_Comms_failure;
    }

    // shutdown clock only if power reached critical threshold continuously for 1sec (100ms*10)
    if (pexPower >= POWER_12VPEX_CRITICAL_THRESHOLD)
    {
    	if (count_12vpex == 10)
    	{
    		ucs_clock_shutdown();
    		is_12vpex_critical_threshold_reached = true;
    		count_12vpex = 0;
    	}
    	count_12vpex = count_12vpex + 1;
    }

    if (powerMode == POWER_MODE_300W) // Both 2x3 and 2x4 AUX connected
    {
    	if ((aux0Power >= POWER_12VAUX_2X4_CRITICAL_THRESHOLD) || (aux1Power >= POWER_12VAUX_2X3_CRITICAL_THRESHOLD))
    	{
    		if (count_12vaux_2x4_2x3 == 10)
    		{
    			ucs_clock_shutdown();
    			is_12v_aux_2x3_2x4_critical_threshold_reached = true;
    			count_12vaux_2x4_2x3 = 0;
    		}
    		count_12vaux_2x4_2x3 = count_12vaux_2x4_2x3 + 1;
    	}
    }
    else // only 2x4 connected
    {
    	if (aux0Power >= POWER_12VAUX_2X4_CRITICAL_THRESHOLD)
    	{
    		if (count_12vaux_2x4 == 10)
    		{
    			ucs_clock_shutdown();
    			is_12v_aux_2x4_critical_threshold_reached = true;
    			count_12vaux_2x4 = 0;
    		}
    		count_12vaux_2x4 = count_12vaux_2x4 + 1;
    	}
    }

    /* clear shutdown status flag if asserted before and less than critical value */

    if (pexPower < POWER_12VPEX_CRITICAL_THRESHOLD)
    {
    	if (is_12vpex_critical_threshold_reached == true)
    	{
    		clear_clock_shutdown_status();
    		is_12vpex_critical_threshold_reached = false;
    	}
    	count_12vpex = 1;
    }

    if (powerMode == POWER_MODE_300W)
    {
    	if(is_12v_aux_2x3_2x4_critical_threshold_reached == true)
    	{
    		clear_clock_shutdown_status();
    		is_12v_aux_2x3_2x4_critical_threshold_reached = false;
    	}
    	count_12vaux_2x4_2x3 = 1;
    }
    else
    {
    	if (is_12v_aux_2x4_critical_threshold_reached == true)
    	{
    		clear_clock_shutdown_status();
    		is_12v_aux_2x4_critical_threshold_reached = false;
    	}
    	count_12vaux_2x4 = 1;
    }

    return status;
}

void sysmon_monitor(void)
{

	float TempReading = 0.0;
	if (XSysMonPsv_ReadTempProcessed(&InstancePtr, XSYSMONPSV_TEMP_MAX, &TempReading))
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
	static bool is_vccint_temp_critical_threshold_reached = false;
	u16 sensorReading = sc_vmc_data.sensor_values[eSC_VCCINT_TEMP];

	if (sensorReading >= TEMP_VCCINT_CRITICAL_THRESHOLD)
	{
	    ucs_clock_shutdown();
	    is_vccint_temp_critical_threshold_reached = true;
	}
	if((is_vccint_temp_critical_threshold_reached == true) &&
		(sensorReading <  TEMP_VCCINT_CRITICAL_THRESHOLD ))
	{
	    clear_clock_shutdown_status();
	    is_vccint_temp_critical_threshold_reached = false;
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

	/* Check if the Response Buffer is greater than Request buffer */
	if (SENSOR_RESP_BUFFER_SIZE > payload->size) {
		VMC_ERR("Resp size 0x%x exceeding Req size 0x%x",SENSOR_RESP_BUFFER_SIZE, payload->size);
		return ret;
	}

	return 0;
}

int cl_vmc_sensor(cl_msg_t *msg)
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
				&g_clk_trottling_params, sizeof(clk_throttling_params_t));
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

	if (payload->pwr_scaling_ovrd_limit > POWER_THROTTLING_THRESOLD_LIMIT) {
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
	VMR_LOG("Clock Throttling Override En:%d Pwr:%d Temp:%d",
			msg->clk_scaling_payload.scaling_en,
			msg->clk_scaling_payload.pwr_scaling_ovrd_limit,
			msg->clk_scaling_payload.temp_scaling_ovrd_limit);
	
	g_clk_trottling_params.limits_update_req = (u8) true;
	g_clk_trottling_params.clk_scaling_enable = msg->clk_scaling_payload.scaling_en;
	
	/*
	 * Apply limits only if Scaling is enabled.
	 * If limits are sent 0 from XRT, apply default limits
	 */
	if(g_clk_trottling_params.clk_scaling_enable) {
		
		if(msg->clk_scaling_payload.temp_scaling_ovrd_limit > 0) {
			
			g_clk_trottling_params.limits.throttle_limit_temp = 
				msg->clk_scaling_payload.temp_scaling_ovrd_limit;
			g_clk_trottling_params.temp_throttling_enabled = true;
		}
		
		if(msg->clk_scaling_payload.pwr_scaling_ovrd_limit > 0) {
			
			g_clk_trottling_params.limits.throttle_limit_pwr = 
				msg->clk_scaling_payload.pwr_scaling_ovrd_limit;
			g_clk_trottling_params.power_throttling_enabled = true;
		}
	} else  {
		/* Clock Throttling Disabled*/
		g_clk_trottling_params.temp_throttling_enabled = false;
		g_clk_trottling_params.power_throttling_enabled = false;
		/* Set the default value*/
		g_clk_trottling_params.limits.throttle_limit_temp  = FPGA_THROTTLING_TEMP_LIMIT;
		g_clk_trottling_params.limits.throttle_limit_pwr = POWER_THROTTLING_THRESOLD_LIMIT;
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

s8 Asdm_Read_Power(snsrRead_t *snsrData)
{
	if (NULL == Power_Read_Ptr)
		return XST_SUCCESS;

	return (*Power_Read_Ptr)(snsrData);
}

void Clock_throttling()
{
	/* Check if we have received update request from XRT */
	if(g_clk_trottling_params.limits_update_req) {
		clock_throttling_std_algorithm.FeatureEnabled = 
						g_clk_trottling_params.clk_scaling_enable;

		/* Update override status */
		clock_throttling_std_algorithm.bUserThrottlingTempLimitEnabled = 
					g_clk_trottling_params.temp_throttling_enabled;
		clock_throttling_std_algorithm.PowerOverRideEnabled = 
					g_clk_trottling_params.power_throttling_enabled;
		
		/* Update the Override limits */
		clock_throttling_std_algorithm.XRTSuppliedUserThrottlingTempLimit = 
					g_clk_trottling_params.limits.throttle_limit_temp;
		clock_throttling_std_algorithm.XRTSuppliedBoardThrottlingThresholdPower = 
					g_clk_trottling_params.limits.throttle_limit_pwr;

		/* Update done, clear the flag */
		g_clk_trottling_params.limits_update_req = false;
	}

	clock_throttling_algorithm_power(&clock_throttling_std_algorithm);
	clock_throttling_algorithm_temperature(&clock_throttling_std_algorithm);
}

void cl_vmc_monitor_sensors()
{
	if(NULL != Monitor_Sensors) {
		Monitor_Sensors();
	}
    	Monitor_Thresholds();

    	Asdm_Update_Sensors();
    	Clock_throttling();

#ifdef VMC_TEST
    	se98a_monitor();
    	max6639_monitor();
    	sysmon_monitor();
    	qsfp_monitor ();
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


