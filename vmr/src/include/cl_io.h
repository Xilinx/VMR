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

	if (len & 0x3)
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

static inline int cl_memcpy_toio(u32 dst, void *buf, size_t len)
{
	int ret = 0;
	u32 last_trunk_size = 0;

	/* len is not 32bit aligned, read last unaligned by ioread8 */
	if (len & 0x3) {
		last_trunk_size = len & 0x3;
	}

	ret = cl_memcpy_toio32(dst, buf, len - last_trunk_size);
	if (ret != (len - last_trunk_size))
		return ret;

	if (last_trunk_size) {
		ret = cl_memcpy_toio8(dst + len - last_trunk_size,
				buf + len - last_trunk_size,
				last_trunk_size);
		if (ret != last_trunk_size)
			return ret;
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

static inline int cl_memcpy_fromio(u32 src, void *buf, size_t len)
{
	int ret = 0;
	u32 last_trunk_size = 0;

	/* len is not 32bit aligned, read last unaligned by ioread8 */
	if (len & 0x3) {
		last_trunk_size = len & 0x3;
	}

	ret = cl_memcpy_fromio32(src, buf, len - last_trunk_size);
	if (ret != (len - last_trunk_size))
		return ret;

	if (last_trunk_size) {
		ret = cl_memcpy_fromio8(src + len - last_trunk_size,
				buf + len - last_trunk_size,
				last_trunk_size);
		if (ret != last_trunk_size)
			return ret;
	}

	return len;
}

/* safe memcpy */
static inline int cl_memcpy(u32 dst_addr, u32 src_addr, size_t n)
{
	size_t i;
	for (i = 0; i < n; i++, src_addr++, dst_addr++) {
		IO_SYNC_WRITE8(IO_SYNC_READ8(src_addr), dst_addr);
	}

	return n;
}

static inline int cl_memset_io8(u32 dst, u8 val, size_t len)
{
	size_t i;

	for (i = 0; i < len; i++, dst++ ) {
		IO_SYNC_WRITE8(val, dst);
	}

	return len;
}

static inline u32 cl_bswap32(u32 x)
{
       return (x >> 24) | (x >> 8 & 0xff00) | (x << 8 & 0xff0000) | (x << 24);
}

#endif
