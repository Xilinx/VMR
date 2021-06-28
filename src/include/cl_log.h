/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#ifndef COMMON_LOG_H
#define COMMON_LOG_H

/**
 * Comment VERBOSE out in production code
 */
#define COMMON_LOG_VERBOSE

/* C includes */
#include "stdlib.h"
#include "stdio.h"

/* Xilinx includes */
#include "xil_printf.h"

#define RMGMT_LOG(fmt, arg...) xil_printf("[INFO]" fmt "\n", ##arg)

#ifdef RMGMT_VERBOSE
#define RMGMT_DBG(fmt, arg...) xil_printf("[DEBUG]" fmt "\n", ##arg)
#else
#define RMGMT_DBG(fmt, arg...)
#endif

#endif
