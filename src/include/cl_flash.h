/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#ifndef COMMON_FLASH_H
#define COMMON_FLASH_H

enum flash_service {
	CL_FLASH_RPU_PDI = 0,
	CL_FLASH_APU_PDI,
	CL_FLASH_SC_IMAGE,
} flash_service_t;

/**
 * @type:	flash service type
 * @buffer:	buffer data
 * @offset:	offset	
 * @len:	buffer length
 */
typedef struct flash_handle {
	flash_type_t type;
	u8 *buffer;
	u32 offset;
	u32 len;

} flash_handle_t;

int ospi_flash_read(flash_handle_t *hdl);
int ospi_flash_write(flash_handle_t *hdl);

#endif
