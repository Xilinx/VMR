/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#ifndef COMMON_MEM_H
#define COMMON_MEM_H

/* FreeRTOS includes */
#include "FreeRTOS.h"

#include "cl_log.h"

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


//#define malloc(app, size) cl_malloc(app, size)
//#define free(prt) cl_free(ptr)


void * Cl_SecureMemcpy(void *dst, size_t dst_size, const void *src, size_t src_size);
void * Cl_SecureMemmove(void *dst, size_t dst_size, const void *src, size_t src_size);
void * Cl_SecureMemset(const void *dst, s32 val, size_t dst_size);
s32 Cl_SecureMemcmp(const void *dst, size_t dst_size, const void *src, size_t src_size);
void * Cl_SecureStrncpy(void *dst, size_t dst_size, const void *src, size_t src_size);
s32 Cl_SecureStrncmp(const void *dst, size_t dst_size, const void *src, size_t src_size);

#endif
