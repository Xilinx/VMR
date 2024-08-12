/******************************************************************************
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#ifndef INC_VMC_VMC_SC_COMMS_H_
#define INC_VMC_VMC_SC_COMMS_H_

#include "xil_types.h"
#include "FreeRTOS.h"
#include "task.h"


#include "xsysmonpsv.h"
#include "cl_uart_rtos.h"
#include "vmc_api.h"

#include "vck5000.h"

extern uart_rtos_handle_t uart_vmcsc_log;

typedef u8 (*msg_id_ptr);
typedef s32 (*Fetch_BoardInfo_Func)(u8 *);

#define SOP_SIZE            (2)
#define MESSAGE_ID_SIZE     (1)
#define FLAGS_SIZE          (1)
#define PAYLOAD_LENGTH_SIZE (1)
#define PAYLOAD_SIZE        (255)
#define CHECKSUM_SIZE       (2)
#define EOP_SIZE            (2)

#define STX                 (0x02)
#define ETX                 (0x03)
#define ESCAPE_CHAR         (0x5C)

#define MSG_ID_OFFSET			(2u)
#define FLAG_OFFSET			(3u)
#define PAYLOAD_LEN_L_OFFSET		(4u)
#define PAYLOAD_LEN_H_OFFSET		(5u)
#define PAYLOAD_START_OFFSET_NO_FLAG	(5u)
#define PAYLOAD_START_OFFSET_W_FLAG	(6u)
#define CHECKSUM_L_OFFSET		(4u)
#define CHECKSUM_H_OFFSET		(3u)
#define TOTAL_FOOTER_SIZE		(4u)

// board info sizes
#define BOARD_NAME_SIZE     17
#define BOARD_REV_SIZE      9
#define BOARD_SERIAL_SIZE   15
#define MAC_ADDRESS_SIZE    18
#define SAT_FW_REV_SIZE     9

#define CMC_NUM_QSFP_CAGES  2
#define CAGE_GPIO_INT_L     (1<<4)
#define CAGE_GPIO_MODPRS_L  (1<<3)
#define CAGE_GPIO_MODSEL_L  (1<<2)
#define CAGE_GPIO_RESET_L   (1<<1)
#define CAGE_GPIO_LPMODE    (1)


#define COMMS_PAYLOAD_START_INDEX    5
#define RAW_PAYLOAD_LENGTH           9

#define MAX_VMC_SC_UART_BUF_SIZE            255
#define MAX_VMC_SC_UART_COUNT_WITH_INTR     60
#define MAX_VMC_SC_UART_COUNT_WITHOUT_INTR  32



#define MSP432_COMMS_SUPPORTED_MSG      (0x0A)
#define MSP432_COMMS_ALERT_RESP         (0x82)
#define MSP432_COMMS_ADV_VERS_RESP      (0x83)
#define MSP432_COMMS_BOARD_SNSR_RESP    (0x85)
#define SC_COMMS_RX_VOLT_SNSR_RESP     (0x86)
#define SC_COMMS_RX_POWER_SNSR_RESP    (0x87)
#define SC_COMMS_RX_TEMP_SNSR_RESP     (0x88)
#define MSP432_COMMS_EN_DEMO_RESP       (0x89)

#define MSP432_COMMS_MSG_GOOD           (0xFE)
#define MSP432_COMMS_MSG_ERR            (0xFF)

#define MSP432_COMMS_GENERAL_RESP_LEN   (0x0B)

#define MSP432_COMMS_OEM_CMD_REQ        (0x6F)
#define MSP432_COMMS_OEM_CMD_RESP       (0xEF)
#define PAYLOAD_SIZE_OEM_ID_RESP        (0x04)

#define MSP432_COMMS_NULL               (0x00)
#define MSP432_COMMS_EN_BSL             (0x01)
#define MSP432_COMMS_ALERT_REQ          (0x02)
#define MSP432_COMMS_ADV_VERS           (0x03)
#define MSP432_COMMS_SET_VERS           (0x04)
#define MSP432_COMMS_BOARD_SNSR_REQ     (0x05)
#define SC_COMMS_RX_VOLT_SNSR      (0x06)
#define SC_COMMS_RX_POWER_SNSR     (0x07)
#define SC_COMMS_RX_TEMP_SNSR       (0x08)
#define MSP432_COMMS_EN_DEMO_MENU       (0x09)

#define MSP432_COMMS_NO_FLAG             (0x00)
#define MSP432_COMMS_VMC_VERSION             (0x50)
#define MSP432_COMMS_VMC_ACTIVE_REQ 	(0xF0)

#define MSP432_COMMS_VMC_VERSION_POWERMODE_REQ (0xF1)
#define MSP432_COMMS_VMC_VERSION_POWERMODE_RESP (0xF2)

#define SC_COMMS_TX_I2C_SNSR (0xF3)
#define SC_COMMS_RX_I2C_SNSR_RESP (0xF4)

#define MSP432_COMMS_VMC_GET_RESP_SIZE_REQ (0xF5)
#define MSP432_COMMS_VMC_GET_RESP_SIZE_RESP (0xF6)

#define SC_COMMS_TX_BOARD_INFO  (0xF7)
#define SC_COMMS_RX_BOARD_INFO_RESP (0xF8)

#define MIN_BYTE_CNT_FOR_SC_PKT_VALIDATION	(0x02)

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

/* List sensor ids used for communicating between VMR and MSP */
typedef enum
{
    eSC_INLET_TEMP = 0,
    eSC_OUTLET_TEMP,
    eSC_BOARD_TEMP,
    eSC_FPGA_TEMP,
    eSC_CAGE_TEMP0,
    eSC_CAGE_TEMP1,
    eSC_TOTAL_POWER
}oob_sensor_ids;

typedef struct SC_VMC_Data{
    bool boardInfoStatus;
    u8   voltSensorLength;
    u8   powerSensorLength;
    u8   tempSensorLength;
    u8   powerMode;
    u8   configmode;
    u8   scVersion[4];
    u32  VCCINT_sensor_value;
    /* Stores Sensor values recvd from SC */
    u16  sensor_values[eSC_SENSOR_ID_MAX];
}SC_VMC_Data;

void VMC_SC_COMMS_Tx_Rx(u8 messageID);
bool vmc_get_sc_status();
void vmc_set_sc_status(bool value);
bool vmc_get_power_mode_status();
void vmc_set_power_mode_status(bool value);
bool vmc_get_snsr_resp_status();
void vmc_set_snsr_resp_status(bool value);
bool vmc_get_boardInfo_status();
void vmc_set_boardInfo_status(bool value);
u8 get_total_req_size();
void set_total_req_size(u8 value);
bool VMC_UART_CMD_Transaction(u8 Message_id , u8 Flags, u8 Payloadlength, u8 *Payload, u8 Expected_Msg_Length);


#endif /* INC_VMC_VMC_SC_COMMS_H_ */
