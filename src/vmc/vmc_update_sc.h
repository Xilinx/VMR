
#ifndef SRC_VMC_VMC_UPDATE_SC_H_
#define SRC_VMC_VMC_UPDATE_SC_H_


#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>

#define ULONG_MAX 						0xFFFFFFFFUL

#define TIMEOUT_MS(x)   				( ((x)*10)     /portTICK_PERIOD_MS )

#define BSL_VERSION_REQ					(0x06)
#define BSL_VERSION_RESP				(0x11)
#define BSL_REBOOT_RESET_REQ			(0x06)
#define BSL_LOAD_PC_REQ					(0x0A)
#define BSL_DATA_TX_32_RESP				(0x08)
#define BSL_MASS_ERASE_REQ				(0x06)
#define BSL_MASS_ERASE_RESP				(0x08)
#define BSL_UNLOCK_PASSWORD_REQ			(0x3E)
#define BSL_UNLOCK_PASSWORD_RESP		(0x08)
#define BSL_SYNCED_REQ					(0x01)
#define BSL_SYNCED_RESP					(0x01)
#define BSL_CRC_RESQ					(0x0C)
#define BSL_CRC_RESP					(0x09)


#define CMD_RX_DATA_BLOCK_32			(0x20)
#define CMD_RX_PASSWORD					(0x21)
#define CMD_MASS_ERASE					(0x15)
#define CMD_REBOOT_RESET				(0x25)
#define CMD_CRC_CHECK_32				(0x26)
#define CMD_LOAD_PC_32					(0x27)
#define CMD_MSP_GET_STATUS				(0x31)
#define CMD_SC_ENABLE_BSL				(0x32)
#define CMD_TX_BSL_VERSION				(0x19)

#define MSP_IN_SC_MODE					(0x02)
#define MSP_IN_BSL_MODE					(0x01)

#define BSL_MSG_HEADER					(0x80)

#define SECTION_START					(0x40)
#define FILE_TERMINATION_CHAR			(0x71)


#define UART_TX_FIFO_SIZE				(0x20)
#define BSL_MAX_DATA_SIZE				(266u)

#define SC_HEADER_SIZE					(0x0000000B)
#define SC_RES_HEADER_SIZE	    		(0x00000005)
#define SC_TOT_HEADER_SIZE				(SC_HEADER_SIZE + SC_RES_HEADER_SIZE)


#define SC_HEADER_MSG					("VERSAL_SCFW")

#define BYTES_TO_READ					(0x01)
#define BSL_DATA_PACKET_CRC_SIZE		(0x02)


typedef struct efpt_sc
{
	u32 start_address;
	u32 size;
} efpt_sc_t;


typedef enum
{
	PARSE_ADDRESS = 0,
	PARSE_DATA
}sc_parse_states;


typedef struct
{
	uint8_t header;
	uint8_t dataLen[2];
	uint8_t *data;
	uint8_t checksum[2];
}bslpkt_req;


typedef struct
{
	uint8_t ack;
	uint8_t header;
	uint8_t dataLen[2];
	uint8_t coreCMD;
	uint8_t respMsg;
	uint8_t checksum[2];
}bslpkt_resp;


typedef enum
{
	STATUS_SUCCESS = 0,
	STATUS_FAILURE,
	STATUS_IN_PROGRESS
}upgrade_status;

typedef enum
{
	SC_STATE_IDLE = 0,
	SC_ENABLE_BSL,
	BSL_SYNCED,
	BSL_UNLOCK_PASSWORD,
	BSL_MASS_ERASE,
	BSL_SECTOR_ERASE,
	BSL_DATA_TX_32,
	BSL_CRC_RX_32,
	BSL_LOAD_PC_32,
	MSP_GET_STATUS,
	BSL_REBOOT_RESET,
	TX_BSL_VERSION
}upgrade_state;



int32_t VMC_Start_SC_Update(void);
void SC_Update_Task_Create(void);
void VMC_Parse_Fpt_SC(uint32_t addr_location, uint8_t *bsl_send_data_pkt , uint16_t *pkt_length);
void VMC_Parse_Fpt_SC_Version(uint32_t addr_location, uint8_t *bsl_send_data_pkt);
bool VMC_Read_SC_FW(void);
uint8_t Get_SC_Checksum(void);
uint8_t Check_Received_SC_Header(void *ptr1, void *ptr2, uint8_t len);
upgrade_status matchCRC_postWrite(unsigned int writeAdd);
uint8_t VMC_SCFW_Program_Progress(void);

#endif /* SRC_VMC_VMC_UPDATE_SC_H_ */
