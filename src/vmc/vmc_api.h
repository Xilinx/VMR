
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
#include "cl_config.h"
#include "cl_io.h"

#define	VMC_STRING	"VMC"

#ifdef VMC_DEBUG
#warning "When enabled RPU UART RX has conflic with APU UART so need to disable XRT code (RMGMT_Launch and cl_msg_service_launch)."

#define VMC_DEMO

#define VMC_DMO(fmt, arg...) 		\
	VMC_Printf(__FILENAME__, __LINE__, VMC_LOG_LEVEL_DEMO_MENU, fmt,##arg)
#define VMC_PRNT(fmt, arg...) 		\
	VMC_Printf(__FILENAME__, __LINE__, VMC_LOG_LEVEL_INFO, fmt,##arg)
#define VMC_LOG(fmt, arg...) 		\
	VMC_Printf(__FILENAME__, __LINE__, VMC_LOG_LEVEL_INFO, "%s: %s " fmt "\r\n", 	\
			VMC_STRING, __FUNCTION__, ##arg)
#define VMC_ERR(fmt, arg...) 		\
	VMC_Printf(__FILENAME__, __LINE__, VMC_LOG_LEVEL_ERROR,"%s[ERROR]: %s" fmt "\r\n", 	\
			VMC_STRING, __FUNCTION__, ##arg)
#define VMC_DBG(fmt, arg...) 		\
	VMC_Printf(__FILENAME__, __LINE__, VMC_LOG_LEVEL_DEBUG,"%s[DEBUG]: %s:%d %s " fmt "\r\n", 	\
			VMC_STRING, __FILENAME__, __LINE__, __FUNCTION__, ##arg)
#else
#define VMC_DMO(fmt, arg...)	\
	CL_UART_DMO(APP_VMC, fmt, ##arg)
#define VMC_PRNT(fmt, arg...)	\
	CL_UART_PRNT(APP_VMC, fmt, ##arg)
#define VMC_LOG(fmt, arg...)	\
	CL_UART_LOG(APP_VMC, fmt, ##arg)
#define VMC_ERR(fmt, arg...)	\
	CL_UART_ERR(APP_VMC, fmt, ##arg)
#define VMC_DBG(fmt, arg...)	\
	CL_UART_DBG(APP_VMC, fmt, ##arg)

#endif

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

typedef struct Versal_BoardInfo
{
    unsigned char product_name[17];
    unsigned char board_rev[9];
    unsigned char board_serial[15];
    unsigned char eeprom_version[4];
    unsigned char board_mac[4][7];
    unsigned char board_act_pas[2];
    unsigned char board_config_mode[2];
    unsigned char board_mfg_date[4];
    unsigned char board_part_num[10];
    unsigned char board_uuid[17];
    unsigned char board_pcie_info[9];
    unsigned char board_max_power_mode[2];
    unsigned char Memory_size[5];
    unsigned char OEM_ID[5];
    unsigned char DIMM_size[5];
    unsigned char Num_MAC_IDS;
} Versal_BoardInfo;

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
/*****************************************************************************/
/**
* @brief This function is to read Board Info from EEPROM
*
* @param None
*
* @return u8
*
* @note None
**
******************************************************************************/
u8 Versal_EEPROM_ReadBoardIno(void);

#ifdef PLATFORM_CS2200

void Temp_LM75_Display(void);
void INA3221_Display(void);
void ISL68220_Display(void);
void VPD_EEPROM_ReadBoardIno(void);
#endif


#endif /* INC_VMC_API_H_ */
