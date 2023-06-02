#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <cmocka.h>
#include "xstatus.h"
#include "cl_i2c.h"
#include "cl_log.h"


/* FreeRTOS includes */
#include <FreeRTOS.h>
#include <task.h>
#include <m24c128.h>

/* I2C parameters */
#define I2CNUM          1
#define SLAVE_ADDRESS   0x18
#define WRITE_LEN       2
#define WRITE_BUF       0x05
#define READ_LEN        1

extern void set_Buffer( unsigned char *buff, int min, int max, long int length );

static void test_M24C128_ReadByte( void **state ) 
{
	( void ) state; /* unused */
    u8 ucStatus                     = 0;
	u16 usAddressOffset             = 0x1523;
    u8 ucRegisterValue              = 0;
	unsigned char pucReadBuff[2]    = {0};
    u8 pucMyValues[2]               = {0x10, 0x1F};

    /* 1. Test */
    /* This function will generate random values between the range.
    * The Min and Max temperature are 0 and 85 */
    set_Buffer( &pucReadBuff[0], 0, 1360, 2 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send_rs_recv, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send_rs_recv, write_length, WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], ( u8 )( usAddressOffset & 0xFF ) );
    expect_value( __wrap_i2c_send_rs_recv, read_length, READ_LEN );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 0 );

    ucStatus = M24C128_ReadByte( I2CNUM, SLAVE_ADDRESS, &usAddressOffset, &ucRegisterValue );
    assert_true( ucStatus == XST_SUCCESS );

}

static void test_M24C128_ReadByteFail( void **state ) 
{
	( void ) state; /* unused */
    u8 ucStatus                     = 0;
	u16 usAddressOffset             = 0x1523;;
    u8 ucRegisterValue              = 0;
    unsigned char pucReadBuff[2]    = {0};
    u8 pucMyValues[2]               = {0x10, 0x1F};

    /* 1. Test NULL*/
    ucStatus = M24C128_ReadByte( I2CNUM, SLAVE_ADDRESS, NULL, &ucRegisterValue );
    assert_true( ucStatus == XST_FAILURE );
    
    /* 2. Test NULL*/
    ucStatus = M24C128_ReadByte( I2CNUM, SLAVE_ADDRESS, &usAddressOffset, NULL );
    assert_true( ucStatus == XST_FAILURE );

    /* 3. Test I2C fail*/
    /* This function will generate random values between the range.
    * The Min and Max temperature are 0 and 85 */
    set_Buffer( &pucReadBuff[0], 0, 1360, 2 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send_rs_recv, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send_rs_recv, write_length, WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], ( u8 )( usAddressOffset & 0xFF ) );
    expect_value( __wrap_i2c_send_rs_recv, read_length, READ_LEN );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 1 );

    ucStatus = M24C128_ReadByte( I2CNUM, SLAVE_ADDRESS, &usAddressOffset, &ucRegisterValue );
    assert_true( ucStatus == XST_FAILURE );
}

static void test_M24C128_ReadMultiBytes( void **state ) 
{
	( void ) state; /* unused */
    u8 ucStatus                     = 0;
	u16 usAddressOffset             = 0x1523;
    size_t xBufSize                 = 20;
	unsigned char pucReadBuff[2]    = {0};
    u8 pucMyValues[20]              = {0};

    /* 1. Test */
    /* This function will generate random values between the range.
    * The Min and Max temperature are 0 and 85 */
    set_Buffer( &pucReadBuff[0], 0, 1360, 2 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send_rs_recv, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send_rs_recv, write_length, WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], ( u8 )( usAddressOffset & 0xFF ) );
    expect_value( __wrap_i2c_send_rs_recv, read_length, xBufSize );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 0 );

    ucStatus = M24C128_ReadMultiBytes( I2CNUM, SLAVE_ADDRESS, &usAddressOffset, pucMyValues, xBufSize );
    assert_true( ucStatus == XST_SUCCESS );

}

static void test_M24C128_ReadMultiBytesFail( void **state ) 
{
	( void ) state; /* unused */
    u8 ucStatus                     = 0;
	u16 usAddressOffset             = 0x1523;;
    u8 ucRegisterValue              = 0;
    size_t xBufSize                 = 20; 
    unsigned char pucReadBuff[2]    = {0};
    u8 pucMyValues[20]              = {0};

    /* 1. Test NULL*/
    ucStatus = M24C128_ReadMultiBytes( I2CNUM, SLAVE_ADDRESS, NULL, pucMyValues, xBufSize );
    assert_true( ucStatus == XST_FAILURE );
    
    /* 2. Test NULL*/
    ucStatus = M24C128_ReadMultiBytes( I2CNUM, SLAVE_ADDRESS, &usAddressOffset, NULL, xBufSize );
    assert_true( ucStatus == XST_FAILURE );

    /* 3. Test I2C fail*/
    /* This function will generate random values between the range.
    * The Min and Max temperature are 0 and 85 */
    set_Buffer( &pucReadBuff[0], 0, 1360, 2 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send_rs_recv, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send_rs_recv, write_length, WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], ( u8 )( usAddressOffset & 0xFF ) );
    expect_value( __wrap_i2c_send_rs_recv, read_length, xBufSize );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 1 );

    ucStatus = M24C128_ReadMultiBytes( I2CNUM, SLAVE_ADDRESS, &usAddressOffset, pucMyValues, xBufSize );
    assert_true( ucStatus == XST_FAILURE );
}


int main( void ) 
{
	srand( time( 0 ) );
	const struct CMUnitTest tests[] = 
    {
        cmocka_unit_test( test_M24C128_ReadByte ),
        cmocka_unit_test( test_M24C128_ReadByteFail ),
        cmocka_unit_test( test_M24C128_ReadMultiBytes ),
        cmocka_unit_test( test_M24C128_ReadMultiBytesFail )

	};

	return cmocka_run_group_tests( tests, NULL, NULL );

}