/******************************************************************************
 * * Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
 * * SPDX-License-Identifier: MIT
 * ****************************************************************************/

#include "cl_config.h"

#ifdef BUILD_FOR_RMI

int cl_rmi_init(void);

/*
 * When task func started, all init works should be done already.
 */
void cl_rmi_func(void *task_args);

#endif