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
#include <max6639.h>

/* I2C parameters */
#define I2CNUM          1
#define SLAVE_ADDRESS   0x18
#define WRITE_LEN       1      
#define WRITE_BUF       0x05
#define READ_LEN        1

extern void set_Buffer( unsigned char *buff, int min, int max, long int length );

static void test_max6639_write_registerFail( void **state ) 
{
    ( void ) state; /* unused */
    u8 ucStatus                     = 0;
    u8 ucRegisterAddress            = 0;
    u8 ucRegisterContent            = 0;
    unsigned char pucReadBuff[2]    = {0};
    u8 pucMyValues[2]               = {0x10, 0x1F};

    /* 1. Test NULL*/
    ucStatus = max6639_write_register( I2CNUM, SLAVE_ADDRESS, ucRegisterAddress, NULL );
    assert_true( ucStatus == XST_FAILURE );
    
    /* 2. Test I2C fail*/
    /* This function will generate random values between the range.
    * The Min and Max temperature are 0 and 85 */
    set_Buffer( &pucReadBuff[0], 0, 1360, 2 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], ucRegisterContent );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 1 );

    ucStatus = max6639_write_register( I2CNUM, SLAVE_ADDRESS, ucRegisterAddress, &ucRegisterContent );
    assert_true( ucStatus == XST_FAILURE );
}

static void test_max6639_write_register( void **state ) 
{
    ( void ) state; /* unused */
    u8 ucStatus                     = 0;
    u8 ucRegisterAddress            = 0;
    u8 ucRegisterContent            = 0;
    unsigned char pucReadBuff[2]    = {0};
    u8 pucMyValues[2]               = {0x10, 0x1F};

    /* 1. Test */
    /* This function will generate random values between the range.
    * The Min and Max temperature are 0 and 85 */
    set_Buffer( &pucReadBuff[0], 0, 1360, 2 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], ucRegisterContent );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    ucStatus = max6639_write_register( I2CNUM, SLAVE_ADDRESS, ucRegisterAddress, &ucRegisterContent );
    assert_true( ucStatus == XST_SUCCESS );

}

static void test_max6639_read_register( void **state ) 
{
    ( void ) state; /* unused */
    u8 ucStatus                     = 0;
    u8 ucRegisterAddress            = 0;
    u8 ucRegisterContent            = 0;
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
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], ucRegisterContent );
    expect_value( __wrap_i2c_send_rs_recv, read_length, READ_LEN );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 0 );

    ucStatus = max6639_read_register( I2CNUM, SLAVE_ADDRESS, ucRegisterAddress, &ucRegisterContent );
    assert_true( ucStatus == XST_SUCCESS );

}

static void test_max6639_read_registerFail( void **state ) 
{
    ( void ) state; /* unused */
    u8 ucStatus                     = 0;
    u8 ucRegisterAddress            = 0;
    u8 ucRegisterContent            = 0;
    unsigned char pucReadBuff[2]    = {0};
    u8 pucMyValues[2]               = {0x10, 0x1F};

    /* 1. Test NULL*/
    ucStatus = max6639_read_register( I2CNUM, SLAVE_ADDRESS, ucRegisterAddress, NULL );
    assert_true( ucStatus == XST_FAILURE );
    
    /* 2. Test I2C fail*/
    /* This function will generate random values between the range.
    * The Min and Max temperature are 0 and 85 */
    set_Buffer( &pucReadBuff[0], 0, 1360, 2 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send_rs_recv, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send_rs_recv, write_length, WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], ucRegisterContent );
    expect_value( __wrap_i2c_send_rs_recv, read_length, READ_LEN );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 1 );

    ucStatus = max6639_read_register( I2CNUM, SLAVE_ADDRESS, ucRegisterAddress, &ucRegisterContent );
    assert_true( ucStatus == XST_FAILURE );
}
/***************************************************************/
static void test_max6639_initFail( void **state ) 
{
    ( void ) state; /* unused */
    u8 ucStatus                     = 0;
    u8 ucRegisterAddress            = 0;
    u8 ucRegisterContent            = 0;
    unsigned char pucReadBuff[2]    = {0};
    u8 pucMyValues[2]               = {0x10, 0x1F};

    /* 1. Test */
    set_Buffer( &pucReadBuff[0], 0, 1360, 2 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN1_CONFIGURATION2A_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 1 );

    ucStatus = max6639_init( I2CNUM, SLAVE_ADDRESS );
    assert_true( ucStatus == XST_FAILURE );

    /* 2. Test */
    set_Buffer( &pucReadBuff[0], 0, 1360, 2 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN1_CONFIGURATION2A_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN2_CONFIGURATION2A_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 1 );

    ucStatus = max6639_init( I2CNUM, SLAVE_ADDRESS );
    assert_true( ucStatus == XST_FAILURE );

    /* 3. Test */
    set_Buffer( &pucReadBuff[0], 0, 1360, 2 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN1_CONFIGURATION2A_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN2_CONFIGURATION2A_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN1_CONFIGURATION3_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 1 );

    ucStatus = max6639_init( I2CNUM, SLAVE_ADDRESS );
    assert_true( ucStatus == XST_FAILURE );

    /* 4. Test */
    set_Buffer( &pucReadBuff[0], 0, 1360, 2 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN1_CONFIGURATION2A_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN2_CONFIGURATION2A_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN1_CONFIGURATION3_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN2_CONFIGURATION3_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 1 );

    ucStatus = max6639_init( I2CNUM, SLAVE_ADDRESS );
    assert_true( ucStatus == XST_FAILURE );

    /* 5. Test */
    set_Buffer( &pucReadBuff[0], 0, 1360, 2 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN1_CONFIGURATION2A_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN2_CONFIGURATION2A_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN1_CONFIGURATION3_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN2_CONFIGURATION3_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN1_CONFIGURATION2B_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 1 );

    ucStatus = max6639_init( I2CNUM, SLAVE_ADDRESS );
    assert_true( ucStatus == XST_FAILURE );

    /* 6. Test */
    set_Buffer( &pucReadBuff[0], 0, 1360, 2 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN1_CONFIGURATION2A_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN2_CONFIGURATION2A_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN1_CONFIGURATION3_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN2_CONFIGURATION3_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN1_CONFIGURATION2B_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN2_CONFIGURATION2B_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 1 );

    ucStatus = max6639_init( I2CNUM, SLAVE_ADDRESS );
    assert_true( ucStatus == XST_FAILURE );

    /* 7. Test */
    set_Buffer( &pucReadBuff[0], 0, 1360, 2 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN1_CONFIGURATION2A_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN2_CONFIGURATION2A_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN1_CONFIGURATION3_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN2_CONFIGURATION3_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN1_CONFIGURATION2B_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN2_CONFIGURATION2B_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN1_START_TACH_COUNT_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 1 );

    ucStatus = max6639_init( I2CNUM, SLAVE_ADDRESS );
    assert_true( ucStatus == XST_FAILURE );

    /* 8. Test */
    set_Buffer( &pucReadBuff[0], 0, 1360, 2 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN1_CONFIGURATION2A_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN2_CONFIGURATION2A_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN1_CONFIGURATION3_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN2_CONFIGURATION3_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN1_CONFIGURATION2B_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN2_CONFIGURATION2B_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN1_START_TACH_COUNT_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN2_START_TACH_COUNT_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 1 );

    ucStatus = max6639_init( I2CNUM, SLAVE_ADDRESS );
    assert_true( ucStatus == XST_FAILURE );

    /* 9. Test */
    set_Buffer( &pucReadBuff[0], 0, 1360, 2 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN1_CONFIGURATION2A_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN2_CONFIGURATION2A_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN1_CONFIGURATION3_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN2_CONFIGURATION3_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN1_CONFIGURATION2B_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN2_CONFIGURATION2B_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN1_START_TACH_COUNT_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN2_START_TACH_COUNT_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN1_MIN_TACH_COUNT_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 1 );

    ucStatus = max6639_init( I2CNUM, SLAVE_ADDRESS );
    assert_true( ucStatus == XST_FAILURE );

    /* 10. Test */
    set_Buffer( &pucReadBuff[0], 0, 1360, 2 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN1_CONFIGURATION2A_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN2_CONFIGURATION2A_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN1_CONFIGURATION3_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN2_CONFIGURATION3_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN1_CONFIGURATION2B_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN2_CONFIGURATION2B_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN1_START_TACH_COUNT_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN2_START_TACH_COUNT_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN1_MIN_TACH_COUNT_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN2_MIN_TACH_COUNT_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 1 );

    ucStatus = max6639_init( I2CNUM, SLAVE_ADDRESS );
    assert_true( ucStatus == XST_FAILURE );

    /* 11. Test */
    set_Buffer( &pucReadBuff[0], 0, 1360, 2 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN1_CONFIGURATION2A_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN2_CONFIGURATION2A_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN1_CONFIGURATION3_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN2_CONFIGURATION3_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN1_CONFIGURATION2B_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN2_CONFIGURATION2B_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN1_START_TACH_COUNT_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN2_START_TACH_COUNT_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN1_MIN_TACH_COUNT_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN2_MIN_TACH_COUNT_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], CHANNEL1_MIN_FAN_START_TEMP_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 1 );

    ucStatus = max6639_init( I2CNUM, SLAVE_ADDRESS );
    assert_true( ucStatus == XST_FAILURE );

    /* 12. Test */
    set_Buffer( &pucReadBuff[0], 0, 1360, 2 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN1_CONFIGURATION2A_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN2_CONFIGURATION2A_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN1_CONFIGURATION3_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN2_CONFIGURATION3_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN1_CONFIGURATION2B_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN2_CONFIGURATION2B_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN1_START_TACH_COUNT_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN2_START_TACH_COUNT_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN1_MIN_TACH_COUNT_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN2_MIN_TACH_COUNT_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], CHANNEL1_MIN_FAN_START_TEMP_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], CHANNEL2_MIN_FAN_START_TEMP_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 1 );

    ucStatus = max6639_init( I2CNUM, SLAVE_ADDRESS );
    assert_true( ucStatus == XST_FAILURE );

    /* 13. Test */
    set_Buffer( &pucReadBuff[0], 0, 1360, 2 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN1_CONFIGURATION2A_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN2_CONFIGURATION2A_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN1_CONFIGURATION3_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN2_CONFIGURATION3_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN1_CONFIGURATION2B_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN2_CONFIGURATION2B_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN1_START_TACH_COUNT_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN2_START_TACH_COUNT_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN1_MIN_TACH_COUNT_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN2_MIN_TACH_COUNT_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], CHANNEL1_MIN_FAN_START_TEMP_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], CHANNEL2_MIN_FAN_START_TEMP_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN1_CONFIGURATION1_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 1 );

    ucStatus = max6639_init( I2CNUM, SLAVE_ADDRESS );
    assert_true( ucStatus == XST_FAILURE );

    /* 14. Test */
    set_Buffer( &pucReadBuff[0], 0, 1360, 2 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN1_CONFIGURATION2A_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN2_CONFIGURATION2A_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN1_CONFIGURATION3_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN2_CONFIGURATION3_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN1_CONFIGURATION2B_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN2_CONFIGURATION2B_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN1_START_TACH_COUNT_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN2_START_TACH_COUNT_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN1_MIN_TACH_COUNT_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN2_MIN_TACH_COUNT_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], CHANNEL1_MIN_FAN_START_TEMP_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], CHANNEL2_MIN_FAN_START_TEMP_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN1_CONFIGURATION1_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN2_CONFIGURATION1_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 1 );

    ucStatus = max6639_init( I2CNUM, SLAVE_ADDRESS );
    assert_true( ucStatus == XST_FAILURE );
}

static void test_max6639_init( void **state ) 
{
    ( void ) state; /* unused */
    u8 ucStatus                     = 0;
    u8 ucRegisterAddress            = 0;
    u8 ucRegisterContent            = 0;
    unsigned char pucReadBuff[2]    = {0};
    u8 pucMyValues[2]               = {0x10, 0x1F};

    /* 1. Test */
    set_Buffer( &pucReadBuff[0], 0, 1360, 2 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN1_CONFIGURATION2A_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN2_CONFIGURATION2A_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN1_CONFIGURATION3_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN2_CONFIGURATION3_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN1_CONFIGURATION2B_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN2_CONFIGURATION2B_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN1_START_TACH_COUNT_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN2_START_TACH_COUNT_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN1_MIN_TACH_COUNT_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN2_MIN_TACH_COUNT_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], CHANNEL1_MIN_FAN_START_TEMP_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], CHANNEL2_MIN_FAN_START_TEMP_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN1_CONFIGURATION1_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, 2 );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], FAN2_CONFIGURATION1_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    ucStatus = max6639_init( I2CNUM, SLAVE_ADDRESS );
    assert_true( ucStatus == XST_SUCCESS );

}
/***************************************************************/
static void test_max6639_ReadFPGATemperature( void **state ) 
{
    ( void ) state; /* unused */
    u8 ucStatus                     = 0;
    float fTemperatureValue         = 0;
    unsigned char pucReadBuff[2]    = {0};
    u8 pucMyValues[2]               = {0x10, 0x1F};

    /* 1. Test Positive Value*/
    /* This function will generate random values between the range.
    * The Min and Max temperature are 0 and 85 */
    set_Buffer( &pucReadBuff[0], 0, 1360, 2 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send_rs_recv, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send_rs_recv, write_length, WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], CHANNEL1_TEMPERATURE_REGISTER );
    expect_value( __wrap_i2c_send_rs_recv, read_length, READ_LEN );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 0 );

    expect_value( __wrap_i2c_send_rs_recv, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send_rs_recv, write_length, WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], CHANNEL1_EXTENDED_TEMPERATURE_REGISTER );
    expect_value( __wrap_i2c_send_rs_recv, read_length, READ_LEN );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 0 );

    ucStatus = max6639_ReadFPGATemperature( I2CNUM, SLAVE_ADDRESS, &fTemperatureValue );
    assert_true( ucStatus == XST_SUCCESS );

}

static void test_max6639_ReadFPGATemperatureFail( void **state ) {
    ( void ) state; /* unused */

    u8 ucStatus                     = 0;
    unsigned char pucReadBuff[2]    = {0};
    float fTemperatureValue         = 0;
    u8 ucChannel                    = 0;

    /* 1. Test NULL pointer */
    ucStatus = max6639_ReadFPGATemperature( I2CNUM, SLAVE_ADDRESS, NULL );
    assert_true( ucStatus == XST_FAILURE );


    /* 2. Test failed i2c call  */
    set_Buffer( &pucReadBuff[0], 0, 1360, 2 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send_rs_recv, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send_rs_recv, write_length, WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], CHANNEL1_TEMPERATURE_REGISTER );
    expect_value( __wrap_i2c_send_rs_recv, read_length, READ_LEN );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 1 );

    ucStatus = max6639_ReadFPGATemperature( I2CNUM, SLAVE_ADDRESS, &fTemperatureValue );
    assert_true( ucStatus == XST_FAILURE );

    /* 3. Test failed 2nd i2c call  */
    set_Buffer( &pucReadBuff[0], 0, 1360, 2 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send_rs_recv, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send_rs_recv, write_length, WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], CHANNEL1_TEMPERATURE_REGISTER );
    expect_value( __wrap_i2c_send_rs_recv, read_length, READ_LEN );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 0 );

    expect_value( __wrap_i2c_send_rs_recv, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send_rs_recv, write_length, WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], CHANNEL1_EXTENDED_TEMPERATURE_REGISTER );
    expect_value( __wrap_i2c_send_rs_recv, read_length, READ_LEN );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 1 );

    ucStatus = max6639_ReadFPGATemperature( I2CNUM, SLAVE_ADDRESS, &fTemperatureValue );
    assert_true( ucStatus == XST_FAILURE );
}


static void test_max6639_ReadFanTach( void **state ) 
{
    ( void ) state; /* unused */
    u8 ucStatus                     = 0;
    u8 ucFanIndex                     = 1;
    u8 ucFanSpeed                     = 0;
    unsigned char pucReadBuff[2]    = {0};
    u8 pucMyValues[2]               = {0x10, 0x1F};

    /* 1. Test First Value*/
    /* This function will generate random values between the range.
    * The Min and Max temperature are 0 and 85 */
    set_Buffer( &pucReadBuff[0], 0, 1360, 2 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send_rs_recv, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send_rs_recv, write_length, WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], FAN1_TACHOMETER_COUNT_REGISTER );
    expect_value( __wrap_i2c_send_rs_recv, read_length, READ_LEN );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 0 );

    ucStatus = max6639_ReadFanTach( I2CNUM, SLAVE_ADDRESS, ucFanIndex, &ucFanSpeed );
    assert_true( ucStatus == XST_SUCCESS );

    /* 2. Test Second Value*/
    /* This function will generate random values between the range.
    * The Min and Max temperature are 0 and 85 */
    ucFanIndex = 2;
    set_Buffer( &pucReadBuff[0], 0, 1360, 2 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send_rs_recv, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send_rs_recv, write_length, WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], FAN2_TACHOMETER_COUNT_REGISTER );
    expect_value( __wrap_i2c_send_rs_recv, read_length, READ_LEN );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 0 );

    ucStatus = max6639_ReadFanTach( I2CNUM, SLAVE_ADDRESS, ucFanIndex, &ucFanSpeed );
    assert_true( ucStatus == XST_SUCCESS );


}

static void test_max6639_ReadFanTachFail( void **state ) 
{
    ( void ) state; /* unused */

    u8 ucStatus                     = 0;
    unsigned char pucReadBuff[2]    = {0};
    u8 ucFanIndex                   = 4;
    u8 ucFanSpeed                   = 0;


    /* 1. Test NULL pointer */
    ucStatus = max6639_ReadFanTach( I2CNUM, SLAVE_ADDRESS, ucFanIndex, NULL );
    assert_true( ucStatus == XST_FAILURE );


    /* 2. Test bad ucFanIndex*/
    ucStatus = max6639_ReadFanTach( I2CNUM, SLAVE_ADDRESS, ucFanIndex, &ucFanSpeed );
    assert_true( ucStatus == XST_FAILURE );

    /* 2. Test failed i2c call  */
    ucFanIndex = 1;
    set_Buffer( &pucReadBuff[0], 0, 1360, 2 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send_rs_recv, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send_rs_recv, write_length, WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], FAN1_TACHOMETER_COUNT_REGISTER );
    expect_value( __wrap_i2c_send_rs_recv, read_length, READ_LEN );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 1 );

    ucStatus = max6639_ReadFanTach( I2CNUM, SLAVE_ADDRESS, ucFanIndex, &ucFanSpeed );
    assert_true( ucStatus == XST_FAILURE );

    /* 3. Test failed 2nd i2c call  */
    ucFanIndex = 2;
    set_Buffer( &pucReadBuff[0], 0, 1360, 2 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send_rs_recv, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send_rs_recv, write_length, WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], FAN2_TACHOMETER_COUNT_REGISTER );
    expect_value( __wrap_i2c_send_rs_recv, read_length, READ_LEN );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 1 );

    ucStatus = max6639_ReadFanTach( I2CNUM, SLAVE_ADDRESS, ucFanIndex, &ucFanSpeed );
    assert_true( ucStatus == XST_FAILURE );
}



static void test_max6639_ReadDDRTemperature( void **state ) 
{
    ( void ) state; /* unused */
    u8 ucStatus                     = 0;
    float fTemperatureValue         = 0;
    unsigned char pucReadBuff[2]    = {0};
    u8 pucMyValues[2]               = {0x10, 0x1F};

    /* 1. Test Positive Value*/
    /* This function will generate random values between the range.
    * The Min and Max temperature are 0 and 85 */
    set_Buffer( &pucReadBuff[0], 0, 1360, 2 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send_rs_recv, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send_rs_recv, write_length, WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], CHANNEL2_TEMPERATURE_REGISTER );
    expect_value( __wrap_i2c_send_rs_recv, read_length, READ_LEN );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 0 );

    expect_value( __wrap_i2c_send_rs_recv, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send_rs_recv, write_length, WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], CHANNEL2_EXTENDED_TEMPERATURE_REGISTER );
    expect_value( __wrap_i2c_send_rs_recv, read_length, READ_LEN );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 0 );

    ucStatus = max6639_ReadDDRTemperature( I2CNUM, SLAVE_ADDRESS, &fTemperatureValue );
    assert_true( ucStatus == XST_SUCCESS );

}

static void test_max6639_ReadDDRTemperatureFail( void **state ) {
    ( void ) state; /* unused */

    u8 ucStatus                     = 0;
    unsigned char pucReadBuff[2]    = {0};
    float fTemperatureValue         = 0;
    u8 ucChannel                    = 0;

    /* 1. Test NULL pointer */
    ucStatus = max6639_ReadDDRTemperature( I2CNUM, SLAVE_ADDRESS, NULL );
    assert_true( ucStatus == XST_FAILURE );


    /* 2. Test failed i2c call  */
    set_Buffer( &pucReadBuff[0], 0, 1360, 2 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send_rs_recv, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send_rs_recv, write_length, WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], CHANNEL2_TEMPERATURE_REGISTER );
    expect_value( __wrap_i2c_send_rs_recv, read_length, READ_LEN );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 1 );

    ucStatus = max6639_ReadDDRTemperature( I2CNUM, SLAVE_ADDRESS, &fTemperatureValue );
    assert_true( ucStatus == XST_FAILURE );

    /* 3. Test failed 2nd i2c call  */
    set_Buffer( &pucReadBuff[0], 0, 1360, 2 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send_rs_recv, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send_rs_recv, write_length, WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], CHANNEL2_TEMPERATURE_REGISTER );
    expect_value( __wrap_i2c_send_rs_recv, read_length, READ_LEN );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 0 );

    expect_value( __wrap_i2c_send_rs_recv, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send_rs_recv, write_length, WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], CHANNEL2_EXTENDED_TEMPERATURE_REGISTER );
    expect_value( __wrap_i2c_send_rs_recv, read_length, READ_LEN );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 1 );

    ucStatus = max6639_ReadDDRTemperature( I2CNUM, SLAVE_ADDRESS, &fTemperatureValue );
    assert_true( ucStatus == XST_FAILURE );
}

int main( void ) 
{
    srand( time( 0 ) );
    const struct CMUnitTest tests[] = 
    {
        cmocka_unit_test( test_max6639_ReadDDRTemperature ),
        cmocka_unit_test( test_max6639_ReadDDRTemperatureFail ),
        cmocka_unit_test( test_max6639_ReadFanTach ),
        cmocka_unit_test( test_max6639_ReadFanTachFail ),
        cmocka_unit_test( test_max6639_ReadFPGATemperature ),
        cmocka_unit_test( test_max6639_ReadFPGATemperatureFail ),
        cmocka_unit_test( test_max6639_read_register ),
        cmocka_unit_test( test_max6639_read_registerFail ),
        cmocka_unit_test( test_max6639_write_register ),
        cmocka_unit_test( test_max6639_write_registerFail ),
        cmocka_unit_test( test_max6639_init ),
        cmocka_unit_test( test_max6639_initFail )
        

    };

    return cmocka_run_group_tests( tests, NULL, NULL );

}