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

/* === start define data-driven endpoints from xparameters.h */
#define EP_RING_BUFFER_BASE 	0x38000000
#define EP_SYSTEM_DTB		0x40000
#define EP_PR_ISOLATION 	XPAR_BLP_BLP_LOGIC_BASE_CLOCKING_PR_RESET_GPIO_BASEADDR

#define EP_UCS_CHANNEL_1	0x0
#define EP_UCS_CHANNEL_2	0x8
#define EP_UCS_CONTROL 		XPAR_BLP_BLP_LOGIC_ULP_CLOCKING_UCS_CONTROL_STATUS_GPIO_UCS_CONTROL_STATUS_BASEADDR + EP_UCS_CHANNEL_2 

#define EP_FIREWALL_USER_BASE 	XPAR_AXI_FIREWALL_0_BASEADDR

#define EP_GAPPING_DEMAND	XPAR_BLP_BLP_LOGIC_ULP_CLOCKING_GAPPING_DEMAND_GPIO_GAPPING_DEMAND_BASEADDR

#define EP_ACLK_KERNEL_0	XPAR_CLK_WIZ_0_BASEADDR
#define EP_ACLK_KERNEL_1	XPAR_CLK_WIZ_1_BASEADDR
#define EP_ACLK_HBM_0		0	
#define EP_ACLK_SHUTDOWN_0	0	

#define EP_ACLK_FREQ_0		XPAR_SHELL_UTILS_FREQUENCY_COUNTER_0_BASEADDR
#define EP_ACLK_FREQ_KERNEL_0	XPAR_SHELL_UTILS_FREQUENCY_COUNTER_1_BASEADDR
#define EP_ACLK_FREQ_KERNEL_1	XPAR_SHELL_UTILS_FREQUENCY_COUNTER_2_BASEADDR
#define EP_ACLK_FREQ_HBM	0
#define EP_ACLK_FREQ_K1_K2	0

#define EP_PMC_REG		0xF1130000
#define EP_FORCE_RESET		XPAR_BLP_BLP_LOGIC_BASE_CLOCKING_FORCE_RESET_GPIO_BASEADDR
/* === end define data-driven endpoints from xparameters.h */

#define FAULT_STATUS            0x0
#define BIT(n) 			(1UL << (n))
#define READ_RESPONSE_BUSY      BIT(0)
#define WRITE_RESPONSE_BUSY     BIT(16)
#define FIREWALL_STATUS_BUSY    (READ_RESPONSE_BUSY | WRITE_RESPONSE_BUSY)
#define IS_FIRED(val) 		(val & ~FIREWALL_STATUS_BUSY)

#define	PMC_ERR1_STATUS_MASK	(1 << 24)
#define	PMC_ERR_OUT1_EN_MASK	(1 << 24)
#define	PMC_POR1_EN_MASK	(1 << 24)
#define	PMC_REG_ERR_OUT1_MASK	0x20
#define	PMC_REG_ERR_OUT1_EN	0x24
#define	PMC_REG_POR1_MASK	0x40
#define	PMC_REG_POR1_EN		0x44
#define	PL_TO_PMC_ERROR_SIGNAL_PATH_MASK	(1 << 0)

#define MDELAY(n) vTaskDelay( pdMS_TO_TICKS(n) )

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
