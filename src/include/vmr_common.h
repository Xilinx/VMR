/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#ifndef VMR_COMMON_H
#define VMR_COMMON_H

#define ENODEV	19

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

#define EP_PLM_MULTIBOOT	0xF1110004
#define EP_PMC_REG		0xF1130000

#define FAULT_STATUS            0x0
#define BIT(n) 			(1UL << (n))
#define READ_RESPONSE_BUSY      BIT(0)
#define WRITE_RESPONSE_BUSY     BIT(16)
#define FIREWALL_STATUS_BUSY    (READ_RESPONSE_BUSY | WRITE_RESPONSE_BUSY)
#define IS_FIRED(val) 		(val & ~FIREWALL_STATUS_BUSY)

#define	PMC_ERR1_STATUS_MASK	(1 << 24)
#define	PMC_ERR_OUT1_EN_MASK	(1 << 24)
#define	PMC_POR1_EN_MASK	(1 << 24)
#define	PMC_POR_ENABLE_BIT	(1 << 24)
#define	PMC_REG_ERR_OUT1_MASK	0x20
#define	PMC_REG_ERR_OUT1_EN	0x24
#define	PMC_REG_POR1_MASK	0x40
#define	PMC_REG_POR1_EN		0x44
#define	PMC_REG_ACTION		0x48
#define	PMC_REG_SRST		0x84
#define	PL_TO_PMC_ERROR_SIGNAL_PATH_MASK	(1 << 0)

/* Note: eventually we should be driven by xparameter.h */
#define RPU_SHARED_MEMORY_START	(0x38000000)
#define RPU_SHARED_MEMORY_END	(0x3FFFF000)
#define RPU_PRELOAD_FPT		(0x7FBF0000)
#define RPU_SQ_BASE (XPAR_BLP_BLP_LOGIC_XGQ_M2R_BASEADDR + XGQ_SQ_TAIL_POINTER)
#define RPU_CQ_BASE (XPAR_BLP_BLP_LOGIC_XGQ_M2R_BASEADDR + XGQ_CQ_TAIL_POINTER)

/* Reserve 4K for partition table */
#define VMR_PARTITION_TABLE_SIZE	0x1000
#define RPU_RING_BUFFER_OFFSET (RPU_SHARED_MEMORY_START + VMR_PARTITION_TABLE_SIZE)


#if defined(CONFIG_FORCE_RESET)
#define EP_FORCE_RESET		XPAR_BLP_BLP_LOGIC_BASE_CLOCKING_FORCE_RESET_GPIO_BASEADDR
static inline int rmgmt_enable_pl_reset()
{
	int val = 0;
	u32 pmc_mux = EP_FORCE_RESET;

	val = IO_SYNC_READ32(pmc_mux);
	val |= PL_TO_PMC_ERROR_SIGNAL_PATH_MASK;
	IO_SYNC_WRITE32(val, pmc_mux); 
	
	return 0;
}
#else
static inline int rmgmt_enable_pl_reset() { return -ENODEV; }
#endif
#endif
