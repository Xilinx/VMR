/******************************************************************************
 * Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
 * SPDX-License-Identifier: MIT
 *******************************************************************************/

#include "FreeRTOS.h"
#include "task.h"

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

extern TaskHandle_t xSensorMonTask;
extern XSysMonPsv InstancePtr;
extern XScuGic IntcInst;
extern SemaphoreHandle_t vmc_sc_lock;
extern SemaphoreHandle_t vmc_sensor_monitoring_lock;


extern SC_VMC_Data sc_vmc_data;

extern uint8_t sc_update_flag;

/*Xgq Msg Handle */
static u8 xgq_sensor_flag = 0;
msg_handle_t *sensor_hdl;
extern s8 Asdm_Process_Sensor_Request(u8 *req, u8 *resp, u16 *respSize);

#define MAX6639_FAN_TACHO_TO_RPM(x) (8000*60)/(x)

Versal_sensor_readings sensor_readings;
u8 i2c_num = 1;  // LPD_I2C0
#define LPD_I2C_0	0x1
#define BITMASK_TO_CLEAR	0xFF00000F
#define ENABLE_FORCE_SHUTDOWN	0x001B6320

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

s8 Temperature_Read_Inlet(snsrRead_t *snsrData)
{
	s8 status = XST_FAILURE;
	s16 tempValue = 0;

	status = SE98A_ReadTemperature(LPD_I2C_0, SLAVE_ADDRESS_SE98A_0, &tempValue);
	if (status == XST_SUCCESS)
	{
		Cl_SecureMemcpy(snsrData->snsrValue,sizeof(tempValue),&tempValue,sizeof(tempValue));
		snsrData->sensorValueSize = sizeof(tempValue);
		snsrData->snsrSatus = Vmc_Snsr_State_Normal;
	}
	else
	{
		snsrData->snsrSatus = Vmc_Snsr_State_Comms_failure;
		VMC_DBG("Failed to read slave : %d \n\r",SLAVE_ADDRESS_SE98A_0);
	}

	return status;
}

s8 Temperature_Read_Outlet(snsrRead_t *snsrData)
{
	s8 status = XST_FAILURE;
	s16 tempValue = 0;

	status = SE98A_ReadTemperature(LPD_I2C_0, SLAVE_ADDRESS_SE98A_1, &tempValue);
	if (status == XST_SUCCESS)
	{
		Cl_SecureMemcpy(snsrData->snsrValue,sizeof(tempValue),&tempValue,sizeof(tempValue));
		snsrData->sensorValueSize = sizeof(tempValue);
		snsrData->snsrSatus = Vmc_Snsr_State_Normal;
	}
	else
	{
		snsrData->snsrSatus = Vmc_Snsr_State_Comms_failure;
		VMC_DBG("Failed to read slave : %d \n\r",SLAVE_ADDRESS_SE98A_1);
	}

	return status;
}

s8 Temperature_Read_Board(snsrRead_t *snsrData)
{
	s8 status = XST_FAILURE;
	float TempReading = 0;

	status = max6639_ReadDDRTemperature(i2c_num, SLAVE_ADDRESS_MAX6639, &TempReading);
	if (status == XST_SUCCESS)
	{
		u16 roundedOffVal = (TempReading > 0) ? TempReading : 0;
		Cl_SecureMemcpy(&snsrData->snsrValue[0],sizeof(roundedOffVal),&roundedOffVal,sizeof(roundedOffVal));
		snsrData->sensorValueSize = sizeof(roundedOffVal);
		snsrData->snsrSatus = Vmc_Snsr_State_Normal;
	}
	else
	{
		snsrData->snsrSatus = Vmc_Snsr_State_Comms_failure;
		VMC_DBG("Failed to read slave : %d \n\r",SLAVE_ADDRESS_MAX6639);
	}

	return status;
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

s8 Fan_RPM_Read(snsrRead_t *snsrData)
{
	s8 status = XST_FAILURE;
	u8 fanSpeed = 0;
	u16 fanRPM1 = 0;
	u16 fanRPM2 = 0;
	u16 avgFanRPM = 0;


	status = max6639_ReadFanTach(i2c_num, SLAVE_ADDRESS_MAX6639, 1, &fanSpeed);
	fanRPM1 = MAX6639_FAN_TACHO_TO_RPM(fanSpeed);

	fanSpeed = 0;
	status = max6639_ReadFanTach(i2c_num, SLAVE_ADDRESS_MAX6639, 2, &fanSpeed);
	fanRPM2 = MAX6639_FAN_TACHO_TO_RPM(fanSpeed);

	avgFanRPM = (fanRPM1 + fanRPM2)/2;
	if (status == XST_SUCCESS)
	{
		Cl_SecureMemcpy(snsrData->snsrValue,sizeof(avgFanRPM),&avgFanRPM,sizeof(avgFanRPM));
		snsrData->sensorValueSize = sizeof(avgFanRPM);
		snsrData->snsrSatus = Vmc_Snsr_State_Normal;
	}
	else
	{
		snsrData->snsrSatus = Vmc_Snsr_State_Comms_failure;
		VMC_DBG("Failed to  read  Fan Speed from slave : 0x%x \n\r",SLAVE_ADDRESS_MAX6639);
	}

	return status;
}
s8 Temperature_Read_QSFP(snsrRead_t *snsrData)
{
	u8 status = XST_FAILURE;
	float TempReading = 0.0;

	static bool is_qsfp_critical_threshold_reached = false;
	status = QSFP_ReadTemperature(&TempReading, snsrData->sensorInstance);

	if (status == XST_SUCCESS)
	{
		u16 roundedOffVal = (TempReading > 0) ? TempReading : 0;
		Cl_SecureMemcpy(&snsrData->snsrValue[0],sizeof(roundedOffVal),&roundedOffVal,sizeof(roundedOffVal));
		snsrData->sensorValueSize = sizeof(roundedOffVal);
		snsrData->snsrSatus = Vmc_Snsr_State_Normal;
	}
	else if (status == XST_FAILURE)
	{
		snsrData->snsrSatus = Vmc_Snsr_State_Comms_failure;
		VMC_ERR("Failed to read slave : %d \n\r",QSFP_SLAVE_ADDRESS);
	}
	else
	{
		snsrData->snsrSatus = Vmc_Snsr_State_Unavailable;
		VMC_DBG("QSFP_%d module not present \n\r",(snsrData->sensorInstance));
	}

	if (TempReading >= TEMP_QSFP_CRITICAL_THRESHOLD)
	{
		ucs_clock_shutdown();
		is_qsfp_critical_threshold_reached = true;

	}
	if((is_qsfp_critical_threshold_reached == true) && 
		(TempReading <  TEMP_QSFP_CRITICAL_THRESHOLD ))
	{
		clear_clock_shutdown_status();
		is_qsfp_critical_threshold_reached = false;
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

s8 Power_Monitor(snsrRead_t *snsrData)
{
    s8 status = XST_SUCCESS;
    u8 power_mode = 0;
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
    	power_mode =  sc_vmc_data.availpower;
    	pexPower = ((sc_vmc_data.sensor_values[PEX_12V]/1000.0) * (sc_vmc_data.sensor_values[PEX_12V_I_IN])/1000.0);
    	aux0Power = ((sc_vmc_data.sensor_values[AUX_12V]/1000.0) * (sc_vmc_data.sensor_values[V12_IN_AUX0_I])/1000.0); //2x4 AUX
    	aux1Power = ((sc_vmc_data.sensor_values[AUX1_12V]/1000.0) * (sc_vmc_data.sensor_values[V12_IN_AUX1_I])/1000.0); //2x3 AUX
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

    if (power_mode == POWER_MODE_300W) // Both 2x3 and 2x4 AUX connected
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

    if (power_mode == POWER_MODE_300W)
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

void se98a_monitor(void)
{
	u8 i = 0;
	u8 status = XST_FAILURE;
	for (i = 0 ; i < BOARD_TEMPERATURE_SENSOR_NUM ; i++)
	{
		status = SE98A_ReadTemperature(i2c_num, SLAVE_ADDRESS_SE98A_0 + i, &sensor_readings.board_temp[i]);
		if (status == XST_FAILURE)
		{
			VMC_DBG("Failed to read SE98A_%d \n\r",i);
		}
	}
	return;
}
void max6639_monitor(void)
{

	u8 status = XST_FAILURE;
	float TempReading = 0;
	u8 fanSpeed = 0;
    	u16 fanRpm1 = 0;
    	u16 fanRpm2 = 0;

	status = max6639_ReadFPGATemperature(i2c_num, SLAVE_ADDRESS_MAX6639, &TempReading);
        if (status == XST_FAILURE)
	{
		VMC_ERR("Failed to read MAX6639 \n\r");
		return;
	}
	//CL_LOG (APP_VMC,"fpga temp %f",TempReading);
	sensor_readings.remote_temp = TempReading;

        status = max6639_ReadDDRTemperature(i2c_num, SLAVE_ADDRESS_MAX6639, &TempReading);
        if (status == XST_FAILURE)
        {
                VMC_DBG( "Failed to read MAX6639 \n\r");
                return;
        }
        //CL_LOG (APP_VMC,"local temp %f",TempReading);
	sensor_readings.local_temp = TempReading;
	
    	status = max6639_ReadFanTach(i2c_num, SLAVE_ADDRESS_MAX6639, 1, &fanSpeed);
    	fanRpm1 = MAX6639_FAN_TACHO_TO_RPM(fanSpeed);

    	fanSpeed = 0;
    	status = max6639_ReadFanTach(i2c_num, SLAVE_ADDRESS_MAX6639, 2, &fanSpeed);
   	fanRpm2 = MAX6639_FAN_TACHO_TO_RPM(fanSpeed);

    	sensor_readings.fanRpm = (fanRpm1 + fanRpm2)/2;
	
	//CL_LOG (APP_VMC,"Fan RPM %d",fanRpm);
	
	return;

}

void sysmon_monitor(void)
{

	float TempReading = 0.0;
	if (XSysMonPsv_ReadTempProcessed(&InstancePtr, XSYSMONPSV_TEMP_MAX, &TempReading))
	{
		CL_LOG(APP_VMC, "Failed to read sysmon temperature \n\r");
		sensor_readings.sysmon_max_temp = -1.0;
		return;
	}

	sensor_readings.sysmon_max_temp = TempReading;
	return;
}

void qsfp_monitor(void)
{
	u8 snsrIndex = 0;
	float TemperatureValue = 0;
	u8 status = XST_FAILURE;

	for (snsrIndex = 0; snsrIndex < QSFP_TEMPERATURE_SENSOR_NUM; snsrIndex++)
	{
		status = QSFP_ReadTemperature(&TemperatureValue,snsrIndex);

		if (status == XST_SUCCESS)
		{
			sensor_readings.qsfp_temp[snsrIndex] = TemperatureValue;
		}
		if (status == XST_FAILURE)
		{
			VMC_PRNT("\n\r Failed to read QSFP_%d temp \n\r", snsrIndex);
		}
		if (status == XST_DEVICE_NOT_FOUND)
		{
			//VMC_PRNT("QSFP_%d module not present", snsrIndex);
		}

	}
	return;
}

void Monitor_Thresholds()
{
	static bool is_vccint_temp_critical_threshold_reached = false;
	u16 sensorReading = sc_vmc_data.sensor_values[VCCINT_TEMP];

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

static int xgq_sensor_cb(cl_msg_t *msg, void *arg)
{
    u32 address = RPU_SHARED_MEMORY_ADDR(msg->sensor_payload.address);
    u32 size = msg->sensor_payload.size;
    u8 reqBuffer[2] = {0};
    u8 respBuffer[SENSOR_RESP_BUFFER_SIZE] = {0};
    u16 respSize = 0;
    s32 ret = 0;

    reqBuffer[0] = msg->sensor_payload.aid;
    reqBuffer[1] = msg->sensor_payload.sid;

    ret = validate_sensor_payload(&msg->sensor_payload);
    if (ret)
	goto done;

    if(Asdm_Process_Sensor_Request(&reqBuffer[0], &respBuffer[0], &respSize))
    {
        VMC_ERR("ERROR: Failed to Process Sensor Request %d 0x%x",
			msg->sensor_payload.aid, msg->sensor_payload.sid);
        ret = -1;
    }
    else
    {
        if(size < respSize)
        {
            VMC_ERR("ERROR: Expected Size %d Actual Size: %d", size, respSize);
            ret = -1;
        }
        else
        {
            cl_memcpy_toio8(address, &respBuffer[0], respSize);
        }
    }

done:

    msg->hdr.rcode = ret;
    VMC_DBG("complete msg id%d, ret %d", msg->hdr.cid, ret);
    cl_msg_handle_complete(msg);
    return 0;
}

void SensorMonitorTask(void *params)
{

    VMC_LOG(" Sensor Monitor Task Created !!!\n\r");

    if (xgq_sensor_flag == 0 &&
        cl_msg_handle_init(&sensor_hdl, CL_MSG_SENSOR, xgq_sensor_cb, NULL) == 0) {
        VMC_LOG("init sensor handle done.");
        xgq_sensor_flag = 1;
    }

	/* Wait for notification from VMC_SC_CommsTask */
	xTaskNotifyWait(ULONG_MAX, ULONG_MAX, NULL, portMAX_DELAY);

    for(;;)
    {
        /* Read All Sensors */
    	if(xSemaphoreTake(vmc_sensor_monitoring_lock, portMAX_DELAY) == pdTRUE)
    	{
    		Monitor_Sensors();
    		Monitor_Thresholds();

#ifdef VMC_TEST
    		se98a_monitor();
    		max6639_monitor();
    		sysmon_monitor();
    		qsfp_monitor ();
#endif
    		xSemaphoreGive(vmc_sensor_monitoring_lock);
    		vTaskDelay(200);
    	}
    }

    vTaskSuspend(NULL);
}
