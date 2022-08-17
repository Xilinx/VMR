/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#ifndef COMMON_VMC_H
#define COMMON_VMC_H

#include "xil_types.h"

struct cl_msg;
struct fpt_sc_version;

typedef struct __attribute__((packed)) {
	u8 shutdown_limit_temp;
	u16 shutdown_limit_pwr;
	u8 throttle_limit_temp;
	u16 throttle_limit_pwr;
}temp_power_limits_t;

typedef struct  __attribute__((packed)) {
	u8 is_clk_scaling_supported;
	u8 clk_scaling_mode;
	u8 clk_scaling_enable;
	temp_power_limits_t limits;

	/* New override values recvd, push into Algo */
	u8 temp_throttling_enabled;
	u8 power_throttling_enabled;
	u8 limits_update_req;
}clk_throttling_params_t;


int cl_vmc_sensor(struct cl_msg *msg);
int cl_vmc_is_ready(void);
void cl_vmc_sc_tx_rx_data(void);
void cl_vmc_monitor_sensors();
void cl_vmc_pdi_scfw_version(struct fpt_sc_version *version);

int cl_vmc_init(void);

/* V5K only APIs */
int cl_vmc_sc_is_ready(void);
int cl_vmc_scfw_init(void);
int cl_vmc_scfw_program(struct cl_msg *msg);
int cl_vmc_scfw_program_progress(void);

int cl_vmc_sysmon_init(void);
int cl_vmc_sysmon_is_ready(void);

int cl_vmc_clk_scaling(struct cl_msg *msg);
void cl_vmc_get_clk_throttling_params(clk_throttling_params_t *params);
#endif
