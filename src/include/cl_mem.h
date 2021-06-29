/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#ifndef COMMON_MEM_H
#define COMMON_MEM_H

/* FreeRTOS includes */
#include "FreeRTOS.h"

/**
 * Current Xilinx FreeRTOS includes heap_4.c with default BSP
 * build. We recomment to have our own memory management by
 * leveraging FreeRTOS pvPortMalloc/vPortFree.
 * We will malloc memory when system started, and maintain
 * different slab blocks managemented by ourselve.
 * In this we, we can easily:
 * 	1) add footprint in each memory to trace memory leaks;
 * 	2) easy to dump memory for easy debugging;
 */

#define APP_ID_CMC 	0x434d4321	/* CMC! */ 
#define APP_ID_RMGMT	0x58525421 	/* XRT! */

typedef struct app_signature {
	u32 app_id;
} app_signature_t;

#define malloc(app_signature, size) cl_malloc(app_signature, size)
#define free(app_signature, prt) cl_free(app_signature, ptr)

#endif
