
/******************************************************************************
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#include "cl_uart_rtos.h"
#include "cl_log.h"
#include "cl_mem.h"
#include "cl_i2c.h"
#include "m24c128.h"
#include <stdio.h>

/* FreeRTOS includes */
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>

#include "vmc_api.h"
#include "vmc_sensors.h"
#include "vmc_main.h"

#define MAX_FILE_NAME_SIZE 25

#define EEPROM_BUF_SIZE 128

extern uart_rtos_handle_t uart_log;

/*
 * This is the example code of migrating all global variables into an unified
 * vmc_global_variable structure, so that we can convert this into MPU supported
 * framework.
 */
extern Vmc_Sensors_Gl_t sensor_glvr;

SemaphoreHandle_t vmc_debug_logbuf_lock; /* used to block until LogBuf is in use */
Versal_BoardInfo board_info;

static char LogBuf[MAX_LOG_SIZE];

void Debug_Printf(char *filename, u32 line, u8 log_level, const char *fmt, va_list *argp);

EEPROM_Content_Details_t Ver_3_0_Offset_Size[eEeprom_max_Offset] =
{
	{EEPROM_V3_0_PRODUCT_NAME_OFFSET, EEPROM_V3_0_PRODUCT_NAME_SIZE},
	{EEPROM_V3_0_BOARD_REV_OFFSET, EEPROM_V3_0_BOARD_REV_SIZE},
	{EEPROM_V3_0_BOARD_SERIAL_OFFSET, EEPROM_V3_0_BOARD_SERIAL_SIZE},
	{EEPROM_V3_0_BOARD_TOT_MAC_ID_OFFSET, EEPROM_V3_0_BOARD_TOT_MAC_ID_SIZE},
	{EEPROM_V3_0_BOARD_MAC_OFFSET, EEPROM_V3_0_BOARD_MAC_SIZE},
	{EEPROM_V3_0_BOARD_ACT_PAS_OFFSET, EEPROM_V3_0_BOARD_ACT_PAS_SIZE},
	{EEPROM_V3_0_BOARD_CONFIG_MODE_OFFSET, EEPROM_V3_0_BOARD_CONFIG_MODE_SIZE},
	{EEPROM_V3_0_MFG_DATE_OFFSET, EEPROM_V3_0_MFG_DATE_SIZE},
	{EEPROM_V3_0_PART_NUM_OFFSET, EEPROM_V3_0_PART_NUM_SIZE},
	{EEPROM_V3_0_UUID_OFFSET, EEPROM_V3_0_UUID_SIZE},
	{EEPROM_V3_0_PCIE_INFO_OFFSET, EEPROM_V3_0_PCIE_INFO_SIZE},
	{EEPROM_V3_0_MAX_POWER_MODE_OFFSET, EEPROM_V3_0_MAX_POWER_MODE_SIZE},
	{EEPROM_V3_0_MEM_SIZE_OFFSET, EEPROM_V3_0_MEM_SIZE_SIZE},
	{EEPROM_V3_0_OEMID_SIZE_OFFSET, EEPROM_V3_0_OEMID_SIZE},
	{EEPROM_V3_0_CAPABILITY_OFFSET, EEPROM_V3_0_CAPABILITY_SIZE},
};

EEPROM_Content_Details_t Ver_2_0_Offset_Size[eEeprom_max_Offset] =
{
	{EEPROM_V2_0_PRODUCT_NAME_OFFSET, EEPROM_V2_0_PRODUCT_NAME_SIZE},
	{EEPROM_V2_0_BOARD_REV_OFFSET, EEPROM_V2_0_BOARD_REV_SIZE},
	{EEPROM_V2_0_BOARD_SERIAL_OFFSET, EEPROM_V2_0_BOARD_SERIAL_SIZE},
	{EEPROM_V2_0_BOARD_TOT_MAC_ID_OFFSET, EEPROM_V2_0_BOARD_NUM_MAC},
	{EEPROM_V2_0_BOARD_MAC_OFFSET, EEPROM_V2_0_BOARD_MAC_SIZE},
	{EEPROM_V2_0_BOARD_ACT_PAS_OFFSET, EEPROM_V2_0_BOARD_ACT_PAS_SIZE},
	{EEPROM_V2_0_BOARD_CONFIG_MODE_OFFSET, EEPROM_V2_0_BOARD_CONFIG_MODE_SIZE},
	{EEPROM_V2_0_MFG_DATE_OFFSET, EEPROM_V2_0_MFG_DATE_SIZE},
	{EEPROM_V2_0_PART_NUM_OFFSET, EEPROM_V2_0_PART_NUM_SIZE},
	{EEPROM_V2_0_UUID_OFFSET, EEPROM_V2_0_UUID_SIZE},
	{EEPROM_V2_0_PCIE_INFO_OFFSET, EEPROM_V2_0_PCIE_INFO_SIZE},
	{EEPROM_V2_0_MAX_POWER_MODE_OFFSET, EEPROM_V2_0_MAX_POWER_MODE_SIZE},
	{EEPROM_V2_0_DIMM_SIZE_OFFSET, EEPROM_V2_0_DIMM_SIZE_SIZE},
	{EEPROM_V2_0_OEMID_SIZE_OFFSET, EEPROM_V2_0_OEMID_SIZE},
	{EEPROM_V2_0_CAPABILITY_OFFSET, EEPROM_V2_0_CAPABILITY_SIZE},
};

void VMC_SetLogLevel(u8 LogLevel)
{
    sensor_glvr.logging_level = (LogLevel <= VMC_LOG_LEVEL_NONE) ? LogLevel : sensor_glvr.logging_level;
}

u8 VMC_GetLogLevel(void)
{
    return sensor_glvr.logging_level;
}

void VMC_Printf(char *filename, u32 line, u8 log_level, const char *fmt, ...)
{
    va_list args;

    va_start(args,fmt);
    Debug_Printf(filename, line, log_level, fmt, &args);
    va_end(args);
}

s32 VMC_User_Input_Read(char *ReadChar, u32 *receivedBytes)
{
    UART_STATUS uart_ret_val = UART_ERROR_EVENT;
    
    if (VMC_GetLogLevel() != VMC_LOG_LEVEL_NONE)
    {
    	uart_ret_val = UART_RTOS_Receive_Set_Buffer(&uart_log, (u8 *)ReadChar, 1, receivedBytes);
    	if (UART_SUCCESS == uart_ret_val)
    	{
    		uart_ret_val = UART_RTOS_Receive_Wait(&uart_log, receivedBytes, portMAX_DELAY);
    		if (UART_SUCCESS == uart_ret_val) {
    			return UART_SUCCESS;
    		} else if (uart_ret_val == UART_ERROR_GENERIC) {
    			if (!uart_shm_release(uart_log.rxSem))
    				return UART_ERROR_GENERIC;
    		}
    	}
    }
    else
    {
    	*receivedBytes = 0;
        *ReadChar = 0;
    }

    return UART_ERROR_GENERIC;
}


void Debug_Printf(char *filename, u32 line, u8 log_level, const char *fmt, va_list *argp)
{
    UART_STATUS uart_rtos_status = UART_ERROR_GENERIC;
    u8 msg_idx = 0;
    u16 max_msg_size = MAX_LOG_SIZE;
    if (log_level < sensor_glvr.logging_level)
    {
        return;
    }

    if (xSemaphoreTake(vmc_debug_logbuf_lock, portMAX_DELAY))
    {
        if ((sensor_glvr.logging_level == VMC_LOG_LEVEL_VERBOSE) && (log_level != VMC_LOG_LEVEL_DEMO_MENU))
	{
    	    for ( ; (filename[msg_idx] != '\0') && (msg_idx < MAX_FILE_NAME_SIZE); msg_idx++)
   	    {
	        LogBuf[msg_idx] = filename[msg_idx];
	    }

	    LogBuf[msg_idx++] = '-';
	    snprintf(&LogBuf[msg_idx], 4, "%d", (int)line);
	    msg_idx+= 4;
	    LogBuf[msg_idx++] = ':';
	}

	max_msg_size -= msg_idx;
	vsnprintf(&LogBuf[msg_idx], max_msg_size, fmt, *argp);

	uart_rtos_status = UART_RTOS_Send(&uart_log, (u8 *)LogBuf, MAX_LOG_SIZE);
	if (uart_rtos_status != UART_SUCCESS) {
		xil_printf("Failed to send UART_RTOS: %d \n\r", uart_rtos_status);
	}

	Cl_SecureMemset(LogBuf , '\0', MAX_LOG_SIZE);
	xSemaphoreGive(vmc_debug_logbuf_lock);
    }
    else
    {
        xil_printf("Failed to get lock for vmc_debug_logbuf_lock \n\r");
    }

}


void BoardInfoTest(void)
{
	u16 mac_num = 0;

	//VMC_LOG("\n\rTBD: Board Info to be printed! %d",1000);
	VMC_DMO( "EEPROM Version        : %s \n\r",board_info.eeprom_version);
	VMC_DMO( "product name          : %s \n\r",board_info.product_name);
	VMC_DMO( "board rev             : %s \n\r",board_info.board_rev);
	VMC_DMO( "board serial          : %s \n\r",board_info.board_serial);

	/* Print MAC info */
	VMC_DMO("Board MAC%d            : %02x:%02x:%02x:%02x:%02x:%02x\n\r", mac_num,
						board_info.board_mac[mac_num][0],
						board_info.board_mac[mac_num][1],
						board_info.board_mac[mac_num][2],
						board_info.board_mac[mac_num][3],
						board_info.board_mac[mac_num][4],
	                                        board_info.board_mac[mac_num][5]);

	for (mac_num = 1; mac_num < board_info.Num_MAC_IDS; mac_num++)
	{
		VMC_DMO("Board MAC%d            : %02x:%02x:%02x:%02x:%02x:%02x\n\r", mac_num, 
							board_info.board_mac[mac_num][0],
							board_info.board_mac[mac_num][1],
							board_info.board_mac[mac_num][2],
							board_info.board_mac[mac_num][3],
					                board_info.board_mac[mac_num][4],
							board_info.board_mac[mac_num][5]);
	}

	VMC_DMO( "board A/P             : %s \n\r",board_info.board_act_pas);

	VMC_DMO( "board config mode     : %02x \n\r",board_info.board_config_mode[0]);

	VMC_DMO( "MFG DATE              : %02x%02x%02x\n\r",
	                                                  board_info.board_mfg_date[0],
	                                                  board_info.board_mfg_date[1],
	                                                  board_info.board_mfg_date[2]);

	VMC_DMO( "board part num        : %s \n\r",board_info.board_part_num);

	VMC_DMO( "UUID                  : %02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x\n\r",
		                                                  board_info.board_uuid[0],
		                                                  board_info.board_uuid[1],
		                                                  board_info.board_uuid[2],
		                                                  board_info.board_uuid[3],
		                                                  board_info.board_uuid[4],
		                                                  board_info.board_uuid[5],
		                                                  board_info.board_uuid[6],
		                                                  board_info.board_uuid[7],
		                                                  board_info.board_uuid[8],
		                                                  board_info.board_uuid[9],
		                                                  board_info.board_uuid[10],
		                                                  board_info.board_uuid[11],
		                                                  board_info.board_uuid[12],
		                                                  board_info.board_uuid[13],
		                                                  board_info.board_uuid[14],
		                                                  board_info.board_uuid[15]);
														  
	VMC_DMO( "PCIe Info             : %02x%02x, %02x%02x, %02x%02x, %02x%02x\n\r",
                                              board_info.board_pcie_info[0],
                                              board_info.board_pcie_info[1],
                                              board_info.board_pcie_info[2],
                                              board_info.board_pcie_info[3],
                                              board_info.board_pcie_info[4],
                                              board_info.board_pcie_info[5],
                                              board_info.board_pcie_info[6],
                                              board_info.board_pcie_info[7]);
											  

	//VMC_DMO( "board max power mode  : %s \n\r",board_info.board_max_power_mode);

	VMC_DMO( "OEM ID                : %02x%02x%02x%02x\n\r", board_info.OEM_ID[3],
                                                          board_info.OEM_ID[2],
                                                          board_info.OEM_ID[1],
                                                          board_info.OEM_ID[0]);

	VMC_DMO( "Capability            : %02x%02x\n\r", board_info.capability[1],
							 board_info.capability[0]);
}

void SensorData_Display(void)
{
	VMC_PRNT("\n\r");
#ifdef VMC_TEST_VCK5000
	VMC_PRNT("SE98A_0 temperature 			: %d \n\r",sensor_glvr.sensor_readings.board_temp[0]);
	VMC_PRNT("SE98A_1 temperature 			: %d \n\r",sensor_glvr.sensor_readings.board_temp[1]);
	VMC_PRNT("local temperature(max6639) 		: %f \n\r",sensor_glvr.sensor_readings.local_temp);
	VMC_PRNT("remote temp or fpga temp(max6639) 	: %f \n\r ",sensor_glvr.sensor_readings.remote_temp);
	VMC_PRNT("Fan RPM (max6639) 			: %d \n\r ",sensor_glvr.sensor_readings.fanRpm);
	VMC_PRNT("Maximum SYSMON temp 			: %f \n\r ",sensor_glvr.sensor_readings.sysmon_max_temp);
	VMC_PRNT("QSFP_0 temperature			: %f \n\r ",sensor_glvr.sensor_readings.qsfp_temp[0]);
	VMC_PRNT("QSFP_1 temperature			: %f \n\r ",sensor_glvr.sensor_readings.qsfp_temp[1]);
#endif

#ifdef VMC_TEST_V70
	VMC_PRNT("Maximum SYSMON Temperature	: %f C \n\r",sensor_glvr.sensor_readings.sysmon_max_temp);
	VMC_PRNT("LM75_0 Temperature 		: %d C \n\r",sensor_glvr.sensor_readings.board_temp[0]);
	VMC_PRNT("LM75_1 Temperature 		: %d C \n\r",sensor_glvr.sensor_readings.board_temp[1]);
	VMC_PRNT("ISL_68221 VCCINT Temperature 	: %f C \n\r",sensor_glvr.sensor_readings.vccint_temp);
	VMC_PRNT("Voltage VCCINT			: %f mV \n\r",sensor_glvr.sensor_readings.voltage[eVCCINT]);
	VMC_PRNT("Voltage 12V_PEX			: %f mV \n\r",sensor_glvr.sensor_readings.voltage[e12V_PEX]);
	VMC_PRNT("Voltage 3V3_PEX			: %f mV \n\r",sensor_glvr.sensor_readings.voltage[e3V3_PEX]);
	VMC_PRNT("Voltage 3V3_AUX			: %f mV \n\r",sensor_glvr.sensor_readings.voltage[e3V3_AUX]);
	VMC_PRNT("Current 12V_PEX			: %f mA \n\r",sensor_glvr.sensor_readings.current[e12V_PEX]);
	VMC_PRNT("Current 3V3_PEX			: %f mA \n\r",sensor_glvr.sensor_readings.current[e3V3_PEX]);
	VMC_PRNT("Current VCCINT			: %f mA \n\r",(sensor_glvr.sensor_readings.current[eVCCINT] * 1000));
	VMC_PRNT("Total Power			: %f W \n\r",sensor_glvr.sensor_readings.total_power);
#endif
	VMC_PRNT("\n\r");

}

void EepromTest(void)
{
	VMC_ERR("\n\rTBD: EEPROM Test to be printed!\n\r");
}

void EepromDump(void)
{
	unsigned char buf[EEPROM_BUF_SIZE] = { 0 };
	u16 i = 0;
	u16 offset = 0;

	M24C128_ReadMultiBytes(LPD_I2C_0, SLAVE_ADDRESS_M24C128, &offset, buf, EEPROM_BUF_SIZE);

	for(i = 0; i < EEPROM_BUF_SIZE; i += 8)
	{
		VMC_DMO( "%.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x\n\r",
			buf[i], buf[i + 1], buf[i + 2], buf[i + 3], buf[i + 4],
			buf[i + 5], buf[i + 6], buf[i + 7]);
	}

	Versal_Print_BoardInfo();
}

static void Print_Eeprom_3_0(void)
{
	VMR_PRNT( "EEPROM Version        : %s ",board_info.eeprom_version);
	VMR_PRNT( "Product Name          : %s ",board_info.product_name);
	VMR_PRNT( "Board Rev             : %s ",board_info.board_rev);
	VMR_PRNT( "Board Serial          : %s ",board_info.board_serial);

	VMR_PRNT( "Board MAC             : %02x:%02x:%02x:%02x:%02x:%02x",
						board_info.board_mac[0][0],
						board_info.board_mac[0][1],
						board_info.board_mac[0][2],
						board_info.board_mac[0][3],
						board_info.board_mac[0][4],
						board_info.board_mac[0][5]);


	VMR_PRNT( "Board A/P             : %s ",board_info.board_act_pas);

	VMR_PRNT( "Board Config          : %02x ",board_info.board_config_mode[0]);

	VMR_PRNT( "MFG DATE              : %02x%02x%02x",
	                                        board_info.board_mfg_date[0],
	                                        board_info.board_mfg_date[1],
	                                        board_info.board_mfg_date[2]);

	VMR_PRNT( "PART NUM              : %s ",board_info.board_part_num);

	VMR_PRNT( "UUID                  : %02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
		                                    board_info.board_uuid[0],
		                                    board_info.board_uuid[1],
		                                    board_info.board_uuid[2],
		                                    board_info.board_uuid[3],
		                                    board_info.board_uuid[4],
		                                    board_info.board_uuid[5],
		                                    board_info.board_uuid[6],
		                                    board_info.board_uuid[7],
		                                    board_info.board_uuid[8],
		                                    board_info.board_uuid[9],
		                                    board_info.board_uuid[10],
		                                    board_info.board_uuid[11],
		                                    board_info.board_uuid[12],
		                                    board_info.board_uuid[13],
		                                    board_info.board_uuid[14],
		                                    board_info.board_uuid[15]);

	VMR_PRNT( "PCIe Info             : %02x%02x, %02x%02x, %02x%02x, %02x%02x",
                                            board_info.board_pcie_info[0],
                                            board_info.board_pcie_info[1],
                                            board_info.board_pcie_info[2],
                                            board_info.board_pcie_info[3],
                                            board_info.board_pcie_info[4],
                                            board_info.board_pcie_info[5],
                                            board_info.board_pcie_info[6],
                                            board_info.board_pcie_info[7]);

	VMR_PRNT( "OEM ID                : %02x%02x%02x%02x", board_info.OEM_ID[3],
								  board_info.OEM_ID[2],
								  board_info.OEM_ID[1],
								  board_info.OEM_ID[0]);


	VMR_PRNT( "Max Power Mode        : %d \n\r", board_info.board_max_power_mode[0]);
	VMR_PRNT( "Memory size           : %s \n\r", board_info.Memory_size);
	VMR_PRNT( "Capabilities          : %02x%02x \n\r", board_info.capability[0], board_info.capability[1]);
}

static bool Verify_Checksum(void)
{
	unsigned char buf[EEPROM_BUF_SIZE] = { 0 };

	u16 addr = 0;

	M24C128_ReadMultiBytes(LPD_I2C_0, SLAVE_ADDRESS_M24C128, &addr, buf, EEPROM_BUF_SIZE);

	u16 checksum = 0;
	u16 expected = buf[EEPROM_V3_0_CHECKSUM_MSB_OFFSET] << 8 | buf[EEPROM_V3_0_CHECKSUM_LSB_OFFSET];

	for(int i = EEPROM_V3_0_CHECKSUM_START; i <= EEPROM_V3_0_CHECKSUM_END; ++i){
		checksum += buf[i];
	}

	checksum = (u16) 0 - checksum;

	if(checksum == expected)
	{
		return true;
	}
	else
	{
		VMC_ERR("Incorrect checksum: calculated %hu %x%x expected %hu %x%x", checksum, checksum >> 8, checksum & 0xFF,																	expected, buf[EEPROM_V3_0_CHECKSUM_MSB_OFFSET], 																buf[EEPROM_V3_0_CHECKSUM_LSB_OFFSET]);
		return false;
	}
}

void VMC_Get_BoardInfo(Versal_BoardInfo *_board_info) {
	Cl_SecureMemcpy(_board_info, sizeof(Versal_BoardInfo),
			&board_info, sizeof(Versal_BoardInfo));
}

u8 Update_BoardInfo_Data(u8 i2c_num, u8 *data_ptr, u16 offset, u8 size)
{
	u8 status = 0;

	for (int i = 0; i < size; i++)
	{
		status = M24C128_ReadByte(i2c_num, SLAVE_ADDRESS_M24C128, &offset, &data_ptr[i]);
		if (data_ptr[i] == EEPROM_DEFAULT_VAL) {
			data_ptr[i] = '\0';
		}
		offset = offset + 0x0100;
	}
	/* Explicitly set \0 at the end of board name string */
	data_ptr[size] = '\0';

	return status;
}

u8 Versal_EEPROM_ReadBoardInfo(void)
{
	s8 i = 0;
	u16 offset = 0x00;
	u8 status = 0;
	u8 MAC_ID[6] = { 0 };
	u16 Value = 0;
	u8 carry = 0;
	u16 mac_num = 0;
	u8 i2c_num = 1;
	u32 eeprom_ver = 0;
	u8 size = 0;

	EEPROM_Content_Details_t *eeprom_offset = NULL;

	u8 *data_ptr = NULL;
	Verify_Checksum();

	status = Update_BoardInfo_Data(i2c_num, board_info.eeprom_version,
			EEPROM_VERSION_OFFSET, EEPROM_VERSION_SIZE);
	if (status != XST_SUCCESS) {
		VMR_ERR("EEPROM version read failed !!");
		return XST_FAILURE;
	}

	eeprom_ver = ((board_info.eeprom_version[0] << 16)
					| (board_info.eeprom_version[1] << 8)
					| (board_info.eeprom_version[2]));
	if (eeprom_ver == EEPROM_V3_0) {
		eeprom_offset = Ver_3_0_Offset_Size;
	} else if (eeprom_ver == EEPROM_V2_0) {
		eeprom_offset = Ver_2_0_Offset_Size;
	} else {
		VMR_ERR("Unable to identify MFG EEPROM version !!");
		return XST_FAILURE;
	}
	VMR_LOG("MFG EEPROM Version is %s",&board_info.eeprom_version[0]);

	status = Update_BoardInfo_Data(i2c_num, board_info.product_name,
			eeprom_offset[eEeprom_Product_Name].offset,
			eeprom_offset[eEeprom_Product_Name].size);
	if (status != XST_SUCCESS) {
		VMR_ERR("Unable to read product name !!");
		return XST_FAILURE;
	}

	status = Update_BoardInfo_Data(i2c_num, board_info.board_rev,
			eeprom_offset[eEeprom_Board_Rev].offset,
			eeprom_offset[eEeprom_Board_Rev].size);

	status = Update_BoardInfo_Data(i2c_num, board_info.board_serial,
			eeprom_offset[eEeprom_Board_Serial].offset,
			eeprom_offset[eEeprom_Board_Serial].size);

	if (eeprom_offset[eEeprom_Board_Tot_Mac_Id].offset != EEPROM_V2_0_BOARD_TOT_MAC_ID_OFFSET) {
		status = Update_BoardInfo_Data(i2c_num, &board_info.Num_MAC_IDS,
				eeprom_offset[eEeprom_Board_Tot_Mac_Id].offset,
				eeprom_offset[eEeprom_Board_Tot_Mac_Id].size);
	} else {
		board_info.Num_MAC_IDS = eeprom_offset[eEeprom_Board_Tot_Mac_Id].size;
	}

	data_ptr = &board_info.board_mac[0][0];
	offset = eeprom_offset[eEeprom_Board_Mac].offset;
	size = eeprom_offset[eEeprom_Board_Mac].size;
	for (i = 0; i < size; i++)
	{
		status = M24C128_ReadByte(i2c_num, SLAVE_ADDRESS_M24C128, &offset, &data_ptr[i]);
		offset = offset + 0x0100;
	}
	/* Explicitly set \0 at the end of board name string */
	data_ptr[size] = '\0';

	for (i = 0; i < size; i++) //copy first MAC ID into MAC_ID
	{
		MAC_ID[i] = data_ptr[i];
	}

	for (mac_num = 1; mac_num < board_info.Num_MAC_IDS; mac_num++)
	{
		for (i = size - 1; i >= 0; i--)
		{
			Value = MAC_ID[i] + 1;
			carry = (Value > 255) ? 1 : 0;
			if (carry == 1) {
				MAC_ID[i] = 0x00;
				continue;
			} else {
				MAC_ID[i] = Value;
				break;
			}
		}
		for (i = 0; i < size; i++) {
			board_info.board_mac[mac_num][i] = MAC_ID[i];
		}
		board_info.board_mac[mac_num][size] = '\0';
	}

	status = Update_BoardInfo_Data(i2c_num, board_info.board_act_pas,
			eeprom_offset[eEeprom_Board_Act_Pas].offset,
			eeprom_offset[eEeprom_Board_Act_Pas].size);

	status = Update_BoardInfo_Data(i2c_num, board_info.board_config_mode,
			eeprom_offset[eEeprom_Board_config_Mode].offset,
			eeprom_offset[eEeprom_Board_config_Mode].size);

	status = Update_BoardInfo_Data(i2c_num, board_info.board_mfg_date,
			eeprom_offset[eEeprom_Mfg_Date].offset,
			eeprom_offset[eEeprom_Mfg_Date].size);

	status = Update_BoardInfo_Data(i2c_num, board_info.board_part_num,
			eeprom_offset[eEeprom_Part_Num].offset,
			eeprom_offset[eEeprom_Part_Num].size);

	status = Update_BoardInfo_Data(i2c_num, board_info.board_uuid,
			eeprom_offset[eEeprom_Uuid].offset,
			eeprom_offset[eEeprom_Uuid].size);

	status = Update_BoardInfo_Data(i2c_num, board_info.board_pcie_info,
			eeprom_offset[eEeprom_Pcie_Info].offset,
			eeprom_offset[eEeprom_Pcie_Info].size);

	status = Update_BoardInfo_Data(i2c_num, board_info.board_max_power_mode,
			eeprom_offset[eEeprom_Max_Power_Mode].offset,
			eeprom_offset[eEeprom_Max_Power_Mode].size);

	status = Update_BoardInfo_Data(i2c_num, board_info.Memory_size,
			eeprom_offset[eEeprom_mem_Size].offset,
			eeprom_offset[eEeprom_mem_Size].size);

	status = Update_BoardInfo_Data(i2c_num, board_info.OEM_ID,
			eeprom_offset[eEeprom_Oemid_Size].offset,
			eeprom_offset[eEeprom_Oemid_Size].size);

	if (eeprom_offset[eEeprom_Capability_Word].offset != EEPROM_V2_0_CAPABILITY_OFFSET) {
		status = Update_BoardInfo_Data(i2c_num, board_info.capability,
				eeprom_offset[eEeprom_Capability_Word].offset,
				eeprom_offset[eEeprom_Capability_Word].size);
	}

	return status;
}

u8 Versal_Print_BoardInfo(void)
{
	if(!strncmp((char *) board_info.eeprom_version, "3.0", EEPROM_VERSION_SIZE))
	{
		Print_Eeprom_3_0();
	}
	else if(!strncmp((char *) board_info.eeprom_version, "2.0", EEPROM_VERSION_SIZE))
	{
		//TODO replace with 2_0
		Print_Eeprom_3_0();
	}
	else
	{
		VMC_ERR("No matching eeprom version %.3s\n\r", board_info.eeprom_version);
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}
