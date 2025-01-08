/******************************************************************************
* Copyright (C) 2024 Advanced Micro Devices, Inc.    All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#ifndef UART_RTOS_H
#define UART_RTOS_H

#include <xuartpsv.h>
#include <FreeRTOS.h>
#include <event_groups.h>
#include <semphr.h>

#include "xil_types.h"
#include "xscugic.h"

#include "../vmc/vmc_api.h"


#define MAX_LOG_SIZE	(1024u)

typedef enum _UART_EVENTS{
	UART_RTOS_COMPLETE = 0x1,
	UART_RTOS_RX_ERROR = 0x2,
}UART_EVENTS;

typedef enum _UART_STATUS{
	UART_SUCCESS = 0,
	UART_ERROR_INIT = -1,
	UART_ERROR_SEMAPHORE = -2,
	UART_ERROR_EVENT = -3,
	UART_ERROR_TIMEOUT = -4,
	UART_INVALID_HANDLE = -5,
	UART_ERROR_GENERIC = -100,
}UART_STATUS;

typedef struct _uart_rtos_cb_t{
	u32 receivedBytes;
	u32 sentBytes;
	u32 errorCount;
	u32 eventState;
}uart_rtos_cb_t;


typedef struct _uart_rtos_handle_t{
	XUartPsv 			uartPsv;
	SemaphoreHandle_t 	rxSem;
	SemaphoreHandle_t 	txSem;
	EventGroupHandle_t 	rxEvent;
	EventGroupHandle_t 	txEvent;
	uart_rtos_cb_t 		cb_msg;
	u8			uart_IRQ_ID;
}uart_rtos_handle_t;

typedef struct _uart_rtos_config_t{
	uart_rtos_handle_t 	*uartHandler;
	u8					uart_ID;
	u8					uart_IRQ_ID;
	u8					INTC_ID;
	XScuGic 			INTC;
}uart_rtos_config_t;

void uart_enable_interrupt(u8 InterruptID);
void uart_disable_interrupt(u8 InterruptID);
bool uart_shm_acquire(SemaphoreHandle_t sem_data);
bool uart_shm_release(SemaphoreHandle_t sem_data);
UART_STATUS UART_RTOS_Send(uart_rtos_handle_t *handle, uint8_t *buf, uint32_t size);
UART_STATUS UART_RTOS_Receive_Set_Buffer(uart_rtos_handle_t *handle, uint8_t *buf, uint32_t size, uint32_t *received);
UART_STATUS UART_RTOS_Receive_Wait(uart_rtos_handle_t *handle, uint32_t *received, uint32_t timeout);
int32_t UART_RTOS_Enable(uart_rtos_config_t *uartConfig, ePlatformType curr_platform);
int32_t UART_RTOS_Disable(uart_rtos_handle_t *handle);
int32_t UART_RTOS_Debug_Enable(uart_rtos_handle_t *handle, ePlatformType curr_platform);
int32_t UART_VMC_SC_Enable(uart_rtos_handle_t *handle, ePlatformType curr_platform);

#endif
