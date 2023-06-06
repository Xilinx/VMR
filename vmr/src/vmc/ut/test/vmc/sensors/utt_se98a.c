#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <cmocka.h>
#include "se98a.h"

/* I2C parameters for se98a */
#define SE98A_I2CNUM_1 1
#define SLAVE_ADDRESS_SE98A_0   0x18
#define SE98A_WRITE_LEN 1
#define SE98A_WRITE_BUF 0X05
#define SE98A_READ_LEN  2

/*Min & Max temperatures for se98a */
#define SE98A_POSITIVE_MIN_TEMP 0
#define SE98A_POSITIVE_MAX_TEMP 85
#define SE98A_NEGATIVE_MIN_TEMP -128
#define SE98A_NEGATIVE_MAX_TEMP -1

extern void set_Buffer( unsigned char *buff, int min, int max, long int length );

static void test_ReadVoltage( void **state )
{
	( void ) state; /* unused */

	int iRet                        = 0;
	s16 ssTemperatureValue          = 0;
	unsigned char pucReadBuff[2]    = {0};
	int i                           = 0;

	for( i = 0; i < 10; i++ )
	{
		/* This function will generate random values between the range.
		 * The Min and Max temperature are 0 and 85 */
		set_Buffer( &pucReadBuff[0], 0, 1360, 2 );

		/*Check mock function parameters*/
		expect_value( __wrap_i2c_send_rs_recv, i2c_num, SE98A_I2CNUM_1 );
		expect_value( __wrap_i2c_send_rs_recv, i2c_addr, SLAVE_ADDRESS_SE98A_0 );
		expect_value( __wrap_i2c_send_rs_recv, write_length, SE98A_WRITE_LEN );
		expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], SE98A_WRITE_BUF );
		expect_value( __wrap_i2c_send_rs_recv, read_length, SE98A_READ_LEN );

		/*set return value for mock function*/
		will_return( __wrap_i2c_send_rs_recv, 0 );

		/*Test Function*/
		iRet = SE98A_ReadTemperature( SE98A_I2CNUM_1, SLAVE_ADDRESS_SE98A_0, &ssTemperatureValue );

		/*Assert results*/
		assert_true( iRet == 0 );
		assert_in_range( ssTemperatureValue, SE98A_POSITIVE_MIN_TEMP, SE98A_POSITIVE_MAX_TEMP );
	}
}


static void test_Negative_SE98A_ReadTemperature( void **state )
{
	( void ) state; /* unused */

	int iRet                        = 0;
	s16 ssTemperatureValue          = 0;
	unsigned char pucReadBuff[2]    = {0};
	int i                           = 0;

	for( i = 0; i < 10; i++ )
	{
		/* This function will generate random values between the range.
		 * The Min and Max temperature are -128 and -1 */
		set_Buffer( &pucReadBuff[0], 6144, 8176, 2 );

		/*Check mock function parameters*/
		expect_value( __wrap_i2c_send_rs_recv, i2c_num, SE98A_I2CNUM_1 );
		expect_value( __wrap_i2c_send_rs_recv, i2c_addr, SLAVE_ADDRESS_SE98A_0 );
		expect_value( __wrap_i2c_send_rs_recv, write_length, SE98A_WRITE_LEN );
		expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], SE98A_WRITE_BUF );
		expect_value( __wrap_i2c_send_rs_recv, read_length, SE98A_READ_LEN );

		/*set return value for mock function*/
		will_return( __wrap_i2c_send_rs_recv, 0 );

		/*Test function*/
		iRet = SE98A_ReadTemperature( SE98A_I2CNUM_1, SLAVE_ADDRESS_SE98A_0, &ssTemperatureValue );

		/*Assert results*/
		assert_true( iRet == 0 );
		assert_in_range( ( ssTemperatureValue & 0xfff ), ( SE98A_NEGATIVE_MIN_TEMP & 0xfff ), ( SE98A_NEGATIVE_MAX_TEMP & 0xfff ) );
	}
}


static void test_Fail_SE98A_ReadTemperature( void **state ) {
	( void ) state; /* unused */

	int iRet                = 0;
	s16 ssTemperatureValue  = 0;

	/*Check mock function parameters*/
	expect_value( __wrap_i2c_send_rs_recv, i2c_num, SE98A_I2CNUM_1 );
	expect_value( __wrap_i2c_send_rs_recv, i2c_addr, SLAVE_ADDRESS_SE98A_0 );
	expect_value( __wrap_i2c_send_rs_recv, write_length, SE98A_WRITE_LEN );
	expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], SE98A_WRITE_BUF );
	expect_value( __wrap_i2c_send_rs_recv, read_length, SE98A_READ_LEN );

	/*set return value for mock function*/
	will_return( __wrap_i2c_send_rs_recv, 1 );

	/*Test function*/
	iRet = SE98A_ReadTemperature( SE98A_I2CNUM_1, SLAVE_ADDRESS_SE98A_0 , &ssTemperatureValue );

	/*Assert results*/
	assert_true( iRet == 1 );

}

int main( void ) 
{
	srand( time( 0 ) );
	const struct CMUnitTest tests[] = {
			cmocka_unit_test( test_ReadVoltage ),
			cmocka_unit_test( test_Negative_SE98A_ReadTemperature ),
			cmocka_unit_test( test_Fail_SE98A_ReadTemperature )
	};

	return cmocka_run_group_tests( tests, NULL, NULL );

}
