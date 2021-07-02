/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#ifndef RMGMT_MAIN_H
#define RMGMT_MAIN_H

#define _RPU_

/* FreeRTOS includes */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"

/* C includes */
#include "stdlib.h"
#include "stdio.h"

/* Xilinx includes */
#include "xil_printf.h"
#include "xil_io.h"
#include "xil_cache.h"
#include "xil_types.h"

#include "cl_log.h"
#include "cl_main.h"
#include "cl_io.h"

#define RMGMT_LOG(fmt, arg...)	\
	CL_LOG(APP_RMGMT, fmt, ##arg)
#define RMGMT_DBG(fmt, arg...)	\
	CL_DBG(APP_RMGMT, fmt, ##arg)
/*
 * temporary use unused register(this address is reserved for RPU-1)
 * to log hartbeat count
 */
#define RMGMT_HEARTBEAT_REG	0x3ef00000

#endif
