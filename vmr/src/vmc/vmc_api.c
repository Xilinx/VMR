
/******************************************************************************
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#include "cl_uart_rtos.h"
#include "cl_log.h"
#include "cl_mem.h"
#include "cl_i2c.h"
#include "sensors/inc/m24c128.h"
#include <stdio.h>

/* FreeRTOS includes */
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>

#include "vmc_api.h"
#include "vmc_sensors.h"
#include "vmc_main.h"

#define MAX_FILE_NAME_SIZE 25

extern uart_rtos_handle_t uart_log;

/*
 * This is the example code of migrating all global variables into an unified
 * vmc_global_variable structure, so that we can convert this into MPU supported
 * framework.
 */
extern Vmc_Global_Variables vmc_g_var;

SemaphoreHandle_t vmc_debug_logbuf_lock; /* used to block until LogBuf is in use */
Versal_BoardInfo board_info;

static char LogBuf[MAX_LOG_SIZE];

void Debug_Printf(char *filename, u32 line, u8 log_level, const char *fmt, va_list *argp);


void VMC_SetLogLevel(u8 LogLevel)
{
    vmc_g_var.logging_level = (LogLevel <= VMC_LOG_LEVEL_NONE) ? LogLevel : vmc_g_var.logging_level;
}

u8 VMC_GetLogLevel(void)
{
    return vmc_g_var.logging_level;
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
    if (VMC_GetLogLevel() != VMC_LOG_LEVEL_NONE)
    {
    	if(UART_RTOS_Receive(&uart_log, (u8 *)ReadChar, 1, receivedBytes,portMAX_DELAY) ==  UART_SUCCESS)
    	{
  	    return UART_SUCCESS;
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
    s8 uart_rtos_status = UART_ERROR_GENERIC;
    u8 msg_idx = 0;
    u16 max_msg_size = MAX_LOG_SIZE;
    if (log_level < vmc_g_var.logging_level)
    {
        return;
    }

    if (xSemaphoreTake(vmc_debug_logbuf_lock, portMAX_DELAY))
    {
        if ((vmc_g_var.logging_level == VMC_LOG_LEVEL_VERBOSE) && (log_level != VMC_LOG_LEVEL_DEMO_MENU))
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

        if((uart_rtos_status = UART_RTOS_Send(&uart_log, (u8 *)LogBuf, MAX_LOG_SIZE)) != UART_SUCCESS){
            xil_printf("Failed to send UART_RTOS: %d \n\r", uart_rtos_status);
        }

	Cl_SecureMemset(LogBuf , '\0' , MAX_LOG_SIZE);
	xSemaphoreGive(vmc_debug_logbuf_lock);
    }
    else
    {
        xil_printf("Failed to get lock for vmc_debug_logbuf_lock \n\r");
    }

}


void BoardInfoTest(void)
{
	u16 mac_num     = 0;
	//VMC_LOG("\n\rTBD: Board Info to be printed! %d",1000);
	VMC_DMO( "EEPROM Version        : %s \n\r",board_info.eeprom_version);
	VMC_DMO( "product name          : %s \n\r",board_info.product_name);
	VMC_DMO( "board rev             : %s \n\r",board_info.board_rev);
	VMC_DMO( "board serial          : %s \n\r",board_info.board_serial);

	/* Print MAC info */
	for(mac_num = 0; mac_num < EEPROM_BOARD_NUM_MAC; mac_num++)
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
}

void SensorData_Display(void)
{
	VMC_PRNT("\n\r");


	//VMC_PRNT("====================================================================\n\r");
	//VMC_PRNT("TBD: Sensor Data to be printed!\n\r");
	//VMC_PRNT("====================================================================\n\r");
	VMC_PRNT("SE98A_0 temperature 			: %d \n\r",vmc_g_var.sensor_readings.board_temp[0]);
	VMC_PRNT("SE98A_1 temperature 			: %d \n\r",vmc_g_var.sensor_readings.board_temp[1]);
	VMC_PRNT("local temperature(max6639) 		: %f \n\r",vmc_g_var.sensor_readings.local_temp);
	VMC_PRNT("remote temp or fpga temp(max6639) 	: %f \n\r ",vmc_g_var.sensor_readings.remote_temp);
	VMC_PRNT("Fan RPM (max6639) 			: %d \n\r ",vmc_g_var.sensor_readings.fanRpm);
	VMC_PRNT("Maximum SYSMON temp 			: %f \n\r ",vmc_g_var.sensor_readings.sysmon_max_temp);
	VMC_PRNT("QSFP_0 temperature			: %f \n\r ",vmc_g_var.sensor_readings.qsfp_temp[0]);
	VMC_PRNT("QSFP_1 temperature			: %f \n\r ",vmc_g_var.sensor_readings.qsfp_temp[1]);
	VMC_PRNT("\n\r");


}

void EepromTest(void)
{
	VMC_ERR("\n\rTBD: EEPROM Test to be printed!\n\r");
}

void EepromDump(void)
{
	VMC_DBG("\n\rTBD: EEPROM data will be dumped out here! %d\n\r", 2000);
}

u8 Versal_EEPROM_ReadBoardInfo(void)
{
	s8 i			= 0;
	u16 offset		= 0x00;
	u8  status              = 0;
	u8  MAC_ID[6] 	        = {0};
	u16 Value 		= 0;
	u8  carry 		= 0;
	u16 mac_num             = 0;
	u8 i2c_num 		= 1;

	unsigned char *data_ptr = NULL;
	data_ptr = board_info.eeprom_version;
	offset = EEPROM_VERSION_OFFSET;

	for (i = 0 ; i < EEPROM_VERSION_SIZE ; i++)
	{
		status = M24C128_ReadByte(i2c_num, SLAVE_ADDRESS_M24C128, &offset, &data_ptr[i]);

        	if (data_ptr[i] == EEPROM_DEFAULT_VAL)
        	{
            		data_ptr[i] = '\0';
        	}
        	offset = offset + 0x0100;
	}
	/* Explicitly set \0 at the end of board name string */
	data_ptr[EEPROM_VERSION_SIZE] = '\0';


	data_ptr = board_info.product_name;
	offset   = EEPROM_PRODUCT_NAME_OFFSET;
	for (i = 0 ; i < EEPROM_PRODUCT_NAME_SIZE ; i++)
	{
		status = M24C128_ReadByte(i2c_num, SLAVE_ADDRESS_M24C128, &offset, &data_ptr[i]);

        	if (data_ptr[i] == EEPROM_DEFAULT_VAL)
        	{
            		data_ptr[i] = '\0';
        	}
        	offset = offset + 0x0100;
	}
	/* Explicitly set \0 at the end of board name string */
	data_ptr[EEPROM_PRODUCT_NAME_SIZE] = '\0';

	data_ptr = board_info.board_rev;
	offset   = EEPROM_BOARD_REV_OFFSET;
	for (i = 0 ; i < EEPROM_BOARD_REV_SIZE ; i++)
	{
		status = M24C128_ReadByte(i2c_num, SLAVE_ADDRESS_M24C128, &offset, &data_ptr[i]);

		if (data_ptr[i] == EEPROM_DEFAULT_VAL)
		{
			data_ptr[i] = '\0';
		}
		offset = offset + 0x0100;
	}
	/* Explicitly set \0 at the end of board name string */
	data_ptr[EEPROM_BOARD_REV_SIZE] = '\0';

	data_ptr = board_info.board_serial;
	offset   = EEPROM_BOARD_SERIAL_OFFSET;
	for (i = 0 ; i < EEPROM_BOARD_SERIAL_SIZE ; i++)
	{
		status = M24C128_ReadByte(i2c_num, SLAVE_ADDRESS_M24C128, &offset, &data_ptr[i]);

		if (data_ptr[i] == EEPROM_DEFAULT_VAL)
		{
			data_ptr[i] = '\0';
		}
		offset = offset + 0x0100;
	}
	/* Explicitly set \0 at the end of board name string */
	data_ptr[EEPROM_BOARD_SERIAL_SIZE] = '\0';

	board_info.Num_MAC_IDS = EEPROM_BOARD_NUM_MAC;

	data_ptr = &board_info.board_mac[0][0];
	offset = EEPROM_BOARD_MAC_OFFSET;
	for (i = 0 ; i < EEPROM_BOARD_MAC_SIZE ; i++)
	{
		status = M24C128_ReadByte(i2c_num, SLAVE_ADDRESS_M24C128, &offset, &data_ptr[i]);
		offset = offset + 0x0100;

	}
	/* Explicitly set \0 at the end of board name string */
	data_ptr[EEPROM_BOARD_MAC_SIZE] = '\0';

    	for (i=0 ; i<EEPROM_BOARD_MAC_SIZE ; i++) //copy first MAC ID into MAC_ID
    	{
       		MAC_ID[i] = data_ptr[i];
    	}

    	for (mac_num = 1; mac_num < EEPROM_BOARD_NUM_MAC; mac_num++)
    	{
        	for (i = EEPROM_BOARD_MAC_SIZE-1 ; i >= 0 ; i--)
        	{
            		Value = MAC_ID[i] + 1;
            		carry = (Value > 255) ? 1 : 0;
            		if (carry == 1)
            		{
                		MAC_ID[i] = 0x00;
                		continue;
            		}
            		else
            		{
                		MAC_ID[i] = Value;
                		break;
           		 }
        	}
        	for (i=0 ; i<EEPROM_BOARD_MAC_SIZE ; i++)
        	{
            		board_info.board_mac[mac_num][i] = MAC_ID[i];
       	 	}
        	board_info.board_mac[mac_num][EEPROM_BOARD_MAC_SIZE] = '\0';
    	}


    	data_ptr = board_info.board_act_pas;
	offset   = EEPROM_BOARD_ACT_PAS_OFFSET;
	for (i = 0 ; i < EEPROM_BOARD_ACT_PAS_SIZE ; i++)
	{
		status = M24C128_ReadByte(i2c_num, SLAVE_ADDRESS_M24C128, &offset, &data_ptr[i]);

		if (data_ptr[i] == EEPROM_DEFAULT_VAL)
		{
			data_ptr[i] = '\0';
		}
		offset = offset + 0x0100;
	}
	/* Explicitly set \0 at the end of board name string */
	data_ptr[EEPROM_BOARD_ACT_PAS_SIZE] = '\0';

	data_ptr = board_info.board_config_mode;
	offset   = EEPROM_BOARD_CONFIG_MODE_OFFSET;
	for (i = 0 ; i < EEPROM_BOARD_CONFIG_MODE_SIZE ; i++)
	{
		status = M24C128_ReadByte(i2c_num, SLAVE_ADDRESS_M24C128, &offset, &data_ptr[i]);
		if (data_ptr[i] == EEPROM_DEFAULT_VAL)
		{
			data_ptr[i] = '\0';
		}
		offset = offset + 0x0100;
	}
	/* Explicitly set \0 at the end of board name string */
	data_ptr[EEPROM_BOARD_CONFIG_MODE_SIZE] = '\0';

	data_ptr = board_info.board_mfg_date;
	offset   = EEPROM_MFG_DATE_OFFSET;
	for (i = 0 ; i < EEPROM_MFG_DATE_SIZE ; i++)
	{
		status = M24C128_ReadByte(i2c_num, SLAVE_ADDRESS_M24C128, &offset, &data_ptr[i]);

		if (data_ptr[i] == EEPROM_DEFAULT_VAL)
		{
			data_ptr[i] = '\0';
		}
		offset = offset + 0x0100;
	}
	/* Explicitly set \0 at the end of board name string */
	data_ptr[EEPROM_MFG_DATE_SIZE] = '\0';

	data_ptr = board_info.board_part_num;
	offset   = EEPROM_PART_NUM_OFFSET;
	for (i = 0 ; i < EEPROM_PART_NUM_SIZE ; i++)
	{
		status = M24C128_ReadByte(i2c_num, SLAVE_ADDRESS_M24C128, &offset, &data_ptr[i]);
		if (data_ptr[i] == EEPROM_DEFAULT_VAL)
		{
			data_ptr[i] = '\0';
		}
		offset = offset + 0x0100;
	}
	/* Explicitly set \0 at the end of board name string */
	data_ptr[EEPROM_PART_NUM_SIZE] = '\0';

	data_ptr = board_info.board_uuid;
	offset   = EEPROM_UUID_OFFSET;
	for (i = 0 ; i < EEPROM_UUID_SIZE ; i++)
	{
		status = M24C128_ReadByte(i2c_num, SLAVE_ADDRESS_M24C128, &offset, &data_ptr[i]);

		if (data_ptr[i] == EEPROM_DEFAULT_VAL)
		{
			data_ptr[i] = '\0';
		}
		offset = offset + 0x0100;
	}
	/* Explicitly set \0 at the end of board name string */
	data_ptr[EEPROM_UUID_SIZE] = '\0';

	data_ptr = board_info.board_pcie_info;
	offset   = EEPROM_PCIE_INFO_OFFSET;
	for (i = 0 ; i < EEPROM_PCIE_INFO_SIZE ; i++)
	{
		status = M24C128_ReadByte(i2c_num, SLAVE_ADDRESS_M24C128, &offset, &data_ptr[i]);

		if (data_ptr[i] == EEPROM_DEFAULT_VAL)
		{
			data_ptr[i] = '\0';
		}
		offset = offset + 0x0100;
	}
	/* Explicitly set \0 at the end of board name string */
	data_ptr[EEPROM_PCIE_INFO_SIZE] = '\0';

	data_ptr = board_info.board_max_power_mode;
	offset   = EEPROM_MAX_POWER_MODE_OFFSET;
	for (i = 0 ; i < EEPROM_MAX_POWER_MODE_SIZE ; i++)
	{
		status = M24C128_ReadByte(i2c_num, SLAVE_ADDRESS_M24C128, &offset, &data_ptr[i]);
		if (data_ptr[i] == EEPROM_DEFAULT_VAL)
		{
			data_ptr[i] = '\0';
		}
		offset = offset + 0x0100;
	}
	/* Explicitly set \0 at the end of board name string */
	data_ptr[EEPROM_MAX_POWER_MODE_SIZE] = '\0';

	data_ptr = board_info.Memory_size;
	offset   = EEPROM_DIMM_SIZE_OFFSET;
	for (i = 0 ; i < EEPROM_DIMM_SIZE_SIZE ; i++)
	{
		status = M24C128_ReadByte(i2c_num, SLAVE_ADDRESS_M24C128, &offset, &data_ptr[i]);

		if (data_ptr[i] == EEPROM_DEFAULT_VAL)
		{
			data_ptr[i] = '\0';
		}
		offset = offset + 0x0100;
	}
	/* Explicitly set \0 at the end of board name string */
	data_ptr[EEPROM_DIMM_SIZE_SIZE] = '\0';

	data_ptr = board_info.OEM_ID;
	offset   = EEPROM_OEMID_SIZE_OFFSET;
	for (i = 0 ; i < EEPROM_OEMID_SIZE ; i++)
	{
		status = M24C128_ReadByte(i2c_num, SLAVE_ADDRESS_M24C128, &offset, &data_ptr[i]);

		if (data_ptr[i] == EEPROM_DEFAULT_VAL)
		{
			data_ptr[i] = '\0';
		}
		offset = offset + 0x0100;
	}
	/* Explicitly set \0 at the end of board name string */
	data_ptr[EEPROM_OEMID_SIZE] = '\0';

return status;
}


