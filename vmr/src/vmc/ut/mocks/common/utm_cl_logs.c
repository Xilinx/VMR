/******************************************************************************
* Copyright (C) 2024 Advanced Micro Devices, Inc.    All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <cmocka.h>
#include "cl_log.h"

/*****************************Mock functions *******************************/
void __wrap_cl_printf(const char *name, uint32_t line, uint8_t log_level,
	const char *app_name, const char *fmt, ...)
{
	/*Do Nothing*/
}

