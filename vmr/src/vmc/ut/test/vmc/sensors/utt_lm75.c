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
#include <lm75.h>

/* I2C parameters */
#define I2CNUM          1
#define SLAVE_ADDRESS   0x18
#define WRITE_LEN_TEMP  1
#define WRITE_BUF       0x05
#define READ_LEN_TEMP   2
#define LM75_TEMP_REG   0

extern void set_Buffer( unsigned char *buff, int min, int max, long int length );

static void test_LM75_ReadTemperature( void **state ) 
{
    ( void ) state; /* unused */
    u8 ucStatus                     = 0;
    s16 ssTemperatureValue          = 0;
    unsigned char pucReadBuff[2]    = {0};
    u8 pucMyValues[2]               = {0x80, 0x8F};

    /* 1. Test Positive Value*/
    /* This function will generate random values between the range.
    * The Min and Max temperature are 0 and 85 */
    set_Buffer( &pucReadBuff[0], 0, 1360, 2 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send_rs_recv, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send_rs_recv, write_length, WRITE_LEN_TEMP );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], LM75_TEMP_REG );
    expect_value( __wrap_i2c_send_rs_recv, read_length, READ_LEN_TEMP );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 0 );

    /*Test Function*/
    ucStatus = LM75_ReadTemperature( I2CNUM, SLAVE_ADDRESS, &ssTemperatureValue );
    assert_true( ucStatus == XST_SUCCESS );
    //assert_in_range( fVoltageInmV, INA3221_POSITIVE_MIN_TEMP, INA3221_POSITIVE_MAX_TEMP );

    /* 2. Test Negative Value*/
    /* This function will generate random values between the range.
    * The Min and Max temperature are 0 and 85 */

    vSetReadBuffer( &pucReadBuff[0], pucMyValues, 2 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send_rs_recv, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send_rs_recv, write_length, WRITE_LEN_TEMP );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], LM75_TEMP_REG );
    expect_value( __wrap_i2c_send_rs_recv, read_length, READ_LEN_TEMP );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 0 );

    /*Test Function*/
    ucStatus = LM75_ReadTemperature( I2CNUM, SLAVE_ADDRESS, &ssTemperatureValue );
    assert_true( ucStatus == XST_SUCCESS );
    //assert_in_range( fVoltageInmV, INA3221_POSITIVE_MIN_TEMP, INA3221_POSITIVE_MAX_TEMP );
}

static void test_LM75_ReadTemperatureFail( void **state )
{
    ( void ) state; /* unused */

    u8 ucStatus                     = 0;
    unsigned char pucReadBuff[2]    = {0};
    s16 ssTemperatureValue          = 0;
    u8 ucChannel                    = 0;

    /* 1. Test NULL pointer */
    ucStatus = LM75_ReadTemperature( I2CNUM, SLAVE_ADDRESS, NULL );
    assert_true( ucStatus == XST_FAILURE );

    /* 2. Test failed i2c call  */
    set_Buffer( &pucReadBuff[0], 0, 1360, 2 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send_rs_recv, i2c_num, I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send_rs_recv, write_length, WRITE_LEN_TEMP );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], LM75_TEMP_REG );
    expect_value( __wrap_i2c_send_rs_recv, read_length, READ_LEN_TEMP );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 1 );

    ucStatus = LM75_ReadTemperature( I2CNUM, SLAVE_ADDRESS, &ssTemperatureValue );
    assert_true( ucStatus == XST_FAILURE );
}

int main( void ) 
{
    srand( time( 0 ) );
    const struct CMUnitTest tests[] = 
    {
        cmocka_unit_test( test_LM75_ReadTemperature ),
        cmocka_unit_test( test_LM75_ReadTemperatureFail )


    };

    return cmocka_run_group_tests( tests, NULL, NULL );

}
