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

#include "vmr_common.h"

#define RMGMT_LOG(fmt, arg...)	\
	CL_LOG(APP_RMGMT, fmt, ##arg)
#define RMGMT_DBG(fmt, arg...)	\
	CL_DBG(APP_RMGMT, fmt, ##arg)

#define MDELAY(n) vTaskDelay( pdMS_TO_TICKS(n) )

struct rmgmt_handler {
	u32 rh_base;
	u32 rh_max_size;
	u32 rh_data_size;
	u8  *rh_data; 	/* static malloc and never free */
};

static void inline axigate_freeze()
{
	IO_SYNC_WRITE32(0x0, EP_PR_ISOLATION);
}

static void inline axigate_free()
{
	IO_SYNC_WRITE32(0x3, EP_PR_ISOLATION);
}

static void inline ucs_stop()
{
	IO_SYNC_WRITE32(0x0, EP_UCS_CONTROL);
}

static void inline ucs_start()
{
	IO_SYNC_WRITE32(0x1, EP_UCS_CONTROL);
}

#endif
