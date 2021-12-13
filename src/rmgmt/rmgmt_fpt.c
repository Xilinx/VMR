/******************************************************************************
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#include <stdbool.h>
#include <stdint.h>

#include "rmgmt_common.h"
#include "rmgmt_fpt.h"

#include "cl_io.h"
#include "cl_msg.h"
#include "cl_flash.h"

#define MULTIBOOT_OFF(x) (x / 1024 / 32 ) // divided by 32k
#define BOOT_TAG_OFFSET 0x14
#define BOOT_TAG_MASK	0xFFFFFFFF

/*
 * Read fpt table and return status
 */
int rmgmt_boot_fpt_query(struct rmgmt_handler *rh, struct cl_msg *msg)
{
	struct fpt_hdr hdr;
	int ret = 0;
	u32 multi_boot_offset = 0;

	ret = ospi_flash_read(CL_FLASH_BOOT, (u8 *)&hdr, FPT_DEFAULT_OFFSET, sizeof(hdr));
	if (ret)
		return ret;

	msg->multiboot_payload.has_fpt = (hdr.fpt_magic == FPT_MAGIC) ? 1 : 0;

	RMGMT_DBG("magic %x fpt_magic %x", hdr.fpt_magic, FPT_MAGIC);
	RMGMT_DBG("version %x", hdr.fpt_version);
	RMGMT_DBG("hdr size %x", hdr.fpt_header_size);
	RMGMT_DBG("entry size %x", hdr.fpt_entry_size);
	RMGMT_DBG("num entries %x", hdr.fpt_num_entries);

	for (int i = 1; i <= hdr.fpt_num_entries; i++) {
		struct fpt_entry entry;
		ret = ospi_flash_read(CL_FLASH_BOOT, (u8 *)&entry,
			hdr.fpt_entry_size * i, sizeof(entry));

		if (entry.partition_type == FPT_DEFAULT_TYPE) {
			msg->multiboot_payload.default_partition_offset =
				entry.partition_base_addr;
			msg->multiboot_payload.default_partition_size =
				entry.partition_size;
		}

		if (entry.partition_type == FPT_BACKUP_TYPE) {
			msg->multiboot_payload.backup_partition_offset =
				entry.partition_base_addr;
			msg->multiboot_payload.backup_partition_size =
				entry.partition_size;
		}

		RMGMT_DBG("type %x", entry.partition_type);
		RMGMT_DBG("base %x", entry.partition_base_addr);
		RMGMT_DBG("size %x", entry.partition_size);
		RMGMT_DBG("flags %x", entry.partition_flags);
	}

	bzero(&hdr, sizeof(hdr));
	ret = ospi_flash_read(CL_FLASH_BOOT, (u8 *)&hdr, FPT_BACKUP_OFFSET, sizeof(hdr));
	if (ret)
		return ret;

	msg->multiboot_payload.has_fpt_recovery = (hdr.fpt_magic == FPT_MAGIC) ? 1 : 0;

	multi_boot_offset = IO_SYNC_READ32(EP_PLM_MULTIBOOT);
	msg->multiboot_payload.multi_boot_offset = multi_boot_offset;

	RMGMT_LOG("A offset %x:%x, B offset %x:%x",
		msg->multiboot_payload.default_partition_offset,
		MULTIBOOT_OFF(msg->multiboot_payload.default_partition_offset),
		msg->multiboot_payload.backup_partition_offset,
		MULTIBOOT_OFF(msg->multiboot_payload.backup_partition_offset));

	if (MULTIBOOT_OFF(msg->multiboot_payload.default_partition_offset) ==
		multi_boot_offset)
		msg->multiboot_payload.boot_on_default = 1;

	if (MULTIBOOT_OFF(msg->multiboot_payload.backup_partition_offset) ==
		multi_boot_offset)
		msg->multiboot_payload.boot_on_backup = 1;

	return ret;
}

static inline bool rmgmt_boot_from_default(struct cl_msg *msg)
{
	return msg->multiboot_payload.boot_on_default;
}

static inline bool rmgmt_boot_from_backup(struct cl_msg *msg)
{
	return msg->multiboot_payload.boot_on_backup;
}

static inline bool rmgmt_boot_from_recovery(struct cl_msg *msg)
{
	return msg->multiboot_payload.boot_on_recovery;
}

static int rmgmt_copy_default_to_backup(struct rmgmt_handler *rh, struct cl_msg *msg)
{
	u32 len = (10 * 1024 * 1024); //bytes, hardcode to 10M for now.
	u32 src = msg->multiboot_payload.default_partition_offset;
	u32 tgt = msg->multiboot_payload.backup_partition_offset;
	int ret = 0;

	if (rmgmt_boot_from_backup(msg)) {
		RMGMT_LOG("if booted from backup, the default image might be"
			"corrupted. Cannot copy default over to backup");
		return -1;
	}

	/* Note: alwasy erase first, otherwise data is corrupted after write */
	ret = ospi_flash_erase(CL_FLASH_BOOT, tgt, len);
	if (ret)
		return ret;

	return ospi_flash_copy(CL_FLASH_BOOT, src, tgt, len);
}

int rmgmt_flush_rpu_pdi(struct rmgmt_handler *rh, struct cl_msg *msg,
	bool flush_default_only)
{
	int ret = 0;
	u32 offset = 0;
	u32 len = 0;
	u32 plm_boot_tag = 0;

	/* Always query latest boot status first */
	ret = rmgmt_boot_fpt_query(rh, msg);

	/* Sync data from cache to memory */
	Xil_DCacheFlush();

	if (!flush_default_only) {
		ret = rmgmt_copy_default_to_backup(rh, msg);
		if (ret)
			return ret;
	}

	len = rh->rh_data_size;
	offset = msg->multiboot_payload.default_partition_offset;
	RMGMT_LOG("flash to offset %x", offset);
	/* TODO: validte pdi, authentication pdi */
	/* Note: always erase and write, otherwise data is corrupted after write */
	ret = ospi_flash_erase(CL_FLASH_BOOT, offset, len);
	if (ret)
		return ret;

#define PLM_BOOT_TAG(data) (*(u32 *)(data + BOOT_TAG_OFFSET))

	/* preserve the XLNX and set to FFFFFFF */
	plm_boot_tag = PLM_BOOT_TAG(rh->rh_data);
	PLM_BOOT_TAG(rh->rh_data) = BOOT_TAG_MASK;
	ret = ospi_flash_write(CL_FLASH_BOOT, rh->rh_data, offset, len);
	if (ret)
		return ret;

	/* restore back to XLNX to enable boot */
	PLM_BOOT_TAG(rh->rh_data) = plm_boot_tag;
	ret = ospi_flash_write(CL_FLASH_BOOT, rh->rh_data, offset, OSPI_VERSAL_PAGESIZE);

	return ret;
}
