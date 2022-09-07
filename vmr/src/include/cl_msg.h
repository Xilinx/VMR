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
	CL_MSG_LOG_PAGE,
	CL_MSG_CLOCK,
	CL_MSG_SENSOR,
	CL_MSG_APUBIN,
	CL_MSG_VMR_CONTROL,
	CL_MSG_PROGRAM_SCFW,
	CL_MSG_CLK_THROTTLING,
} cl_msg_type_t;

typedef enum cl_sensor_type {
	CL_SENSOR_GET_SIZE	= 0x0,
	CL_SENSOR_BDINFO 	= 0xC0,
	CL_SENSOR_TEMP 		= 0xC1,
	CL_SENSOR_VOLTAGE	= 0xC2,
	CL_SENSOR_CURRENT	= 0xC3,
	CL_SENSOR_POWER 	= 0xC4,
	CL_SENSOR_QSFP 		= 0xC5,
	CL_SENSOR_ALL 		= 0xFF,
} cl_sensor_type_t;

typedef enum cl_log_type {
	CL_LOG_UNKNOWN		= 0x0,
	CL_LOG_AF_CHECK		= 0x1,
	CL_LOG_FW		= 0x2,
	CL_LOG_INFO		= 0x3,
	CL_LOG_AF_CLEAR		= 0x4,
	CL_LOG_ENDPOINT		= 0x5,
	CL_LOG_TASK_STATS	= 0x6,
	CL_LOG_MEM_STATS	= 0x7,
	CL_LOG_SYSTEM_DTB	= 0x8,
} cl_log_type_t;

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
	CL_VMR_DEBUG		= 0x4,
} cl_vmr_control_type_t;

typedef enum cl_vmr_debug_type {
	CL_DBG_CLEAR		= 0x0,
	CL_DBG_DISABLE_RMGMT	= 0x1,
	CL_DBG_DISABLE_VMC	= 0x2,
} cl_vmr_debug_type_t;


typedef enum cl_clk_scaling_type {
	CL_CLK_SCALING_READ	= 0x1,
	CL_CLK_SCALING_SET	= 0x2,
} cl_clk_scaling_type_t;

struct xgq_vmr_data_payload {
	uint32_t address;
	uint32_t size;
	uint32_t addr_type:4;
	uint32_t flash_no_backup:1;
	uint32_t flash_to_legacy:1;
	uint32_t rsvd1:26;
	uint32_t pad1;
};

struct xgq_vmr_log_payload {
	uint32_t address;
	uint32_t size;
	uint32_t offset;
	uint32_t pid:16;
	uint32_t addr_type:3;
	uint32_t rsvd1:13;
};

struct fpt_sc_version {
	uint8_t	fsv_major;
	uint8_t	fsv_minor;
	uint8_t	fsv_revision;
	uint8_t	fsv_rsvd;
};

struct xgq_vmr_sensor_payload {
	uint32_t address;
	uint32_t size;
	uint32_t offset;
	uint32_t aid:8;
	uint32_t sid:8;
	uint32_t addr_type:3;
	uint32_t sensor_id:8;
	uint32_t rsvd1:5;
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
	uint16_t current_multi_boot_offset;
	uint16_t boot_on_offset;
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
	uint8_t vmr_debug_type;
};

struct xgq_vmr_clk_scaling_payload {
	uint32_t aid:3;
	uint32_t scaling_en:1;
	uint32_t pwr_scaling_ovrd_limit:16;
	uint32_t temp_scaling_ovrd_limit:8;
	uint32_t rsvd1:4;
};

struct xgq_vmr_head {
	u16 version;
	u16 type;
	u16 cid;
	u16 pad;
	u32 rcode;
};

struct xgq_vmr_cmd_identify{
	uint16_t major;
	uint16_t minor;
};

/*TODO: rename request payload and result payload */
typedef struct cl_msg {
	struct xgq_vmr_head hdr;
	union {
		struct xgq_vmr_data_payload data_payload;
		struct xgq_vmr_log_payload log_payload;
		struct xgq_vmr_clock_payload clock_payload;
		struct xgq_vmr_sensor_payload sensor_payload;
		struct xgq_vmr_clk_scaling_payload clk_scaling_payload;
	};
	union {
		struct xgq_vmr_multiboot_payload multiboot_payload;
	};
} cl_msg_t;

void cl_msg_handle_complete(cl_msg_t *msg);

static inline void cl_msg_set_rcode(cl_msg_t *msg, int rcode)
{
	msg->hdr.rcode = (u32)rcode;
}

static inline u32 cl_msg_get_rcode(cl_msg_t *msg)
{
	return msg->hdr.rcode;
}


#endif
