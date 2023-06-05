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
#include <cat34ts02.h>

/* I2C parameters */
#define I2CNUM          1
#define SLAVE_ADDRESS   0x18
#define WRITE_LEN_TEMP  1
#define WRITE_LEN       2
#define WRITE_BUF       0x05
#define READ_LEN_TEMP   2
#define READ_LEN        1

extern void set_Buffer(  unsigned char *buff, int min, int max, long int length );

static void test_ucCAT34TS02ReadTemperature(  void **state ) 
{
    (  void ) state; /* unused */
    u8 ucStatus                     = 0;
    s16 ssTemperatureValue          = 0;
    unsigned char pucReadBuff[2]    = {0};
    u8 pucMyValues[2]               = {0x10, 0x1F};

    /* 1. Test Positive Value*/
    /* This function will generate random values between the range.
    * The Min and Max temperature are 0 and 85 */
    set_Buffer(  &pucReadBuff[0], 0, 1360, 2 );

    /*Check mock function parameters*/
    expect_value(  __wrap_i2c_send_rs_recv, i2c_num, I2CNUM );
    expect_value(  __wrap_i2c_send_rs_recv, i2c_addr, CAT34TS02_SLAVE_ADDRESS );
    expect_value(  __wrap_i2c_send_rs_recv, write_length, WRITE_LEN_TEMP );
    expect_value(  __wrap_i2c_send_rs_recv, i2c_write_buff[0], CAT34TS02_TEMPERATURE_REGISTER );
    expect_value(  __wrap_i2c_send_rs_recv, read_length, READ_LEN_TEMP );

    /*set return value for mock function*/
    will_return(  __wrap_i2c_send_rs_recv, 0 );

    /*Test Function*/
    ucStatus = ucCAT34TS02ReadTemperature(  I2CNUM, CAT34TS02_SLAVE_ADDRESS, &ssTemperatureValue );
    assert_true(  ucStatus == XST_SUCCESS );
    //assert_in_range(  fVoltageInmV, INA3221_POSITIVE_MIN_TEMP, INA3221_POSITIVE_MAX_TEMP );

    /* 2. Test Negative Value*/
    /* This function will generate random values between the range.
    * The Min and Max temperature are 0 and 85 */

    vSetReadBuffer(  &pucReadBuff[0], pucMyValues, 2 );


    /*Check mock function parameters*/
    expect_value(  __wrap_i2c_send_rs_recv, i2c_num, I2CNUM );
    expect_value(  __wrap_i2c_send_rs_recv, i2c_addr, CAT34TS02_SLAVE_ADDRESS );
    expect_value(  __wrap_i2c_send_rs_recv, write_length, WRITE_LEN_TEMP );
    expect_value(  __wrap_i2c_send_rs_recv, i2c_write_buff[0], CAT34TS02_TEMPERATURE_REGISTER );
    expect_value(  __wrap_i2c_send_rs_recv, read_length, READ_LEN_TEMP );

    /*set return value for mock function*/
    will_return(  __wrap_i2c_send_rs_recv, 0 );

    /*Test Function*/
    ucStatus = ucCAT34TS02ReadTemperature(  I2CNUM, CAT34TS02_SLAVE_ADDRESS, &ssTemperatureValue );
    assert_true(  ucStatus == XST_SUCCESS );
    //assert_in_range(  fVoltageInmV, INA3221_POSITIVE_MIN_TEMP, INA3221_POSITIVE_MAX_TEMP );
}

static void test_ucCAT34TS02ReadTemperatureFail(  void **state ) {
    (  void ) state; /* unused */

    u8 ucStatus                     = 0;
    unsigned char pucReadBuff[2]    = {0};
    s16 ssTemperatureValue          = 0;
    u8 ucChannel                    = 0;

    /* 1. Test NULL pointer */
    ucStatus = ucCAT34TS02ReadTemperature(  I2CNUM, CAT34TS02_SLAVE_ADDRESS, NULL );
    assert_true(  ucStatus == XST_FAILURE );


    /* 2. Test failed i2c call  */
    set_Buffer(  &pucReadBuff[0], 0, 1360, 2 );

    /*Check mock function parameters*/
    expect_value(  __wrap_i2c_send_rs_recv, i2c_num, I2CNUM );
    expect_value(  __wrap_i2c_send_rs_recv, i2c_addr, CAT34TS02_SLAVE_ADDRESS );
    expect_value(  __wrap_i2c_send_rs_recv, write_length, WRITE_LEN_TEMP );
    expect_value(  __wrap_i2c_send_rs_recv, i2c_write_buff[0], CAT34TS02_TEMPERATURE_REGISTER );
    expect_value(  __wrap_i2c_send_rs_recv, read_length, READ_LEN_TEMP );

    /*set return value for mock function*/
    will_return(  __wrap_i2c_send_rs_recv, 1 );

    ucStatus = ucCAT34TS02ReadTemperature(  I2CNUM, CAT34TS02_SLAVE_ADDRESS, &ssTemperatureValue );
    assert_true(  ucStatus == XST_FAILURE );

}

static void test_ucCAT34TS02ReadByte(  void **state ) 
{
    (  void ) state; /* unused */
    u8 ucStatus                     = 0;
    u16 usAddressOffset             = 0x1523;
    u8 ucRegisterValue              = 0;
    unsigned char pucReadBuff[2]    = {0};
    u8 pucMyValues[2]               = {0x10, 0x1F};

    /* 1. Test */
    /* This function will generate random values between the range.
    * The Min and Max temperature are 0 and 85 */
    set_Buffer(  &pucReadBuff[0], 0, 1360, 2 );

    /*Check mock function parameters*/
    expect_value(  __wrap_i2c_send_rs_recv, i2c_num, I2CNUM );
    expect_value(  __wrap_i2c_send_rs_recv, i2c_addr, CAT34TS02_SLAVE_ADDRESS );
    expect_value(  __wrap_i2c_send_rs_recv, write_length, WRITE_LEN );
    expect_value(  __wrap_i2c_send_rs_recv, i2c_write_buff[0], (  u8 )(  usAddressOffset & 0xFF ) );
    expect_value(  __wrap_i2c_send_rs_recv, read_length, READ_LEN );

    /*set return value for mock function*/
    will_return(  __wrap_i2c_send_rs_recv, 0 );

    ucStatus = ucCAT34TS02ReadByte(  I2CNUM, CAT34TS02_SLAVE_ADDRESS, &usAddressOffset, &ucRegisterValue );
    assert_true(  ucStatus == XST_SUCCESS );

}

static void test_ucCAT34TS02ReadByteFail(  void **state ) 
{
    (  void ) state; /* unused */
    u8 ucStatus                     = 0;
    u16 usAddressOffset             = 0x1523;;
    u8 ucRegisterValue              = 0;
    unsigned char pucReadBuff[2]    = {0};
    u8 pucMyValues[2]               = {0x10, 0x1F};

    /* 1. Test NULL*/
    ucStatus = ucCAT34TS02ReadByte(  I2CNUM, CAT34TS02_SLAVE_ADDRESS, NULL, &ucRegisterValue );
    assert_true(  ucStatus == XST_FAILURE );
    
    /* 2. Test NULL*/
    ucStatus = ucCAT34TS02ReadByte(  I2CNUM, CAT34TS02_SLAVE_ADDRESS, &usAddressOffset, NULL );
    assert_true(  ucStatus == XST_FAILURE );

    /* 3. Test I2C fail*/
    /* This function will generate random values between the range.
    * The Min and Max temperature are 0 and 85 */
    set_Buffer(  &pucReadBuff[0], 0, 1360, 2 );

    /*Check mock function parameters*/
    expect_value(  __wrap_i2c_send_rs_recv, i2c_num, I2CNUM );
    expect_value(  __wrap_i2c_send_rs_recv, i2c_addr, CAT34TS02_SLAVE_ADDRESS );
    expect_value(  __wrap_i2c_send_rs_recv, write_length, CAT34TS02_EEPROM_ADDRESS_SIZE );
    expect_value(  __wrap_i2c_send_rs_recv, i2c_write_buff[0], (  u8 )(  usAddressOffset & 0xFF ) );
    expect_value(  __wrap_i2c_send_rs_recv, read_length, READ_LEN );

    /*set return value for mock function*/
    will_return(  __wrap_i2c_send_rs_recv, 1 );

    ucStatus = ucCAT34TS02ReadByte(  I2CNUM, CAT34TS02_SLAVE_ADDRESS, &usAddressOffset, &ucRegisterValue );
    assert_true(  ucStatus == XST_FAILURE );
}

static void test_ucCAT34TS02ReadMultiBytes(  void **state ) 
{
    (  void ) state; /* unused */
    u8 ucStatus                     = 0;
    u16 usAddressOffset             = 0x1523;
    size_t xBufSize                 = 20;
    unsigned char pucReadBuff[2]    = {0};
    u8 pucMyValues[20]              = {0};

    /* 1. Test */
    /* This function will generate random values between the range.
    * The Min and Max temperature are 0 and 85 */
    set_Buffer(  &pucReadBuff[0], 0, 1360, 2 );

    /*Check mock function parameters*/
    expect_value(  __wrap_i2c_send_rs_recv, i2c_num, I2CNUM );
    expect_value(  __wrap_i2c_send_rs_recv, i2c_addr, CAT34TS02_SLAVE_ADDRESS );
    expect_value(  __wrap_i2c_send_rs_recv, write_length, CAT34TS02_EEPROM_ADDRESS_SIZE );
    expect_value(  __wrap_i2c_send_rs_recv, i2c_write_buff[0], (  u8 )(  usAddressOffset & 0xFF ) );
    expect_value(  __wrap_i2c_send_rs_recv, read_length, xBufSize );

    /*set return value for mock function*/
    will_return(  __wrap_i2c_send_rs_recv, 0 );

    ucStatus = ucCAT34TS02ReadMultiBytes(  I2CNUM, CAT34TS02_SLAVE_ADDRESS, &usAddressOffset, pucMyValues, xBufSize );
    assert_true(  ucStatus == XST_SUCCESS );

}

static void test_ucCAT34TS02ReadMultiBytesFail(  void **state ) 
{
    (  void ) state; /* unused */
    u8 ucStatus                     = 0;
    u16 usAddressOffset             = 0x1523;;
    u8 ucRegisterValue              = 0;
    size_t xBufSize                 = 20; 
    unsigned char pucReadBuff[2]    = {0};
    u8 pucMyValues[20]              = {0};

    /* 1. Test NULL*/
    ucStatus = ucCAT34TS02ReadMultiBytes(  I2CNUM, CAT34TS02_SLAVE_ADDRESS, NULL, pucMyValues, xBufSize );
    assert_true(  ucStatus == XST_FAILURE );
    
    /* 2. Test NULL*/
    ucStatus = ucCAT34TS02ReadMultiBytes(  I2CNUM, CAT34TS02_SLAVE_ADDRESS, &usAddressOffset, NULL, xBufSize );
    assert_true(  ucStatus == XST_FAILURE );

    /* 3. Test I2C fail*/
    /* This function will generate random values between the range.
    * The Min and Max temperature are 0 and 85 */
    set_Buffer(  &pucReadBuff[0], 0, 1360, 2 );

    /*Check mock function parameters*/
    expect_value(  __wrap_i2c_send_rs_recv, i2c_num, I2CNUM );
    expect_value(  __wrap_i2c_send_rs_recv, i2c_addr, CAT34TS02_SLAVE_ADDRESS );
    expect_value(  __wrap_i2c_send_rs_recv, write_length, CAT34TS02_EEPROM_ADDRESS_SIZE );
    expect_value(  __wrap_i2c_send_rs_recv, i2c_write_buff[0], (  u8 )(  usAddressOffset & 0xFF ) );
    expect_value(  __wrap_i2c_send_rs_recv, read_length, xBufSize );

    /*set return value for mock function*/
    will_return(  __wrap_i2c_send_rs_recv, 1 );

    ucStatus = ucCAT34TS02ReadMultiBytes(  I2CNUM, CAT34TS02_SLAVE_ADDRESS, &usAddressOffset, pucMyValues, xBufSize );
    assert_true(  ucStatus == XST_FAILURE );
}

int main(  void ) 
{
    srand(  time(  0 ) );
    const struct CMUnitTest tests[] = 
    {
        cmocka_unit_test(  test_ucCAT34TS02ReadTemperature ),
        cmocka_unit_test(  test_ucCAT34TS02ReadTemperatureFail ),
        cmocka_unit_test(  test_ucCAT34TS02ReadByte ),
        cmocka_unit_test(  test_ucCAT34TS02ReadByteFail ),
        cmocka_unit_test(  test_ucCAT34TS02ReadMultiBytes ),
        cmocka_unit_test(  test_ucCAT34TS02ReadMultiBytesFail )

    };

    return cmocka_run_group_tests(  tests, NULL, NULL );

}
