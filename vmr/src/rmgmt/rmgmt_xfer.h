/******************************************************************************
* Copyright (C) 2024 Advanced Micro Devices, Inc.    All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#ifndef RMGMT_XFER_H
#define RMGMT_XFER_H

/* Versal Platform related definition */
#define TEST_ADDRESS	0x0

#define BITSTREAM_SIZE	0x8000000U /* Bin or bit or PDI image max size (128M) */
#define LOG_MSG_SIZE	0x1000 /* 4096 bytes */

#define versal
#ifdef versal
#define PDI_LOAD        0U
#endif

/* Versal transfer cache packet definition */

#define XRT_XFR_VER                     1

#define XRT_XFR_RES_SIZE		0x1000 //4096 bytes
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
#define XRT_XFR_PKT_TYPE_APU_PDI        2

#define XRT_XFR_PKT_FLAGS_LAST          (1 << 0)
#define XRT_XFR_PKT_FLAGS_PDI           (XRT_XFR_PKT_TYPE_PDI << \
        XRT_XFR_PKT_TYPE_SHIFT)
#define XRT_XFR_PKT_FLAGS_XCLBIN        (XRT_XFR_PKT_TYPE_XCLBIN << \
        XRT_XFR_PKT_TYPE_SHIFT)
#define XRT_XFR_PKT_FLAGS_APU_PDI        (XRT_XFR_PKT_TYPE_APU_PDI << \
        XRT_XFR_PKT_TYPE_SHIFT)
#define XRT_XFR_PKT_FLAGS_VER           (XRT_XFR_VER << XRT_XFR_PKT_VER_SHIFT)

#define FDT_BEGIN_NODE  0x00000001
#define FDT_PROP        0x00000003
#define FDT_END         0x00000009
#define ALIGN(x, a)     (((x) + ((a) - 1)) & ~((a) - 1))
#define PALIGN(p, a)    ((char *)(ALIGN((int)(p), (int)(a))))
#define GET_CELL(p)     (p += 4, *((u32 *)(p-4)))
#define UUID_BYTES_LEN	(33) // last char is '\0'

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

struct fdt_header {
       uint32_t magic;
       uint32_t totalsize;
       uint32_t off_dt_struct;
       uint32_t off_dt_strings;
       uint32_t off_mem_rsvmap;
       uint32_t version;
       uint32_t last_comp_version;
       uint32_t boot_cpuid_phys;
       uint32_t size_dt_strings;
       uint32_t size_dt_struct;
};

int rmgmt_init_handler(struct rmgmt_handler *rh);

int rmgmt_download_apu_pdi(struct rmgmt_handler *rh);
int rmgmt_download_rpu_pdi(struct rmgmt_handler *rh);
int rmgmt_download_xclbin(struct rmgmt_handler *rh);
int rmgmt_load_apu(struct rmgmt_handler *rh);

/* TODO: remove obsolated APIs when test is stable */
int rmgmt_check_for_status(struct rmgmt_handler *rh, u8 status);
u8 rmgmt_get_pkt_flags(struct rmgmt_handler *rh);

int rmgmt_fdt_get_uuids(u32 fdt_addr, char *int_uuid, u32 uuid_size);
#endif
