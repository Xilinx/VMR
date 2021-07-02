/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#ifndef COMMON_FLASH_H
#define COMMON_FLASH_H

typedef enum flash_area {
	CL_FLASH_BOOT = 0,
	CL_FLASH_APU,
	CL_FLASH_SC_IMAGE,
} flash_area_t;

int ospi_flash_read(flash_area_t area, u8 *buffer, u32 offset, u32 len);
int ospi_flash_write(flash_area_t area, u8 *buffer, u32 offset, u32 len);

/* default flash media page size */
#define OSPI_VERSAL_PAGESIZE	256
#define MAGIC_NUM32 0x585254 /* XRT */

#endif
