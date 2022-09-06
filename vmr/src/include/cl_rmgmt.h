/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#ifndef COMMON_RMGMT_H
#define COMMON_RMGMT_H

struct cl_msg;

int cl_rmgmt_fpt_get_debug_type(struct cl_msg *msg, u8 *debug_type);
int cl_rmgmt_enable_boot_default(struct cl_msg *msg);
int cl_rmgmt_enable_boot_backup(struct cl_msg *msg);
int cl_rmgmt_fpt_query(struct cl_msg *msg);
int cl_rmgmt_fpt_debug(struct cl_msg *msg);
u32 cl_rmgmt_boot_on_offset();

int cl_rmgmt_log_page(struct cl_msg *msg);
int cl_rmgmt_clock(struct cl_msg *msg);
int cl_rmgmt_vmr_control(struct cl_msg *msg);

int cl_rmgmt_program_base_pdi(struct cl_msg *msg);
int cl_rmgmt_program_apu_pdi(struct cl_msg *msg);
int cl_rmgmt_program_xclbin(struct cl_msg *msg);

int cl_rmgmt_apu_channel_probe(void);
int cl_rmgmt_apu_identify(struct xgq_cmd_resp_identify *id_cmd);
int cl_rmgmt_apu_info(char *buf, u32 size);
int cl_rmgmt_apu_download_xclbin(char *data, u32 size);

int cl_rmgmt_init(void);
int cl_rmgmt_is_ready(void);

int cl_rmgmt_apu_is_ready(void);
int cl_rmgmt_pl_is_ready(void);

#endif
