/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#ifndef COMMON_RMGMT_H
#define COMMON_RMGMT_H

struct cl_msg;

struct rmgmt_handler {
	u32 rh_base;   /* obsolated */
	u32 rh_boot_on_offset;
	u32 rh_data_max_size;
	u32 rh_data_size;
	void *rh_data_priv;
	u32 rh_log_max_size; 
	char *rh_log;	/* static malloc and never free */
	u8   *rh_data; 	/* static malloc and never free */
	int  rh_already_flashed; /* enforce reset/reboot after successfully flashed */
};

/* APIs for cl_main and cl_vmc tasks */
int cl_rmgmt_fpt_query(struct cl_msg *msg);

/* APIs for cl_main and cl_xgq tasks */
int cl_rmgmt_init(void);
int cl_rmgmt_program_apu_pdi(struct cl_msg *msg);
int cl_rmgmt_program_base_pdi(struct cl_msg *msg);
int cl_rmgmt_program_xclbin(struct cl_msg *msg);

int cl_rmgmt_apu_channel_probe(void);
int cl_rmgmt_apu_is_ready(void);
int cl_rmgmt_pl_is_ready(void);
int cl_vmc_scfw_is_ready(void);

int cl_rmgmt_log_page(struct cl_msg *msg);
int cl_rmgmt_clock(struct cl_msg *msg);
int cl_rmgmt_vmr_control(struct cl_msg *msg);

#endif
