/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#ifndef RMGMT_PM_H
#define RMGMT_PM_H

struct cl_msg;

int rmgmt_pm_init(void);
int rmgmt_pm_reset(struct cl_msg *msg);

#endif
