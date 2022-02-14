/******************************************************************************
 * Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
 * SPDX-License-Identifier: MIT
 *******************************************************************************/

#include "FreeRTOS.h"
#include "task.h"

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
#include "sysmon.h"
#include "vmc_sc_comms.h"
#include "vmc_update_sc.h"
#include "vmr_common.h"

extern TaskHandle_t xSensorMonTask;
extern XSysMonPsv InstancePtr;
extern XScuGic IntcInst;
extern SemaphoreHandle_t vmc_sc_lock;

extern SC_VMC_Data sc_vmc_data;

extern uint8_t sc_update_flag;

/*Xgq Msg Handle */
static u8 xgq_sensor_flag = 0;
msg_handle_t *sensor_hdl;
extern s8 Asdm_Process_Sensor_Request(u8 *req, u8 *resp, u16 *respSize);

#define MAX6639_FAN_TACHO_TO_RPM(x) (8000*60)/(x*2)

Versal_sensor_readings sensor_readings;
u8 i2c_num = 1;  // LPD_I2C0
#define LPD_I2C_0	0x1

void clear_clock_shutdown_status()
{
    /* shutdown state can be cleared by writing a ‘1’ followed by a ‘0’ to bit 16
     * But this requires a hardware change which hasn't been applied to the hardware yet
     * The only way to clear shutdown state bit is by reloading the FPGA (Hot reset or cold reset)
     */

    u32 shutdownSatus = IO_SYNC_READ32(XPAR_BLP_BLP_LOGIC_ULP_CLOCKING_GAPPING_DEMAND_GPIO_GAPPING_DEMAND_BASEADDR);
    shutdownSatus = (shutdownSatus | (1 << 0xF));
    IO_SYNC_WRITE32(shutdownSatus, XPAR_BLP_BLP_LOGIC_ULP_CLOCKING_GAPPING_DEMAND_GPIO_GAPPING_DEMAND_BASEADDR);
    shutdownSatus = (shutdownSatus &(~ (1 << 0xF)));
    IO_SYNC_WRITE32(shutdownSatus, XPAR_BLP_BLP_LOGIC_ULP_CLOCKING_GAPPING_DEMAND_GPIO_GAPPING_DEMAND_BASEADDR);
    return;
}

void ucs_clock_shutdown()
{
    u32 shutdownSatus = 0 ;

    // offset for clock shutdown
    u32 originalValue = IO_SYNC_READ32(EP_UCS_CONTROL);

    // clear 23:4 bits
    u32 triggerValue = originalValue & 0xFF00000F;

    //to trigger clock shutdown write 0x1B632 at [23:4] bits
    triggerValue = triggerValue | 0x001B6320;

     //the bits can be immediately cleared back to 0, as the shutdown state is latched by the hardware
    IO_SYNC_WRITE32(triggerValue, EP_UCS_CONTROL) ;
    IO_SYNC_WRITE32(originalValue, EP_UCS_CONTROL) ;

    //offset to read shutdown status
    shutdownSatus = IO_SYNC_READ32(XPAR_BLP_BLP_LOGIC_ULP_CLOCKING_UCS_CONTROL_STATUS_GPIO_UCS_CONTROL_STATUS_BASEADDR);

    if(shutdownSatus & 0x01)
    	VMC_ERR("Clock shutdown due to power value reached to critical threshold \n\r");

    /*
     * To-DO : Log clock shutdown event to dmesg
     */
   return;
}

s8 Temperature_Read_Inlet(snsrRead_t *snsrData)
{
	s8 status = XST_FAILURE;
	s16 tempValue = 0;

	status = SE98A_ReadTemperature(LPD_I2C_0, SLAVE_ADDRESS_SE98A_0, &tempValue);
	if (status == XST_SUCCESS)
	{
		memcpy(snsrData->snsrValue,&tempValue,sizeof(tempValue));
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
		memcpy(snsrData->snsrValue,&tempValue,sizeof(tempValue));
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
		memcpy(&snsrData->snsrValue[0],&TempReading,sizeof(TempReading));
		snsrData->sensorValueSize = sizeof(TempReading);
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

	status = XSysMonPsv_ReadTempProcessed(&InstancePtr, SYSMONPSV_TEMP_MAX, &TempReading);
	if (status == XST_SUCCESS)
	{
		memcpy(&snsrData->snsrValue[0],&TempReading,sizeof(TempReading));
		snsrData->sensorValueSize = sizeof(TempReading);
		snsrData->snsrSatus = Vmc_Snsr_State_Normal;
	}
	else
	{
		snsrData->snsrSatus = Vmc_Snsr_State_Comms_failure;
		VMC_DBG("Failed to read Sysmon : %d \n\r",SYSMONPSV_TEMP_MAX);
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
		memcpy(snsrData->snsrValue,&avgFanRPM,sizeof(avgFanRPM));
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

	memcpy(&snsrData->snsrValue[0],&TempReading,sizeof(TempReading));
	snsrData->sensorValueSize = sizeof(TempReading);

	if (status == XST_SUCCESS)
	{
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
		VMC_DBG("QSFP_%d module not present \n\r",(snsrData->sensorInstance-1));
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
        memcpy(&snsrData->snsrValue[0],&sensorReading,sizeof(sensorReading));
        snsrData->sensorValueSize = sizeof(sensorReading);
        snsrData->snsrSatus = Vmc_Snsr_State_Normal;
    }
    else
    {
    	memcpy(&snsrData->snsrValue[0],&sensorReading,sizeof(sensorReading));
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
    	pexPower = ((sc_vmc_data.sensor_values[PEX_12V]/1000) * (sc_vmc_data.sensor_values[PEX_12V_I_IN])/1000);
    	aux0Power = ((sc_vmc_data.sensor_values[AUX_12V]/1000) * (sc_vmc_data.sensor_values[V12_IN_AUX0_I])/1000); //2x4 AUX
    	aux1Power = ((sc_vmc_data.sensor_values[AUX1_12V]/1000) * (sc_vmc_data.sensor_values[V12_IN_AUX1_I])/1000); //2x3 AUX
    	totalPower = (pexPower + aux0Power +aux1Power);
    	xSemaphoreGive(vmc_sc_lock);
    }
    else
    {
    	VMC_ERR("vmc_sc_lock lock failed \r\n");
    }

    if(totalPower != 0)
    {
        memcpy(&snsrData->snsrValue[0],&totalPower,sizeof(totalPower));
        snsrData->sensorValueSize = sizeof(totalPower);
        snsrData->snsrSatus = Vmc_Snsr_State_Normal;
    }
    else
    {
        memcpy(&snsrData->snsrValue[0],&totalPower,sizeof(totalPower));
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
	if (XSysMonPsv_ReadTempProcessed(&InstancePtr, SYSMONPSV_TEMP_MAX, &TempReading))
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

	for (snsrIndex = 1 ; snsrIndex <= QSFP_TEMPERATURE_SENSOR_NUM ; snsrIndex++)
	{
		status = QSFP_ReadTemperature(&TemperatureValue,snsrIndex);

		sensor_readings.qsfp_temp[snsrIndex-1] = TemperatureValue;

		if (status == XST_FAILURE)
		{
			VMC_PRNT("\n\r Failed to read QSFP_%d temp \n\r",(snsrIndex-1));
		}
		if (status == XST_DEVICE_NOT_FOUND)
		{
			//VMC_PRNT("QSFP_%d module not present",(snsrIndex-1));
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

static int xgq_sensor_cb(cl_msg_t *msg, void *arg)
{
    u32 address = RPU_SHARED_MEMORY_ADDR(msg->sensor_payload.address);
    u32 size = msg->sensor_payload.size;
    u8 reqBuffer[2] = {0};
    u8 respBuffer[512] = {0};
    u16 respSize = 0;
    s32 ret = 0;

    reqBuffer[0] = msg->sensor_payload.aid;
    reqBuffer[1] = msg->sensor_payload.sid;
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

    if(Init_Asdm())
    {
         VMC_ERR(" ASDM Init Failed \n\r");
    }

    for(;;)
    {
        /* Read All Sensors */
    	if(!sc_update_flag)
    	{
    		Monitor_Sensors();
    		Monitor_Thresholds();

#ifdef VMC_TEST
    		se98a_monitor();
    		max6639_monitor();
    		sysmon_monitor();
    		qsfp_monitor ();
#endif
    		vTaskDelay(200);
    	}
    	else
    	{
    		/* Wait for SC update complete ~20Sec*/
    		vTaskDelay(DELAY_MS(1000 * 20));
    	}

    }

    vTaskSuspend(NULL);
}
