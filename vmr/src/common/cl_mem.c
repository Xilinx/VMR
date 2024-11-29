/******************************************************************************
* Copyright (C) 2024 AMD, Inc.    All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#include "cl_mem.h"
#include "cl_log.h"
#include "stdio.h"
#include "string.h"


void * Cl_SecureMemcpy(void *dst, size_t dst_size, const void *src, size_t src_size){

	const char *cl_src = (const char *)src;
	char *cl_dst = (char *)dst;

	configASSERT((NULL != cl_src) && (0 != cl_src));
	configASSERT((NULL != cl_dst) && (0 != cl_dst));
	configASSERT(0 != dst_size);
	configASSERT(0 != src_size);
	configASSERT(src_size <= dst_size);

	/* Note: memmove is safer than memcpy. */
	return memmove(cl_dst, cl_src, src_size);
}

void * Cl_SecureMemmove(void *dst, size_t dst_size, const void *src, size_t src_size){

	const char *cl_src = (const char *)src;
	char *cl_dst = (char *)dst;

	configASSERT((NULL != cl_src) && (0 != cl_src));
	configASSERT((NULL != cl_dst) && (0 != cl_dst));
	configASSERT(0 != dst_size);
	configASSERT(0 != src_size);
	configASSERT(src_size <= dst_size);

	return memmove(cl_dst, cl_src, src_size);
}

void * Cl_SecureMemset(const void *dst, s32 val, size_t dst_size){

	char *cl_dst = (char *)dst;

	configASSERT(NULL != cl_dst);

	return memset(cl_dst, (int)val, dst_size);
}

s32 Cl_SecureMemcmp(const void *dst, size_t dst_size, const void *src, size_t src_size){

	const char *cl_src = (const char *)src;
	const char *cl_dst = (const char *)dst;

	configASSERT((NULL != cl_src) && (0 != cl_src));
	configASSERT((NULL != cl_dst) && (0 != cl_dst));
	configASSERT(0 != dst_size);
	configASSERT(0 != src_size);
	configASSERT(src_size <= dst_size);

	return (s32)memcmp(cl_dst, cl_src, src_size);
}

void * Cl_SecureStrncpy(void *dst, size_t dst_size, const void *src, size_t src_size){

	const char *cl_src = (const char *)src;
	char *cl_dst = (char *)dst;

	configASSERT((NULL != cl_src) && (0 != cl_src));
	configASSERT((NULL != cl_dst) && (0 != cl_dst));
	configASSERT(0 != dst_size);
	configASSERT(0 != src_size);
	configASSERT(src_size <= dst_size);

	return strncpy(cl_dst, cl_src, src_size);
}

s32 Cl_SecureStrncmp(const void *dst, size_t dst_size, const void *src, size_t src_size){

	const char *cl_src = (const char *)src;
	const char *cl_dst = (const char *)dst;

	configASSERT((NULL != cl_src) && (0 != cl_src));
	configASSERT((NULL != cl_dst) && (0 != cl_dst));
	configASSERT(0 != dst_size);
	configASSERT(0 != src_size);
	configASSERT(src_size <= dst_size);

	return (s32)strncmp(cl_dst, cl_src, src_size);
}
