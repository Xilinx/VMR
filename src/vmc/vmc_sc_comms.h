/******************************************************************************
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/
#include "xil_types.h"
#include "FreeRTOS.h"
#include "task.h"


#include "sysmon.h"
#include "cl_uart_rtos.h"
#include "vmc_api.h"


#define SOP_SIZE            (2)
#define MESSAGE_ID_SIZE     (1)
#define FLAGS_SIZE          (1)
#define PAYLOAD_LENGTH_SIZE (1)
#define CHECKSUM_SIZE       (2)
#define EOP_SIZE            (2)

#define STX                 (0x02)
#define ETX                 (0x03)
#define ESCAPE_CHAR         (0x5C)

#define COMMS_PAYLOAD_START_INDEX    5
#define RAW_PAYLOAD_LENTGH           9

#define MAX_VMC_SC_UART_BUF_SIZE     255
#define MIN_VMC_SC_UART_BUF_SIZE     2
#define SENSOR_RSP_LENGTH            11
#define TEMP_SNSR_LENGTH             12
#define VERSION_POWERMODE_LENGTH     18
#define VMC_ACTIVE_LENGTH 			 10


#define MSP432_COMMS_SUPPORTED_MSG      (0x0A)
#define MSP432_COMMS_ALERT_RESP         (0x82)
#define MSP432_COMMS_ADV_VERS_RESP      (0x83)
#define MSP432_COMMS_BOARD_SNSR_RESP    (0x85)
#define MSP432_COMMS_VOLT_SNSR_RESP     (0x86)
#define MSP432_COMMS_POWER_SNSR_RESP    (0x87)
#define MSP432_COMMS_TEMP_SNSR_RESP     (0x88)
#define MSP432_COMMS_EN_DEMO_RESP       (0x89)

#define MSP432_COMMS_MSG_GOOD           (0xFE)
#define MSP432_COMMS_MSG_ERR            (0xFF)

#define MSP432_COMMS_OEM_CMD_REQ        (0x6F)
#define MSP432_COMMS_OEM_CMD_RESP       (0xEF)
#define PAYLOAD_SIZE_OEM_ID_RESP        (0x04)

#define MSP432_COMMS_NULL               (0x00)
#define MSP432_COMMS_EN_BSL             (0x01)
#define MSP432_COMMS_ALERT_REQ          (0x02)
#define MSP432_COMMS_ADV_VERS           (0x03)
#define MSP432_COMMS_SET_VERS           (0x04)
#define MSP432_COMMS_BOARD_SNSR_REQ     (0x05)
#define MSP432_COMMS_VOLT_SNSR_REQ      (0x06)
#define MSP432_COMMS_POWER_SNSR_REQ     (0x07)
#define MSP432_COMMS_TEMP_SNSR_REQ      (0x08)
#define MSP432_COMMS_EN_DEMO_MENU       (0x09)

#define MSP432_COMMS_NO_FLAG            (0x00)
#define MSP432_COMMS_VMC_VERSION        (0x50)
#define MSP432_COMMS_VMC_ACTIVE_REQ 	(0xF0)

#define MSP432_COMMS_VMC_VERSION_POWERMODE_REQ (0xF1)
#define MSP432_COMMS_VMC_VERSION_POWERMODE_RESP (0xF2)

#define MSP432_COMMS_VMC_SEND_I2C_SNSR_REQ (0xF3)
#define MSP432_COMMS_VMC_SEND_I2C_SNSR_RESP (0xF4)

#define MSP432_COMMS_VMC_GET_RESP_SIZE_REQ (0xF5)
#define MSP432_COMMS_VMC_GET_RESP_SIZE_RESP (0xF6)


typedef enum return_error_codes {
    FIELD_PARSE_SUCCESSFUL,                 /* The current field was parsed successfully */
    MSP432_COMMS_CHKSUM_ERR,                /* did not match for message */
    MSP432_COMMS_EOP_ERR,                   /* EOP was detected too early (i.e. dropped byte during communication) */
    MSP432_COMMS_SOP_ERR,                   /* SOP was detected before EOP of last message. (i.e. dropped byte during communication) */
    MSP432_COMMS_ESC_SEQ_ERR,               /* Undefined escape character sequence */
    MSP432_COMMS_BAD_MSG_ID,                /* Unrecognized message id */
    MSP432_COMMS_BAD_VERSION,               /* Unable to configure to specified version */
    MSP432_COMMS_RX_BUF_OVERFLOW,           /* XMC's Receive buffer overflowed*/
    MSP432_COMMS_BAD_SNSR_ID,               /* Unrecognized sensor (Byte 2 should be the sensor ID) */
    MSP432_COMMS_NS_MSG_ID,                 /* Message ID not supported */
    MSP432_COMMS_SC_FUN_ERR,                /* Indicates SC functionality Error */
    MSP432_COMMS_FAIL_TO_EN_BSL             /* Unable to Enter into BSL mode*/
}return_error_codes;

typedef enum Comms_field_parser {

	SOP_ESCAPE_CHAR,
	SOP_ETX,
	Rsp_MessageID,
	Flags,
	PayloadLength,
	EOP_ESCAPE_CHAR,
	EOP_ETX

}Comms_field_parser;


typedef struct vmc_sc_uart_cmd {
    uint8_t SOP[2];
    uint8_t MessageID;
    uint8_t Flags;
    uint8_t PayloadLength;
    uint8_t Payload[255];
    uint8_t Checksum[2];
    uint8_t EOP[2];

}vmc_sc_uart_cmd;


/* List sensor ids used for communicating between MB and MSP */
typedef enum
{
    PEX_12V = 0,
    PEX_3V3,
    AUX_3V3,
    AUX_12V,
    DDR4_VPP_BTM,
    SYS_5V5,
    VCC1V2_TOP,
    VCC1V8,
    VCC0V85,
    DDR4_VPP_TOP,
    MGT0V9AVCC,
    SW_12V,
    MGTAVTT,
    VCC1V2_BTM,
    PEX_12V_I_IN,
    AUX_12V_I_IN,
    VCCINT,
    VCCINT_I,
    FPGA_TEMP,
    FAN_TEMP,
    DIMM_TEMP0,
    DIMM_TEMP1,
    DIMM_TEMP2,
    DIMM_TEMP3,
    SE98_TEMP0,
    SE98_TEMP1,
    SE98_TEMP2,
    FAN_SPEED,
    CAGE_TEMP0,
    CAGE_TEMP1,
    CAGE_TEMP2,
    CAGE_TEMP3,
    RESERVED5,
    BOARD_SN,
    MAC_ADDRESS0,
    MAC_ADDRESS1,
    MAC_ADDRESS2,
    MAC_ADDRESS3,
    BOARD_REV,
    BOARD_NAME,
    BMC_VERSION,
    TOTAL_POWER_AVAIL,
    FAN_PRESENCE,
    CONFIGURATION_MODE,
    HBM_TEMP1 = 0x30,
    VCC_3V3,
    PEX3V3_I_IN,
    VCC0V85_I,
    HBM_1V2,
    VPP2V5,
    VCCINT_BRAM,
    HBM_TEMP2,
    AUX1_12V,
    VCCINT_TEMP = 0x39,
    PEX_12V_POWER,
    PEX_3V3_POWER,
    AUX_3V3_I,
    VCC1V2_I = 0x3F,
    V12_IN_I,
    V12_IN_AUX0_I,
    V12_IN_AUX1_I,
    VCCAUX,
    VCCAUX_PMC,
    VCCRAM,
    POWER_GOOD = 0x46,
    NEW_MAC_SCHEME_ID = 0x4B,
    HBM_T_1V2 = 0x4C,
    HBM_B_1V2,
    VPP_T_2V5,
    VPP_B_2V5,
    SENSOR_ID_MAX
}sensor_id_list;

typedef struct SC_VMC_Data{
    u8   voltsensorlength;
    u8   powersensorlength;
    u8   availpower;
    u8   scVersion[4];
    u32  VCCINT_sensor_value;
    u16  sensor_values[SENSOR_ID_MAX];
}SC_VMC_Data;



