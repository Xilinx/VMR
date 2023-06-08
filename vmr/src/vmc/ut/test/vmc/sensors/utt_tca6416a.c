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
#include <tca6416a.h>

/* I2C parameters */
#define I2CNUM          1
#define SLAVE_ADDRESS   0x20
#define WRITE_LEN       1
#define WRITE_LEN_2     2
#define WRITE_BUF       0x05
#define READ_LEN        1

extern void set_Buffer( unsigned char *buff, int min, int max, long int length );

static void test_ucTca6416aRegisterRead( void **state ) 
{
	( void ) state; /* unused */
    u8 ucStatus                     = 0;
	u8 usAddressOffset             = 0x06;
    u8 ucRegisterValue              = 0;
	unsigned char pucReadBuff[2]    = {0};
    u8 pucMyValues[1]               = {0x10};

    /* 1. Test */
    vSetReadBuffer( &pucReadBuff[0], pucMyValues, 1 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send_rs_recv, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send_rs_recv, write_length, WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], usAddressOffset  );
    expect_value( __wrap_i2c_send_rs_recv, read_length, READ_LEN );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 0 );

    ucStatus = ucTca6416aRegisterRead( I2CNUM, SLAVE_ADDRESS, usAddressOffset, &ucRegisterValue );
    assert_true( ucStatus == XST_SUCCESS );
    assert_true( ucRegisterValue == pucMyValues[0] );

}

static void test_ucTca6416aRegisterReadFail( void **state ) 
{
	( void ) state; /* unused */
    u8 ucStatus                     = 0;
	u8 usAddressOffset             = 0x15;
    u8 ucRegisterValue              = 0;
    unsigned char pucReadBuff[2]    = {0};
    u8 pucMyValues[2]               = {0x10, 0x1F};

    /* 1. Test NULL*/
    ucStatus = ucTca6416aRegisterRead( I2CNUM, SLAVE_ADDRESS, usAddressOffset, NULL );
    assert_true( ucStatus == XST_FAILURE );

    /* 2. Test I2C fail*/
    vSetReadBuffer( &pucReadBuff[0], pucMyValues, 1 );
    
    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send_rs_recv, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send_rs_recv, write_length, WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], ( u8 )( usAddressOffset & 0xFF ) );
    expect_value( __wrap_i2c_send_rs_recv, read_length, READ_LEN );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 1 );

    ucStatus = ucTca6416aRegisterRead( I2CNUM, SLAVE_ADDRESS, usAddressOffset, &ucRegisterValue );
    assert_true( ucStatus == XST_FAILURE );
}

static void test_ucTca6416aRegisterWrite( void **state ) 
{
	( void ) state; /* unused */
    u8 ucStatus                     = 0;
	u8 usAddressOffset              = 0x15;
	unsigned char pucReadBuff[2]    = {0};
    u8 MyValue                      = 0x40;

    /* 1. Test */  
    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, WRITE_LEN_2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], usAddressOffset );
    expect_value( __wrap_i2c_send, i2c_write_buff[1], MyValue );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    ucStatus = ucTca6416aRegisterWrite( I2CNUM, SLAVE_ADDRESS, usAddressOffset, MyValue );
    assert_true( ucStatus == XST_SUCCESS );

}

static void test_ucTca6416aRegisterWriteFail( void **state ) 
{
	( void ) state; /* unused */
    u8 ucStatus                     = 0;
	u8 usAddressOffset              = 0x15;
    u8 ucRegisterValue              = 0;
    unsigned char pucReadBuff[2]    = {0};
    u8 MyValue                      = 0x40;

    /* 1. Test I2C fail */
    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, WRITE_LEN_2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], usAddressOffset );
    expect_value( __wrap_i2c_send, i2c_write_buff[1], MyValue );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 1 );

    ucStatus = ucTca6416aRegisterWrite( I2CNUM, SLAVE_ADDRESS, usAddressOffset, MyValue );
    assert_true( ucStatus == XST_FAILURE );
}

static void test_ucEnableDDRDIMM( void **state ) 
{
	( void ) state; /* unused */
    u8 ucStatus                     = 0;
	unsigned char pucReadBuff[2]    = {0};
    u8 MyValue                      = 0x40;
    u8 pucMyValues[2]               = {0xFF, 0xFF};

    /* 1. Test */ 
    vSetReadBuffer( &pucReadBuff[0], pucMyValues, 1 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send_rs_recv, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send_rs_recv, write_length, WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], TCA6416AR_CONFIGURATION_0  );
    expect_value( __wrap_i2c_send_rs_recv, read_length, READ_LEN );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 0 );
 
    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, WRITE_LEN_2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], TCA6416AR_CONFIGURATION_0 );
    expect_value( __wrap_i2c_send, i2c_write_buff[1], 0xBF );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    vSetReadBuffer( &pucReadBuff[0], pucMyValues, 1 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send_rs_recv, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send_rs_recv, write_length, WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], TCA6416AR_OUTPUT_PORT_0  );
    expect_value( __wrap_i2c_send_rs_recv, read_length, READ_LEN );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, WRITE_LEN_2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], TCA6416AR_OUTPUT_PORT_0 );
    expect_value( __wrap_i2c_send, i2c_write_buff[1], MyValue );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    ucStatus = ucEnableDDRDIMM( );
    assert_true( ucStatus == XST_SUCCESS );

}

static void test_ucEnableDDRDIMMFail( void **state ) 
{
	( void ) state; /* unused */
    u8 ucStatus                     = 0;
	unsigned char pucReadBuff[2]    = {0};
    u8 MyValue                      = 0x40;
    u8 pucMyValues[2]               = {0x10, 0x1F};

    /* 1. Test */ 
    vSetReadBuffer( &pucReadBuff[0], pucMyValues, 1 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send_rs_recv, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send_rs_recv, write_length, WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], TCA6416AR_CONFIGURATION_0  );
    expect_value( __wrap_i2c_send_rs_recv, read_length, READ_LEN );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 0 );
 
    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, WRITE_LEN_2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], TCA6416AR_CONFIGURATION_0 );
    expect_value( __wrap_i2c_send, i2c_write_buff[1], 0x10 );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    vSetReadBuffer( &pucReadBuff[0], pucMyValues, 1 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send_rs_recv, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send_rs_recv, write_length, WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], TCA6416AR_OUTPUT_PORT_0  );
    expect_value( __wrap_i2c_send_rs_recv, read_length, READ_LEN );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, WRITE_LEN_2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], TCA6416AR_OUTPUT_PORT_0 );
    expect_value( __wrap_i2c_send, i2c_write_buff[1], 0x40 );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 1 );

    ucStatus = ucEnableDDRDIMM( );
    assert_true( ucStatus == XST_FAILURE );

    /* 2. Test */ 
    vSetReadBuffer( &pucReadBuff[0], pucMyValues, 1 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send_rs_recv, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send_rs_recv, write_length, WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], TCA6416AR_CONFIGURATION_0  );
    expect_value( __wrap_i2c_send_rs_recv, read_length, READ_LEN );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 0 );
 
    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, WRITE_LEN_2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], TCA6416AR_CONFIGURATION_0 );
    expect_value( __wrap_i2c_send, i2c_write_buff[1], 0x10 );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    vSetReadBuffer( &pucReadBuff[0], pucMyValues, 1 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send_rs_recv, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send_rs_recv, write_length, WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], TCA6416AR_OUTPUT_PORT_0  );
    expect_value( __wrap_i2c_send_rs_recv, read_length, READ_LEN );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 1 );

    ucStatus = ucEnableDDRDIMM( );
    assert_true( ucStatus == XST_FAILURE );

    /* 3. Test */ 
    vSetReadBuffer( &pucReadBuff[0], pucMyValues, 1 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send_rs_recv, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send_rs_recv, write_length, WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], TCA6416AR_CONFIGURATION_0  );
    expect_value( __wrap_i2c_send_rs_recv, read_length, READ_LEN );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 0 );
 
    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, WRITE_LEN_2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], TCA6416AR_CONFIGURATION_0 );
    expect_value( __wrap_i2c_send, i2c_write_buff[1], 0x10 );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 1 );

    ucStatus = ucEnableDDRDIMM( );
    assert_true( ucStatus == XST_FAILURE );

    /* 4. Test */ 
    vSetReadBuffer( &pucReadBuff[0], pucMyValues, 1 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send_rs_recv, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send_rs_recv, write_length, WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], TCA6416AR_CONFIGURATION_0  );
    expect_value( __wrap_i2c_send_rs_recv, read_length, READ_LEN );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 1 );

    ucStatus = ucEnableDDRDIMM( );
    assert_true( ucStatus == XST_FAILURE );
}

int main( void ) 
{
	srand( time( 0 ) );
	const struct CMUnitTest tests[] = 
    {
        cmocka_unit_test( test_ucTca6416aRegisterRead ),
        cmocka_unit_test( test_ucTca6416aRegisterReadFail ),
        cmocka_unit_test( test_ucTca6416aRegisterWrite ),
        cmocka_unit_test( test_ucTca6416aRegisterWriteFail ),
        cmocka_unit_test( test_ucEnableDDRDIMM ),
        cmocka_unit_test( test_ucEnableDDRDIMMFail )
	};

	return cmocka_run_group_tests( tests, NULL, NULL );

}