/******************************************************************************
* Copyright (C) 2024 Advanced Micro Devices, Inc.    All rights reserved.
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

struct xgq_vmr_cmd_identify;
struct rmgmt_handler;
struct cl_msg;

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

int rmgmt_apu_identify(struct xgq_vmr_cmd_identify *id_cmd);
int rmgmt_apu_info(char *buf, u32 size);
int rmgmt_apu_log(char *buf, u32 off, u32 size);
int rmgmt_apu_download_xclbin(struct rmgmt_handler *rh);

int rmgmt_fpt_get_debug_type(struct cl_msg *msg, u8 *debug_type);
int rmgmt_enable_boot_default(struct cl_msg *msg);
int rmgmt_enable_boot_backup(struct cl_msg *msg);
int rmgmt_fpt_debug(struct cl_msg *msg);
u32 rmgmt_boot_on_offset();

int rmgmt_is_ready(void);

#endif
