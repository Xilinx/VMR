/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#ifndef COMMON_IO_H
#define COMMON_IO_H

int cl_memcpy_toio8(u32 dst, void *buf, size_t len);

void IO_SYNC_WRITE32(u32 val, u32 addr); 

u32 IO_SYNC_READ32(u32 addr);

#endif