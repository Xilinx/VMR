
/******************************************************************************
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/
#ifndef INC_VMC_API_H_
#define INC_VMC_API_H_


#include <stdbool.h>
#include <stdint.h>


/* Xilinx includes */
#include "xil_printf.h"
#include "xil_io.h"
#include "xil_cache.h"
#include "xil_types.h"

#include "cl_log.h"
#include "cl_main.h"
#include "cl_io.h"

#define VMC_DMO(fmt, arg...)	\
	CL_DMO(APP_VMC, fmt, ##arg)
#define VMC_PRNT(fmt, arg...)	\
	CL_PRNT(APP_VMC, fmt, ##arg)
#define VMC_LOG(fmt, arg...)	\
	CL_LOG(APP_VMC, fmt, ##arg)
#define VMC_ERR(fmt, arg...)	\
	CL_ERR(APP_VMC, fmt, ##arg)
#define VMC_DBG(fmt, arg...)	\
	CL_DBG(APP_VMC, fmt, ##arg)

#include <string.h>

#define VMC_LOG_LEVEL_VERBOSE 		0
#define VMC_LOG_LEVEL_DEBUG   		1
#define VMC_LOG_LEVEL_INFO    		2
#define VMC_LOG_LEVEL_WARN    		3
#define VMC_LOG_LEVEL_ERROR   		4
#define VMC_LOG_LEVEL_DEMO_MENU    	5
#define VMC_LOG_LEVEL_NONE    		6     /* disable logging */

#ifndef __FILENAME__
#define __FILENAME__                 (strrchr(__FILE__, '/') ? (strrchr(__FILE__, '/')+1) : __FILE__)
#endif

/*****************************************************************************/
/**
* @brief Set Logging level for log messages
*
* @param[in]   LogLevel log messages equal or above this logging level will be logged
*
* @return    None
*
* @note None
**
******************************************************************************/
void VMC_SetLogLevel(uint8_t LogLevel);

/*****************************************************************************/
/**
* @brief Get Logging level set for log messages
*
* @param   None
*
* @return  Currently set Log Level
*
* @note None
**
******************************************************************************/
uint8_t VMC_GetLogLevel(void);

/*****************************************************************************/
/**
* @brief This is the print function for VMC which displays on UART.
*
* @param[in]   filename Filename from which the log is generated
* @param[in]   line Line number in the file from which the log is generated
* @param[in]   log_level Loglevel for this log message
* @param[in]   fmt Format of message
* @param[in]   ... variable arguments
*
* @return    None
*
* @note None
**
******************************************************************************/
void VMC_Printf(char *filename, uint32_t line, uint8_t log_level, const char *fmt, ...);

/*****************************************************************************/
/**
* @brief This is used to read the character entered on the serial teminal.
*
* @param[out]   single character read
*
* @return    Number of bytes read or error code
*
* @note None
**
******************************************************************************/
int32_t VMC_User_Input_Read(char *ReadChar, uint32_t *receivedBytes);

/*****************************************************************************/
/**
* @brief Test to get board information
*
**
******************************************************************************/
void BoardInfoTest(void);

/*****************************************************************************/
/**
* @brief Display of all the readings from different sensors
*
**
******************************************************************************/
void SensorData_Display(void);

/**
* @brief This function is to test the read/write functionality of EEPROM
*
* @param None
*
* @return None
*
* @note None
**
******************************************************************************/
void EepromTest(void);
/*****************************************************************************/
/**
* @brief This function is to dump the contents of EEPROM, 64 bytes at a time
*
* @param None
*
* @return None
*
* @note None
**
******************************************************************************/
void EepromDump(void);

#endif /* INC_VMC_API_H_ */
