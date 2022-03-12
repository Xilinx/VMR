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
 * different slab blocks managed by ourselves.
 * In this we, we can easily:
 * 	1) add footprint in each memory to trace memory leaks;
 * 	2) easy to dump memory for easy debugging;
 */


void *cl_malloc(app_type_t app, size_t size);
void cl_free(void *ptr);

#define malloc(app, size) cl_malloc(app, size)
#define free(prt) cl_free(ptr)

#endif
