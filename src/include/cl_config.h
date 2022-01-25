/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#ifndef COMMON_CONFIG_H
#define COMMON_CONFIG_H


/**
 * Uncomment for enabling CS2200 code.
 * Make sure to build the board with CS2200 XSA and program it with CS2200 PDI.
 */
//#define PLATFORM_CS2200


/* Uncomment for enabling VMC debug. */
//#define VMC_DEBUG

#ifdef VMC_DEBUG
#define VMR_BUILD_VMC_ONLY
#endif

#endif
