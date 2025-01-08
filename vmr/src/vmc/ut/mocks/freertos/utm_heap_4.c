/******************************************************************************
* Copyright (C) 2024 Advanced Micro Devices, Inc.    All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <stdlib.h>
#include <cmocka.h>
#include <assert.h>
#include "string.h"

/*****************************Mock functions *******************************/
void * __wrap_pvPortMalloc( size_t xWantedSize )
{
	return malloc(xWantedSize);
}

void __wrap_vPortFree( void * pv )
{
	if( pv != NULL )
	{
		free(pv);
	}	
}
