/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#ifndef RMGMT_XFER_H
#define RMGMT_XFER_H

/* Versal transfer cache packet definition */

#define XRT_XFR_VER                     1

#define XRT_XFR_RES_SIZE				0x1000 //4096 bytes
/*
 * Note: we should keep the existing PKT status and flags
 *       stable to not breaking the old versal platform
 */
#define XRT_XFR_PKT_STATUS_IDLE         0
#define XRT_XFR_PKT_STATUS_NEW          1
#define XRT_XFR_PKT_STATUS_DONE         2
#define XRT_XFR_PKT_STATUS_FAIL         3

#define XRT_XFR_PKT_TYPE_SHIFT          1
#define XRT_XFR_PKT_TYPE_MASK           7

#define XRT_XFR_PKT_VER_SHIFT           4
#define XRT_XFR_PKT_VER_MASK            3

#define XRT_XFR_PKT_TYPE_PDI            0
#define XRT_XFR_PKT_TYPE_XCLBIN         1

#define XRT_XFR_PKT_FLAGS_LAST          (1 << 0)
#define XRT_XFR_PKT_FLAGS_PDI           (XRT_XFR_PKT_TYPE_PDI << \
        XRT_XFR_PKT_TYPE_SHIFT)
#define XRT_XFR_PKT_FLAGS_XCLBIN        (XRT_XFR_PKT_TYPE_XCLBIN << \
        XRT_XFR_PKT_TYPE_SHIFT)
#define XRT_XFR_PKT_FLAGS_VER           (XRT_XFR_VER << XRT_XFR_PKT_VER_SHIFT)

struct pdi_packet {
	union {
		struct {
			u8	pkt_status;
			u8	pkt_flags;
			u16	pkt_size;
		};
		u32 header;
	};
};



struct zocl_ov_pkt_node {
	size_t	zn_size;
	u32		*zn_datap;
	struct zocl_ov_pkt_node *zn_next;
};

struct zocl_ov_dev {
	u32 base;
	size_t size;
	struct zocl_ov_pkt_node *head;
};

#define OSPI_VERSAL_BASE 		(XPAR_PSV_OCM_RAM_0_S_AXI_BASEADDR + 0x8000)

#define TEST_ADDRESS		0x0

/* For Versal platform Passing the below definition is Optional */
#define BITSTREAM_SIZE	0x1000000U /* Bin or bit or PDI image size */
#define versal
#ifdef versal
#define PDI_LOAD        0U
#endif

u8 rmgmt_get_pkt_flags(struct zocl_ov_dev *ov);
int rmgmt_check_for_status(struct zocl_ov_dev *ov, u8 status);
int rmgmt_init_xfer(struct zocl_ov_dev *ov);
int rmgmt_get_xsabin(struct zocl_ov_dev *ov);
int rmgmt_get_xclbin(struct zocl_ov_dev *ov);

#endif
