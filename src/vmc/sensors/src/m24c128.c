/******************************************************************************
 * Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
 * SPDX-License-Identifier: MIT
 *******************************************************************************/

#include "cl_uart_rtos.h"
#include "cl_log.h"
#include "cl_i2c.h"
#include "../inc/m24c128.h"

u8 M24C128_ReadByte(u8 i2c_num, u8 slaveAddr, u16 *AddressOffset,u8 *RegisterValue)
{
	u8 status = 0;
	status = i2c_send_rs_recv(i2c_num, slaveAddr, (u8 *)AddressOffset, 2, RegisterValue, 1);

	if (status == XST_FAILURE)
	{
		CL_LOG(APP_VMC, "Failed to read M24C128 \n\r");
		return status;
	}

	return status;

}
