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
 * @val: 	u32 value to write
 * @addr:	offset of IO address
 *
 * Write data and then sync cache.
 */
#define IO_SYNC_WRITE32(val, addr) ({ 				\
		Xil_Out32(addr, val);				\
		Xil_DCacheFlushRange(addr, sizeof(u32)); })

static inline void cl_memcpy_toio(u32 dst, void *buf, size_t len)
{
	size_t i;
	u32 *src = (u32 *)buf;

	for (i = 0; i < len / 4; i++, dst += 4) {
		IO_SYNC_WRITE32(src[i], dst);
	}
}

static inline void cl_memcpy_fromio(u32 src, void *buf, size_t len)
{
	size_t i;
	u32 *dst = (u32 *)buf;
	
	for (i = 0; i < len / 4; i++, src += 4) {
		dst[i] = IO_SYNC_READ32(src);
	}
}

#endif
