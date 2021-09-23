
/* (c) Copyright 2018 Xilinx, Inc. All rights reserved.
 *
 *  This file contains confidential and proprietary information
 *  of Xilinx, Inc. and is protected under U.S. and
 *  international copyright and other intellectual property
 *  laws.
 *
 *  DISCLAIMER
 *  This disclaimer is not a license and does not grant any
 *  rights to the materials distributed herewith. Except as
 *  otherwise provided in a valid license issued to you by
 *  Xilinx, and to the maximum extent permitted by applicable
 *  law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND
 *  WITH ALL FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES
 *  AND CONDITIONS, EXPRESS, IMPLIED, OR STATUTORY, INCLUDING
 *  BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, NON-
 *  INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE; and
 *  (2) Xilinx shall not be liable (whether in contract or tort,
 *  including negligence, or under any other theory of
 *  liability) for any loss or damage of any kind or nature
 *  related to, arising under or in connection with these
 *  materials, including for any direct, or any indirect,
 *  special, incidental, or consequential loss or damage
 *  (including loss of data, profits, goodwill, or any type of
 *  loss or damage suffered as a result of any action brought
 *  by a third party) even if such damage or loss was
 *  reasonably foreseeable or Xilinx had been advised of the
 *  possibility of the same.
 *
 *  CRITICAL APPLICATIONS
 *  Xilinx products are not designed or intended to be fail-
 *  safe, or for use in any application requiring fail-safe
 *  performance, such as life-support or safety devices or
 *  systems, Class III medical devices, nuclear facilities,
 *  applications related to the deployment of airbags, or any
 *  other applications that could lead to death, personal
 *  injury, or severe property or environmental damage
 *  (individually and collectively, "Critical
 *  Applications"). Customer assumes the sole risk and
 *  liability of any use of Xilinx products in Critical
 *  Applications, subject only to applicable laws and
 *  regulations governing limitations on product liability.
 *
 *  THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS
 *  PART OF THIS FILE AT ALL TIMES.
 */

/***
 *
 * @file uart_rtos.h
 *
 * Author: Shahab.A
 * Date: 2021-09-03
 *
 * @brief FreeRTOS wrapper for UART driver.
 *
 */


#ifndef UART_RTOS_H
#define UART_RTOS_H

#include <xuartpsv.h>
#include <FreeRTOS.h>
#include <event_groups.h>
#include <semphr.h>

#include "xil_types.h"
#include "xscugic.h"


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
}uart_rtos_handle_t;

typedef struct _uart_rtos_config_t{
	uart_rtos_handle_t 	*uartHandler;
	u8					uart_ID;
	u8					uart_IRQ_ID;
	u8					INTC_ID;
	XScuGic 			INTC;
}uart_rtos_config_t;

s32 UART_RTOS_Send(uart_rtos_handle_t *handle, u8 *buf, u32 size);
s32 UART_RTOS_Receive(uart_rtos_handle_t *handle, u8 *buf, u32 size, u32 *received);
s32 UART_RTOS_Enable(uart_rtos_config_t *uartConfig);
s32 UART_RTOS_Disable(uart_rtos_handle_t *handle);
s32 UART_RTOS_Debug_Enable(uart_rtos_handle_t *handle);

#endif
