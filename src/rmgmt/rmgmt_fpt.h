/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#ifndef RMGMT_FPT_H
#define RMGMT_FPT_H

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

int rmgmt_boot_fpt_query(struct rmgmt_handler *rh, struct cl_msg *msg);

int rmgmt_flush_rpu_pdi(struct rmgmt_handler *rh, struct cl_msg *msg,
	bool flush_default_only);

#endif
