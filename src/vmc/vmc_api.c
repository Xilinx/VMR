
/******************************************************************************
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#include "vmc_api.h"

#include "cl_uart_rtos.h"
#include <stdio.h>

/* FreeRTOS includes */
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>


#define MAX_FILE_NAME_SIZE 25

extern uart_rtos_handle_t uart_log;
static uint8_t    logging_level = VMC_LOG_LEVEL_NONE;
static char LogBuf[MAX_LOG_SIZE];
SemaphoreHandle_t logbuf_lock; /* used to block until LogBuf is in use */


void Debug_Printf(char *filename, uint32_t line, uint8_t log_level, const char *fmt, va_list *argp);


void VMC_SetLogLevel(uint8_t LogLevel)
{
    logging_level = (LogLevel <= VMC_LOG_LEVEL_NONE)? LogLevel:logging_level;
}

uint8_t VMC_GetLogLevel(void)
{
    return logging_level;
}

void VMC_Printf(char *filename, uint32_t line, uint8_t log_level, const char *fmt, ...)
{
    va_list args;

    va_start(args,fmt);
    Debug_Printf(filename, line, log_level, fmt, &args);
    va_end(args);
}

int32_t VMC_User_Input_Read(char *ReadChar, uint32_t *receivedBytes)
{
    if (VMC_GetLogLevel() != VMC_LOG_LEVEL_NONE)
    {
    	if(UART_RTOS_Receive(&uart_log, (uint8_t *)ReadChar, 1, receivedBytes) ==  UART_SUCCESS)
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


void Debug_Printf(char *filename, uint32_t line, uint8_t log_level, const char *fmt, va_list *argp)
{
    uint8_t msg_idx = 0;
    uint16_t max_msg_size = MAX_LOG_SIZE;
    if (log_level < logging_level)
    {
        return;
    }

    if (xSemaphoreTake(logbuf_lock, portMAX_DELAY))
    {
        if (logging_level == VMC_LOG_LEVEL_VERBOSE && log_level != VMC_LOG_LEVEL_DEMO_MENU)
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
		UART_RTOS_Send(&uart_log, (uint8_t *)LogBuf, MAX_LOG_SIZE);
		memset(LogBuf , '\0' , MAX_LOG_SIZE);
		xSemaphoreGive(logbuf_lock);
    }
    else
    {
        xil_printf("Failed to get lock for logbuf_lock \n\r");
    }

}


void BoardInfoTest(void)
{
	VMC_LOG("\n\rTBD: Board Info to be printed! %d",1000);
}

void SensorData_Display(void)
{
	VMC_PRNT("\n\r");


	VMC_PRNT("====================================================================\n\r");
	VMC_PRNT("TBD: Sensor Data to be printed!\n\r");
	VMC_PRNT("====================================================================\n\r");

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
