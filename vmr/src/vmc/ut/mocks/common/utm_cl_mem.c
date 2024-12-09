/******************************************************************************
* Copyright (C) 2024 Advanced Micro Devices, Inc.    All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/


#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <cmocka.h>
#include <assert.h>
#include "cl_mem.h"
#include "string.h"

/*****************************Mock functions *******************************/
void * __wrap_Cl_SecureMemcpy(void *dst, size_t dst_size, const void *src, size_t src_size)
{
	const char *cl_src = (const char *)src;
	char *cl_dst = (char *)dst;

	assert((NULL != cl_src) && (0 != cl_src));
	assert((NULL != cl_dst) && (0 != cl_dst));
	assert(0 != dst_size);
	assert(0 != src_size);
	assert(src_size <= dst_size);

	/* Note: memmove is safer than memcpy. */
	return memmove(cl_dst, cl_src, src_size);
}

void * __wrap_Cl_SecureMemset(const void *dst, s32 val, size_t dst_size)
{
	char *cl_dst = (char *)dst;

	assert(NULL != cl_dst);

	return memset(cl_dst, (int)val, dst_size);
}

s32 __wrap_Cl_SecureMemcmp(const void *dst, size_t dst_size, const void *src, size_t src_size)
{
	const char *cl_src = (const char *)src;
	const char *cl_dst = (const char *)dst;

	assert((NULL != cl_src) && (0 != cl_src));
	assert((NULL != cl_dst) && (0 != cl_dst));
	assert(0 != dst_size);
	assert(0 != src_size);
	assert(src_size <= dst_size);

	return (s32)memcmp(cl_dst, cl_src, src_size);
}

s32 __wrap_Cl_SecureStrncmp(const void *dst, size_t dst_size, const void *src, size_t src_size)
{
	const char *cl_src = (const char *)src;
	const char *cl_dst = (const char *)dst;

	assert((NULL != cl_src) && (0 != cl_src));
	assert((NULL != cl_dst) && (0 != cl_dst));
	assert(0 != dst_size);
	assert(0 != src_size);
	assert(src_size <= dst_size);

	return (s32)strncmp(cl_dst, cl_src, src_size);
}
