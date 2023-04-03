/******************************************************************************
 * * Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
 * * SPDX-License-Identifier: MIT
 * ****************************************************************************/

#ifndef COMMON_RMI_H_
#define COMMON_RMI_H_

#include "cl_config.h"

#ifdef BUILD_FOR_RMI

#include "RMI/rmi_sensors.h"

int cl_rmi_init(void);
s8 rmi_init_all(void);

/*
 * When task func started, all init works should be done already.
 */
void cl_rmi_func(void *task_args);
void cl_rmi_exit(void);
u8 cl_rmi_is_ready(void);

#endif

#endif
