
#include "FreeRTOS.h"
#include "task.h"


#include "vmc_sc_comms.h"

extern TaskHandle_t xVMCSCTask;
extern TaskHandle_t xVMCUartpoll;

SC_VMC_Data sc_vmc_data;

SemaphoreHandle_t vmc_sc_lock;
extern uint8_t sc_update_flag;
/* VMC SC Comms handles and flags */
extern uart_rtos_handle_t uart_vmcsc_log;

u8 g_scData[MAX_VMC_SC_UART_BUF_SIZE] = {0x00};
u16 g_scDataCount = 0;
bool isInterruptFlag;
bool isPacketReceived;


u8 VMC_SC_Comms_Msg[] = {
        MSP432_COMMS_VOLT_SNSR_REQ,
        MSP432_COMMS_POWER_SNSR_REQ,
//        MSP432_COMMS_VMC_SEND_I2C_SNSR_REQ,
};

#define MAX_MSGID_COUNT     (sizeof(VMC_SC_Comms_Msg)/sizeof(VMC_SC_Comms_Msg[0]))

extern u8 Asdm_Send_I2C_Sensors_SC(u8 *scPayload);

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
	return (uint32_t)payload[0] | (((uint32_t)payload[1]) << 8) | (((uint32_t)payload[2]) << 16) | (((uint32_t)payload[3]) << 24);
}
uint64_t vmcU8ToU64(uint8_t * payload)
{
    return (uint64_t)payload[5] | (((uint64_t)payload[4]) << 8) | (((uint64_t)payload[3]) << 16) | (((uint64_t)payload[2]) << 24) |
         (((uint64_t)payload[1]) << 32) | (((uint64_t)payload[0]) << 40);
}


void vmc_StoreSensor_Value(u8 id, u32 value)
{
    switch(id)
    {
        case BMC_VERSION:
            /* Fill the SC version here */
          break;
        default:
            break;
    }
    if (xSemaphoreTake(vmc_sc_lock, portMAX_DELAY))
    {
        sc_vmc_data.sensor_values[id] = value;
        xSemaphoreGive(vmc_sc_lock);
    }
    else{
    	VMC_ERR("vmc_sc_lock lock failed \r\n");
    }
}

void vmc_Update_Sensors(u16 length,u8 *payload)
{
    u16 i;

    for(i=0; i<length;)
    {

    	 if (payload[i + 1] == 1)
    	 {
    		 vmc_StoreSensor_Value(payload[i], (uint32_t)payload[i + 2]);
    	 }
    	 else if(payload[i + 1] == 2)
    	 {
    		 vmc_StoreSensor_Value(payload[i], (uint32_t)vmcU8ToU16((uint8_t*)&payload[i + 2]));
    	 }
    	 else if (payload[i + 1] == 4)
    	 {
    		 vmc_StoreSensor_Value(payload[i], (uint32_t)vmcU8ToU32((uint8_t*)&payload[i + 2]));
    	 }
        i = i + 2 + payload[i + 1];
    }

}


void Update_SNSR_Data(u8 PayloadLength , u8 * payload)
{
#if 0
	u8 i = 0;
	u8 NumberOfMACs =0 ;


    /*
     *  only parsing configmode , poweravilability,fanpresence
     */

	for(i=0; i<PayloadLength;)
	{
		switch(payload[i])
		{
		case SNSR_ID_BOARD_SN:
			i += (1+strlen((char *)(payload+i+1))+1);
			break;
		case SNSR_ID_MAC_ADDRESS0:
			i += 7;
			break;
		case SNSR_ID_MAC_ADDRESS1:
			i += 7;
			break;
		case SNSR_ID_MAC_ADDRESS2:
			i += 7;
			break;
		case SNSR_ID_MAC_ADDRESS3:
			i += 7;
			break;
		case SNSR_ID_BOARD_REV:
			i += (1+strlen((char *)(payload+i+1))+1);
			break;
		case SNSR_ID_BOARD_NAME:
			i += (1+strlen((char *)(payload+i+1))+1);
			break;
		case SNSR_ID_SAT_VERSION:
			i += (1+strlen((char *)(payload+i+1))+1);
			break;
		case SNSR_ID_TOTAL_POWER_AVAIL:
			sc_vmc_data.availpower = payload[i+1];
			i += 2;
			break;
		case SNSR_ID_FAN_PRESENCE:
			sc_vmc_data.fanpresence = payload[i+1];
			i += 2;
			break;
		case SNSR_ID_CONFIG_MODE:
			sc_vmc_data.configmode = payload[i+1];
			i += 2;
			break;
		case NEW_MAC_SCHEME_ID:
			NumberOfMACs = vmcU8ToU16(&payload[i + 1]);
			i += 9;
			break;
		default:
			VMC_LOG("default case :  %x \r\n",payload[i]);
			goto exit;
		}
	}
	exit:;
#endif

}

bool Parse_SCData(u8 *Payload)
{


	if(Payload[SOP_ESCAPE_CHAR] == ESCAPE_CHAR && Payload[SOP_ETX] == STX)
	{
		switch(Payload[Rsp_MessageID])
		{
		case MSP432_COMMS_BOARD_SNSR_RESP:
			Update_SNSR_Data(Payload[PayloadLength], &Payload[COMMS_PAYLOAD_START_INDEX]);
			break;
		case MSP432_COMMS_VOLT_SNSR_RESP:
		case MSP432_COMMS_POWER_SNSR_RESP:
			vmc_Update_Sensors(Payload[PayloadLength], &Payload[COMMS_PAYLOAD_START_INDEX]);
			break;
		case MSP432_COMMS_VMC_VERSION_POWERMODE_RESP:
			break;
		case MSP432_COMMS_MSG_ERR:
			VMC_LOG("received error message  \r\n");
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

bool Vmc_send_packet(u8 Message_id , u8 Flags,u8 Payloadlength, u8 *Payload)
{
	u8 buf[MAX_VMC_SC_UART_BUF_SIZE] = {0x00};
	u8 i = 0;
	u8 length = 0;
	u16 checksum = 0;
    bool retVal = true;

    vmc_sc_uart_cmd *pkt_framing;

    memset(&pkt_framing,0x00,sizeof(vmc_sc_uart_cmd));

	pkt_framing->SOP[0] = ESCAPE_CHAR;
	pkt_framing->SOP[1] = STX;

	pkt_framing->EOP[0] = ESCAPE_CHAR;
	pkt_framing->EOP[1] = ETX;

	pkt_framing->MessageID = Message_id;
	pkt_framing->Flags = Flags;
	pkt_framing->PayloadLength = Payloadlength;

	if(pkt_framing->PayloadLength)
	{
	    for(i = 0; i < pkt_framing->PayloadLength; ++i)
	    {
	    	pkt_framing->Payload[i] = Payload[i];
	    }
	}

	checksum = calculate_checksum(pkt_framing);
	pkt_framing->Checksum[0] = checksum & (0x00FF);
	pkt_framing ->Checksum[1] = (checksum >> 8);

	(void)memcpy(&buf[0], &pkt_framing->SOP[0], SOP_SIZE);
	length += SOP_SIZE;

	/* MessageID */
	if (pkt_framing->MessageID == ESCAPE_CHAR)
	{
		buf[length++] = ESCAPE_CHAR;
	}
	buf[length++] = pkt_framing->MessageID;

	/* Add flags */
	if (pkt_framing->Flags == ESCAPE_CHAR)
	{
		buf[length++] = ESCAPE_CHAR;
	}
	buf[length++] = pkt_framing->Flags;

	/* Add payload length */
	if (pkt_framing->PayloadLength == ESCAPE_CHAR)
	{
		buf[length++] = ESCAPE_CHAR;
	}
	buf[length++] = pkt_framing->PayloadLength;

	/* Add payload */
	for(i = 0; i < pkt_framing->PayloadLength; i++)
	{
		if (pkt_framing->Payload[i] == ESCAPE_CHAR)
		{
			buf[length++] = ESCAPE_CHAR;
		}
		buf[length++] = pkt_framing->Payload[i];

	}

	/* Add checksum */
	for(i = 0; i < CHECKSUM_SIZE; i++)
	{
		if (pkt_framing->Checksum[i] == ESCAPE_CHAR)
		{
			buf[length++] = ESCAPE_CHAR;
		}
		buf[length++] = pkt_framing->Checksum[i];

	}

	/* Add EOP */
	(void)memcpy(&buf[length], &pkt_framing->EOP[0], 2);
	length += EOP_SIZE;

	retVal = UART_RTOS_Send(&uart_vmcsc_log,&buf[0],length);
	if(retVal == UART_SUCCESS){
		return true;
	}
	else{
		return false;
	}

}

bool VMC_SC_SetActive()
{
    u8 buf[2] = {0x00};
    bool status = false;

    buf[0]=MSP432_COMMS_VMC_ACTIVE_REQ;
    isInterruptFlag = false;
    vPortDisableInterrupt(XPAR_XUARTPS_0_INTR);
    status = Vmc_send_packet(MSP432_COMMS_VMC_ACTIVE_REQ,MSP432_COMMS_NO_FLAG,0x01,buf);
    if(isPacketReceived)
    {
    	Parse_SCData(g_scData);
    	isPacketReceived = false;
	    status = true;
    }
    memset(g_scData,0x00,MAX_VMC_SC_UART_BUF_SIZE);
    g_scDataCount = 0;
    return status ;
}

bool vmc_sc_advertise_version()
{
	u8 buf[2] = {0x00};
	bool status = false;

	isInterruptFlag = false;
	vPortDisableInterrupt(XPAR_XUARTPS_0_INTR);
    Vmc_send_packet(MSP432_COMMS_ADV_VERS,MSP432_COMMS_NO_FLAG,0x00,buf);
    if(isPacketReceived)
    {
	    Parse_SCData(g_scData);
	    isPacketReceived = false;
	    status = true;
    }
    memset(g_scData,0x00,MAX_VMC_SC_UART_BUF_SIZE);
    g_scDataCount = 0;

    return status ;
}


bool vmc_sc_getboardinfo()
{
	u8 buf[2] = {0x00};

    isInterruptFlag = true;
	vPortEnableInterrupt(XPAR_XUARTPS_0_INTR);
    Vmc_send_packet(MSP432_COMMS_BOARD_SNSR_REQ,MSP432_COMMS_NO_FLAG,0x00,buf);
    if(isPacketReceived)
    {
	    Parse_SCData(g_scData);
	    isPacketReceived = false;
    }
    memset(g_scData,0x00,MAX_VMC_SC_UART_BUF_SIZE);
    g_scDataCount = 0;

    if(sc_vmc_data.availpower != 0 || sc_vmc_data.configmode != 0 )
    {
    	return true;
    }
    else
    {
    	return false;
    }
}

void VMC_Fetch_SC_SensorData(u8 messageID)
{
    u8 buf[32] = {0x00};

    switch(messageID)
    {
        case MSP432_COMMS_VOLT_SNSR_REQ:
        {
            isInterruptFlag = true;
            vPortEnableInterrupt(XPAR_XUARTPS_0_INTR);
            Vmc_send_packet(MSP432_COMMS_VOLT_SNSR_REQ,MSP432_COMMS_NO_FLAG,0x00,buf);
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
            isInterruptFlag = false;
            vPortEnableInterrupt(XPAR_XUARTPS_0_INTR);
            Vmc_send_packet(MSP432_COMMS_POWER_SNSR_REQ,MSP432_COMMS_NO_FLAG,0x00,buf);
            if(isPacketReceived)
            {
                Parse_SCData(g_scData);
                isPacketReceived = false;
            }

            memset(g_scData,0x00,MAX_VMC_SC_UART_BUF_SIZE);
            g_scDataCount = 0;
            isInterruptFlag = false;
            break;
        }

        case MSP432_COMMS_VMC_SEND_I2C_SNSR_REQ:
        {
            isInterruptFlag = false;
            u8 scPayload[128] = {0};
            u8 payloadLength = 0;
            payloadLength = Asdm_Send_I2C_Sensors_SC(scPayload);
            vPortDisableInterrupt(XPAR_XUARTPS_0_INTR);

            Vmc_send_packet(MSP432_COMMS_VMC_SEND_I2C_SNSR_REQ,
                            MSP432_COMMS_NO_FLAG,payloadLength,scPayload);

            if(isPacketReceived)
            {
                Parse_SCData(g_scData);
                isPacketReceived = false;
            }

            memset(g_scData,0x00,MAX_VMC_SC_UART_BUF_SIZE);
            g_scDataCount = 0;
            isInterruptFlag = false;
            break;
        }

        case MSP432_COMMS_VMC_VERSION_POWERMODE_REQ:
        {
            isInterruptFlag = false;
            vPortDisableInterrupt(XPAR_XUARTPS_0_INTR);
            Vmc_send_packet(MSP432_COMMS_VMC_VERSION_POWERMODE_REQ,
                            MSP432_COMMS_NO_FLAG,0x00,buf);
            if(isPacketReceived)
            {
                Parse_SCData(g_scData);
                isPacketReceived = false;
            }

            memset(g_scData,0x00,MAX_VMC_SC_UART_BUF_SIZE);
            g_scDataCount = 0;
            isInterruptFlag = false;
            break;
        }
        case MSP432_COMMS_VMC_ACTIVE_REQ:
        {
            isInterruptFlag = false;
            vPortDisableInterrupt(XPAR_XUARTPS_0_INTR);
            Vmc_send_packet(MSP432_COMMS_VMC_ACTIVE_REQ,MSP432_COMMS_NO_FLAG,0x00,buf);
            if(isPacketReceived)
            {
                Parse_SCData(g_scData);
                isPacketReceived = false;
            }

            memset(g_scData,0x00,MAX_VMC_SC_UART_BUF_SIZE);
            g_scDataCount = 0;
            isInterruptFlag = false;
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

void Vmc_uartpoll( void *pvParameters )
{
    u32 retval;
    u8 Data[MAX_VMC_SC_UART_BUF_SIZE] = {0x00};
    u32 receivedcount = 0 ;
    for(;;)
    {
        if(isInterruptFlag == true)
        {
            vPortEnableInterrupt(XPAR_XUARTPS_0_INTR);
            retval = UART_RTOS_Receive(&uart_vmcsc_log, Data, MAX_VMC_SC_UART_COUNT_WITH_INTR ,&receivedcount);
            if(retval)
            {
                memcpy(&g_scData[g_scDataCount],Data,retval);
                g_scDataCount += retval;
                retval = 0;
                if ((g_scData[g_scDataCount-1] == ETX) && (g_scData[g_scDataCount-2] == ESCAPE_CHAR))
                {
                    isPacketReceived = true;
                }
            }
            if(receivedcount)
            {
                memcpy(&g_scData[g_scDataCount],Data,receivedcount);
                g_scDataCount += receivedcount;
                receivedcount = 0;
                if ((g_scData[g_scDataCount-1] == ETX) && (g_scData[g_scDataCount-2] == ESCAPE_CHAR))
                {
                    isPacketReceived = true;
                }
            }
        }
        else if(isInterruptFlag == false)
        {
            vPortDisableInterrupt(XPAR_XUARTPS_0_INTR);
            retval = UART_RTOS_Receive(&uart_vmcsc_log, Data, MAX_VMC_SC_UART_COUNT_WITHOUT_INTR ,&receivedcount);
            if(retval)
            {
                memcpy(&g_scData[g_scDataCount],Data,retval);
                g_scDataCount += retval;
                retval = 0;
                if ((g_scData[g_scDataCount-1] == ETX) && (g_scData[g_scDataCount-2] == ESCAPE_CHAR))
                {
                    isPacketReceived = true;
                }
            }
        }
        vTaskDelay(20);
    }
    vTaskSuspend(NULL);
}



void VMC_SC_CommsTask(void *params)
{
    VMC_LOG("VMC SC Comms Task Created !!!\n\r");

    xTaskCreate( Vmc_uartpoll,
             ( const char * ) "UART_POLL",
             1024,
             NULL,
             tskIDLE_PRIORITY + 1,
             &xVMCUartpoll );

    /* vmc_sc_lock */
    vmc_sc_lock = xSemaphoreCreateMutex();
    if(vmc_sc_lock == NULL){
    VMC_ERR("vmc_sc_lock creation failed \n\r");
    }

    /* Notify SC of VMC Presence */
//   VMC_Fetch_SC_SensorData(MSP432_COMMS_VMC_ACTIVE_REQ);

    /* Fetch the SC Version and Power Config  */
//  VMC_Fetch_SC_SensorData(MSP432_COMMS_VMC_VERSION_POWERMODE_REQ);

    for(;;)
    {
	if(!sc_update_flag)
	{	
        	VMC_Mointor_SC_Sensors();
        	vTaskDelay(100);
	}
	else
	{
		vTaskDelay(2000);
	}
    }

    vTaskSuspend(NULL);
}
