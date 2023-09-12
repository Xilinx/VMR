/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#ifndef RMGMT_FPT_H
#define RMGMT_FPT_H

#define FPT_MAGIC 		0x3A0BC563
#define FPT_PDIMETA_MAGIC	0x4D494450 /* PDIM */
#define FPT_DEFAULT_OFFSET	0x0
#define FPT_BACKUP_OFFSET	0x20000
#define FPT_TYPE_EXTENSION	0xFFFD
#define FPT_TYPE_RECOVERY	0xFFFE
#define FPT_TYPE_SC_FW		0x0C00
#define FPT_TYPE_BOOT		0x0E00
#define FPT_TYPE_BOOT_BACKUP	0x0E01
#define FPT_TYPE_XSABIN		0x0E02
#define FPT_TYPE_GOLDEN		0x0E03
#define FPT_TYPE_SYSDTB		0x0E04
#define FPT_TYPE_PDIMETA	0x0E05
#define FPT_TYPE_PDIMETA_BACKUP	0x0E06

#define MULTIBOOT_OFF(x) (x / 1024 / 32 ) // divided by 32k
#define BOOT_TAG_OFFSET 0x14
#define BOOT_TAG_MASK	0xFFFFFFFF

#define PLM_BOOT_TAG(data) (*(u32 *)(data + BOOT_TAG_OFFSET))

struct rmgmt_handler;
struct cl_msg;

struct fpt_hdr {
	uint32_t	fpt_magic;
	uint8_t		fpt_version;
	uint8_t		fpt_header_size;
	uint8_t		fpt_entry_size;
	uint8_t		fpt_num_entries;
	uint32_t	fpt_checksum;
};

struct fpt_entry {
	uint32_t	partition_type;
	uint32_t	partition_sub_type;
	uint32_t	partition_device_id;
	uint32_t	partition_base_addr;
	uint32_t	partition_size;
	uint32_t	partition_flags;
	uint8_t		rsvd[1];
};

struct fpt_pdi_meta {
	uint32_t	fpt_pdi_magic;
	uint32_t	fpt_pdi_version;
	uint32_t	fpt_pdi_size;
	uint32_t	fpt_pdi_checksum;
	uint32_t	fpt_pdi_debug_type:8;
	uint32_t	fpt_pdi_rsvd0:24;
};

void rmgmt_fpt_query(struct cl_msg *msg);
void rmgmt_boot_fpt_query(struct cl_msg *msg);
void rmgmt_extension_fpt_query(struct cl_msg *msg);

int rmgmt_flash_rpu_pdi(struct rmgmt_handler *rh, struct cl_msg *msg);

int rmgmt_fpt_get_xsabin(struct cl_msg *msg, u32 *addr, u32 *size);
int rmgmt_fpt_get_scfw(struct cl_msg *msg, u32 *addr, u32 *size);
int rmgmt_fpt_get_systemdtb(struct cl_msg *msg, u32 *addr, u32 *size);

/* Debug APIs */
int rmgmt_fpt_set_debug_type(struct cl_msg *msg);
int rmgmt_fpt_get_debug_type(struct cl_msg *msg, u8 *debug_type);

#endif
