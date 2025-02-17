/******************************************************************************
* Copyright (C) 2024 Advanced Micro Devices, Inc.    All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#ifndef RMGMT_PM_H
#define RMGMT_PM_H

struct cl_msg;

int rmgmt_pm_init(void);
int rmgmt_pm_reset_rpu(struct cl_msg *msg);
int rmgmt_eemi_pm_reset(struct cl_msg *msg);

#endif
