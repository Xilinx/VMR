/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#ifndef COMMON_VMC_H
#define COMMON_VMC_H

struct cl_msg;

int cl_vmc_sensor(struct cl_msg *msg);
int cl_vmc_is_ready(void);
void cl_vmc_sc_update(void);
void cl_vmc_monitor_sensors();

int cl_vmc_init(void);

/* V5K only APIs */
int cl_vmc_sc_is_ready(void);
int cl_vmc_scfw_init(void);
int cl_vmc_scfw_program(struct cl_msg *msg);
int cl_vmc_scfw_program_progress(void);

#endif
