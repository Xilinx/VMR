#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <cmocka.h>
#include "xstatus.h"
#include "cl_i2c.h"
#include "cl_log.h"


/* FreeRTOS includes */
#include <FreeRTOS.h>
#include <task.h>
#include <ina3221.h>


/*****************************************************************************/
/* I2C parameters for INA3221 */
#define INA3221_I2CNUM_1 1
#define SLAVE_ADDRESS_INA3221_0   0x18
#define INA3221_WRITE_LEN 1
#define INA3221_WRITE_BUF 0X05
#define INA3221_READ_LEN  2

/*Min & Max temperatures for INA3221 */
#define INA3221_POSITIVE_MIN_TEMP 0
#define INA3221_POSITIVE_MAX_TEMP 85
#define INA3221_NEGATIVE_MIN_TEMP -128
#define INA3221_NEGATIVE_MAX_TEMP -1

extern void set_Buffer( unsigned char *buff, int min, int max, long int length );

static void test_INA3221_ReadVoltage( void **state ) 
{
	( void ) state; /* unused */

    u8 ucStatus                     = 0;
	unsigned char pucReadBuff[2]    = {0};
	int i                           = 0;
    int j                           = 0;
    float fVoltageInmV              = 0;
    u8 ucChannel                    = 0;

    /* 1. Test all channels */
    for( ucChannel = 0; ucChannel < 3; ucChannel++ )
	{
        for( i = 0; i < 10; i++ )
        {
            /* This function will generate random values between the range.
            * The Min and Max temperature are 0 and 85 */
            set_Buffer( &pucReadBuff[0], 0, 1360, 2 );

            /*Check mock function parameters*/
            expect_value( __wrap_i2c_send_rs_recv, i2c_num, INA3221_I2CNUM_1 );
            expect_value( __wrap_i2c_send_rs_recv, i2c_addr, SLAVE_ADDRESS_INA3221_0 );
            expect_value( __wrap_i2c_send_rs_recv, write_length, INA3221_WRITE_LEN );
            expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], ( ucChannel+1 )*2 );
            expect_value( __wrap_i2c_send_rs_recv, read_length, INA3221_READ_LEN );

            /*set return value for mock function*/
            will_return( __wrap_i2c_send_rs_recv, 0 );

            /*Test Function*/
            ucStatus = INA3221_ReadVoltage( INA3221_I2CNUM_1, SLAVE_ADDRESS_INA3221_0, ucChannel, &fVoltageInmV );
            assert_true( ucStatus == XST_SUCCESS );
            //assert_in_range( fVoltageInmV, INA3221_POSITIVE_MIN_TEMP, INA3221_POSITIVE_MAX_TEMP );
        }
    }

}

static void test_INA3221_ReadVoltageFail( void **state )
{
	( void ) state; /* unused */

    u8 ucStatus                     = 0;
	unsigned char pucReadBuff[2]    = {0};
    float fVoltageInmV              = 0;
    u8 ucChannel                    = 0;

    /* 1. Test NULL pointer */
    ucStatus = INA3221_ReadVoltage( INA3221_I2CNUM_1, SLAVE_ADDRESS_INA3221_0, ucChannel, NULL );
    assert_true( ucStatus == XST_FAILURE );


    /* 2. Test failed i2c call  */
    set_Buffer( &pucReadBuff[0], 0, 1360, 2 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send_rs_recv, i2c_num, INA3221_I2CNUM_1 );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, SLAVE_ADDRESS_INA3221_0 );
    expect_value( __wrap_i2c_send_rs_recv, write_length, INA3221_WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], INA3221_CH1_BUS_VOLTAGE );
    expect_value( __wrap_i2c_send_rs_recv, read_length, INA3221_READ_LEN );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 1 );

    ucStatus = INA3221_ReadVoltage( INA3221_I2CNUM_1, SLAVE_ADDRESS_INA3221_0, ucChannel, &fVoltageInmV );
    assert_true( ucStatus == XST_FAILURE );

}

static void test_INA3221_ReadCurrent( void **state )
{
	( void ) state; /* unused */

    u8 ucStatus                     = 0;
	s16 ssTemperatureValue          = 0;
	unsigned char pucReadBuff[2]    = {0};
	int i                           = 0;
    int j                           = 0;
    float fcurrentInmA              = 0;
    u8 ucChannel                    = 0;

    /* 1. Test all channels */
    for( ucChannel = 0; ucChannel < 3; ucChannel++ )
	{
        for( i = 0; i < 10; i++ )
        {
            /* This function will generate random values between the range.
            * The Min and Max temperature are 0 and 85 */
            set_Buffer( &pucReadBuff[0], 0, 1360, 2 );

            /*Check mock function parameters*/
            expect_value( __wrap_i2c_send_rs_recv, i2c_num, INA3221_I2CNUM_1 );
            expect_value( __wrap_i2c_send_rs_recv, i2c_addr, SLAVE_ADDRESS_INA3221_0 );
            expect_value( __wrap_i2c_send_rs_recv, write_length, INA3221_WRITE_LEN );
            expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], ( ucChannel*2 )+1 );
            expect_value( __wrap_i2c_send_rs_recv, read_length, INA3221_READ_LEN );

            /*set return value for mock function*/
            will_return( __wrap_i2c_send_rs_recv, 0 );

            /*Test Function*/
            ucStatus = INA3221_ReadCurrent( INA3221_I2CNUM_1, SLAVE_ADDRESS_INA3221_0, ucChannel, &fcurrentInmA );
            assert_true( ucStatus == XST_SUCCESS );
            //assert_in_range( fVoltageInmV, INA3221_POSITIVE_MIN_TEMP, INA3221_POSITIVE_MAX_TEMP );
        }
    }

}

static void test_INA3221_ReadCurrentFail( void **state )
{
	( void ) state; /* unused */

    u8 ucStatus                     = 0;
	unsigned char pucReadBuff[2]    = {0};
    float fcurrentInmA              = 0;
    u8 ucChannel                    = 0;

    /* 1. Test NULL pointer */
    ucStatus = INA3221_ReadCurrent( INA3221_I2CNUM_1, SLAVE_ADDRESS_INA3221_0, ucChannel, NULL );
    assert_true( ucStatus == XST_FAILURE );


    /* 2. Test failed i2c call  */
    set_Buffer( &pucReadBuff[0], 0, 1360, 2 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send_rs_recv, i2c_num, INA3221_I2CNUM_1 );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, SLAVE_ADDRESS_INA3221_0 );
    expect_value( __wrap_i2c_send_rs_recv, write_length, INA3221_WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], ( ucChannel*2 )+1 );
    expect_value( __wrap_i2c_send_rs_recv, read_length, INA3221_READ_LEN );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 1 );

    ucStatus = INA3221_ReadCurrent( INA3221_I2CNUM_1, SLAVE_ADDRESS_INA3221_0, ucChannel, &fcurrentInmA );
    assert_true( ucStatus == XST_FAILURE );

}

static void test_INA3221_ReadPower( void **state )
{
	( void ) state; /* unused */

    u8 ucStatus                     = 0;
	unsigned char pucReadBuff[2]    = {0};
	int i                           = 0;
    int j                           = 0;
    float fPowerInmW                = 0;
    u8 ucChannel                    = 0;

    /* 1. Test all channels */
    for( ucChannel = 0; ucChannel < 3; ucChannel++ )
	{
        for( i = 0; i < 10; i++ )
        {
            /* Read Current */
            /* This function will generate random values between the range.
            * The Min and Max temperature are 0 and 85 */
            set_Buffer( &pucReadBuff[0], 0, 1360, 2 );

            /*Check mock function parameters*/
            expect_value( __wrap_i2c_send_rs_recv, i2c_num, INA3221_I2CNUM_1 );
            expect_value( __wrap_i2c_send_rs_recv, i2c_addr, SLAVE_ADDRESS_INA3221_0 );
            expect_value( __wrap_i2c_send_rs_recv, write_length, INA3221_WRITE_LEN );
            expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], ( ucChannel*2 )+1 );
            expect_value( __wrap_i2c_send_rs_recv, read_length, INA3221_READ_LEN );

            /*set return value for mock function*/
            will_return( __wrap_i2c_send_rs_recv, 0 );

            /* Read Voltage */
            /* This function will generate random values between the range.
            * The Min and Max temperature are 0 and 85 */
            set_Buffer( &pucReadBuff[0], 0, 1360, 2 );

            /*Check mock function parameters*/
            expect_value( __wrap_i2c_send_rs_recv, i2c_num, INA3221_I2CNUM_1 );
            expect_value( __wrap_i2c_send_rs_recv, i2c_addr, SLAVE_ADDRESS_INA3221_0 );
            expect_value( __wrap_i2c_send_rs_recv, write_length, INA3221_WRITE_LEN );
            expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], ( ucChannel+1 )*2 );
            expect_value( __wrap_i2c_send_rs_recv, read_length, INA3221_READ_LEN );

            /*set return value for mock function*/
            will_return( __wrap_i2c_send_rs_recv, 0 );

            /*Test Function*/
            ucStatus = INA3221_ReadPower( INA3221_I2CNUM_1, SLAVE_ADDRESS_INA3221_0, ucChannel, &fPowerInmW );
            assert_true( ucStatus == XST_SUCCESS );
            //assert_in_range( fVoltageInmV, INA3221_POSITIVE_MIN_TEMP, INA3221_POSITIVE_MAX_TEMP );
        }
    }

}

static void test_INA3221_ReadPowerFail( void **state )
{
	( void ) state; /* unused */

    u8 ucStatus                     = 0;
	unsigned char pucReadBuff[2]    = {0};
    float fPowerInmW                = 0;
    u8 ucChannel                    = 0;

    /* 1. Test NULL pointer */
    ucStatus = INA3221_ReadPower( INA3221_I2CNUM_1, SLAVE_ADDRESS_INA3221_0, ucChannel, NULL );
    assert_true( ucStatus == XST_FAILURE );


    /* 2. Test failed i2c call  */
    set_Buffer( &pucReadBuff[0], 0, 1360, 2 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send_rs_recv, i2c_num, INA3221_I2CNUM_1 );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, SLAVE_ADDRESS_INA3221_0 );
    expect_value( __wrap_i2c_send_rs_recv, write_length, INA3221_WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], ( ucChannel*2 )+1 );
    expect_value( __wrap_i2c_send_rs_recv, read_length, INA3221_READ_LEN );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 1 );

    ucStatus = INA3221_ReadPower( INA3221_I2CNUM_1, SLAVE_ADDRESS_INA3221_0, ucChannel, &fPowerInmW );
    assert_true( ucStatus == XST_FAILURE );

    /* 3. Test failed 2nd i2c call  */
    set_Buffer( &pucReadBuff[0], 0, 1360, 2 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send_rs_recv, i2c_num, INA3221_I2CNUM_1 );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, SLAVE_ADDRESS_INA3221_0 );
    expect_value( __wrap_i2c_send_rs_recv, write_length, INA3221_WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], ( ucChannel*2 )+1 );
    expect_value( __wrap_i2c_send_rs_recv, read_length, INA3221_READ_LEN );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 0 );

    set_Buffer( &pucReadBuff[0], 0, 1360, 2 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send_rs_recv, i2c_num, INA3221_I2CNUM_1 );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, SLAVE_ADDRESS_INA3221_0 );
    expect_value( __wrap_i2c_send_rs_recv, write_length, INA3221_WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], ( ucChannel+1 )*2 );
    expect_value( __wrap_i2c_send_rs_recv, read_length, INA3221_READ_LEN );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 1 );

    ucStatus = INA3221_ReadPower( INA3221_I2CNUM_1, SLAVE_ADDRESS_INA3221_0, ucChannel, &fPowerInmW );
    assert_true( ucStatus == XST_FAILURE );
}

int main( void ) 
{
	srand( time( 0 ) );
	const struct CMUnitTest tests[] = 
    { 
        cmocka_unit_test( test_INA3221_ReadVoltage ),
	    cmocka_unit_test( test_INA3221_ReadVoltageFail ),
        cmocka_unit_test( test_INA3221_ReadCurrent ),
	    cmocka_unit_test( test_INA3221_ReadCurrentFail ),
        cmocka_unit_test( test_INA3221_ReadPower ),
	    cmocka_unit_test( test_INA3221_ReadPowerFail )
	};

	return cmocka_run_group_tests( tests, NULL, NULL );

}