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
int cl_rmgmt_program_sc(struct cl_msg *msg);
int cl_rmgmt_fpt_debug(struct cl_msg *msg);

#endif
