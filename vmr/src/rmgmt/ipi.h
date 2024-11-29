/******************************************************************************
* Copyright (C) 2024 AMD, Inc.    All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/
#ifndef LIBESWPM_PM_IPI_H_
#define LIBESWPM_PM_IPI_H_

#include <xstatus.h>
#include <xscugic.h>
#include <xipipsu.h>

typedef void (*IpiCallback)(XIpiPsu *const InstancePtr);

XStatus IpiInit(XScuGic *const GicInst, XIpiPsu *const InstancePtr);
XStatus IpiRegisterCallback(XIpiPsu *const IpiInst, const u32 SrcMask,
			    IpiCallback Callback);

#endif


