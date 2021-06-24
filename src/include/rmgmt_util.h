/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#ifndef RMGMT_MAIN_H
#define RMGMT_MAIN_H

#define RMGMT_VERBOSE
#define _RPU_

/* FreeRTOS includes */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"

/* C includes */
#include "stdlib.h"
#include "stdio.h"

/* Xilinx includes */
#include "xil_printf.h"
#include "xil_io.h"
#include "xil_cache.h"
#include "xil_types.h"

#define ARRAY_SIZE(x) (sizeof(x) / sizeof (*(x)))

#ifdef RMGMT_VERBOSE
#define RMGMT_DBG(format, ...) xil_printf(format, ##__VA_ARGS__)
#else
#define RMGMT_DBG(format, ...)
#endif
#define RMGMT_LOG(format, ...) xil_printf(format, ##__VA_ARGS__)

#define IO_READ32(addr) Xil_In32(addr)
#define IO_WRITE32(val, addr) Xil_Out32(addr, val)

#define IO_SYNC_READ32(addr) ({ 				\
		Xil_DCacheFlushRange(addr, sizeof(u32));	\
		Xil_In32(addr); })

#define IO_SYNC_WRITE32(val, addr) ({ 				\
		Xil_Out32(addr, val);				\
		Xil_DCacheFlushRange(addr, sizeof(u32)); })

/*
 * temporary use unused register(this address is reserved for RPU-1)
 * to log hartbeat count
 */
#define RMGMT_HEARTBEAT_REG	0x3ef00000

//When FreeRTOS hardcoded configTOTAL_HEAP_SIZE can be changed,
//we will switch to heap_4 to manage memory */
//#define malloc(size) pvPortMalloc(size)
//#define free(ptr) vPortFree(ptr)

#endif
