
#include "FreeRTOS.h"
#include "task.h"


#include "vmc_sc_comms.h"


SC_VMC_Data sc_vmc_data;

extern u8 g_scData[MAX_VMC_SC_UART_BUF_SIZE];
extern u16 g_scDataCount;
extern bool isinterruptflag ;
extern bool ispacketreceived;

SemaphoreHandle_t vmc_sc_lock;


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
    u8 cageIndex;
    u8 sampleCount;

    switch(id)
    {
        case SNSR_ID_CAGE_TEMP0:
        case SNSR_ID_CAGE_TEMP1:
        case SNSR_ID_CAGE_TEMP2:
        case SNSR_ID_CAGE_TEMP3:
            cageIndex = (id-SNSR_ID_CAGE_TEMP0);
            sampleCount = sc_vmc_data.modulePresent[cageIndex];
            sc_vmc_data.modulePresent[cageIndex] |= 0b1;
            if(0 != value)
            {
            	sc_vmc_data.modulePresent[cageIndex]++;
            }
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
	u8 i = 0;
	u8 NumberOfMACs;


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
		case MSP432_COMMS_TEMP_SNSR_RESP:
			vmc_Update_Sensors(Payload[PayloadLength], &Payload[COMMS_PAYLOAD_START_INDEX]);
			break;
		case MSP432_COMMS_MSG_ERR:
			VMC_LOG("received error message  \r\n");
			break;
		default :
			VMC_LOG("Unknown messageID : 0x%x\r\n",Payload[Rsp_MessageID])
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

bool vmc_sc_setversion()
{
	u8 buf[2] = {0x00};
	bool status = false;

    buf[0]=MSP432_COMMS_VMC_VERSION;
	isinterruptflag = false;
	vPortDisableInterrupt(XPAR_XUARTPS_0_INTR);
	status = Vmc_send_packet(MSP432_COMMS_SET_VERS,MSP432_COMMS_NO_FLAG,0x01,buf);
    if(ispacketreceived)
    {
    	Parse_SCData(g_scData);
	    ispacketreceived = false;
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

	isinterruptflag = false;
	vPortDisableInterrupt(XPAR_XUARTPS_0_INTR);
    Vmc_send_packet(MSP432_COMMS_ADV_VERS,MSP432_COMMS_NO_FLAG,0x00,buf);
    if(ispacketreceived)
    {
	    Parse_SCData(g_scData);
	    ispacketreceived = false;
	    status = true;
    }
    memset(g_scData,0x00,MAX_VMC_SC_UART_BUF_SIZE);
    g_scDataCount = 0;

    return status ;
}


bool vmc_sc_getboardinfo()
{
	u8 buf[2] = {0x00};

    isinterruptflag = true;
	vPortEnableInterrupt(XPAR_XUARTPS_0_INTR);
    Vmc_send_packet(MSP432_COMMS_BOARD_SNSR_REQ,MSP432_COMMS_NO_FLAG,0x00,buf);
    if(ispacketreceived)
    {
	    Parse_SCData(g_scData);
	    ispacketreceived = false;
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

void vmc_sc_sensordata()
{

	static u8 messageID = 1;
	u8 buf[32] = {0x00};


	switch(messageID)
	{
	case 1:
		messageID = 2;
		isinterruptflag = true;
		vPortEnableInterrupt(XPAR_XUARTPS_0_INTR);
		Vmc_send_packet(MSP432_COMMS_VOLT_SNSR_REQ,MSP432_COMMS_NO_FLAG,0x00,buf);
		if(ispacketreceived)
		{
			Parse_SCData(g_scData);
			ispacketreceived = false;
		}
		memset(g_scData,0x00,MAX_VMC_SC_UART_BUF_SIZE);
		g_scDataCount = 0;
		break;
	case 2:
		messageID = 1;
		isinterruptflag = false;
		vPortDisableInterrupt(XPAR_XUARTPS_0_INTR);
		Vmc_send_packet(MSP432_COMMS_POWER_SNSR_REQ,MSP432_COMMS_NO_FLAG,0x00,buf);
		if(ispacketreceived)
		{
			Parse_SCData(g_scData);
			ispacketreceived = false;
		}
		memset(g_scData,0x00,MAX_VMC_SC_UART_BUF_SIZE);
		g_scDataCount = 0;
		isinterruptflag = false;
		break;
	default:
		break;
	}
}

void vmc_sc_monitor()
{

	bool status = false;

	if(!sc_vmc_data.boardInfoStatus)
	{
		status = vmc_sc_getboardinfo();
		if(status)
		{
			sc_vmc_data.boardInfoStatus = true;
		}
	}
	else
	{
		vmc_sc_sensordata();
	}

}
