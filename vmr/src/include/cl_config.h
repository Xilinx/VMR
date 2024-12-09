/******************************************************************************
* Copyright (C) 2024 Advanced Micro Devices, Inc.    All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#ifndef COMMON_CONFIG_H
#define COMMON_CONFIG_H


/* Uncomment for enabling VMC debug. */
//#define VMC_DEBUG

#ifdef VMC_DEBUG
#define VMR_BUILD_VMC_ONLY
#endif

/* Uncomment this for build VMR with RMI submodule. */
//#define BUILD_FOR_RMI

#endif
