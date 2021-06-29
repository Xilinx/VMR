/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#ifndef RMGMT_OSPI_H
#define RMGMT_OSPI_H

#define OSPIPSV_DEVICE_ID		XPAR_XOSPIPSV_0_DEVICE_ID

/*
 *  * Flash address to which data is to be written.
 *   */
#define RPU_PDI_ADDRESS		0x0
#define APU_PDI_ADDRESS		(RPU_PDI_ADDRESS + 0x1000000) /* RPU + 16 M */
/* default flash media page size */
#define OSPI_VERSAL_PAGESIZE	256

#define MAGIC_NUM32 0x585254 /* XRT */

int ospi_flash_init();

int ospi_flash_read(u32 baseAddress, u8 *ReadBuffer, u32 len);
int ospi_flash_write(u32 baseAddress, u8 *WriteBuffer, u32 len);
int ospi_flash_erase(u32 baseAddress, u32 len);

#endif
