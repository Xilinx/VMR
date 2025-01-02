/******************************************************************************
* Copyright (C) 2024 Advanced Micro Devices, Inc.    All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/


#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <stdlib.h>
#include <cmocka.h>
#include "cl_i2c.h"

unsigned char *rd_buffer = NULL;

/*****************************Internal functions*******************************/
void set_Buffer(unsigned char *buff, int min, int max, long int length)
{
	unsigned int value  = 0;
	int i               = 0;

	rd_buffer = buff;

	/*Generate random values between the range and copy it to buffer*/

	value = (rand() % (max - min + 1)) + min;
	for(i = 0; i < length; i++)
	{
		rd_buffer[i] = (unsigned char)(value >> ((length - i -1) * 8));
	}
}

void vSetReadBuffer(unsigned char *ucBuff, unsigned char *pucValue, long int ulLength)
{
	int i = 0;

	rd_buffer = ucBuff;

	/* copy it to buffer*/
	for(i = 0; i < ulLength; i++)
	{
		rd_buffer[i] = (unsigned char)(*(pucValue++));
	}
}


/*****************************Mock functions *******************************/
u8  __wrap_i2c_send_rs_recv(u8 i2c_num, unsigned char i2c_addr, unsigned char * i2c_write_buff, long int write_length, unsigned char * i2c_read_buff, long int read_length)
{
	int i = 0;

	/*Check parameter values set by 'expect_*()' written in testcase*/
	check_expected(i2c_num);
	check_expected(i2c_addr);
	check_expected(write_length);
	check_expected(i2c_write_buff[0]);
	check_expected(read_length);

	/*Copy the buffer values set by 'set_buffer()' function*/
	for(i = 0; i < read_length; i++)
	{
		if(rd_buffer != NULL)
		{
			i2c_read_buff[i] = *(rd_buffer++);
		}
	}

	/*Returns the value passed to 'will_return()' written in testcase */
	return mock_type(u8);
}

u8  __wrap_i2c_send(u8 i2c_num, unsigned char i2c_addr, unsigned char * i2c_write_buff, long int write_length)
{
	int i = 0;

	/*Check parameter values set by 'expect_*()' written in testcase*/
	check_expected(i2c_num);
	check_expected(i2c_addr);
	check_expected(write_length);
	check_expected(i2c_write_buff[0]);
    if( 2 == write_length ) check_expected(i2c_write_buff[1]);

	/*Returns the value passed to 'will_return()' written in testcase */
	return mock_type(u8);
}
