
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

#include "vmc_asdm.h"

#define	VMC_STRING	"VMC"

#ifdef VMC_DEBUG
//#warning "When enabled RPU UART RX has conflic with APU UART so need to disable XRT code (RMGMT_Launch and cl_msg_service_launch)."

#define VMC_TEST

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
	CL_DMO(APP_VMC, fmt, ##arg)
#define VMC_PRNT(fmt, arg...)	\
	CL_PRNT(APP_VMC, fmt, ##arg)
#define VMC_LOG(fmt, arg...)	\
	CL_LOG(APP_VMC, fmt, ##arg)
#define VMC_ERR(fmt, arg...)	\
	CL_ERR(APP_VMC, fmt, ##arg)
#define VMC_DBG(fmt, arg...)	\
	CL_DBG(APP_VMC, fmt, ##arg)

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

/* Default register content in EEPROM if a particular register has not been programmed */
#define EEPROM_DEFAULT_VAL               0xFF

#define EEPROM_VERSION_OFFSET            0x0000
#define EEPROM_VERSION_SIZE              3

#define EEPROM_PRODUCT_NAME_OFFSET       0x0300
#define EEPROM_PRODUCT_NAME_SIZE         16

#define EEPROM_BOARD_REV_OFFSET          0x1300
#define EEPROM_BOARD_REV_SIZE            8

#define EEPROM_BOARD_SERIAL_OFFSET       0x1B00
#define EEPROM_BOARD_SERIAL_SIZE         14

#define EEPROM_BOARD_MAC_OFFSET          0x2900 // EEPROM offset for the first MAC address
#define EEPROM_BOARD_MAC_SIZE            6
#define EEPROM_BOARD_NUM_MAC             2    // The EEPROM has 4 MAC addresses

#define EEPROM_BOARD_ACT_PAS_OFFSET      0x4100
#define EEPROM_BOARD_ACT_PAS_SIZE        1

#define EEPROM_BOARD_CONFIG_MODE_OFFSET  0x4200
#define EEPROM_BOARD_CONFIG_MODE_SIZE    1

#define EEPROM_MFG_DATE_OFFSET           0x4300
#define EEPROM_MFG_DATE_SIZE             3

#define EEPROM_PART_NUM_OFFSET           0x4600
#define EEPROM_PART_NUM_SIZE             9

#define EEPROM_UUID_OFFSET               0x4F00
#define EEPROM_UUID_SIZE                 16

#define EEPROM_PCIE_INFO_OFFSET          0x5F00
#define EEPROM_PCIE_INFO_SIZE            8

#define EEPROM_MAX_POWER_MODE_OFFSET     0x6700
#define EEPROM_MAX_POWER_MODE_SIZE       1

#define EEPROM_DIMM_SIZE_OFFSET          0x6800
#define EEPROM_DIMM_SIZE_SIZE            4

#define EEPROM_OEMID_SIZE_OFFSET         0x6C00
#define EEPROM_OEMID_SIZE                4


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

#define 	MAX_PLATFORM_NAME_LEN (20u)
#define 	MAX_FUNCTION_NAME_LEN (20u)

typedef enum{
	eVCK5000 = 0, /* VCK5000 or V350, so 2 platforms in one enum*/
	eV70 = 2,
	eMax_Platforms,
} ePlatformType;

typedef struct __attribute__((packed)) {
	ePlatformType 	product_type_id;
	char		product_type_name[MAX_PLATFORM_NAME_LEN];
} Platform_t;

typedef enum{
	eTemperature_Sensor_Inlet = 0,
	eTemperature_Sensor_Outlet,
	eTemperature_Sensor_Board,
	eTemperature_Sensor_QSFP,
	//eFAN_RPM_READ,
	eMax_Sensor_Functions,
} eSensor_Functions;

#if 0
typedef struct __attribute__((packed)) {
	eSensor_Functions	sensor_type;
	char	product_type_name[MAX_FUNCTION_NAME_LEN];
	//s8		(*sensor_handler)(snsrRead_t *snsrData);
	sensorMonitorFunc	sensor_handler;
}Sensor_Handler_t;
#endif

typedef struct __attribute__((packed)) {
	ePlatformType 	product_type_id;
	eSensor_Functions	sensor_type;
	//s8		(*sensor_handler)(snsrRead_t *snsrData);
	sensorMonitorFunc	sensor_handler;
}Platform_Sensor_Handler_t;

typedef u8 (*Platform_Func_Ptr)(void);

typedef enum{
	ePlatform_Init,
	eMax_Platform_Functions
} ePlatform_Functions;

typedef struct __attribute__((packed)){
	ePlatformType 	product_type_id;
	ePlatform_Functions	func_type;
	Platform_Func_Ptr	func_handler;
}Platform_Function_Handler_t;

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
void VMC_SetLogLevel(u8 LogLevel);

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
u8 VMC_GetLogLevel(void);

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
void VMC_Printf(char *filename, u32 line, u8 log_level, const char *fmt, ...);

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
s32 VMC_User_Input_Read(char *ReadChar, u32 *receivedBytes);

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
u8 Versal_EEPROM_ReadBoardInfo(void);

extern sensorMonitorFunc Temperature_Read_Inlet_Ptr;
extern sensorMonitorFunc Temperature_Read_Outlet_Ptr;
extern sensorMonitorFunc Temperature_Read_Board_Ptr;
extern sensorMonitorFunc Temperature_Read_QSFP_Ptr;
//extern sensorMonitorFunc Fan_RPM_Read_Ptr;

void VMC_Get_BoardInfo(Versal_BoardInfo *ptr);
s32 VMC_Send_BoardInfo_SC(u8 *board_snsr_data);

#endif /* INC_VMC_API_H_ */
