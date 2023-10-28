/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#ifndef VMR_COMMON_H
#define VMR_COMMON_H

/* compatible with linux OS error codes */
#define ENOENT	2	
#define EIO	5
#define ENOMEM	12
#define EBUSY	16
#define ENODEV	19
#define EINVAL	22

#define OCL_MAX_ID	3
#define OCL_MAX_NUM	4

#define XGQ_SQ_TAIL_POINTER     0x0
#define XGQ_SQ_INTR_REG         0x4
#define XGQ_SQ_INTR_CTRL        0xC
#define XGQ_CQ_TAIL_POINTER     0x100
#define XGQ_CQ_INTR_REG         0x104
#define XGQ_CQ_INTR_CTRL        0x10C

#define GCQ_CLIENT_SQ_TAIL_POINTER 0x0
#define GCQ_CLIENT_SQ_INTR_REG     0x4
#define GCQ_CLIENT_SQ_INTR_CTRL    0xC
#define GCQ_CLIENT_CQ_TAIL_POINTER 0x100
#define GCQ_CLIENT_CQ_INTR_REG     0x104
#define GCQ_CLIENT_CQ_INTR_CTRL    0x10C

#define GCQ_SERVER_SQ_TAIL_POINTER 0x100
#define GCQ_SERVER_SQ_INTR_REG     0x104
#define GCQ_SERVER_SQ_INTR_CTRL    0x10C
#define GCQ_SERVER_CQ_TAIL_POINTER 0x0
#define GCQ_SERVER_CQ_INTR_REG     0x4
#define GCQ_SERVER_CQ_INTR_CTRL    0xC

#define RPU_XGQ_SLOT_SIZE 	512
#define RPU_RING_BUFFER_LEN	0x1000

/* === start define data-driven endpoints from xparameters.h */
#define VMR_EP_SYSTEM_DTB	0x40000
#define VMR_EP_PR_ISOLATION 	XPAR_BLP_BLP_LOGIC_BASE_CLOCKING_PR_RESET_GPIO_BASEADDR

#define VMR_EP_UCS_CHANNEL_1	0x0
#define VMR_EP_UCS_CHANNEL_2	0x8
#define VMR_EP_UCS_SHUTDOWN 	VMR_EP_UCS_CONTROL_STATUS_BASEADDR + VMR_EP_UCS_CHANNEL_2

#define VMR_EP_FIREWALL_USER_BASE XPAR_AXI_FIREWALL_0_BASEADDR

#define VMR_EP_ACLK_KERNEL_0	XPAR_CLK_WIZ_0_BASEADDR
#define VMR_EP_ACLK_KERNEL_1	XPAR_CLK_WIZ_1_BASEADDR
#define VMR_EP_ACLK_HBM_0	0	
#define VMR_EP_ACLK_SHUTDOWN_0	0	

/* Select between UCC and UCS IP */
#ifdef XPAR_SHELL_UTILS_UCC_0_BASEADDR
#define VMR_UCC_GAPPING_CTRL			0x0000
#define VMR_UCC_UCS_STATUS			0x1000
#define VMR_UCC_ACLK_FREQ_CTRL			0x2000
#define VMR_UCC_ACLK_FREQ_KERNEL_0_CTRL		0x8000
#define VMR_UCC_ACLK_FREQ_KERNEL_1_CTRL		0x9000

#define VMR_EP_GAPPING_DEMAND		(XPAR_SHELL_UTILS_UCC_0_BASEADDR + VMR_UCC_GAPPING_CTRL)
#define VMR_EP_UCS_CONTROL_STATUS_BASEADDR	(XPAR_SHELL_UTILS_UCC_0_BASEADDR + VMR_UCC_UCS_STATUS)
#define VMR_EP_ACLK_FREQ_0		(XPAR_SHELL_UTILS_UCC_0_BASEADDR + VMR_UCC_ACLK_FREQ_CTRL)
#define VMR_EP_ACLK_FREQ_KERNEL_0	(XPAR_SHELL_UTILS_UCC_0_BASEADDR + VMR_UCC_ACLK_FREQ_KERNEL_0_CTRL)
#define VMR_EP_ACLK_FREQ_KERNEL_1	(XPAR_SHELL_UTILS_UCC_0_BASEADDR + VMR_UCC_ACLK_FREQ_KERNEL_1_CTRL)
#else 
#define VMR_EP_GAPPING_DEMAND	\
	XPAR_BLP_BLP_LOGIC_ULP_CLOCKING_GAPPING_DEMAND_GPIO_GAPPING_DEMAND_BASEADDR
#define VMR_EP_UCS_CONTROL_STATUS_BASEADDR 	\
	XPAR_BLP_BLP_LOGIC_ULP_CLOCKING_UCS_CONTROL_STATUS_GPIO_UCS_CONTROL_STATUS_BASEADDR
#define VMR_EP_ACLK_FREQ_0		XPAR_SHELL_UTILS_FREQUENCY_COUNTER_0_BASEADDR
#define VMR_EP_ACLK_FREQ_KERNEL_0	XPAR_SHELL_UTILS_FREQUENCY_COUNTER_1_BASEADDR
#define VMR_EP_ACLK_FREQ_KERNEL_1	XPAR_SHELL_UTILS_FREQUENCY_COUNTER_2_BASEADDR
#endif

#define VMR_EP_ACLK_FREQ_HBM	0
#define VMR_EP_ACLK_FREQ_K1_K2	0

#define VMR_EP_PLM_MULTIBOOT	0xF1110004
#define VMR_EP_PMC_REG		0xF1130000

/* Note: eventually we should be driven by xparameter.h */
#define VMR_EP_RPU_SHARED_MEMORY_START	(0x38000000)
#define VMR_EP_RPU_SHARED_MEMORY_END	(0x3FFFF000)
#define VMR_EP_RPU_PRELOAD_FPT		(0x7FBF0000)

#define VMR_EP_APU_SHARED_MEMORY_START (0x37000000)
#define VMR_EP_APU_SHARED_MEMORY_END 	(0x37FF0000)

/*PLM Log Data Macros Start Address and Total Size Designated on Device*/
/* TODO: switch to plm API instead of accessing hardcoded address */
#define VMR_PLM_DATA_START_ADDRESS     (0xF2019000)
#define VMR_PLM_DATA_TOTAL_SIZE        (4096 * 4)

/* Select between GCQ and XGQ IP */
#ifdef XPAR_BLP_BLP_LOGIC_GCQ_M2R_S01_AXI_BASEADDR
#define VMR_EP_RPU_SQ_BASE (XPAR_BLP_BLP_LOGIC_GCQ_M2R_S01_AXI_BASEADDR + GCQ_SERVER_SQ_TAIL_POINTER)
#define VMR_EP_RPU_CQ_BASE (XPAR_BLP_BLP_LOGIC_GCQ_M2R_S01_AXI_BASEADDR + GCQ_SERVER_CQ_TAIL_POINTER)
#else
#define VMR_EP_RPU_SQ_BASE ((u32)XPAR_BLP_BLP_LOGIC_XGQ_M2R_BASEADDR + XGQ_SQ_TAIL_POINTER)
#define VMR_EP_RPU_CQ_BASE ((u32)XPAR_BLP_BLP_LOGIC_XGQ_M2R_BASEADDR + XGQ_CQ_TAIL_POINTER)
#endif

#ifdef XPAR_BLP_BLP_LOGIC_GCQ_R2A_S00_AXI_BASEADDR
#define VMR_EP_APU_SQ_BASE ((u32)XPAR_BLP_BLP_LOGIC_GCQ_R2A_S00_AXI_BASEADDR + GCQ_CLIENT_SQ_TAIL_POINTER)
#define VMR_EP_APU_CQ_BASE ((u32)XPAR_BLP_BLP_LOGIC_GCQ_R2A_S00_AXI_BASEADDR + GCQ_CLIENT_CQ_TAIL_POINTER)
#else
#define VMR_EP_APU_SQ_BASE ((u32)XPAR_BLP_BLP_LOGIC_XGQ_R2A_BASEADDR + XGQ_SQ_TAIL_POINTER)
#define VMR_EP_APU_CQ_BASE ((u32)XPAR_BLP_BLP_LOGIC_XGQ_R2A_BASEADDR + XGQ_CQ_TAIL_POINTER)
#endif

/* Reserve 4K for partition table */
#define VMR_PARTITION_TABLE_SIZE	0x1000
/* note: this should be set by APU, hardcode for now */
#define APU_RING_BUFFER_SIZE		0x1000 

/* xgq ring buffer is right after the partition table */
#define VMR_EP_RPU_RING_BUFFER_BASE (VMR_EP_RPU_SHARED_MEMORY_START + VMR_PARTITION_TABLE_SIZE)

#define RPU_SHARED_MEMORY_ADDR(offset) (VMR_EP_RPU_SHARED_MEMORY_START + (u32)offset)
#define APU_SHARED_MEMORY_ADDR(offset) (VMR_EP_APU_SHARED_MEMORY_START + (u32)offset)

#define SHUTDOWN_LATCHED_STATUS	0x01
#define MAX_GAPPING_DEMAND_RATE 128
#define CONVERT_TO_PERCENTAGE  100
#define MAX_CLOCK_SPEED_PERCENTAGE 100

#define BIT(n) 			(1UL << (n))
#define MIN(x, y) 		(((x) < (y)) ? (x) : (y))

struct vmr_endpoints {
	char *vmr_ep_name;
	u32  vmr_ep_address;
};

/*
 * Start platform dependent MACROs
 */

#if defined(CONFIG_FORCE_RESET)
#define VMR_EP_FORCE_RESET		XPAR_BLP_BLP_LOGIC_BASE_CLOCKING_FORCE_RESET_GPIO_BASEADDR
static inline int rmgmt_enable_pl_reset()
{
	int val = 0;
	u32 pmc_mux = VMR_EP_FORCE_RESET;

	val = IO_SYNC_READ32(pmc_mux);
	val |= PL_TO_PMC_ERROR_SIGNAL_PATH_MASK;
	IO_SYNC_WRITE32(val, pmc_mux); 
	
	return 0;
}
#else
static inline int rmgmt_enable_pl_reset() { return -ENODEV; }
#endif //endif of CONFIG_FORCE_RESET

#if defined(CONFIG_2022_1_VITIS)
/*
 * This is the workaround hardcode for 2022.1 xparameters.h only
#undef STDIN_BASEADDRESS
#define STDIN_BASEADDRESS 0xFF010000

#undef STDOUT_BASEADDRESS
#define STDOUT_BASEADDRESS 0xFF010000
 */

#undef XPAR_BLP_BLP_LOGIC_XGQ_R2A_BASEADDR
#define XPAR_BLP_BLP_LOGIC_XGQ_R2A_BASEADDR 0x80011000

#undef XPAR_BLP_BLP_LOGIC_XGQ_R2A_HIGHADDR
#define XPAR_BLP_BLP_LOGIC_XGQ_R2A_HIGHADDR 0x80011FFF

#undef XPAR_BLP_BLP_LOGIC_XGQ_M2R_BASEADDR
#define XPAR_BLP_BLP_LOGIC_XGQ_M2R_BASEADDR 0x80010000

#undef XPAR_BLP_BLP_LOGIC_XGQ_M2R_HIGHADDR
#define XPAR_BLP_BLP_LOGIC_XGQ_M2R_HIGHADDR 0x80010FFF
#endif //endif of CONFIG_2022_1_VITIS

#if defined(CONFIG_RAVE)
/*
 * This is a workaround for RAVE Build only.
 */
#undef VCCINT
#define VCCINT 1 
#endif // endif of CONFIG_RAVE

#endif //endif of VMR_COMMON_H
