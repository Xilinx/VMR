
#include "FreeRTOS.h"
#include "task.h"

#include "vmc_sc_comms.h"
#include "vmc_update_sc.h"

extern TaskHandle_t xVMCSCTask;

SC_VMC_Data sc_vmc_data;

extern SemaphoreHandle_t vmc_sc_lock;

/* VMC SC Comms handles and flags */
extern uart_rtos_handle_t uart_vmcsc_log;

u8 g_scData[MAX_VMC_SC_UART_BUF_SIZE] = {0x00};
u16 g_scDataCount = 0;
bool isPacketReceived;
u8 scPayload[128] = {0};

volatile bool isVMCActive ;
volatile bool isPowerModeActive ;
volatile bool getSensorRespLen ;

extern uint8_t sc_update_flag;

u8 VMC_SC_Comms_Msg[] = {
       MSP432_COMMS_VOLT_SNSR_REQ,
       MSP432_COMMS_POWER_SNSR_REQ,
	   MSP432_COMMS_TEMP_SNSR_REQ,
       MSP432_COMMS_VMC_SEND_I2C_SNSR_REQ,
};

#define MAX_MSGID_COUNT     (sizeof(VMC_SC_Comms_Msg)/sizeof(VMC_SC_Comms_Msg[0]))

extern u8 Asdm_Send_I2C_Sensors_SC(u8 *scPayload);
extern void Asdm_Update_Active_MSP_sensor();

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

u16 vmcU8ToU16(u8 *payload) {
    return (u16)payload[0] | (((u16)payload[1])<<8);
}
uint32_t vmcU8ToU32(uint8_t* payload)
{
	return (uint32_t)payload[3] | (((uint32_t)payload[2]) << 8) | (((uint32_t)payload[1]) << 16) | (((uint32_t)payload[0]) << 24);
}
uint64_t vmcU8ToU64(uint8_t * payload)
{
    return (uint64_t)payload[5] | (((uint64_t)payload[4]) << 8) | (((uint64_t)payload[3]) << 16) | (((uint64_t)payload[2]) << 24) |
         (((uint64_t)payload[1]) << 32) | (((uint64_t)payload[0]) << 40);
}

void VMC_Update_Sensor_Length(u16 length,u8 *payload)
{

    switch(payload[0])
    {
    case MSP432_COMMS_VOLT_SNSR_REQ:
    	sc_vmc_data.voltsensorlength = payload[1]+RAW_PAYLOAD_LENTGH;
    	break;
    case MSP432_COMMS_POWER_SNSR_REQ:
    	sc_vmc_data.powersensorlength = payload[1]+RAW_PAYLOAD_LENTGH;
    	break;
    }

}

void VMC_Update_Version_PowerMode(u16 length,u8 *payload)
{
	u16 i=0;
	static u8 isActiveMSPVerUpdated = false;
	for(i=0; i<length;)
	{
		switch(payload[i])
		{
		case BMC_VERSION:
			sc_vmc_data.scVersion[0] = payload[i+2];  // fw_version
			sc_vmc_data.scVersion[1] = payload[i+3];  // fw_rev_major
			sc_vmc_data.scVersion[2] = payload[i+4];  // fw_rev_minor
			/* Update the ASDM Active MSP Version */
			if((isActiveMSPVerUpdated == false) &&
					(sc_vmc_data.scVersion[0] != 0))
			{
				Asdm_Update_Active_MSP_sensor();
				isActiveMSPVerUpdated = true;
			}
			break;

		case TOTAL_POWER_AVAIL:
			sc_vmc_data.availpower = payload[i+2];
			break;
		}
		i = i + 2 + payload[i + 1];
	}

}

void VMC_StoreSensor_Value(u8 id, u32 value)
{

	if (xSemaphoreTake(vmc_sc_lock, portMAX_DELAY))
	{
		sc_vmc_data.sensor_values[id] = value;
		xSemaphoreGive(vmc_sc_lock);
	}
	else
	{
		VMC_LOG("vmc_sc_lock lock failed \r\n");
	}
}

void VMC_Update_Sensors(u16 length,u8 *payload)
{
    u16 i;

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
        i = i + 2 + payload[i + 1];
    }

}

bool Parse_SCData(u8 *Payload)
{


	if(Payload[SOP_ESCAPE_CHAR] == ESCAPE_CHAR && Payload[SOP_ETX] == STX)
	{
		switch(Payload[Rsp_MessageID])
		{
		case MSP432_COMMS_VOLT_SNSR_RESP:
		case MSP432_COMMS_POWER_SNSR_RESP:
		case MSP432_COMMS_TEMP_SNSR_RESP:
			VMC_Update_Sensors(Payload[PayloadLength], &Payload[COMMS_PAYLOAD_START_INDEX]);
			break;
		case MSP432_COMMS_VMC_VERSION_POWERMODE_RESP:
			VMC_Update_Version_PowerMode(Payload[PayloadLength], &Payload[COMMS_PAYLOAD_START_INDEX]);
			break;
		case MSP432_COMMS_VMC_GET_RESP_SIZE_RESP:
			VMC_Update_Sensor_Length(Payload[PayloadLength], &Payload[COMMS_PAYLOAD_START_INDEX]);
			break;
		case MSP432_COMMS_MSG_ERR:
			VMC_LOG("received error message  \r\n");
			break;
		case MSP432_COMMS_MSG_GOOD:
			VMC_LOG("Received MSG_GOOD  \r\n");
			break;
		default :
			VMC_LOG("Unknown messageID : 0x%x\r\n",Payload[Rsp_MessageID]);
			break;
		}
	}
	else
	{
		VMC_LOG("Received data is not in protocol format ignore the data \r\n");
	}

return true;
}

bool VMC_send_packet(u8 Message_id , u8 Flags,u8 Payloadlength, u8 *Payload)
{
	u8 buf[MAX_VMC_SC_UART_BUF_SIZE] = {0x00};
	u8 i = 0;
	u8 length = 0;
	u16 checksum = 0;
    int32_t retVal ;

    vmc_sc_uart_cmd pkt_framing;

    memset(&pkt_framing,0x00,sizeof(vmc_sc_uart_cmd));

	pkt_framing.SOP[0] = ESCAPE_CHAR;
	pkt_framing.SOP[1] = STX;

	pkt_framing.EOP[0] = ESCAPE_CHAR;
	pkt_framing.EOP[1] = ETX;

	pkt_framing.MessageID = Message_id;
	pkt_framing.Flags = Flags;
	pkt_framing.PayloadLength = Payloadlength;

	if(pkt_framing.PayloadLength)
	{
	    for(i = 0; i < pkt_framing.PayloadLength; ++i)
	    {
	    	pkt_framing.Payload[i] = Payload[i];
	    }
	}

	checksum = calculate_checksum(&pkt_framing);
	pkt_framing.Checksum[0] = checksum & (0x00FF);
	pkt_framing.Checksum[1] = (checksum >> 8);

	(void)memcpy(&buf[0], &pkt_framing.SOP[0], SOP_SIZE);
	length += SOP_SIZE;

	/* MessageID */
	if (pkt_framing.MessageID == ESCAPE_CHAR)
	{
		buf[length++] = ESCAPE_CHAR;
	}
	buf[length++] = pkt_framing.MessageID;

	/* Add flags */
	if (pkt_framing.Flags == ESCAPE_CHAR)
	{
		buf[length++] = ESCAPE_CHAR;
	}
	buf[length++] = pkt_framing.Flags;

	/* Add payload length */
	if (pkt_framing.PayloadLength == ESCAPE_CHAR)
	{
		buf[length++] = ESCAPE_CHAR;
	}
	buf[length++] = pkt_framing.PayloadLength;

	/* Add payload */
	for(i = 0; i < pkt_framing.PayloadLength; i++)
	{
		if (pkt_framing.Payload[i] == ESCAPE_CHAR)
		{
			buf[length++] = ESCAPE_CHAR;
		}
		buf[length++] = pkt_framing.Payload[i];

	}

	/* Add checksum */
	for(i = 0; i < CHECKSUM_SIZE; i++)
	{
		if (pkt_framing.Checksum[i] == ESCAPE_CHAR)
		{
			buf[length++] = ESCAPE_CHAR;
		}
		buf[length++] = pkt_framing.Checksum[i];

	}

	/* Add EOP */
	(void)memcpy(&buf[length], &pkt_framing.EOP[0], EOP_SIZE);
	length += EOP_SIZE;

	retVal = UART_RTOS_Send(&uart_vmcsc_log,&buf[0],length);
	if(retVal == UART_SUCCESS){
		return true;
	}
	return false;
}

void VMC_uart_receive(u8  Expected_Msg_Length)
{
	u32 receivedcount = 0 ;
	u8 Data[MAX_VMC_SC_UART_BUF_SIZE] = {0x00};

	if(UART_RTOS_Receive(&uart_vmcsc_log, Data, Expected_Msg_Length ,&receivedcount,0x104) == UART_SUCCESS)
	{
		if(receivedcount > 2 )   // condition to avoid negative indexing
		{
			memcpy(g_scData,Data,receivedcount);
			if ((g_scData[receivedcount-1] == ETX) && (g_scData[receivedcount-2] == ESCAPE_CHAR))
			{
				isPacketReceived = true;
			}
		}
	}
}

void Get_Sensor_Response_Length(u8 Data)
{
	u8  Expected_Msg_Length = 11;
	u8 payloadLength = 1;

	scPayload[0] = Data ;
	VMC_send_packet(MSP432_COMMS_VMC_GET_RESP_SIZE_REQ,MSP432_COMMS_NO_FLAG,payloadLength,scPayload);
	VMC_uart_receive(Expected_Msg_Length);
	if(isPacketReceived)
	{
		Parse_SCData(g_scData);
		isPacketReceived = false;
	}
	memset(g_scData,0x00,MAX_VMC_SC_UART_BUF_SIZE);
	g_scDataCount = 0;
}

void VMC_Fetch_SC_SensorData(u8 messageID)
{
    u8 buf[32] = {0x00};

    switch(messageID)
    {
        case MSP432_COMMS_VOLT_SNSR_REQ:
        {
            VMC_send_packet(MSP432_COMMS_VOLT_SNSR_REQ,MSP432_COMMS_NO_FLAG,0x00,buf);
            VMC_uart_receive(sc_vmc_data.voltsensorlength);
            if(isPacketReceived)
            {
                Parse_SCData(g_scData);
                isPacketReceived = false;
            }
            memset(g_scData,0x00,MAX_VMC_SC_UART_BUF_SIZE);
            g_scDataCount = 0;
            break;
        }

        case MSP432_COMMS_POWER_SNSR_REQ:
        {
            VMC_send_packet(MSP432_COMMS_POWER_SNSR_REQ,MSP432_COMMS_NO_FLAG,0x00,buf);
            VMC_uart_receive(sc_vmc_data.powersensorlength);
            if(isPacketReceived)
            {
            	Parse_SCData(g_scData);
            	isPacketReceived = false;
            }
            memset(g_scData,0x00,MAX_VMC_SC_UART_BUF_SIZE);
            g_scDataCount = 0;
            break;
        }
        case MSP432_COMMS_TEMP_SNSR_REQ:
        {
            u8  Expected_Msg_Length = 12;
            VMC_send_packet(MSP432_COMMS_TEMP_SNSR_REQ,MSP432_COMMS_NO_FLAG,0x00,buf);
            VMC_uart_receive(Expected_Msg_Length);
            if(isPacketReceived)
            {
            	Parse_SCData(g_scData);
            	isPacketReceived = false;
            }
            memset(g_scData,0x00,MAX_VMC_SC_UART_BUF_SIZE);
            g_scDataCount = 0;
            break;
        }
        case MSP432_COMMS_VMC_SEND_I2C_SNSR_REQ:
        {
            u8 payloadLength = 0;
            memset(scPayload ,0x00,128);
            payloadLength = Asdm_Send_I2C_Sensors_SC(scPayload);
            VMC_send_packet(MSP432_COMMS_VMC_SEND_I2C_SNSR_REQ,
                            MSP432_COMMS_NO_FLAG,payloadLength,scPayload);
            if(isPacketReceived)
            {
                Parse_SCData(g_scData);
                isPacketReceived = false;
            }
            memset(g_scData,0x00,MAX_VMC_SC_UART_BUF_SIZE);
            g_scDataCount = 0;
            break;
        }

        case MSP432_COMMS_VMC_VERSION_POWERMODE_REQ:
        {
            u8  Expected_Msg_Length = 18;
            VMC_send_packet(MSP432_COMMS_VMC_VERSION_POWERMODE_REQ,
                            MSP432_COMMS_NO_FLAG,0x00,buf);
            VMC_uart_receive(Expected_Msg_Length);
            if(isPacketReceived)
            {
                Parse_SCData(g_scData);
                isPacketReceived = false;
                isPowerModeActive = true;
            }
            memset(g_scData,0x00,MAX_VMC_SC_UART_BUF_SIZE);
            g_scDataCount = 0;
            break;
        }
        case MSP432_COMMS_VMC_ACTIVE_REQ:
        {
            u8  Expected_Msg_Length = 10;
            VMC_send_packet(MSP432_COMMS_VMC_ACTIVE_REQ,MSP432_COMMS_NO_FLAG,0x00,buf);
            VMC_uart_receive(Expected_Msg_Length);
            if(isPacketReceived)
            {
                Parse_SCData(g_scData);
                isPacketReceived = false;
                isVMCActive = true;
            }
            memset(g_scData,0x00,MAX_VMC_SC_UART_BUF_SIZE);
            g_scDataCount = 0;
            break;
        }

        case MSP432_COMMS_VMC_GET_RESP_SIZE_REQ:
        {
	    Get_Sensor_Response_Length(MSP432_COMMS_VOLT_SNSR_REQ);
	    Get_Sensor_Response_Length(MSP432_COMMS_POWER_SNSR_REQ);

	    if((sc_vmc_data.voltsensorlength != 0) && (sc_vmc_data.powersensorlength != 0 ))
	    {
		    getSensorRespLen = true ;
	    }
	    break;
        }
        default:
            break;
}
}

void VMC_Mointor_SC_Sensors()
{
    u8 msgId = 0;


    for(msgId=0; msgId < MAX_MSGID_COUNT; msgId++)
    {
        VMC_Fetch_SC_SensorData(VMC_SC_Comms_Msg[msgId]);
        vTaskDelay(20);
    }
}

void VMC_SC_CommsTask(void *params)
{
    VMC_LOG("VMC SC Comms Task Created !!!\n\r");

    for(;;)
    {
    	/* Notify SC of VMC Presence */
    	if(!sc_update_flag)
    	{
    		if(!isVMCActive)
    		{
    			VMC_Fetch_SC_SensorData(MSP432_COMMS_VMC_ACTIVE_REQ);
    		}
    		/* Fetch the SC Version and Power Config  */
    		if(!isPowerModeActive)
    		{
    			VMC_Fetch_SC_SensorData(MSP432_COMMS_VMC_VERSION_POWERMODE_REQ);
    		}
    		/* Fetch the Volt & power Sensor length  */
    		if( isVMCActive &&  (!getSensorRespLen))
    		{
    			VMC_Fetch_SC_SensorData(MSP432_COMMS_VMC_GET_RESP_SIZE_REQ);
    		}
    		/*
    		 *  Fetching Sensor values from SC
    		 */
    		VMC_Mointor_SC_Sensors();
    		vTaskDelay(100);
    	}
    	else
    	{
    		/* Wait for SC update complete ~20Sec*/
    		vTaskDelay(DELAY_MS(1000 * 20));
    	}
    }

    vTaskSuspend(NULL);
}
