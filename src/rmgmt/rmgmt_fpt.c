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

static inline bool rmgmt_flash_no_backup(struct cl_msg *msg)
{
	return msg->data_payload.flash_no_backup == 1;
}

static u32 rmgmt_fpt_pdi_get_base(struct cl_msg *msg, int fpt_type)
{
	switch (fpt_type) {
	case FPT_TYPE_PDIMETA:
		return msg->multiboot_payload.pdimeta_offset;
	case FPT_TYPE_PDIMETA_BACKUP:
		return msg->multiboot_payload.pdimeta_backup_offset;
	default:
		RMGMT_ERR("Warn: unhandled fpt_type 0x%x", fpt_type);
		break;
	}

	return 0;
}

static int rmgmt_fpt_pdi_meta_erase(struct cl_msg *msg, int fpt_type)
{
	u32 base_addr = rmgmt_fpt_pdi_get_base(msg, fpt_type);
	int ret = 0;

	if (base_addr == 0) {
		RMGMT_ERR("base addr of fpt_type 0x%x cannot be 0", fpt_type);
		return -1;
	}

	RMGMT_LOG("reseting 0x%x metdata", fpt_type); 

	ret = ospi_flash_erase(CL_FLASH_BOOT, base_addr, OSPI_VERSAL_PAGESIZE);

	return ret;
}

static int rmgmt_fpt_pdi_meta_get(struct cl_msg *msg, int fpt_type,
	struct fpt_pdi_meta *meta)
{
	u32 base_addr = rmgmt_fpt_pdi_get_base(msg, fpt_type);
	char buf[OSPI_VERSAL_PAGESIZE] = { 0 };
	int ret = 0;

	if (base_addr == 0) {
		RMGMT_ERR("base addr of fpt_type 0x%x cannot be 0", fpt_type);
		return -1;
	}

	RMGMT_LOG("fpt_type 0x%x", fpt_type);

	ret = ospi_flash_read(CL_FLASH_BOOT, (u8 *)buf, base_addr, sizeof(buf));
	if (ret)
		return ret;

	memcpy(meta, buf, sizeof(*meta));
	return 0;
}

static int rmgmt_fpt_pdi_meta_set(struct cl_msg *msg, int fpt_type,
	struct fpt_pdi_meta *meta)
{
	u32 base_addr = rmgmt_fpt_pdi_get_base(msg, fpt_type);
	char buf[OSPI_VERSAL_PAGESIZE] = { 0 };
	int ret = 0;

	if (base_addr == 0) {
		RMGMT_ERR("base addr of fpt_type 0x%x cannot be 0", fpt_type);
		return -1;
	}

	RMGMT_LOG("fpt_type 0x%x", fpt_type);

	memcpy(buf, meta, sizeof(*meta));

	ret = ospi_flash_write(CL_FLASH_BOOT, (u8 *)buf, base_addr, sizeof(buf));
	if (ret)
		return ret;

	return 0;
}

/*
 * Extension table is pre-loaded into DDR
 */
void rmgmt_extension_fpt_query(struct cl_msg *msg)
{
	struct fpt_hdr hdr = { 0 };

	cl_memcpy_fromio8(RPU_PRELOAD_FPT, &hdr, sizeof(hdr));

	msg->multiboot_payload.has_extfpt = (hdr.fpt_magic == FPT_MAGIC) ? 1 : 0;

	RMGMT_DBG("magic %x fpt_magic %x", hdr.fpt_magic, FPT_MAGIC);
	RMGMT_DBG("version %x", hdr.fpt_version);
	RMGMT_DBG("hdr size %x", hdr.fpt_header_size);
	RMGMT_DBG("entry size %x", hdr.fpt_entry_size);
	RMGMT_DBG("num entries %x", hdr.fpt_num_entries);

	if (!msg->multiboot_payload.has_extfpt)
		return;

	for (int i = 1; i <= hdr.fpt_num_entries; i++) {
		struct fpt_entry entry;
		cl_memcpy_fromio8(RPU_PRELOAD_FPT + hdr.fpt_entry_size * i,
			&entry, sizeof(entry));

		if (entry.partition_type == FPT_TYPE_SC_FW) {
			msg->multiboot_payload.scfw_offset = entry.partition_base_addr;
			msg->multiboot_payload.scfw_size = entry.partition_size;
			msg->multiboot_payload.has_ext_scfw = 1;
		}
		if (entry.partition_type == FPT_TYPE_XSABIN) {
			msg->multiboot_payload.xsabin_offset = entry.partition_base_addr;
			msg->multiboot_payload.xsabin_size = entry.partition_size;
			msg->multiboot_payload.has_ext_xsabin = 1;
		}
		if (entry.partition_type == FPT_TYPE_SYSDTB) {
			msg->multiboot_payload.sysdtb_offset = entry.partition_base_addr;
			msg->multiboot_payload.sysdtb_size = entry.partition_size;
			msg->multiboot_payload.has_ext_sysdtb = 1;

		}
		
		RMGMT_DBG("type %x", entry.partition_type);
		RMGMT_DBG("base %x", entry.partition_base_addr);
		RMGMT_DBG("size %x", entry.partition_size);
		RMGMT_DBG("flags %x", entry.partition_flags);
	}
}

int rmgmt_fpt_get_xsabin(struct cl_msg *msg, u32 *offset, u32 *size)
{
	rmgmt_extension_fpt_query(msg);

	if (!msg->multiboot_payload.has_ext_xsabin) {
		RMGMT_ERR("no xsabin metadata");
		return -1;
	}

	*offset = msg->multiboot_payload.xsabin_offset;
	*size = msg->multiboot_payload.xsabin_size;
	return 0;
}

int rmgmt_fpt_get_systemdtb(struct cl_msg *msg, u32 *offset, u32 *size)
{
	rmgmt_extension_fpt_query(msg);

	if (!msg->multiboot_payload.has_ext_sysdtb) {
		RMGMT_ERR("no system.dtb metadata");
		return -1;
	}

	*offset = msg->multiboot_payload.sysdtb_offset;
	*size = msg->multiboot_payload.sysdtb_size;
	return 0;
}

/*
 * Read fpt table and return status
 */
void rmgmt_boot_fpt_query(struct cl_msg *msg)
{
	struct fpt_hdr hdr = { 0 };
	struct fpt_pdi_meta meta = { 0 };
	int ret = 0;
	u32 multi_boot_offset = 0;
	
	ret = ospi_flash_read(CL_FLASH_BOOT, (u8 *)&hdr, FPT_DEFAULT_OFFSET, sizeof(hdr));
	if (ret)
		return;

	msg->multiboot_payload.has_fpt = (hdr.fpt_magic == FPT_MAGIC) ? 1 : 0;

	RMGMT_DBG("magic %x fpt_magic %x", hdr.fpt_magic, FPT_MAGIC);
	RMGMT_DBG("version %x", hdr.fpt_version);
	RMGMT_DBG("hdr size %x", hdr.fpt_header_size);
	RMGMT_DBG("entry size %x", hdr.fpt_entry_size);
	RMGMT_DBG("num entries %x", hdr.fpt_num_entries);

	if (!msg->multiboot_payload.has_fpt) {
		RMGMT_ERR("invalid fpt magic %x", hdr.fpt_magic);
		return;
	}

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

		if (entry.partition_type == FPT_TYPE_PDIMETA) {
			msg->multiboot_payload.pdimeta_offset =
				entry.partition_base_addr;
		}

		if (entry.partition_type == FPT_TYPE_PDIMETA_BACKUP) {
			msg->multiboot_payload.pdimeta_backup_offset =
				entry.partition_base_addr;
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
		return;
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

	if (!multi_boot_offset) {
		RMGMT_LOG("multi_boot_offset is 0");
		return;
	}

	if (multi_boot_offset ==
		MULTIBOOT_OFF(msg->multiboot_payload.default_partition_offset))
		msg->multiboot_payload.boot_on_default = 1;

	if (multi_boot_offset ==
		MULTIBOOT_OFF(msg->multiboot_payload.backup_partition_offset))
		msg->multiboot_payload.boot_on_backup = 1;

	ret = rmgmt_fpt_pdi_meta_get(msg, FPT_TYPE_PDIMETA, &meta);
	if (ret)
		return;
	if (meta.fpt_pdi_magic == FPT_PDIMETA_MAGIC) {
		msg->multiboot_payload.pdimeta_size = meta.fpt_pdi_size;
	}

	ret = rmgmt_fpt_pdi_meta_get(msg, FPT_TYPE_PDIMETA_BACKUP, &meta);
	if (ret)
		return;
	if (meta.fpt_pdi_magic == FPT_PDIMETA_MAGIC) {
		msg->multiboot_payload.pdimeta_backup_size = meta.fpt_pdi_size;
	}

	RMGMT_LOG("A size %x, B size %x", msg->multiboot_payload.pdimeta_size,
		msg->multiboot_payload.pdimeta_backup_size);
}

static int rmgmt_copy_default_to_backup(struct cl_msg *msg)
{
	u32 src = msg->multiboot_payload.default_partition_offset;
	u32 tgt = msg->multiboot_payload.backup_partition_offset;
	u32 tgt_capacity =  msg->multiboot_payload.backup_partition_size;
	struct fpt_pdi_meta meta = { 0 };
	u32 src_size = 0;
	int ret = 0;

	if (rmgmt_boot_from_backup(msg)) {
		RMGMT_ERR("if booted from backup, the default image might be "
			"corrupted. Cannot copy default over to backup");
		return -1;
	}

	if (src == 0 || tgt == 0) {
		RMGMT_ERR("addresses cannot be 0: src 0x%x, tgt 0x%x", src, tgt);
		return -1;
	}

	ret = rmgmt_fpt_pdi_meta_get(msg, FPT_TYPE_PDIMETA, &meta);
	if (ret)
		return ret;
	if (meta.fpt_pdi_magic != FPT_PDIMETA_MAGIC) {
		RMGMT_ERR("invalid default pdi meta magic: %x, size: %x",
			meta.fpt_pdi_magic, meta.fpt_pdi_size);
		return -1;
	}

	src_size = meta.fpt_pdi_size;
	if (src_size == 0) {
		RMGMT_ERR("src size cannot be 0");
		return -1;
	}

	if (src_size > tgt_capacity) {
		RMGMT_ERR("backup partition size %d is smaller than source size %d",
			tgt_capacity, src_size);
		return -1;
	}

	/* Due to a bug in ospi driver, we can get 0xffff after a write.
	 * So, we preserve the metadata before any write,
	 *     then write updated meta back.
	 */
	ret = rmgmt_fpt_pdi_meta_get(msg, FPT_TYPE_PDIMETA_BACKUP, &meta);
	if (ret)
		return ret;
	
	/* always reset backup pdi metadata if it is invalid */
	if (meta.fpt_pdi_magic != FPT_PDIMETA_MAGIC) {
		RMGMT_ERR("WARN: reset invalid meta magic: %x, size: %x",
			meta.fpt_pdi_magic, meta.fpt_pdi_size);
		meta.fpt_pdi_magic = FPT_PDIMETA_MAGIC;
	}

	/* Note: alwasy erase first, otherwise data is corrupted after write */
	ret = ospi_flash_erase(CL_FLASH_BOOT, tgt, src_size);
	if (ret)
		return ret;

	ret = ospi_flash_copy(CL_FLASH_BOOT, src, tgt, src_size);
	if (ret)
		return ret;

	/* finally update the tgt meta with updated size */
	meta.fpt_pdi_size = src_size;
	ret = rmgmt_fpt_pdi_meta_set(msg, FPT_TYPE_PDIMETA_BACKUP, &meta);
	if (ret)
		return ret;

	return 0;
}

void rmgmt_fpt_query(struct cl_msg *msg) {
	rmgmt_extension_fpt_query(msg);
	rmgmt_boot_fpt_query(msg);
}

/*
 * Flush flow:
 * 1) if flash_to_legacy is set, enfore to flash to offset 0x0;
 * 2) else if flash_no_backup is set, skip backup, otherwise backup A to B;
 * 3) flash pdi onto A and update size in metadata
 */
int rmgmt_flash_rpu_pdi(struct rmgmt_handler *rh, struct cl_msg *msg)
{
	int ret = 0;
	u32 offset = 0;
	u32 len = 0;
	u32 plm_boot_tag = 0;
	struct axlf *axlf = (struct axlf *)rh->rh_data;
	uint64_t pdi_offset = 0;
	uint64_t pdi_size = 0;
	u8 *pdi_data = NULL;
	struct fpt_pdi_meta meta = { 0 };

	/* Sync data from cache to memory */
	Xil_DCacheFlush();

	ret = rmgmt_xclbin_section_info(axlf, PDI, &pdi_offset, &pdi_size);
	if (ret) {
		RMGMT_ERR("get PDI from xsabin failed %d", ret);
		return ret;
	}
	pdi_data = (u8 *)axlf + pdi_offset;
	len = pdi_size;

	if (msg->data_payload.flash_to_legacy) {
		RMGMT_ERR("WARN: force to flash back to legacy mode, PDI starts at 0x0");

		offset = 0x0;
		ret = ospi_flash_erase(CL_FLASH_BOOT, offset, len);
		if (ret)
			return ret;
		return ospi_flash_write(CL_FLASH_BOOT, pdi_data, offset, len);
	}

	/* Always query latest fpt status first */
	rmgmt_fpt_query(msg);
	if (!rmgmt_boot_has_fpt(msg)) {
		RMGMT_ERR("cannot read fpt table");
		return -1;
	}

	/*TODO: avoid 2nd request might wipe out B exactly as A, add checksum */

	/* If enfore to skip copy or boot from B image, skip copy to B */
	if (!rmgmt_flash_no_backup(msg) && !rmgmt_boot_from_backup(msg)) {
		ret = rmgmt_copy_default_to_backup(msg);
		if (ret)
			return ret;
	}

	if (len > msg->multiboot_payload.default_partition_size) {
		RMGMT_ERR("default partition size %d is smaller than requested PDI size %d",
			msg->multiboot_payload.default_partition_size, len);
		return -1;
	}
	offset = msg->multiboot_payload.default_partition_offset;
	if (offset == 0) {
		RMGMT_ERR("default partition offset cannot be 0");
		return -1;
	}
	RMGMT_LOG("flash %d data to offset %x", len, offset);

	/* TODO: validte pdi, authentication pdi */

	/* Due to a bug in ospi driver, we can get 0xffff after a write.
	 * preserve metadata prior to any write */
	ret = rmgmt_fpt_pdi_meta_get(msg, FPT_TYPE_PDIMETA, &meta);
	if (ret)
		return ret;

	if (meta.fpt_pdi_magic != FPT_PDIMETA_MAGIC) {
		RMGMT_ERR("Invalid PDIMETA magic: %x", meta.fpt_pdi_magic);
		RMGMT_ERR("WARN: enforce to flash pdi onto default partition");

		/* Note: always erase before write and no erase after write */
		ret = rmgmt_fpt_pdi_meta_erase(msg, FPT_TYPE_PDIMETA);
		if (ret)
			return ret;

		/* reset to valide magic number */
		meta.fpt_pdi_magic = FPT_PDIMETA_MAGIC;
	}

	/* TODO: check not same checksum, avoid dup flash */

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
	if (ret) {
		return ret;
	}

	/* finaly step, update metadata */
	meta.fpt_pdi_size = len;
	ret = rmgmt_fpt_pdi_meta_set(msg, FPT_TYPE_PDIMETA, &meta);
	if (ret)
		return ret;

	return ret;
}
