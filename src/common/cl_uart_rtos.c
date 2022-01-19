/******************************************************************************
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "semphr.h"
#include "event_groups.h"
#include "portmacro.h"

#include "cl_uart_rtos.h"




/**************************** Type Definitions ******************************/


#define UART_RTOS_TASK_STACK_SIZE	(configMINIMAL_STACK_SIZE + 100)
#define UART_RTOS_TASK_PRIORITY		(5)

#define WELCOME_MSG							"\n\rHello from UART_RTOS to you!\n\r"
#define UART_TX_FIFO_THRESHOLD				(XUARTPSV_UARTIFLS_TXIFLSEL_1_8)

/* The FIFO triggers at 2 bytes larger than FIFO trigger level */
#define UART_TX_FIFO_THRESHOLD_TRIGGER		(UART_TX_FIFO_THRESHOLD + 2)

/************************** Function Prototypes *****************************/

static void 	UART_Task(void* pvParameters);
static void 	UART_RTOS_Handler(void *CallBackRef, u32 Event, unsigned int EventData);
static int32_t 	UART_Config(uart_rtos_handle_t *handle, XUartPsv *UartInstPtr, uint16_t DeviceId, uint16_t UartIntrId);

/************************** Variable Definitions ***************************/


/************************** Code ******************************************/


/**************************************************************************/
/**
*
* This function is the handler which performs processing to handle data events
* from the device.  It is called from an interrupt context. so the amount of
* processing should be minimal.
*
* This handler provides an example of how to handle data for the device and
* is application specific.
*
* @param	CallBackRef contains a callback reference from the driver,
*		in this case it is the instance pointer for the XUARTPSV driver.
* @param	Event contains the specific kind of event that has occurred.
* @param	EventData contains the number of bytes sent or received for sent
*		and receive events.
*
* @return	None.
*
* @note		None.
*
***************************************************************************/
static void UART_RTOS_Handler(void *CallBackRef, u32 Event, unsigned int EventData)
{
	uart_rtos_handle_t *uartHandle	= (uart_rtos_handle_t *)CallBackRef;
	BaseType_t xResult, xHigherPriorityTaskWoken;

	xResult						= pdFAIL;
	xHigherPriorityTaskWoken	= pdFALSE;

	uartHandle->cb_msg.eventState = Event;


	/* All of the data has been sent */
	if (Event == XUARTPSV_EVENT_SENT_DATA) {
		uartHandle->cb_msg.sentBytes = EventData;
		xResult = xEventGroupSetBitsFromISR(uartHandle->txEvent,UART_RTOS_COMPLETE,&xHigherPriorityTaskWoken);

	}

	/* All of the data has been received */
	if (Event == XUARTPSV_EVENT_RECV_DATA) {
		uartHandle->cb_msg.receivedBytes = EventData;
		xResult = xEventGroupSetBitsFromISR(uartHandle->rxEvent,UART_RTOS_COMPLETE,&xHigherPriorityTaskWoken);
	}

	/*
	 * Data was received, but not the expected number of bytes, a
	 * timeout just indicates the data stopped for 8 character times
	 */
	if (Event == XUARTPSV_EVENT_RECV_TOUT) {
		uartHandle->cb_msg.receivedBytes = EventData;
		xResult = xEventGroupSetBitsFromISR(uartHandle->rxEvent,UART_RTOS_COMPLETE,&xHigherPriorityTaskWoken);
	}

	/*
	 * Data was received with an error, keep the data but determine
	 * what kind of errors occurred
	 */
	if (Event == XUARTPSV_EVENT_RECV_ERROR) {
		uartHandle->cb_msg.receivedBytes = EventData;
		uartHandle->cb_msg.errorCount++;
		xResult = xEventGroupSetBitsFromISR(uartHandle->rxEvent,UART_RTOS_RX_ERROR,&xHigherPriorityTaskWoken);
	}

	/*
	 * Data was received with an parity or frame or break error, keep the data
	 * but determine what kind of errors occurred. Specific to Zynq Ultrascale+
	 * MP.
	 */
	if (Event == XUARTPSV_EVENT_PARE_FRAME_BRKE) {
		uartHandle->cb_msg.receivedBytes = EventData;
		uartHandle->cb_msg.errorCount++;
		xResult = xEventGroupSetBitsFromISR(uartHandle->rxEvent,UART_RTOS_RX_ERROR,&xHigherPriorityTaskWoken);
	}

	/*
	 * Data was received with an overrun error, keep the data but deterfmine
	 * what kind of errors occurred. Specific to Zynq Ultrascale+ MP.
	 */
	if (Event == XUARTPSV_EVENT_RECV_ORERR) {
		uartHandle->cb_msg.receivedBytes = EventData;
		uartHandle->cb_msg.errorCount++;
		xResult = xEventGroupSetBitsFromISR(uartHandle->rxEvent,UART_RTOS_RX_ERROR,&xHigherPriorityTaskWoken);
	}

	if(xResult == pdPASS)
	{
		portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	}
}




/**************************************************************************/
/**
*
* This creates FreeRTOS semaphores and wait-events for UART RTOS driver.
* Then it calls UART driver config API for the driver and interrupt configuration.
*
*
* @param	handle is a pointer to the instance of the UART RTOS driver handler.
*
* @param	IntcInstPtr is a pointer to the instance of the Scu Gic driver.
*
* @param	DeviceId is the device Id of the UART device and is typically
*		XPAR_<UartPsv_instance>_DEVICE_ID value from xparameters.h.
*
* @param	UartIntrId is the interrupt Id and is typically
*		XPAR_<UartPsv_instance>_INTR value from xparameters.h.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note
*
**************************************************************************/
static int32_t UART_RTOS_Init(uart_rtos_handle_t *handle, XScuGic *IntcInstPtr, u16 DeviceId, u16 UartIntrId){
	UART_STATUS ret = UART_SUCCESS;
	if(NULL == handle){
		return UART_ERROR_INIT;
	}


	handle->rxSem = xSemaphoreCreateMutex();
	if(NULL == handle->rxSem){
		return UART_ERROR_INIT;
	}

	handle->txSem = xSemaphoreCreateMutex();
	if(NULL == handle->txSem){
		vSemaphoreDelete(handle->rxSem);
		return UART_ERROR_INIT;
	}

	handle->rxEvent = xEventGroupCreate();
	if(NULL == handle->rxEvent){
		vSemaphoreDelete(handle->rxSem);
		vSemaphoreDelete(handle->txSem);
		return UART_ERROR_INIT;
	}

	handle->txEvent = xEventGroupCreate();
	if(NULL == handle->txEvent){
		vSemaphoreDelete(handle->rxSem);
		vSemaphoreDelete(handle->txSem);
		vEventGroupDelete(handle->rxEvent);
		return UART_ERROR_INIT;
	}

	memset(&handle->cb_msg,0,sizeof(handle->cb_msg));


	ret = UART_Config(handle, &handle->uartPsv,DeviceId,UartIntrId);

	return ret;
}


/**************************************************************************/
/**
*
* This function configures UART driver and its corresponding interrupt handler.
*
*
* This function uses interrupt mode of the device.
*
* @param	handle is a pointer to the instance of the UART RTOS driver handler.
*
* @param	UartInstPtr is a pointer to the instance of the UART driver
*		which is going to be connected to the interrupt controller.
*
* @param	DeviceId is the device Id of the UART device and is typically
*		XPAR_<UartPsv_instance>_DEVICE_ID value from xparameters.h.
*
* @param	UartIntrId is the interrupt Id and is typically
*		XPAR_<UartPsv_instance>_INTR value from xparameters.h.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note
*
**************************************************************************/
static int32_t UART_Config(uart_rtos_handle_t *handle, XUartPsv *UartInstPtr, uint16_t DeviceId, uint16_t UartIntrId)
{
	int32_t Status;
	XUartPsv_Config *Config;
	uint32_t IntrMask;
    uint32_t LineCtrlRegister;



	/*
	 * Initialize the UART driver so that it's ready to use
	 * Look up the configuration in the config table, then initialize it.
	 */
	Config = XUartPsv_LookupConfig(DeviceId);
	if (NULL == Config) {
		return XST_FAILURE;
	}

	Status = XUartPsv_CfgInitialize(UartInstPtr, Config, Config->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

#ifndef VMC_DEBUG

	LineCtrlRegister = XUartPsv_ReadReg(Config->BaseAddress, XUARTPSV_UARTLCR_OFFSET);

	LineCtrlRegister |= XUARTPSV_UARTLCR_PARITY_EVEN;               // Set Even parity
	LineCtrlRegister |= XUARTPSV_UARTLCR_PARITY_MASK;               // Enable parity

#endif
	/* Write the line controller register out */
	XUartPsv_WriteReg(Config->BaseAddress,
			XUARTPSV_UARTLCR_OFFSET, LineCtrlRegister);

	/* Check hardware build. */
	Status = XUartPsv_SelfTest(UartInstPtr);
	if (Status != XST_SUCCESS) {
		xil_printf("XUartPs_SelfTest FAILED with %d\n",Status );
	}
	/*
	 * Setup the handlers for the UART that will be called from the
	 * interrupt context when data has been sent and received, specify
	 * a pointer to the UART driver instance as the callback reference
	 * so the handlers are able to access the instance data
	 */
	XUartPsv_SetHandler(UartInstPtr, (XUartPsv_Handler)UART_RTOS_Handler, handle);


	XUartPsv_SetRxFifoThreshold(UartInstPtr, XUARTPSV_UARTIFLS_RXIFLSEL_1_8);

	XUartPsv_SetTxFifoThreshold(UartInstPtr, UART_TX_FIFO_THRESHOLD);
	/*
	 * Enable the interrupt of the UART so interrupts will occur.
	 */
	IntrMask = (XUARTPSV_UARTIMSC_RXIM | XUARTPSV_UARTIMSC_TXIM |
			XUARTPSV_UARTIMSC_RTIM | XUARTPSV_UARTIMSC_FEIM |
			XUARTPSV_UARTIMSC_PEIM | XUARTPSV_UARTIMSC_BEIM |
			XUARTPSV_UARTIMSC_OEIM);

	XUartPsv_SetInterruptMask(UartInstPtr, IntrMask);

	/*
	 * Set driver mode to Normal.
	 * */
	XUartPsv_SetOperMode(UartInstPtr, XUARTPSV_OPER_MODE_NORMAL);

	/*
	 * Assign interrupt handler and enable interrupt.
	 * */
	xPortInstallInterruptHandler(UartIntrId, (XInterruptHandler)XUartPsv_InterruptHandler, UartInstPtr);
#ifdef VMC_DEBUG
	vPortEnableInterrupt(UartIntrId);
#endif
	return XST_SUCCESS;
}


/**************************************************************************/
/**
*
* This function deletes semaphores.
*
*
* @param	handle is a pointer to the instance of the UART RTOS driver handler.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note
*
**************************************************************************/
static int32_t UART_RTOS_Deinit(uart_rtos_handle_t *handle)
{
	vEventGroupDelete(handle->rxEvent);
	vEventGroupDelete(handle->txEvent);


	xSemaphoreGive(handle->txSem);
	xSemaphoreGive(handle->rxSem);

	vSemaphoreDelete(handle->txSem);
	vSemaphoreDelete(handle->rxSem);

	return XST_SUCCESS;

}


/**************************************************************************/
/**
*
* This function is called to transmit data bytes.
* If size is 2 bytes larger than TX FIFO threshold, wait-event will be used
* and when transmit is complete callback is called.
*
*
* @param	handle is a pointer to the instance of the UART RTOS driver handler.
*
* @param	buf holds data to be sent out.
*
* @param 	size is maximum number of bytes that needs to be sent which is
* 			equal to buf size.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note
*
**************************************************************************/
int32_t UART_RTOS_Send(uart_rtos_handle_t *handle, uint8_t *buf, uint32_t size)
{

	EventBits_t ev;
	UART_STATUS retVal = UART_SUCCESS;
	//uint32_t byteSent = 0;

	if(handle == NULL)
	{
		return UART_ERROR_GENERIC;
	}
	if(buf == NULL)
	{
		return UART_ERROR_GENERIC;
	}
	if(size == 0)
	{
		return UART_ERROR_GENERIC;
	}

	if(pdFALSE == xSemaphoreTake(handle->txSem,0))
	{
		return UART_ERROR_SEMAPHORE;
	}


	/*
	 * Send the buffer using the UART and ignore the number of bytes sent
	 * as the return value since we are using it in interrupt mode.
	 */
	XUartPsv_Send(&handle->uartPsv, buf, size);
	//xil_printf("\n\rbytes: %d", byteSent);
	if(size >= UART_TX_FIFO_THRESHOLD_TRIGGER)
	{

		ev = xEventGroupWaitBits(handle->txEvent, UART_RTOS_COMPLETE, pdTRUE, pdFALSE, 0xff);
		if(!(ev & UART_RTOS_COMPLETE))
		{
			retVal = UART_ERROR_EVENT;
		}
	}

	if(pdFALSE == xSemaphoreGive(handle->txSem))
	{
		retVal = UART_ERROR_SEMAPHORE;
	}

	return retVal;

}


/**************************************************************************/
/**
*
* This function is called to receive data bytes.
* It waits until all requested received bytes are received or an error occurs.
*
*
* @param	handle is a pointer to the instance of the UART RTOS driver handler.
*
* @param	buf is used to hold received data bytes.
*
* @param 	size is maximum size of buf to receive data bytes.
*
* @param 	received is a pointer which returns number of received bytes.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note
*
**************************************************************************/
int32_t UART_RTOS_Receive(uart_rtos_handle_t *handle, uint8_t *buf, uint32_t size, uint32_t *received){


	EventBits_t ev;
	UART_STATUS retVal = UART_SUCCESS;
	u32 ret;

	if(received == NULL)
	{
		return UART_ERROR_GENERIC;
	}
	if(handle == NULL)
	{
		return UART_ERROR_GENERIC;
	}
	if(buf == NULL)
	{
		return UART_ERROR_GENERIC;
	}
	if(size == 0)
	{
		*received = 0;
		return UART_SUCCESS;
	}

	if(pdFALSE == xSemaphoreTake(handle->rxSem,0))
	{
		return UART_ERROR_SEMAPHORE;
	}


	ret = XUartPsv_Recv(&handle->uartPsv, buf, size);

	ev = xEventGroupWaitBits(handle->rxEvent, UART_RTOS_COMPLETE, pdTRUE, pdFALSE, 0xff);
	if((ev & UART_RTOS_COMPLETE))
	{
		*received = handle->cb_msg.receivedBytes;
	}
	else if((ev & UART_RTOS_RX_ERROR))
	{
		*received = 0;
		retVal = UART_ERROR_EVENT;
	}

	if(pdFALSE == xSemaphoreGive(handle->rxSem))
	{
		retVal = UART_ERROR_SEMAPHORE;
	}
	vPortDisableInterrupt(XPAR_XUARTPS_0_INTR);
	return ret;
}



/**************************************************************************/
/**
*
* This task is created only with intend to initialize and configure UART
* driver when FreeRTOS is running.
*
* It prints a welcome message when initialization and configuration is successful.
*
*
* @param	handle is a pointer to UART RTOS driver configuration parameter.
*
* @return
*
* @note		Task will suspends itself at the end.
*
**************************************************************************/
static void UART_Task(void* pvParameters)
{
	int32_t status;

	uart_rtos_config_t *uartConf = (uart_rtos_config_t *)pvParameters;

	status = UART_RTOS_Init(uartConf->uartHandler, &uartConf->INTC, uartConf->uart_ID, uartConf->uart_IRQ_ID);
	if (status != XST_SUCCESS) {
		xil_printf("Uart RTOS Initialization Failed\r\n");
	}

	//UART_RTOS_Send(uartConf->uartHandler, (u8 *)WELCOME_MSG, strlen(WELCOME_MSG));

	vTaskSuspend(NULL);
}



/**************************************************************************/
/**
*
* This function is called to create a FreeRTOS task for UART RTOS driver
* configuration and initialization.
*
*
* @param	handle is a pointer to UART RTOS driver configuration parameter.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note
*
**************************************************************************/
int32_t UART_RTOS_Enable(uart_rtos_config_t *uartConfig)
{
    if(xTaskCreate(UART_Task,"UART_Task", UART_RTOS_TASK_STACK_SIZE, uartConfig, UART_RTOS_TASK_PRIORITY, NULL) != pdPASS)
    {
    	xil_printf("UART Task Create Failed!");
    	return XST_FAILURE;
    }

    return XST_SUCCESS;
}


/**************************************************************************/
/**
*
* This function is called to deinitialize UART RTOS driver.
*
*
* @param	handle is a pointer to the instance of the UART RTOS driver handler.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note
*
**************************************************************************/
int32_t UART_RTOS_Disable(uart_rtos_handle_t *handle)
{
	return UART_RTOS_Deinit(handle);
}



/**************************************************************************/
/**
*
* This function is called to enable, configure, and initialize debug UART
* with embedded UART configuration.
*
*
* @param	handle is a pointer to the instance of the UART RTOS driver handler.
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note
*
**************************************************************************/
s32 UART_RTOS_Debug_Enable(uart_rtos_handle_t *handle)
{

	static uart_rtos_config_t debugUartConig = {
			.INTC_ID = XPAR_SCUGIC_SINGLE_DEVICE_ID,
			.uart_ID = XPAR_XUARTPSV_1_DEVICE_ID,
			.uart_IRQ_ID = XPAR_XUARTPS_1_INTR
	};

	debugUartConig.uartHandler = handle;

	return UART_RTOS_Enable(&debugUartConig);
}

s32 UART_VMC_SC_Enable(uart_rtos_handle_t *handle)
{

	static uart_rtos_config_t vmcscUartConfig = {
			.INTC_ID = XPAR_SCUGIC_SINGLE_DEVICE_ID,
			.uart_ID = XPAR_XUARTPSV_0_DEVICE_ID,
			.uart_IRQ_ID = XPAR_XUARTPS_0_INTR
	};

	vmcscUartConfig.uartHandler = handle;

	return UART_RTOS_Enable(&vmcscUartConfig);
}
