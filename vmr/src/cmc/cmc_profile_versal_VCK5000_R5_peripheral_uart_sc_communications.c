/*----------------------------------------------------------------------------------------------*/
/*                                                                                              */
/* ? Copyright 2019 Xilinx, Inc. All rights reserved.                                           */
/*                                                                                              */
/* This file contains confidential and proprietary information of Xilinx, Inc.                  */
/* and is protected under U.S. and international copyright and other intellectual               */
/* property laws.                                                                               */
/*                                                                                              */
/*                                                                                              */
/* DISCLAIMER                                                                                   */
/*                                                                                              */
/* This disclaimer is not a license and does not grant any rights to the materials              */
/* distributed herewith. Except as otherwise provided in a valid license issued                 */
/* to you by Xilinx, and to the maximum extent permitted by applicable law:                     */
/*                                                                                              */
/* (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND WITH ALL FAULTS,                          */
/* AND XILINX HEREBY DISCLAIMS ALL WARRANTIES AND CONDITIONS, EXPRESS, IMPLIED,                 */
/* OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY,                    */
/* NON-INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE; and                                 */
/*                                                                                              */
/* (2) Xilinx shall not be liable (whether in contract or tort, including negligence,           */
/* or under any other theory of liability) for any loss or damage of any kind or                */
/* nature related to, arising under or in connection with these materials,                      */
/* including for any direct, or any indirect, special, incidental, or consequential             */
/* loss or damage (including loss of data, profits, goodwill, or any type of loss or            */
/* damage suffered as a result of any action brought by a third party) even if such             */
/* damage or loss was reasonably foreseeable or Xilinx had been advised of the                  */
/* possibility of the same.                                                                     */
/*                                                                                              */
/*                                                                                              */
/* CRITICAL APPLICATIONS                                                                        */
/*                                                                                              */
/* Xilinx products are not designed or intended to be fail-safe, or for use in                  */
/* any application requiring fail-safe performance, such as life-support or safety              */
/* devices or systems, Class III medical devices, nuclear facilities, applications              */
/* related to the deployment of airbags, or any other applications that could lead              */
/* to death, personal injury, or severe property or environmental damage (individually          */
/* and collectively, "Critical Applications"). Customer assumes the sole risk and               */
/* liability of any use of Xilinx products in Critical Applications, subject                    */
/* only to applicable laws and regulations governing limitations on product liability.          */
/*                                                                                              */
/*                                                                                              */
/* THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS PART OF THIS FILE AT ALL TIMES.     */
/*                                                                                              */
/*----------------------------------------------------------------------------------------------*/


/* PROFILE_LINUX_VERSAL_VCK5000_R5 */ 
/* LINUX_DIST */






/*
 *
 *  RCS Keyword Metadata
 *
 *  $Change: 3095780 $
 *  $Date: 2021/01/13 $
 *  $Revision: #13 $
 *
 */

//#include <poll.h>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>
#include "cmc_peripheral_axi_uart_lite_satellite.h"
#include "cmc_profile_versal_VCK5000_R5.h"
#include "cmc_ram.h"
#include "FreeRTOS.h"
#include "task.h"
#include "xparameters.h"
#include "xuartpsv.h"
#include "xil_printf.h"


XUartPsv Uart_Psv;

typedef struct
{
	int pUART;
} CMC_BuildProfile_Versal_VCK5000_R5_Peripheral_uart_sc_communications_Context_s;

CMC_BuildProfile_Versal_VCK5000_R5_Peripheral_uart_sc_communications_Context_s CMC_BuildProfile_Versal_VCK5000_R5_Peripheral_uart_sc_communications_Context;


void CMC_BuildProfile_Versal_VCK5000_R5_Peripheral_uart_sc_communications_UART_SERVICES_SEND_BYTE_NON_BLOCKING(void *pUserContext, CMC_PERIPHERAL_BASE_ADDRESS_TYPE BaseAddress, char Data)
{
	UNUSED(BaseAddress);
	UNUSED(pUserContext);

	u8 buffer = (u8)Data;

	XUartPsv_Send(&Uart_Psv, &buffer, 1);
}




char  CMC_BuildProfile_Versal_VCK5000_R5_Peripheral_uart_sc_communications_UART_SERVICES_RECV_BYTE_BLOCKING(void *pUserContext, CMC_PERIPHERAL_BASE_ADDRESS_TYPE BaseAddress)
{
	u8 Data;

	UNUSED(BaseAddress);
	UNUSED(pUserContext);

	u32 retval =  XUartPsv_Recv(&Uart_Psv, &Data, 1);
	if( retval == 1 )
	{
		xil_printf("byte read from SC uart 0x%x \r\n", Data);
	}

	return (char)Data;
}


bool CMC_BuildProfile_Versal_VCK5000_R5_Peripheral_uart_sc_communications_UART_SERVICES_IS_TRANSMIT_FULL(void *pUserContext, CMC_PERIPHERAL_BASE_ADDRESS_TYPE BaseAddress)
{
	CMC_BuildProfile_Versal_VCK5000_R5_Peripheral_uart_sc_communications_Context_s *pUartContext = (CMC_BuildProfile_Versal_VCK5000_R5_Peripheral_uart_sc_communications_Context_s *)pUserContext;
	UNUSED(pUartContext);
	UNUSED(BaseAddress);

	return (bool)XUartPsv_IsSending(&Uart_Psv);
}


#define XUartPsv_IsReceiveEmpty(InstancePtr) \
		((IO_SYNC_READ32(((InstancePtr)->Config.BaseAddress) + \
		(u32)XUARTPSV_UARTFR_OFFSET) & (u32)XUARTPSV_UARTFR_RXFE) \
		== (u32)XUARTPSV_UARTFR_RXFE)


bool CMC_BuildProfile_Versal_VCK5000_R5_Peripheral_uart_sc_communications_UART_SERVICES_IS_RECEIVE_EMPTY(void *pUserContext, CMC_PERIPHERAL_BASE_ADDRESS_TYPE BaseAddress)
{
	bool retVal = true;
	retVal = (bool)XUartPsv_IsReceiveEmpty(&Uart_Psv);
	if(retVal == false) xil_printf("%s:%d\r\n",__FILE__, __LINE__);

	return retVal;
}




static TaskHandle_t uartPollTaskHandle;

void CMC_BuildProfile_Versal_VCK5000_R5_Peripheral_uart_sc_communications_UART_SERVICES_INITIALIZE(void *pUserContext, CMC_PERIPHERAL_BASE_ADDRESS_TYPE BaseAddress)
{

}

static void CMC_BuildProfile_Versal_VCK5000_R5_Peripheral_uart_sc_communications_POLL_THREAD( void *pvParameters )
{
	u32 retval;
	u8 Data[32];
	int i;

	while (1)
	{
		/* Receive the byte */
		retval = XUartPsv_Recv(&Uart_Psv, Data, 32);

		if ((retval) && (NULL != AXI_UART_LITE_Satellite_Context.pFN_Push))
		{
			for(i=0;i<retval;i++)
			{
				(*AXI_UART_LITE_Satellite_Context.pFN_Push)(AXI_UART_LITE_Satellite_Context.pPushContext, Data[i]);
			}
		}
	}
}



void CMC_BuildProfile_Versal_VCK5000_R5_Peripheral_uart_sc_communications_UART_SERVICES_INITIALIZE_FREERTOS(void)
{
	int Status;
	XUartPsv_Config *Config;
	uint32_t LineCtrlRegister;

	/*
	 * Initialize the UART driver so that it's ready to use
	 * Look up the configuration in the config table and then
	 * initialize it.
	 */
	Config = XUartPsv_LookupConfig(XPAR_XUARTPSV_0_DEVICE_ID);
	if (NULL == Config) {
		xil_printf("XUartPsv_LookupConfig FAILED.\n");
	}



	Status = XUartPsv_CfgInitialize(&Uart_Psv, Config,
					Config->BaseAddress);
	xil_printf("%s:%d\r\n",__FILE__, __LINE__);
	if (Status != XST_SUCCESS) {
		xil_printf("XUartPsv_CfgInitialize FAILED with %d\n",Status );
	}
	

	XUartPsv_SetBaudRate(&Uart_Psv, 115200);


	LineCtrlRegister = XUartPsv_ReadReg(Config->BaseAddress, XUARTPSV_UARTLCR_OFFSET);

	LineCtrlRegister |= XUARTPSV_UARTLCR_PARITY_EVEN;		// Set Even parity
	LineCtrlRegister |= XUARTPSV_UARTLCR_PARITY_MASK;		// Enable parity

	/* Write the line controller register out */
	XUartPsv_WriteReg(Config->BaseAddress,
			XUARTPSV_UARTLCR_OFFSET, LineCtrlRegister);

	/* Check hardware build. */
	Status = XUartPsv_SelfTest(&Uart_Psv);
	if (Status != XST_SUCCESS) {
		xil_printf("XUartPs_SelfTest FAILED with %d\n",Status );
	}

	XUartPsv_SetOperMode(&Uart_Psv, XUARTPSV_OPER_MODE_NORMAL);

	xTaskCreate( CMC_BuildProfile_Versal_VCK5000_R5_Peripheral_uart_sc_communications_POLL_THREAD,
						 ( const char * ) "UART_POLL",
						 configMINIMAL_STACK_SIZE,
						 NULL,
						 tskIDLE_PRIORITY + 1,
						 &uartPollTaskHandle );


}


void CMC_BuildProfile_Versal_VCK5000_R5_Peripheral_uart_sc_communications_UART_SERVICES_INTERRUPT(void)
{
	char ReceivedOctet;

	while (!CMC_BuildProfile_Versal_VCK5000_R5_Peripheral_uart_sc_communications_UART_SERVICES_IS_RECEIVE_EMPTY(AXI_UART_LITE_Satellite_Context.UART_Services.pUserContext, AXI_UART_LITE_Satellite_Context.BaseAddress))
	{
		ReceivedOctet = CMC_BuildProfile_Versal_VCK5000_R5_Peripheral_uart_sc_communications_UART_SERVICES_RECV_BYTE_BLOCKING(AXI_UART_LITE_Satellite_Context.UART_Services.pUserContext, AXI_UART_LITE_Satellite_Context.BaseAddress); /* previous check means won't block */

		if (NULL != AXI_UART_LITE_Satellite_Context.pFN_Push)
		{
			(*AXI_UART_LITE_Satellite_Context.pFN_Push)(AXI_UART_LITE_Satellite_Context.pPushContext, ReceivedOctet);
		}
	}
}


void CMC_BuildProfile_Versal_VCK5000_R5_Peripheral_uart_sc_communications(CMC_BUILD_PROFILE_TYPE * pProfile)
{
	pProfile->Peripherals.UART[UART_CATEGORY_SC_COMMUNICATIONS].pUserContext = &CMC_BuildProfile_Versal_VCK5000_R5_Peripheral_uart_sc_communications_Context;
	pProfile->Peripherals.UART[UART_CATEGORY_SC_COMMUNICATIONS].pFN_Initialize = CMC_BuildProfile_Versal_VCK5000_R5_Peripheral_uart_sc_communications_UART_SERVICES_INITIALIZE;
	pProfile->Peripherals.UART[UART_CATEGORY_SC_COMMUNICATIONS].pFN_IsReceiveEmpty = CMC_BuildProfile_Versal_VCK5000_R5_Peripheral_uart_sc_communications_UART_SERVICES_IS_RECEIVE_EMPTY;
	pProfile->Peripherals.UART[UART_CATEGORY_SC_COMMUNICATIONS].pFN_IsTransmitFull = CMC_BuildProfile_Versal_VCK5000_R5_Peripheral_uart_sc_communications_UART_SERVICES_IS_TRANSMIT_FULL;
	pProfile->Peripherals.UART[UART_CATEGORY_SC_COMMUNICATIONS].pFN_RecvByteBlocking = CMC_BuildProfile_Versal_VCK5000_R5_Peripheral_uart_sc_communications_UART_SERVICES_RECV_BYTE_BLOCKING;
	pProfile->Peripherals.UART[UART_CATEGORY_SC_COMMUNICATIONS].pFN_SendByteNonBlocking = CMC_BuildProfile_Versal_VCK5000_R5_Peripheral_uart_sc_communications_UART_SERVICES_SEND_BYTE_NON_BLOCKING;
	pProfile->Peripherals.UART[UART_CATEGORY_SC_COMMUNICATIONS].pFN_Interrupt = CMC_BuildProfile_Versal_VCK5000_R5_Peripheral_uart_sc_communications_UART_SERVICES_INTERRUPT;
}

