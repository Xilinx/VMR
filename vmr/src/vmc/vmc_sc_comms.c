
#include "FreeRTOS.h"
#include "task.h"

#include "cl_mem.h"
#include "vmc_sc_comms.h"
#include "vmc_update_sc.h"


#define MAX_RETRY_TO_CHK_SC_ACTIVE	  (10u)
#define SENSOR_MONITOR_TASK_NOTIFIED      (1)
#define SENSOR_MONITOR_TASK_NOT_NOTIFIED  (0)
#define SENSOR_MONITOR_LOCK_ACQUIRED      (1)
#define SENSOR_MONITOR_LOCK_RELEASED      (0)
#define SENSOR_MONITOR_TASK_MAX_NOTIFY_COUNT (3)

SC_VMC_Data sc_vmc_data;
extern SemaphoreHandle_t vmc_sc_lock;
/* VMC SC Comms handles and flags */
extern uart_rtos_handle_t uart_vmcsc_log;

u8 g_scData[MAX_VMC_SC_UART_BUF_SIZE] = {0x00};
u16 g_scDataCount = 0;
bool isPacketReceived;
u8 scPayload[MAX_VMC_SC_UART_BUF_SIZE] = {0};

static u8 vmc_active_resp_len = MSP432_COMMS_GENERAL_RESP_LEN;

static volatile bool is_SC_active  = false ;
static volatile bool isPowerModeActive = false;
static volatile bool getSensorRespLen = false;
static volatile bool is_boardInfo_sentTo_SC = false;
static u8 platform_specific_msgid_count = 0;

extern u8 Asdm_Send_I2C_Sensors_SC(u8 *scPayload);

Platform_Func_Ptr vmc_sc_comms_ptr = NULL;
Fetch_BoardInfo_Func fetch_boardinfo_ptr = NULL;
msg_id_ptr msg_id_handler_ptr = { 0 };

static uint16_t calculate_checksum(vmc_sc_uart_cmd *data)
{
    uint16_t checksum = 0;
    int i;
    checksum += data->MessageID;
    checksum += data->Flags;
    checksum += data->PayloadLength;

    for(i = 0; i < data->PayloadLength; ++i)
    {
        checksum += data->Payload[i];
    }

	return checksum;
}

u8 vmc_get_active_resp_len()
{
	return vmc_active_resp_len;
}

void vmc_set_active_resp_len(u8 value)
{
	vmc_active_resp_len = value;
}

bool vmc_get_sc_status()
{
	return is_SC_active;
}

void vmc_set_sc_status(bool value)
{
	is_SC_active = value;
}

bool vmc_get_power_mode_status()
{
	return isPowerModeActive;
}

void vmc_set_power_mode_status(bool value)
{
	isPowerModeActive = value;
}

bool vmc_get_snsr_resp_status()
{
	return getSensorRespLen;
}

void vmc_set_snsr_resp_status(bool value)
{
	getSensorRespLen = value;
}

bool vmc_get_boardInfo_status()
{
	return is_boardInfo_sentTo_SC;
}

void vmc_set_boardInfo_status(bool value)
{
	is_boardInfo_sentTo_SC = value;
}

void set_total_req_size(u8 value)
{
	platform_specific_msgid_count = value;
}

u8 get_total_req_size()
{
	return platform_specific_msgid_count;
}

u16 vmcU8ToU16(u8 *payload) {
	return (u16)payload[0] | (((u16)payload[1])<<8);
}

uint32_t vmcU8ToU32(uint8_t* payload)
{
	return (uint32_t)payload[0] | (((uint32_t)payload[1]) << 8) | (((uint32_t)payload[2]) << 16) | (((uint32_t)payload[3]) << 24);
}

uint64_t vmcU8ToU64(uint8_t * payload)
{
    return (uint64_t)payload[5] | (((uint64_t)payload[4]) << 8) | (((uint64_t)payload[3]) << 16) | (((uint64_t)payload[2]) << 24) |
         (((uint64_t)payload[1]) << 32) | (((uint64_t)payload[0]) << 40);
}

bool VMC_Validate_SC_Packet(u8 *rcvd_data, u32 length)
{
	bool status = false;
	u16 payload_len = 0;
	u16 rcvd_csum = 0;
	u16 calc_csum = 0;
	u16 payload_start_offset = 0;

	if ((rcvd_data[length - 1] == ETX) && (rcvd_data[length - 2] == ESCAPE_CHAR)) {
		if (rcvd_data[FLAG_OFFSET] == MSP432_COMMS_NO_FLAG) {
			payload_len = rcvd_data[PAYLOAD_LEN_L_OFFSET];
			payload_start_offset = PAYLOAD_START_OFFSET_NO_FLAG;
		} else {
			payload_len = ((rcvd_data[PAYLOAD_LEN_H_OFFSET] << 8) | rcvd_data[PAYLOAD_LEN_L_OFFSET]);
			payload_start_offset = PAYLOAD_START_OFFSET_W_FLAG;
		}

		calc_csum += rcvd_data[MSG_ID_OFFSET];
		calc_csum += rcvd_data[FLAG_OFFSET];
		calc_csum += payload_len;

		for (int i = 0; ((i < payload_len) && (payload_start_offset < (length - TOTAL_FOOTER_SIZE))); i++) {
			calc_csum += rcvd_data[payload_start_offset];
			payload_start_offset++;
		}

		rcvd_csum = ((rcvd_data[length - CHECKSUM_H_OFFSET] << 8) | (rcvd_data[length - CHECKSUM_L_OFFSET]));
		if (rcvd_csum == calc_csum) {
			status = true;
		}
	}

	return status;
}

void VMC_Update_Sensor_Length(u16 length,u8 *payload)
{
    switch(payload[0])
    {
    case SC_COMMS_RX_VOLT_SNSR:
    	sc_vmc_data.voltSensorLength = payload[1]+RAW_PAYLOAD_LENGTH;
    	break;
    case SC_COMMS_RX_POWER_SNSR:
    	sc_vmc_data.powerSensorLength = payload[1]+RAW_PAYLOAD_LENGTH;
    	break;
    case SC_COMMS_RX_TEMP_SNSR:
    	sc_vmc_data.tempSensorLength = payload[1]+RAW_PAYLOAD_LENGTH;
    	break;
    }
}

void VMC_Update_Version_PowerMode(u16 length,u8 *payload)
{
	u16 i = 0;

	for (i = 0; i < length;)
	{
		switch (payload[i]) {
		case eSC_BMC_VERSION:
			sc_vmc_data.scVersion[0] = payload[i + 2];  // fw_version
			sc_vmc_data.scVersion[1] = payload[i + 3];  // fw_rev_major
			sc_vmc_data.scVersion[2] = payload[i + 4];  // fw_rev_minor
			break;
		case eSC_POWER_MODE:
			sc_vmc_data.powerMode = payload[i + 2];
			break;
		}
		i = i + 2 + payload[i + 1];
	}
}

void VMC_StoreSensor_Value(u8 id, u32 value)
{
	if (xSemaphoreTake(vmc_sc_lock, portMAX_DELAY))
	{
		if(id == eSC_VCCINT_I) {
			sc_vmc_data.VCCINT_sensor_value = value;
		} else {
			sc_vmc_data.sensor_values[id] = value;
		}
		xSemaphoreGive(vmc_sc_lock);
	}
	else
	{
		VMC_LOG("vmc_sc_lock lock failed \r\n");
	}
}

void VMC_Update_Sensors(u16 length,u8 *payload)
{
    u16 i = 0;

    for(i=0; i<length;)
    {
	if (payload[i + 1] == 1)
	{
	    VMC_StoreSensor_Value(payload[i], (uint32_t)payload[i + 2]);
	}
	else if(payload[i + 1] == 2)
	{
	    VMC_StoreSensor_Value(payload[i], (uint32_t)vmcU8ToU16((uint8_t*)&payload[i + 2]));
	}
	else if (payload[i + 1] == 4)
	{
	    VMC_StoreSensor_Value(payload[i], (uint32_t)vmcU8ToU32((uint8_t*)&payload[i + 2]));
	}
	else {
	    VMC_ERR("Unknown Message size ignoring the packet \r\n");
	    break;
	}

	    i = i + 2 + payload[i + 1];
	}

}

bool Parse_SC_Data(u8 *Payload, u8 expected_msgid)
{
	if ((Payload[SOP_ESCAPE_CHAR] != ESCAPE_CHAR) || (Payload[SOP_ETX] != STX)) {
		VMC_LOG("Unexpected response !!");
		return false;
	}
	if (Payload[Rsp_MessageID] != expected_msgid) {
		VMC_LOG("Expected: %02x Received : %02x", expected_msgid, Payload[Rsp_MessageID]);
		return false;
	}
	switch (Payload[Rsp_MessageID])
	{
	case SC_COMMS_RX_VOLT_SNSR_RESP:
	case SC_COMMS_RX_POWER_SNSR_RESP:
	case SC_COMMS_RX_TEMP_SNSR_RESP:
		VMC_Update_Sensors(Payload[PayloadLength], &Payload[COMMS_PAYLOAD_START_INDEX]);
		break;
	case MSP432_COMMS_VMC_VERSION_POWERMODE_RESP:
		vmc_set_power_mode_status(true);
		VMC_Update_Version_PowerMode(Payload[PayloadLength], &Payload[COMMS_PAYLOAD_START_INDEX]);
		break;
	case MSP432_COMMS_VMC_GET_RESP_SIZE_RESP:
		VMC_Update_Sensor_Length(Payload[PayloadLength], &Payload[COMMS_PAYLOAD_START_INDEX]);
		break;
	case SC_COMMS_RX_I2C_SNSR_RESP:
		break;
	case SC_COMMS_RX_BOARD_INFO_RESP:
		vmc_set_boardInfo_status(true);
		break;
	case MSP432_COMMS_MSG_ERR:
		VMC_LOG("received error message");
		break;
	case MSP432_COMMS_MSG_GOOD:
		vmc_set_sc_status(true);
		VMC_LOG("Received MSG_GOOD");
		break;
	default:
		VMC_LOG("Unknown messageID : 0x%x\r\n", Payload[Rsp_MessageID]);
		break;
	}
	return true;
}

/**
  * @brief  Read full receiver fifo to clear all unwanted data that we received.
  * @param  buff: Pointer to receive data buffer.
  * @param  size: Max amount of data to be cleared.
  * @retval Status
  */
bool VMC_Recover_UART(u8 *buff, u32 size)
{
	UART_STATUS uart_ret_val = UART_ERROR_GENERIC;
	u32 rcvd_cnt = 0;

	uart_ret_val = UART_RTOS_Receive_Set_Buffer(&uart_vmcsc_log, buff, size, &rcvd_cnt);
	if (UART_SUCCESS != uart_ret_val)
		return false;

	uart_ret_val = UART_RTOS_Receive_Wait(&uart_vmcsc_log, &rcvd_cnt, RCV_TIMEOUT_MS(2000));
	if (UART_SUCCESS != uart_ret_val) {
		VMC_DBG("UART buffer flushed");
	}

	Cl_SecureMemset(buff, 0x00, size);

	return true;
}

void VMC_Uart_Frame_Tx_packet(u8 msg_id, u8 payload_len, u8 *tx_buff, u8 *length, u8 *payload)
{
	u16 checksum = 0;
	u8 len = 0;
	vmc_sc_uart_cmd pkt_framing = { 0 };

	Cl_SecureMemset(&pkt_framing, 0x00, sizeof(vmc_sc_uart_cmd));

	pkt_framing.SOP[0] = ESCAPE_CHAR;
	pkt_framing.SOP[1] = STX;

	pkt_framing.EOP[0] = ESCAPE_CHAR;
	pkt_framing.EOP[1] = ETX;

	pkt_framing.MessageID = msg_id;
	pkt_framing.Flags = Flags;
	pkt_framing.PayloadLength = payload_len;

	if (pkt_framing.PayloadLength) {
		for (int i = 0; i < pkt_framing.PayloadLength; ++i) {
			pkt_framing.Payload[i] = payload[i];
		}
	}

	checksum = calculate_checksum(&pkt_framing);
	pkt_framing.Checksum[0] = checksum & (0x00FF);
	pkt_framing.Checksum[1] = (checksum >> 8);

	(void) Cl_SecureMemcpy(&tx_buff[0], SOP_SIZE, &pkt_framing.SOP[0], SOP_SIZE);
	len += SOP_SIZE;

	/* MessageID */
	if (pkt_framing.MessageID == ESCAPE_CHAR) {
		tx_buff[len++] = ESCAPE_CHAR;
	}
	tx_buff[len++] = pkt_framing.MessageID;

	/* Add flags */
	if (pkt_framing.Flags == ESCAPE_CHAR) {
		tx_buff[len++] = ESCAPE_CHAR;
	}
	tx_buff[len++] = pkt_framing.Flags;

	/* Add payload length */
	if (pkt_framing.PayloadLength == ESCAPE_CHAR) {
		tx_buff[len++] = ESCAPE_CHAR;
	}
	tx_buff[len++] = pkt_framing.PayloadLength;

	/* Add payload */
	for (int i = 0; i < pkt_framing.PayloadLength; i++) {
		if (pkt_framing.Payload[i] == ESCAPE_CHAR) {
			tx_buff[len++] = ESCAPE_CHAR;
		}
		tx_buff[len++] = pkt_framing.Payload[i];
	}

	/* Add checksum */
	for (int i = 0; i < CHECKSUM_SIZE; i++) {
		if (pkt_framing.Checksum[i] == ESCAPE_CHAR) {
			tx_buff[len++] = ESCAPE_CHAR;
		}
		tx_buff[len++] = pkt_framing.Checksum[i];
	}

	/* Add EOP */
	(void) Cl_SecureMemcpy(&tx_buff[len], EOP_SIZE, &pkt_framing.EOP[0], EOP_SIZE);
	len += EOP_SIZE;
	*length = len;
}

bool VMC_UART_CMD_Transaction(u8 Message_id, u8 Flags, u8 Payloadlength, u8 *Payload, u8 Expected_Msg_Length)
{
	UART_STATUS uart_ret_val = UART_ERROR_GENERIC;

	static u8 tx_buf[MAX_VMC_SC_UART_BUF_SIZE] = { 0x00 };
	static u8 rx_buf[MAX_VMC_SC_UART_BUF_SIZE] = { 0x00 };
	u8 length = 0;
	u32 received_count = 0;
	bool ret_val = false;

	Cl_SecureMemset(rx_buf, 0x00, MAX_VMC_SC_UART_BUF_SIZE);
	Cl_SecureMemset(tx_buf, 0x00, MAX_VMC_SC_UART_BUF_SIZE);

	uart_ret_val = UART_RTOS_Receive_Set_Buffer(&uart_vmcsc_log, &rx_buf[0], Expected_Msg_Length, &received_count);
	if (UART_SUCCESS != uart_ret_val)
		return false;

	VMC_Uart_Frame_Tx_packet(Message_id, Payloadlength, tx_buf, &length, Payload);

	uart_ret_val = UART_RTOS_Send(&uart_vmcsc_log, &tx_buf[0], length);
	if (UART_SUCCESS != uart_ret_val) {
		ret_val = false;
		uart_disable_interrupt(uart_vmcsc_log.uart_IRQ_ID);
		if (!uart_shm_release(uart_vmcsc_log.rxSem))
			ret_val = false;
		return ret_val;
	}
	uart_ret_val = UART_RTOS_Receive_Wait(&uart_vmcsc_log, &received_count,	RCV_TIMEOUT_MS(500));
	if (UART_SUCCESS == uart_ret_val) {
		(Expected_Msg_Length == received_count) ? (ret_val = true) : (ret_val =	false);

		VMR_ASSERT(received_count <= MAX_VMC_SC_UART_BUF_SIZE,
				"fatal error: received_count %d > MAX BUF SIZE %d",
				received_count, MAX_VMC_SC_UART_BUF_SIZE);
		if (received_count > MIN_BYTE_CNT_FOR_SC_PKT_VALIDATION) { /* Condition to avoid over bound */
			if (VMC_Validate_SC_Packet(&rx_buf[0], received_count)) {
				Cl_SecureMemcpy(g_scData, received_count, rx_buf, received_count);
				isPacketReceived = true;
			} else {
				bool flush_ret = false;
				received_count = 0;
				flush_ret = VMC_Recover_UART(rx_buf, MAX_VMC_SC_UART_BUF_SIZE);
				if (flush_ret)
					VMC_DBG("Recovering UART !!");
			}
		} else {
			ret_val = false;
		}
	} else {
		ret_val = false;
		uart_disable_interrupt(uart_vmcsc_log.uart_IRQ_ID);
		if (uart_ret_val == UART_ERROR_GENERIC) {
			if (!uart_shm_release(uart_vmcsc_log.rxSem))
				return false;
		}
	}

	return ret_val;
}

void Get_Sensor_Response_Length(u8 Data)
{
	u8 Expected_Msg_Length = 11;
	u8 payloadLength = 1;

	scPayload[0] = Data;
	VMC_UART_CMD_Transaction(MSP432_COMMS_VMC_GET_RESP_SIZE_REQ,
			MSP432_COMMS_NO_FLAG, payloadLength, scPayload,
			Expected_Msg_Length);
	if(isPacketReceived)
	{
		Parse_SC_Data(g_scData, MSP432_COMMS_VMC_GET_RESP_SIZE_RESP);
		isPacketReceived = false;
	}
	Cl_SecureMemset(g_scData,0x00,MAX_VMC_SC_UART_BUF_SIZE);
	g_scDataCount = 0;
}

void VMC_SC_COMMS_Tx_Rx(u8 messageID)
{
	u8 buf[32] = { 0x00 };

	switch (messageID) {
	case SC_COMMS_RX_VOLT_SNSR:
	{
		VMC_UART_CMD_Transaction(SC_COMMS_RX_VOLT_SNSR, MSP432_COMMS_NO_FLAG,
				0x00, buf, sc_vmc_data.voltSensorLength);
		if (isPacketReceived) {
			Parse_SC_Data(g_scData, SC_COMMS_RX_VOLT_SNSR_RESP);
			isPacketReceived = false;
		}
		Cl_SecureMemset(g_scData, 0x00, MAX_VMC_SC_UART_BUF_SIZE);
		g_scDataCount = 0;
		break;
	}
	case SC_COMMS_RX_POWER_SNSR:
	{
		VMC_UART_CMD_Transaction(SC_COMMS_RX_POWER_SNSR, MSP432_COMMS_NO_FLAG,
				0x00, buf, sc_vmc_data.powerSensorLength);
		if (isPacketReceived) {
			Parse_SC_Data(g_scData, SC_COMMS_RX_POWER_SNSR_RESP);
			isPacketReceived = false;
		}
		Cl_SecureMemset(g_scData, 0x00, MAX_VMC_SC_UART_BUF_SIZE);
		g_scDataCount = 0;
		break;
	}
	case SC_COMMS_RX_TEMP_SNSR:
	{
		VMC_UART_CMD_Transaction(SC_COMMS_RX_TEMP_SNSR, MSP432_COMMS_NO_FLAG,
				0x00, buf, sc_vmc_data.tempSensorLength);
		if (isPacketReceived) {
			Parse_SC_Data(g_scData, SC_COMMS_RX_TEMP_SNSR_RESP);
			isPacketReceived = false;
		}
		Cl_SecureMemset(g_scData, 0x00, MAX_VMC_SC_UART_BUF_SIZE);
		g_scDataCount = 0;
		break;
	}
	case SC_COMMS_TX_I2C_SNSR:
	{
		u8 payloadLength = 0;
		Cl_SecureMemset(scPayload, 0x00, MAX_VMC_SC_UART_BUF_SIZE);
		payloadLength = Asdm_Send_I2C_Sensors_SC(scPayload);
		VMC_UART_CMD_Transaction(SC_COMMS_TX_I2C_SNSR, MSP432_COMMS_NO_FLAG,
				payloadLength, scPayload, MSP432_COMMS_GENERAL_RESP_LEN);
		if (isPacketReceived) {
			Parse_SC_Data(g_scData, SC_COMMS_RX_I2C_SNSR_RESP);
			isPacketReceived = false;
		}
		Cl_SecureMemset(g_scData, 0x00, MAX_VMC_SC_UART_BUF_SIZE);
		g_scDataCount = 0;
		break;
	}
	case SC_COMMS_TX_BOARD_INFO:
	{
		u8 payloadLength = 0;
		Cl_SecureMemset(scPayload, 0x00, MAX_VMC_SC_UART_BUF_SIZE);
		if (fetch_boardinfo_ptr != NULL) {
			payloadLength = (*fetch_boardinfo_ptr)(scPayload);
		} else {
			payloadLength = 0;
			break;
		}
		if (payloadLength < 0) {
			VMC_ERR("BoardInfo Size exceeding UART Max Payload Size");
			break;
		}
		VMC_UART_CMD_Transaction(SC_COMMS_TX_BOARD_INFO, MSP432_COMMS_NO_FLAG,
				payloadLength, scPayload, MSP432_COMMS_GENERAL_RESP_LEN);
		if (isPacketReceived) {
			Parse_SC_Data(g_scData, SC_COMMS_RX_BOARD_INFO_RESP);
			isPacketReceived = false;
		}
		Cl_SecureMemset(g_scData, 0x00, MAX_VMC_SC_UART_BUF_SIZE);
		g_scDataCount = 0;
		break;
	}
	case MSP432_COMMS_VMC_VERSION_POWERMODE_REQ:
	{
		u8 Expected_Msg_Length = 18;
		VMC_UART_CMD_Transaction(MSP432_COMMS_VMC_VERSION_POWERMODE_REQ,
		MSP432_COMMS_NO_FLAG, 0x00, buf, Expected_Msg_Length);
		if (isPacketReceived) {
			Parse_SC_Data(g_scData, MSP432_COMMS_VMC_VERSION_POWERMODE_RESP);
			isPacketReceived = false;
		}
		Cl_SecureMemset(g_scData, 0x00, MAX_VMC_SC_UART_BUF_SIZE);
		g_scDataCount = 0;
		break;
	}
	case MSP432_COMMS_VMC_ACTIVE_REQ:
	{
		u8 Expected_Msg_Length = vmc_get_active_resp_len();
		VMC_UART_CMD_Transaction(MSP432_COMMS_VMC_ACTIVE_REQ,
		MSP432_COMMS_NO_FLAG, 0x00, buf, Expected_Msg_Length);
		if (isPacketReceived) {
			Parse_SC_Data(g_scData, MSP432_COMMS_MSG_GOOD);
			isPacketReceived = false;
		}
		Cl_SecureMemset(g_scData, 0x00, MAX_VMC_SC_UART_BUF_SIZE);
		g_scDataCount = 0;
		break;
	}
	case MSP432_COMMS_VMC_GET_RESP_SIZE_REQ:
	{
		Get_Sensor_Response_Length(SC_COMMS_RX_VOLT_SNSR);
		Get_Sensor_Response_Length(SC_COMMS_RX_POWER_SNSR);
		Get_Sensor_Response_Length(SC_COMMS_RX_TEMP_SNSR);

		if ((sc_vmc_data.voltSensorLength != 0)
				&& (sc_vmc_data.powerSensorLength != 0)
				&& (sc_vmc_data.tempSensorLength != 0)) {
			vmc_set_snsr_resp_status(true);
		}
		break;
	}
	default:
		break;
	}
}

void VMC_SC_UART_Messages_All()
{
	u8 msgId = 0;

	if (msg_id_handler_ptr == NULL)
		return;

	for (msgId = 0; msgId < get_total_req_size(); msgId++) {
		VMC_SC_COMMS_Tx_Rx(msg_id_handler_ptr[msgId]);
		vTaskDelay(DELAY_MS(500));
	}
}

static void cl_vmc_sc_active()
{
	for (int i = 0; i < MAX_RETRY_TO_CHK_SC_ACTIVE; i++) {
		if (vmc_get_sc_status()) 
			return;

		vTaskDelay(pdMS_TO_TICKS(1000));
		vmc_set_active_resp_len(MSP432_COMMS_GENERAL_RESP_LEN);
		VMC_SC_COMMS_Tx_Rx(MSP432_COMMS_VMC_ACTIVE_REQ);
	}
}

static void VMC_Send_BoardInfo(void)
{
	if (!vmc_get_boardInfo_status()) {
		VMC_SC_COMMS_Tx_Rx(SC_COMMS_TX_BOARD_INFO);
	}
}

static void VMC_Get_SCVersion_PowerMode(void)
{
	/* Fetch the SC Version and Power Config */
	if (!vmc_get_power_mode_status()) {
		VMC_SC_COMMS_Tx_Rx(MSP432_COMMS_VMC_VERSION_POWERMODE_REQ);
	}
}

int cl_vmc_sc_is_ready()
{
	cl_vmc_sc_active();

	return (vmc_get_sc_status() == true);
}

u8 VMC_Poll_SC_Sensors()
{
	if(vmc_sc_comms_ptr != NULL) {
		return (*vmc_sc_comms_ptr)();
	}

	return XST_SUCCESS;
}

void cl_vmc_sc_tx_rx_data()
{
	if (!cl_vmc_sc_is_ready())
		return;

	VMC_Get_SCVersion_PowerMode();
	(void)VMC_Poll_SC_Sensors();
	VMC_Send_BoardInfo();
	VMC_SC_UART_Messages_All();
}
