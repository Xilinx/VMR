/******************************************************************************
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#include <stdbool.h>
#include <stdint.h>

#include "rmgmt_common.h"
#include "rmgmt_fpt.h"
#include "rmgmt_xclbin.h"

#include "cl_io.h"
#include "cl_msg.h"
#include "cl_flash.h"

/*
 * Extension table is pre-loaded into DDR
 */
int rmgmt_fpt_get_sc(u32 *addr, u32 *size)
{
	struct fpt_hdr hdr;

	cl_memcpy_fromio8(RPU_PRELOAD_FPT, &hdr, sizeof(hdr));

	RMGMT_DBG("magic %x fpt_magic %x", hdr.fpt_magic, FPT_MAGIC);
	RMGMT_DBG("version %x", hdr.fpt_version);
	RMGMT_DBG("hdr size %x", hdr.fpt_header_size);
	RMGMT_DBG("entry size %x", hdr.fpt_entry_size);
	RMGMT_DBG("num entries %x", hdr.fpt_num_entries);

	for (int i = 1; i <= hdr.fpt_num_entries; i++) {
		struct fpt_entry entry;
		cl_memcpy_fromio8(RPU_PRELOAD_FPT + hdr.fpt_entry_size * i,
			&entry, sizeof(entry));

		if (entry.partition_type = FPT_TYPE_SC_FW) {
			*addr = entry.partition_base_addr;
			*size = entry.partition_size;
			return 0;
		}
	}

	return -1;
}

/*
 * Read fpt table and return status
 */
int rmgmt_boot_fpt_query(struct cl_msg *msg)
{
	struct fpt_hdr hdr = { 0 };
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

		if (entry.partition_type == FPT_TYPE_BOOT) {
			msg->multiboot_payload.default_partition_offset =
				entry.partition_base_addr;
			msg->multiboot_payload.default_partition_size =
				entry.partition_size;
		}

		if (entry.partition_type == FPT_TYPE_BOOT_BACKUP) {
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
	if (ret) {
		RMGMT_ERR("no backup fpt");
		return ret;
	}

	RMGMT_DBG("hdr recovery magic %x", hdr.fpt_magic);
	msg->multiboot_payload.has_fpt_recovery = (hdr.fpt_magic == FPT_MAGIC) ? 1 : 0;

	multi_boot_offset = IO_SYNC_READ32(EP_PLM_MULTIBOOT);
	msg->multiboot_payload.multi_boot_offset = multi_boot_offset;

	RMGMT_LOG("A offset %x:%x, B offset %x:%x",
		msg->multiboot_payload.default_partition_offset,
		MULTIBOOT_OFF(msg->multiboot_payload.default_partition_offset),
		msg->multiboot_payload.backup_partition_offset,
		MULTIBOOT_OFF(msg->multiboot_payload.backup_partition_offset));

	if (multi_boot_offset != 0 && multi_boot_offset ==
		MULTIBOOT_OFF(msg->multiboot_payload.default_partition_offset))
		msg->multiboot_payload.boot_on_default = 1;

	if (multi_boot_offset != 0 && multi_boot_offset ==
		MULTIBOOT_OFF(msg->multiboot_payload.backup_partition_offset))
		msg->multiboot_payload.boot_on_backup = 1;

	return ret;
}

static inline bool rmgmt_boot_has_fpt(struct cl_msg *msg)
{
	return msg->multiboot_payload.has_fpt;
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

/*
 * TODO: retrieve size info from metadata at the end of each PDI partition
 * if no size info, use initial size to copy over
 */
static int rmgmt_copy_default_to_backup(struct rmgmt_handler *rh, struct cl_msg *msg)
{
	/*TODO: read metadata to get size of A to copy, validate enough size in B */
	u32 len = (10 * 1024 * 1024); //bytes, hardcode to 10M for now.
	u32 src = msg->multiboot_payload.default_partition_offset;
	u32 tgt = msg->multiboot_payload.backup_partition_offset;
	int ret = 0;

	if (src == 0 || tgt == 0) {
		RMGMT_ERR("addresses cannot be 0: src 0x%x, tgt 0x%x", src, tgt);
		return -1;
	}
	if (rmgmt_boot_from_backup(msg)) {
		RMGMT_ERR("if booted from backup, the default image might be "
			"corrupted. Cannot copy default over to backup");
		return -1;
	}

	/* Note: alwasy erase first, otherwise data is corrupted after write */
	ret = ospi_flash_erase(CL_FLASH_BOOT, tgt, len);
	if (ret)
		return ret;

	ret = ospi_flash_copy(CL_FLASH_BOOT, src, tgt, len);
	/*TODO: update metadata at B */
	return ret;
}

/*
 * Flush flow:
 * 1) if flush_to_legacy is set, enfore to flush to offset 0x0;
 * 2) else if flush_no_backup is set, skip backup, otherwise backup A to B;
 * 3) flush pdi onto A and update size in metadata
 */
int rmgmt_flush_rpu_pdi(struct rmgmt_handler *rh, struct cl_msg *msg)
{
	int ret = 0;
	u32 offset = 0;
	u32 len = 0;
	u32 plm_boot_tag = 0;
	struct axlf *axlf = (struct axlf *)rh->rh_data;
	uint64_t pdi_offset = 0;
	uint64_t pdi_size = 0;
	u8 *pdi_data = NULL;

	/* Sync data from cache to memory */
	Xil_DCacheFlush();

	ret = rmgmt_xclbin_section_info(axlf, PDI, &pdi_offset, &pdi_size);
	if (ret) {
		RMGMT_ERR("get PDI from xsabin failed %d", ret);
		return ret;
	}
	pdi_data = (u8 *)axlf + pdi_offset;
	len = pdi_size;
	RMGMT_LOG("get PDI size %d from xsabin data %d", len, rh->rh_data_size);

	/* enforce to legacy layout, flush entire PDI onto offset 0x0 */
	if (msg->data_payload.flush_to_legacy) {

		RMGMT_ERR("WARN: force to flash back to legacy mode, PDI starts at 0x0");

		offset = 0x0;
		ret = ospi_flash_erase(CL_FLASH_BOOT, offset, len);
		if (ret)
			return ret;
		return ospi_flash_write(CL_FLASH_BOOT, pdi_data, offset, len);
	}

	/* Always query latest boot status first */
	ret = rmgmt_boot_fpt_query(msg);
	if (ret || !rmgmt_boot_has_fpt(msg)) {
		RMGMT_ERR("cannot read fpt table");
		return -1;
	}

	/*TODO: avoid 2nd request might wipe out B exactly as A, add checksum */
	if (!msg->data_payload.flush_no_backup) {
		ret = rmgmt_copy_default_to_backup(rh, msg);
		if (ret)
			return ret;
	}

	/*TODO: validae enough size in A for new PDI */
	offset = msg->multiboot_payload.default_partition_offset;
	if (offset == 0) {
		RMGMT_LOG("default partition offset cannot be 0");
		return -1;
	}
	RMGMT_LOG("flash to offset %x", offset);
	/* TODO: validte pdi, authentication pdi */
	/* Note: always erase and write, otherwise data is corrupted after write */
	ret = ospi_flash_erase(CL_FLASH_BOOT, offset, len);
	if (ret)
		return ret;

	/* preserve the XLNX and set to FFFFFFF */
	plm_boot_tag = PLM_BOOT_TAG(pdi_data);
	PLM_BOOT_TAG(pdi_data) = BOOT_TAG_MASK;
	ret = ospi_flash_write(CL_FLASH_BOOT, pdi_data, offset, len);
	if (ret)
		return ret;

	/* restore back to XLNX to enable boot */
	PLM_BOOT_TAG(pdi_data) = plm_boot_tag;
	ret = ospi_flash_write(CL_FLASH_BOOT, pdi_data, offset, OSPI_VERSAL_PAGESIZE);

	/*TODO: update metadata at A */
	return ret;
}
