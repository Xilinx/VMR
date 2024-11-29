/******************************************************************************
* Copyright (C) 2024 AMD, Inc.    All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/
#ifndef _PM_INIT_H_
#define _PM_INIT_H_

XStatus PmInit(XScuGic *const GicInst, XIpiPsu *const IpiInst, u32 FullBoot);

#endif
