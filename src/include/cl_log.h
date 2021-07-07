/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#ifndef COMMON_LOG_H
#define COMMON_LOG_H

/**
 * Comment VERBOSE out in production code
 */
#define CL_VERBOSE

/* C includes */
#include "stdlib.h"
#include "stdio.h"

/* Xilinx includes */
#include "xil_printf.h"

/**
 * Note: preprocessors are complier related...
 */
#define CL_LOG(app, fmt, arg...) 		\
	xil_printf("%d: %s " fmt "\r\n", 	\
		app, __FUNCTION__, ##arg)

#ifdef CL_VERBOSE
#define CL_DBG(app, fmt, arg...) 		\
	xil_printf("%d: %s:%d %s " fmt "\r\n", 	\
		app, __FILE__, __LINE__, __FUNCTION__, ##arg)
#else
#define CL_DBG(app, fmt, arg...)
#endif

#endif
