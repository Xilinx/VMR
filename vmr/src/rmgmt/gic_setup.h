/******************************************************************************
* Copyright (C) 2020-2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/
#ifndef LIBESWPM_GIC_SETUP_H_
#define LIBESWPM_GIC_SETUP_H_

#include <xscugic.h>

s32 GicSetupInterruptSystem(XScuGic *GicInst);
s32 GicResume(XScuGic *GicInst);
void GicSuspend(XScuGic *const GicInst);

#endif

