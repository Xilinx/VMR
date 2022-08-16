/******************************************************************************
* Copyright (C) 2021-2022 Xilinx, Inc.  All rights reserved.
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
#include "stdbool.h"

/* Xilinx includes */
#include "xil_printf.h"
#include "xil_io.h"
#include "xil_cache.h"
#include "xil_types.h"

#include "cl_log.h"
#include "cl_main.h"
#include "cl_io.h"

#include "vmr_common.h"

#define MDELAY(n) vTaskDelay( pdMS_TO_TICKS(n) )

struct rmgmt_handler {
	u32 rh_base;   /* obsolated */
	u32 rh_boot_on_offset;
	u32 rh_data_max_size;
	u32 rh_data_size;
	u32 rh_log_max_size; 
	char *rh_log;	/* static malloc and never free */
	u8   *rh_data; 	/* static malloc and never free */
	bool rh_already_flashed; /* enforce reset/reboot after successfully flashed */
};

static void inline axigate_freeze()
{
	IO_SYNC_WRITE32(0x0, VMR_EP_PR_ISOLATION);
}

static void inline axigate_free()
{
	IO_SYNC_WRITE32(0x3, VMR_EP_PR_ISOLATION);
}

static void inline ucs_stop()
{
	IO_SYNC_WRITE32(0x0, VMR_EP_UCS_SHUTDOWN);
}

static void inline ucs_start()
{
	IO_SYNC_WRITE32(0x1, VMR_EP_UCS_SHUTDOWN);
}

u32 rmgmt_boot_on_offset();
#endif
