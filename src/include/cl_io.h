/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#ifndef COMMON_IO_H
#define COMMON_IO_H

/* FreeRTOS includes */
#include "FreeRTOS.h"

/* Xilinx includes */
#include "xil_io.h"
#include "xil_cache.h"
#include "xil_types.h"

/**
 * @addr: 	offset of IO address
 */
#define IO_READ32(addr) Xil_In32(addr)

/**
 * @val:	u32 value to write
 * @addr:	offset of IO address
 */
#define IO_WRITE32(val, addr) Xil_Out32(addr, val)


/**
 * @addr: 	offset of IO address
 *
 * Sync cache before reading.
 */
#define IO_SYNC_READ32(addr) ({ 				\
		Xil_DCacheFlushRange(addr, sizeof(u32));	\
		Xil_In32(addr); })

/**
 * @addr: 	offset of IO address
 *
 * Sync cache before reading.
 */
#define IO_SYNC_READ8(addr) ({ 				\
		Xil_DCacheFlushRange(addr, sizeof(u8));	\
		Xil_In8(addr); })


/**
 * @val: 	u32 value to write
 * @addr:	offset of IO address
 *
 * Write data and then sync cache.
 */
#define IO_SYNC_WRITE32(val, addr) ({ 				\
		Xil_Out32(addr, val);				\
		Xil_DCacheFlushRange(addr, sizeof(u32)); })

/**
 * @val: 	u8 value to write
 * @addr:	offset of IO address
 *
 * Write data and then sync cache.
 */
#define IO_SYNC_WRITE8(val, addr) ({ 				\
		Xil_Out8(addr, val);				\
		Xil_DCacheFlushRange(addr, sizeof(u8)); })


static inline int cl_memcpy_toio32(u32 dst, void *buf, size_t len)
{
	size_t i;
	u32 *src = (u32 *)buf;

	if (len % sizeof(u32))
		return -1;

	for (i = 0; i < len / 4; i++, dst += 4) {
		IO_SYNC_WRITE32(src[i], dst);
	}

	return len;
}

static inline int cl_memcpy_toio8(u32 dst, void *buf, size_t len)
{
	size_t i;
	u8 *src = (u8 *)buf;

	for (i = 0; i < len; i++, dst++ ) {
		IO_SYNC_WRITE8(src[i], dst);
	}

	return len;
}

static inline int cl_memcpy_fromio32(u32 src, void *buf, size_t len)
{
	size_t i;
	u32 *dst = (u32 *)buf;
	
	if (len % sizeof(u32))
		return -1;

	for (i = 0; i < len / 4; i++, src += 4) {
		dst[i] = IO_SYNC_READ32(src);
	}

	return len;
}

static inline int cl_memcpy_fromio8(u32 src, void *buf, size_t len)
{
	size_t i;
	u8 *dst = (u8 *)buf;
	
	for (i = 0; i < len; i++, src++) {
		dst[i] = IO_SYNC_READ8(src);
	}

	return len;
}

#endif
