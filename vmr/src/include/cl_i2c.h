/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#ifndef INCLUDE_CL_I2C_H_
#define INCLUDE_CL_I2C_H_

#define IIC_SCLK_RATE		100000

#include "xil_types.h"

u8 I2CInit(void);
u8 i2c_send(u8 i2c_num, unsigned char i2c_addr, unsigned char * i2c_data, long int data_length);
u8 i2c_recv(u8 i2c_num, unsigned char i2c_addr, unsigned char * i2c_read_buff, long int i2c_CRS);
u8 i2c_send_rs_recv(u8 i2c_num,
		    unsigned char i2c_addr,
		    unsigned char * write_data,
		    long int write_length,
		    unsigned char * i2c_read_buff,
		    long int read_length);

#endif
