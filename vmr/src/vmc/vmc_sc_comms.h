/******************************************************************************
 * Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
 * SPDX-License-Identifier: MIT
 *******************************************************************************/

#ifndef SRC_VMC_VMC_SC_COMMS_H_
#define SRC_VMC_VMC_SC_COMMS_H_

#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>


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

#define COMMS_PAYLOAD_START_INDEX    (5)
#define RAW_PAYLOAD_LENTGH           (9)

#define MAX_VMC_SC_UART_BUF_SIZE		(255)
#define MAX_VMC_SC_UART_COUNT_WITH_INTR		(60)
#define MAX_VMC_SC_UART_COUNT_WITHOUT_INTR	(32)

#define MSP432_COMMS_SUPPORTED_MSG		(0x0A)
#define MSP432_COMMS_ALERT_RESP			(0x82)
#define MSP432_COMMS_ADV_VERS_RESP		(0x83)
#define MSP432_COMMS_BOARD_SNSR_RESP		(0x85)
#define MSP432_COMMS_VOLT_SNSR_RESP		(0x86)
#define MSP432_COMMS_POWER_SNSR_RESP		(0x87)
#define MSP432_COMMS_TEMP_SNSR_RESP		(0x88)
#define MSP432_COMMS_EN_DEMO_RESP		(0x89)

#define MSP432_COMMS_MSG_GOOD			(0xFE)
#define MSP432_COMMS_MSG_ERR			(0xFF)

#define MSP432_COMMS_MSG_GOOD_LEN		(0x0A)
#define MSP432_COMMS_MSG_ERR_LEN		(0x0B)

#define MSP432_COMMS_OEM_CMD_REQ		(0x6F)
#define MSP432_COMMS_OEM_CMD_RESP		(0xEF)
#define PAYLOAD_SIZE_OEM_ID_RESP		(0x04)

#define MSP432_COMMS_NULL			(0x00)
#define MSP432_COMMS_EN_BSL			(0x01)
#define MSP432_COMMS_ALERT_REQ			(0x02)
#define MSP432_COMMS_ADV_VERS			(0x03)
#define MSP432_COMMS_SET_VERS			(0x04)
#define MSP432_COMMS_BOARD_SNSR_REQ		(0x05)
#define MSP432_COMMS_VOLT_SNSR_REQ		(0x06)
#define MSP432_COMMS_POWER_SNSR_REQ		(0x07)
#define MSP432_COMMS_TEMP_SNSR_REQ		(0x08)
#define MSP432_COMMS_EN_DEMO_MENU		(0x09)

#define MSP432_COMMS_NO_FLAG			(0x00)
#define MSP432_COMMS_VMC_VERSION		(0x50)
#define MSP432_COMMS_VMC_ACTIVE_REQ 		(0xF0)

#define MSP432_COMMS_VMC_VERSION_POWERMODE_REQ 	(0xF1)
#define MSP432_COMMS_VMC_VERSION_POWERMODE_RESP (0xF2)

#define MSP432_COMMS_VMC_SEND_I2C_SNSR_REQ 	(0xF3)
#define MSP432_COMMS_VMC_SEND_I2C_SNSR_RESP 	(0xF4)

#define MSP432_COMMS_VMC_GET_RESP_SIZE_REQ 	(0xF5)
#define MSP432_COMMS_VMC_GET_RESP_SIZE_RESP 	(0xF6)

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

/* List of sensor id's used for communicating between VMC and MSP */
typedef enum
{
	PEX_12V_SC = 0,
	PEX_3V3_SC,
	AUX_3V3_SC,
	AUX_12V_SC,
	PEX_12V_I_IN_SC = 0x0E,
	VCCINT_SC = 0x10,
	VCCINT_I_SC,
	FPGA_TEMP_SC,
	FAN_TEMP_SC,
	SE98_TEMP0_SC = 0x18,
	SE98_TEMP1_SC,
	CAGE_TEMP0_SC = 0x1C,
	CAGE_TEMP1_SC,
	BOARD_SN_SC = 0x21,
	MAC_ADDRESS0_SC,
	MAC_ADDRESS1_SC,
	MAC_ADDRESS2_SC,
	MAC_ADDRESS3_SC,
	BOARD_REV_SC,
	BOARD_NAME_SC,
	BMC_VERSION_SC,
	TOTAL_POWER_AVAIL_SC,
	FAN_PRESENCE_SC,
	AUX1_12V_SC = 0x38,
	VCCINT_TEMP_SC,
	V12_IN_AUX0_I_SC = 0x41,
	V12_IN_AUX1_I_SC,
	SENSOR_ID_MAX_SC
}sensor_id_list;

typedef struct SC_VMC_Data {
	bool boardInfoStatus;
	u8 voltsensorlength;
	u8 powersensorlength;
	u8 availpower;
	u8 configmode;
	u8 scVersion[4];
	u32 VCCINT_sensor_value;
	u16 sensor_values[SENSOR_ID_MAX_SC];
}SC_VMC_Data;


bool vmc_get_sc_status();
void vmc_set_sc_status(bool value);
bool vmc_get_power_mode_status();
void vmc_set_power_mode_status(bool value);
bool vmc_get_snsr_resp_status();
void vmc_set_snsr_resp_status(bool value);

#endif /* SRC_VMC_VMC_SC_COMMS_H_ */
