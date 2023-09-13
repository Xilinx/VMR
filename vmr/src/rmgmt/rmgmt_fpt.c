/******************************************************************************
* Copyright (C) 2020-2022 Xilinx, Inc.  All rights reserved.
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
#include "cl_rmgmt.h"

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
		VMR_ERR("Warn: unhandled fpt_type 0x%x", fpt_type);
		break;
	}

	return 0;
}

static int rmgmt_fpt_pdi_meta_erase(struct cl_msg *msg, int fpt_type)
{
	u32 base_addr = rmgmt_fpt_pdi_get_base(msg, fpt_type);
	int ret = 0;

	if (base_addr == 0) {
		VMR_ERR("base addr of fpt_type 0x%x cannot be 0", fpt_type);
		return -1;
	}

	VMR_LOG("reseting 0x%x metdata", fpt_type); 

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
		VMR_ERR("base addr of fpt_type 0x%x cannot be 0", fpt_type);
		return -1;
	}

	VMR_DBG("fpt_type 0x%x", fpt_type);

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
		VMR_ERR("base addr of fpt_type 0x%x cannot be 0", fpt_type);
		return -1;
	}

	VMR_LOG("fpt_type 0x%x", fpt_type);

	memcpy(buf, meta, sizeof(*meta));

	ret = ospi_flash_safe_write(CL_FLASH_BOOT, (u8 *)buf, base_addr, sizeof(buf));
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

	cl_memcpy_fromio(VMR_EP_RPU_PRELOAD_FPT, &hdr, sizeof(hdr));

	msg->multiboot_payload.has_extfpt = (hdr.fpt_magic == FPT_MAGIC) ? 1 : 0;

	VMR_DBG("magic %x fpt_magic %x", hdr.fpt_magic, FPT_MAGIC);
	VMR_DBG("version %x", hdr.fpt_version);
	VMR_DBG("hdr size %x", hdr.fpt_header_size);
	VMR_DBG("entry size %x", hdr.fpt_entry_size);
	VMR_DBG("num entries %x", hdr.fpt_num_entries);

	if (!msg->multiboot_payload.has_extfpt)
		return;

	for (int i = 1; i <= hdr.fpt_num_entries; i++) {
		struct fpt_entry entry;
		cl_memcpy_fromio(VMR_EP_RPU_PRELOAD_FPT + hdr.fpt_entry_size * i,
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
		
		VMR_DBG("type %x", entry.partition_type);
		VMR_DBG("base %x", entry.partition_base_addr);
		VMR_DBG("size %x", entry.partition_size);
		VMR_DBG("flags %x", entry.partition_flags);
	}
}

int rmgmt_fpt_get_xsabin(struct cl_msg *msg, u32 *offset, u32 *size)
{
	rmgmt_extension_fpt_query(msg);

	if (!msg->multiboot_payload.has_ext_xsabin) {
		VMR_ERR("no xsabin metadata");
		return -EINVAL;
	}

	*offset = msg->multiboot_payload.xsabin_offset;
	*size = msg->multiboot_payload.xsabin_size;
	return 0;
}

int rmgmt_fpt_get_systemdtb(struct cl_msg *msg, u32 *offset, u32 *size)
{
	rmgmt_extension_fpt_query(msg);

	if (!msg->multiboot_payload.has_ext_sysdtb) {
		VMR_ERR("no system.dtb metadata");
		return -1;
	}

	*offset = msg->multiboot_payload.sysdtb_offset;
	*size = msg->multiboot_payload.sysdtb_size;
	return 0;
}

/*
 * Validate default FPT and query recovery FPT if default FPT fails.
 */
int rmgmt_boot_fpt_validate(struct cl_msg *msg, struct fpt_hdr *hdr)
{
	int ret = 0;
	ret = ospi_flash_read(CL_FLASH_BOOT, (u8 *)hdr, FPT_DEFAULT_OFFSET, sizeof(struct fpt_hdr));
	if (ret)
		return ret;
	if (hdr->fpt_magic != FPT_MAGIC) {
		VMR_ERR("Default FPT magic is Corrupt! reading backup FPT");
		bzero(hdr, sizeof(struct fpt_hdr));
		ret = ospi_flash_read(CL_FLASH_BOOT, (u8 *)hdr, FPT_BACKUP_OFFSET, sizeof(struct fpt_hdr));
		if (ret)
			return ret;
		if (hdr->fpt_magic != FPT_MAGIC) {
			msg->multiboot_payload.has_fpt = 0;
			VMR_ERR("Backup fpt magic invalid");
			return -EINVAL;
		}
	}

	msg->multiboot_payload.has_fpt = 1;
	VMR_DBG("magic %x fpt_magic %x", hdr->fpt_magic, FPT_MAGIC);
        VMR_DBG("version %x", hdr->fpt_version);
        VMR_DBG("hdr size %x", hdr->fpt_header_size);
        VMR_DBG("entry size %x", hdr->fpt_entry_size);
        VMR_DBG("num entries %x", hdr->fpt_num_entries);
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
	u32 current_multi_boot_offset = 0;
	u32 boot_on_offset = 0;

	ret = rmgmt_boot_fpt_validate(msg, &hdr);
	if (ret)
		return;

	for (int i = 1; i <= hdr.fpt_num_entries; i++) {
		struct fpt_entry entry;
		ret = ospi_flash_read(CL_FLASH_BOOT, (u8 *)&entry,
			hdr.fpt_entry_size * i, sizeof(entry));
		if (ret) {
			VMR_ERR("read fpt entry %d failed", i);
			return;
		}

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

		VMR_DBG("type %x", entry.partition_type);
		VMR_DBG("base %x", entry.partition_base_addr);
		VMR_DBG("size %x", entry.partition_size);
		VMR_DBG("flags %x", entry.partition_flags);
	}

	bzero(&hdr, sizeof(hdr));
	ret = ospi_flash_read(CL_FLASH_BOOT, (u8 *)&hdr, FPT_BACKUP_OFFSET, sizeof(hdr));
	if (ret) {
		VMR_ERR("no backup fpt");
		return;
	}

	VMR_DBG("hdr recovery magic 0x%x", hdr.fpt_magic);
	msg->multiboot_payload.has_fpt_recovery = (hdr.fpt_magic == FPT_MAGIC) ? 1 : 0;

	/* This is the current multi_boot_offset, can be changed after reboot */
	current_multi_boot_offset = IO_SYNC_READ32(VMR_EP_PLM_MULTIBOOT);
	if (current_multi_boot_offset == 0)
		VMR_WARN("WARN: current multi_boot_offset is 0x0");

	msg->multiboot_payload.current_multi_boot_offset = current_multi_boot_offset;

	VMR_LOG("A offset 0x%x:0x%x, B offset 0x%x:0x%x",
		msg->multiboot_payload.default_partition_offset,
		MULTIBOOT_OFF(msg->multiboot_payload.default_partition_offset),
		msg->multiboot_payload.backup_partition_offset,
		MULTIBOOT_OFF(msg->multiboot_payload.backup_partition_offset));


	/* We should use the cached boot_offset saved right after boot */
	boot_on_offset = rmgmt_boot_on_offset();
	msg->multiboot_payload.boot_on_offset = boot_on_offset;

	if (boot_on_offset ==
		MULTIBOOT_OFF(msg->multiboot_payload.default_partition_offset))
		msg->multiboot_payload.boot_on_default = 1;

	if (boot_on_offset ==
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

	VMR_LOG("A size 0x%x, B size 0x%x", msg->multiboot_payload.pdimeta_size,
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
		VMR_ERR("if booted from backup, the default image might be "
			"corrupted. Cannot copy default over to backup");
		return -1;
	}

	if (src == 0 || tgt == 0) {
		VMR_ERR("addresses cannot be 0: src 0x%x, tgt 0x%x", src, tgt);
		return -1;
	}

	ret = rmgmt_fpt_pdi_meta_get(msg, FPT_TYPE_PDIMETA, &meta);
	if (ret)
		return ret;
	if (meta.fpt_pdi_magic != FPT_PDIMETA_MAGIC) {
		VMR_ERR("invalid default pdi meta magic: 0x%x, size: 0x%x",
			meta.fpt_pdi_magic, meta.fpt_pdi_size);
		return -1;
	}

	src_size = meta.fpt_pdi_size;
	if (src_size == 0) {
		VMR_ERR("src size cannot be 0");
		return -1;
	}

	if (src_size > tgt_capacity) {
		VMR_ERR("backup partition size %d is smaller than source size %d",
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
		VMR_ERR("WARN: reset invalid meta magic: 0x%x, size: 0x%x",
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
	meta.fpt_pdi_debug_type = CL_DBG_CLEAR;
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
	struct axlf *axlf = (struct axlf *)rh->rh_data_base;
	uint64_t pdi_offset = 0;
	uint64_t pdi_size = 0;
	u8 *pdi_data = NULL;
	struct fpt_pdi_meta meta = { 0 };

	if (rh->rh_already_flashed) {
		VMR_ERR("PDI has been successfully flashed, cannot reflash before reboot");
		return -EINVAL;
	}

	/* Sync data from cache to memory */
	Xil_DCacheFlush();

	ret = rmgmt_xclbin_section_info(axlf, PDI, &pdi_offset, &pdi_size);
	if (ret) {
		VMR_ERR("get PDI from xsabin failed %d", ret);
		return ret;
	}
	pdi_data = (u8 *)axlf + pdi_offset;
	len = (u32)pdi_size;

	if (msg->data_payload.flash_to_legacy) {
		VMR_WARN("WARN: force to flash back to legacy mode, PDI starts at 0x0");

		offset = 0x0;
		ret = ospi_flash_erase(CL_FLASH_BOOT, offset, len);
		if (ret)
			return ret;
		ret = ospi_flash_write(CL_FLASH_BOOT, pdi_data, offset, len);
		goto done;
	}

	/* Always query latest fpt status first */
	rmgmt_fpt_query(msg);
	if (!rmgmt_boot_has_fpt(msg)) {
		VMR_ERR("cannot read fpt table");
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
		VMR_ERR("default partition size %d is smaller than requested PDI size %d",
			msg->multiboot_payload.default_partition_size, len);
		return -1;
	}
	offset = msg->multiboot_payload.default_partition_offset;
	if (offset == 0) {
		VMR_ERR("default partition offset cannot be 0");
		return -1;
	}
	VMR_LOG("flash %d data to offset 0x%x", len, offset);

	/* TODO: validte pdi, authentication pdi */

	/* Due to a bug in ospi driver, we can get 0xffff after a write.
	 * preserve metadata prior to any write */
	ret = rmgmt_fpt_pdi_meta_get(msg, FPT_TYPE_PDIMETA, &meta);
	if (ret)
		return ret;

	if (meta.fpt_pdi_magic != FPT_PDIMETA_MAGIC) {
		VMR_ERR("Invalid PDIMETA magic: 0x%x", meta.fpt_pdi_magic);
		VMR_ERR("WARN: enforce to flash pdi onto default partition");

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
	if (ret)
		return ret;

	/* finaly step, update metadata */
	meta.fpt_pdi_size = len;
	meta.fpt_pdi_debug_type = CL_DBG_CLEAR;
	ret = rmgmt_fpt_pdi_meta_set(msg, FPT_TYPE_PDIMETA, &meta);
	if (ret)
		return ret;
done:
	if (!ret) {
		VMR_WARN("set already_flashed to ture, need to reset/reboot to take effect.");
		rh->rh_already_flashed = true;
	}
	return ret;
}

/*
 * Debug only API.
 * Only set debug_type onto default pdi meta.
 */
int rmgmt_fpt_set_debug_type(struct cl_msg *msg)
{
	int ret = 0;
	struct fpt_pdi_meta meta = { 0 };

	/* Retrieve latest meta status */
	rmgmt_boot_fpt_query(msg);

	ret = rmgmt_fpt_pdi_meta_get(msg, FPT_TYPE_PDIMETA, &meta);
	if (ret)
		return ret;

	VMR_LOG("get debug_type %d", meta.fpt_pdi_debug_type);
	meta.fpt_pdi_debug_type = msg->multiboot_payload.vmr_debug_type;

	ret = rmgmt_fpt_pdi_meta_set(msg, FPT_TYPE_PDIMETA, &meta);
	if (ret) {
		VMR_ERR("failed: %d", ret);
		return ret;
	}

	bzero(&meta, sizeof(meta));
	ret = rmgmt_fpt_pdi_meta_get(msg, FPT_TYPE_PDIMETA, &meta);
	VMR_WARN("set debug_type %d", meta.fpt_pdi_debug_type);

	return ret;
}

int rmgmt_fpt_get_debug_type(struct cl_msg *msg, u8 *debug_type)
{
	int ret = 0;
	struct fpt_pdi_meta meta = { 0 };

	/* Retrieve latest meta status */
	rmgmt_boot_fpt_query(msg);

	ret = rmgmt_fpt_pdi_meta_get(msg, FPT_TYPE_PDIMETA, &meta);
	if (ret)
		return ret;

	*debug_type = meta.fpt_pdi_debug_type;

	VMR_LOG("debug_type %d", meta.fpt_pdi_debug_type);
	return ret;
}
