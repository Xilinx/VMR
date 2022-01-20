/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#ifndef COMMON_MSG_H
#define COMMON_MSG_H

typedef enum cl_msg_type {
	CL_MSG_UNKNOWN = 0,
	CL_MSG_PDI,
	CL_MSG_XCLBIN,
	CL_MSG_AF,
	CL_MSG_CLOCK,
	CL_MSG_SENSOR,
	CL_MSG_APUBIN,
	CL_MSG_VMR_CONTROL,
} cl_msg_type_t;

typedef enum cl_sensor_type {
	CL_SENSOR_ALL 		= 0x0,
	CL_SENSOR_BDINFO 	= 0xC0,
	CL_SENSOR_TEMP 		= 0xC1,
	CL_SENSOR_VOLTAGE	= 0xC2,
	CL_SENSOR_POWER 	= 0xC3,
	CL_SENSOR_QSFP 		= 0xC4,
} cl_sensor_type_t;

typedef enum cl_clock_type {
	CL_CLOCK_UNKNOWN 	= 0x0,
	CL_CLOCK_WIZARD	 	= 0x1,
	CL_CLOCK_COUNTER	= 0x2,
	CL_CLOCK_SCALE		= 0x3,
} cl_clock_type_t;

typedef enum cl_vmr_control_type {
	CL_VMR_QUERY		= 0x0,
	CL_MULTIBOOT_DEFAULT	= 0x1,
	CL_MULTIBOOT_BACKUP	= 0x2,
	CL_PROGRAM_SC		= 0x3,
} cl_vmr_control_type_t;

struct xgq_vmr_data_payload {
	uint32_t address;
	uint32_t size;
	uint32_t addr_type:4;
	uint32_t flush_no_backup:1;
	uint32_t flush_to_legacy:1;
	uint32_t rsvd1:26;
	uint32_t pad1;
};

struct xgq_vmr_log_payload {
	uint32_t address;
	uint32_t size;
	uint32_t pid:16;
	uint32_t addr_type:3;
	uint32_t rsvd1:13;
};

struct xgq_vmr_clock_payload {
	uint32_t ocl_region;
	uint32_t ocl_req_type;
	uint32_t ocl_req_id;
	uint32_t ocl_req_num;
	uint32_t ocl_req_freq[4];
};

struct xgq_vmr_multiboot_payload {
	uint32_t req_type;
	uint16_t has_fpt:1;
	uint16_t has_fpt_recovery:1;
	uint16_t boot_on_default:1;
	uint16_t boot_on_backup:1;
	uint16_t boot_on_recovery:1;
	uint16_t has_extfpt:1;
	uint16_t has_ext_xsabin:1;
	uint16_t has_ext_scfw:1;
	uint16_t has_ext_sysdtb:1;
	uint16_t rsvd:7;
	uint16_t multi_boot_offset;
	uint32_t default_partition_offset;
	uint32_t default_partition_size;
	uint32_t backup_partition_offset;
	uint32_t backup_partition_size;
	uint32_t scfw_offset;
	uint32_t scfw_size;
	uint32_t xsabin_offset;
	uint32_t xsabin_size;
	uint32_t sysdtb_offset;
	uint32_t sysdtb_size;
	uint32_t pdimeta_offset;
	uint32_t pdimeta_size;
	uint32_t pdimeta_backup_offset;
	uint32_t pdimeta_backup_size;
};

struct xgq_vmr_head {
	u16 version;
	u16 type;
	u16 cid;
	u16 rcode;
};

/*TODO: rename request payload and result payload */
typedef struct cl_msg {
	struct xgq_vmr_head hdr;
	union {
		struct xgq_vmr_data_payload data_payload;
		struct xgq_vmr_log_payload log_payload;
		struct xgq_vmr_clock_payload clock_payload;
	};
	union {
		struct xgq_vmr_multiboot_payload multiboot_payload;
	};
} cl_msg_t;

typedef int (*process_msg_cb)(cl_msg_t *msg, void *arg);

typedef struct msg_handle {
	cl_msg_type_t 	type;
	process_msg_cb 	msg_cb;
	void 		*arg;
	const char	*name;
} msg_handle_t;

int cl_msg_handle_init(msg_handle_t **hdl, cl_msg_type_t type,
	process_msg_cb cb, void *arg);
int cl_msg_handle_complete(cl_msg_t *msg);
void cl_msg_handle_fini(msg_handle_t *hdl);

#endif
