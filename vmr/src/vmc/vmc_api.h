
/******************************************************************************
* Copyright (C) 2024 AMD, Inc.    All rights reserved.
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

#define VMC_STRING  "VMC"

//#define DISABLE_LIMITS_FOR_TEST
//#define INCREASE_LIMITS_FOR_TEST

#ifdef VMC_DEBUG
//#warning "When enabled RPU UART RX has conflic with APU UART so need to disable XRT code (RMGMT_Launch and cl_msg_service_launch)."

//#define VMC_TEST_VCK5000
//#define VMC_TEST_V70

#define VMC_DMO(fmt, arg...)        \
    VMC_Printf(__FILENAME__, __LINE__, VMC_LOG_LEVEL_DEMO_MENU, fmt,##arg)
#define VMC_PRNT(fmt, arg...)       \
    VMC_Printf(__FILENAME__, __LINE__, VMC_LOG_LEVEL_INFO, fmt,##arg)
#define VMC_LOG(fmt, arg...)        \
    VMC_Printf(__FILENAME__, __LINE__, VMC_LOG_LEVEL_INFO, "%s: %s " fmt "\r\n",    \
            VMC_STRING, __FUNCTION__, ##arg)
#define VMC_ERR(fmt, arg...)        \
    VMC_Printf(__FILENAME__, __LINE__, VMC_LOG_LEVEL_ERROR,"%s[ERROR]: %s" fmt "\r\n",  \
            VMC_STRING, __FUNCTION__, ##arg)
#define VMC_DBG(fmt, arg...)        \
    VMC_Printf(__FILENAME__, __LINE__, VMC_LOG_LEVEL_DEBUG,"%s[DEBUG]: %s:%d %s " fmt "\r\n",   \
            VMC_STRING, __FILENAME__, __LINE__, __FUNCTION__, ##arg)
#else
#define VMC_DMO(fmt, arg...)    \
    CL_DMO(APP_VMC, fmt, ##arg)
#define VMC_PRNT(fmt, arg...)   \
    CL_PRNT(APP_VMC, fmt, ##arg)
#define VMC_LOG(fmt, arg...)    \
    CL_LOG(APP_VMC, fmt, ##arg)
#define VMC_ERR(fmt, arg...)    \
    CL_ERR(APP_VMC, fmt, ##arg)
#define VMC_DBG(fmt, arg...)    \
    CL_DBG(APP_VMC, fmt, ##arg)

#endif

#include <string.h>

#define VMC_LOG_LEVEL_VERBOSE       0
#define VMC_LOG_LEVEL_DEBUG         1
#define VMC_LOG_LEVEL_INFO          2
#define VMC_LOG_LEVEL_WARN          3
#define VMC_LOG_LEVEL_ERROR         4
#define VMC_LOG_LEVEL_DEMO_MENU     5
#define VMC_LOG_LEVEL_NONE          6     /* disable logging */

#ifndef __FILENAME__
#define __FILENAME__                 (strrchr(__FILE__, '/') ? (strrchr(__FILE__, '/')+1) : __FILE__)
#endif

/* Current EEPROM versions that VMR supports */
#define EEPROM_V3_0               (0x332E30u)
#define EEPROM_V2_0               (0x322E30u)

/* Default register content in EEPROM if a particular register has not been programmed */
#define EEPROM_DEFAULT_VAL          0xFF

#define EEPROM_VERSION_OFFSET               0x0000
#define EEPROM_VERSION_SIZE                 3

#define EEPROM_V2_0_PRODUCT_NAME_OFFSET         0x0300
#define EEPROM_V2_0_PRODUCT_NAME_SIZE           16

#define EEPROM_V2_0_BOARD_REV_OFFSET            0x1300
#define EEPROM_V2_0_BOARD_REV_SIZE              8

#define EEPROM_V2_0_BOARD_SERIAL_OFFSET         0x1B00
#define EEPROM_V2_0_BOARD_SERIAL_SIZE           14

#define EEPROM_V2_0_BOARD_MAC_OFFSET            0x2900
#define EEPROM_V2_0_BOARD_MAC_SIZE              6

#define EEPROM_V2_0_BOARD_TOT_MAC_ID_OFFSET     0xFFFF /* Not present in v2.0 version */
#define EEPROM_V2_0_BOARD_NUM_MAC               2

#define EEPROM_V2_0_BOARD_ACT_PAS_OFFSET        0x4100
#define EEPROM_V2_0_BOARD_ACT_PAS_SIZE          1

#define EEPROM_V2_0_BOARD_CONFIG_MODE_OFFSET    0x4200
#define EEPROM_V2_0_BOARD_CONFIG_MODE_SIZE      1

#define EEPROM_V2_0_MFG_DATE_OFFSET             0x4300
#define EEPROM_V2_0_MFG_DATE_SIZE               3

#define EEPROM_V2_0_PART_NUM_OFFSET             0x4600
#define EEPROM_V2_0_PART_NUM_SIZE               9

#define EEPROM_V2_0_UUID_OFFSET                 0x4F00
#define EEPROM_V2_0_UUID_SIZE                   16

#define EEPROM_V2_0_PCIE_INFO_OFFSET            0x5F00
#define EEPROM_V2_0_PCIE_INFO_SIZE              8

#define EEPROM_V2_0_MAX_POWER_MODE_OFFSET       0x6700
#define EEPROM_V2_0_MAX_POWER_MODE_SIZE         1

#define EEPROM_V2_0_DIMM_SIZE_OFFSET            0x6800
#define EEPROM_V2_0_DIMM_SIZE_SIZE              4

#define EEPROM_V2_0_OEMID_SIZE_OFFSET       0x6C00
#define EEPROM_V2_0_OEMID_SIZE                  4

#define EEPROM_V2_0_CAPABILITY_OFFSET           0xFFFF /* Not present in v2.0 version */
#define EEPROM_V2_0_CAPABILITY_SIZE             0

#define EEPROM_V3_0_PRODUCT_NAME_OFFSET         0x0800
#define EEPROM_V3_0_PRODUCT_NAME_SIZE           24

#define EEPROM_V3_0_BOARD_REV_OFFSET            0x2000
#define EEPROM_V3_0_BOARD_REV_SIZE              8

#define EEPROM_V3_0_BOARD_SERIAL_OFFSET         0x2800
#define EEPROM_V3_0_BOARD_SERIAL_SIZE           14

#define EEPROM_V3_0_BOARD_TOT_MAC_ID_OFFSET     0x3600
#define EEPROM_V3_0_BOARD_TOT_MAC_ID_SIZE   1

#define EEPROM_V3_0_BOARD_MAC_OFFSET            0x3700
#define EEPROM_V3_0_BOARD_MAC_SIZE              6

#define EEPROM_V3_0_BOARD_ACT_PAS_OFFSET        0x3D00
#define EEPROM_V3_0_BOARD_ACT_PAS_SIZE          1

#define EEPROM_V3_0_BOARD_CONFIG_MODE_OFFSET    0x3E00
#define EEPROM_V3_0_BOARD_CONFIG_MODE_SIZE      1

#define EEPROM_V3_0_MFG_DATE_OFFSET             0x3F00
#define EEPROM_V3_0_MFG_DATE_SIZE               3

#define EEPROM_V3_0_PART_NUM_OFFSET             0x4200
#define EEPROM_V3_0_PART_NUM_SIZE               24

#define EEPROM_V3_0_UUID_OFFSET                 0x5A00
#define EEPROM_V3_0_UUID_SIZE                   16

#define EEPROM_V3_0_PCIE_INFO_OFFSET            0x6A00
#define EEPROM_V3_0_PCIE_INFO_SIZE              8

#define EEPROM_V3_0_MAX_POWER_MODE_OFFSET       0x7200
#define EEPROM_V3_0_MAX_POWER_MODE_SIZE         1

#define EEPROM_V3_0_MEM_SIZE_OFFSET             0x7300
#define EEPROM_V3_0_MEM_SIZE_SIZE               4

#define EEPROM_V3_0_OEMID_SIZE_OFFSET           0x7700
#define EEPROM_V3_0_OEMID_SIZE                  4

#define EEPROM_V3_0_CAPABILITY_OFFSET           0x7B00
#define EEPROM_V3_0_CAPABILITY_SIZE             2

#define EEPROM_V3_0_CHECKSUM_LSB_OFFSET         0x06
#define EEPROM_V3_0_CHECKSUM_MSB_OFFSET         0x07

#define EEPROM_V3_0_CHECKSUM_START              8
#define EEPROM_V3_0_CHECKSUM_END                124

typedef enum eeprom_data_e
{
    eEeprom_Product_Name,
    eEeprom_Board_Rev,
    eEeprom_Board_Serial,
    eEeprom_Board_Tot_Mac_Id,
    eEeprom_Board_Mac,
    eEeprom_Board_Act_Pas,
    eEeprom_Board_config_Mode,
    eEeprom_Mfg_Date,
    eEeprom_Part_Num,
    eEeprom_Uuid,
    eEeprom_Pcie_Info,
    eEeprom_Max_Power_Mode,
    eEeprom_mem_Size,
    eEeprom_Oemid_Size,
    eEeprom_Capability_Word,
    eEeprom_max_Offset,
} eEEPROM_Offsets_t;

typedef struct eeprom_data_s
{
    u16 offset;
    u8 size;
} EEPROM_Content_Details_t;


typedef struct Versal_BoardInfo
{
    u8 eeprom_version[EEPROM_VERSION_SIZE + 1];
    u8 product_name[EEPROM_V3_0_PRODUCT_NAME_SIZE + 1];
    u8 board_rev[EEPROM_V3_0_BOARD_REV_SIZE + 1];
    u8 board_serial[EEPROM_V3_0_BOARD_SERIAL_SIZE + 1];
    u8 Num_MAC_IDS;
    u8 board_mac[4][7];
    u8 board_act_pas[EEPROM_V3_0_BOARD_ACT_PAS_SIZE + 1];
    u8 board_config_mode[EEPROM_V3_0_BOARD_CONFIG_MODE_SIZE + 1];
    u8 board_mfg_date[EEPROM_V3_0_MFG_DATE_SIZE + 1];
    u8 board_part_num[EEPROM_V3_0_PART_NUM_SIZE + 1];
    u8 board_uuid[EEPROM_V3_0_UUID_SIZE + 1];
    u8 board_pcie_info[EEPROM_V3_0_PCIE_INFO_SIZE + 1];
    u8 board_max_power_mode[EEPROM_V3_0_MAX_POWER_MODE_SIZE + 1];
    u8 Memory_size[EEPROM_V3_0_MEM_SIZE_SIZE + 1];
    u8 OEM_ID[EEPROM_V3_0_OEMID_SIZE + 1];
    u8 DIMM_size[EEPROM_V3_0_MEM_SIZE_SIZE + 1];
    u8 capability[EEPROM_V3_0_CAPABILITY_SIZE + 1];
} Versal_BoardInfo;

#define     MAX_PLATFORM_NAME_LEN (20u)
#define     MAX_FUNCTION_NAME_LEN (20u)

typedef enum{
    eVCK5000 = 0, /* VCK5000 or V350, so 2 platforms in one enum*/
    eV70 = 2,
    eV80,
    eMax_Platforms,
} ePlatformType;

typedef struct __attribute__((packed)) {
    ePlatformType   product_type_id;
    char        product_type_name[MAX_PLATFORM_NAME_LEN];
} Platform_t;

typedef enum{
    eTemperature_Sensor_Inlet = 0,
    eTemperature_Sensor_Outlet,
    eTemperature_Sensor_Board,
    eTemperature_Sensor_QSFP,
    eTemperature_Sensors,
    eVccint_Temp,
    eVoltage_Sensors,
    eCurrent_Sensors,
    eQSFP_Sensors,
    ePower_Sensor,
    eMax_Sensor_Functions,
} eSensor_Functions;

typedef struct __attribute__((packed)) {
    ePlatformType   product_type_id;
    eSensor_Functions   sensor_type;
    sensorMonitorFunc   sensor_handler;
    snsrNameFunc    sensor_name_handler;
}Platform_Sensor_Handler_t;

typedef u8 (*Platform_Func_Ptr)(void);

typedef enum{
    ePlatform_Init,
    eSc_Comms,
    eMax_Platform_Functions
} ePlatform_Functions;

typedef struct __attribute__((packed)){
    ePlatformType   product_type_id;
    ePlatform_Functions func_type;
    Platform_Func_Ptr   func_handler;
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
u8 Versal_Print_BoardInfo(void);

extern sensorMonitorFunc Temperature_Read_Inlet_Ptr;
extern sensorMonitorFunc Temperature_Read_Outlet_Ptr;
extern sensorMonitorFunc Temperature_Read_Board_Ptr;
extern sensorMonitorFunc Temperature_Read_QSFP_Ptr;
extern sensorMonitorFunc Temperature_Read_VCCINT_Ptr;
extern sensorMonitorFunc Power_Read_Ptr;

extern snsrNameFunc Temperature_Read_Ptr;
extern snsrNameFunc Voltage_Read_Ptr;
extern snsrNameFunc Current_Read_Ptr;
extern snsrNameFunc QSFP_Read_Ptr;


void VMC_Get_BoardInfo(Versal_BoardInfo *ptr);
s32 VMC_Send_BoardInfo_SC(u8 *board_snsr_data);

typedef void (*platform_sensors_monitor_ptr)(void);
typedef void (*supported_sdr_info_ptr) (u32 *supported_pdr, u32 *pdr_count);

#endif /* INC_VMC_API_H_ */
