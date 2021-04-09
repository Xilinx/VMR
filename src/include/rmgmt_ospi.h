/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#ifndef RMGMT_OSPI_H
#define RMGMT_OSPI_H

#define OSPIPSV_DEVICE_ID		XPAR_XOSPIPSV_0_DEVICE_ID

int ospi_flush_polled(u8 *WriteBuffer, u32 len);
int ospi_flush_polled_non_block(u8 *WriteBuffer, u32 len);
int ospi_flush_intr(u8 *WriteBuffer, u32 len);

#endif
