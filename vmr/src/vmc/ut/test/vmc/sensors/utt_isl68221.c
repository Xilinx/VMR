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
#include <isl68221.h>


/*****************************************************************************/
/* I2C parameters for ISL68221 */
#define ISL68221_I2CNUM 1
#define SLAVE_ADDRESS_ISL68221_0    0x18
#define ISL68221_WRITE_LEN_1        1
#define ISL68221_WRITE_LEN          2
#define ISL68221_WRITE_BUF          0X05
#define ISL68221_READ_LEN           2

/*Min & Max temperatures for ISL68221 */
//#define ISL68221_POSITIVE_MIN_TEMP 0
//#define ISL68221_POSITIVE_MAX_TEMP 85
//#define ISL68221_NEGATIVE_MIN_TEMP -128
//#define ISL68221_NEGATIVE_MAX_TEMP -1

extern void set_Buffer( unsigned char *buff, int min, int max, long int length );

static void test_ucISL68221WriteRegister( void **state )
{
	( void ) state; /* unused */

    u8 ucStatus                 = 0;
    u8 ucRegisterContent        = 0;
    u8 ucRegisterAddress        = 0;

    /* Test 1 */

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, ISL68221_I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS_ISL68221_0 );
    expect_value( __wrap_i2c_send, write_length, ISL68221_WRITE_LEN );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], ucRegisterAddress );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    /*Test Function*/
    ucStatus = ucISL68221WriteRegister( ISL68221_I2CNUM, SLAVE_ADDRESS_ISL68221_0, ucRegisterAddress, &ucRegisterContent );
    assert_true( ucStatus == XST_SUCCESS );
    //assert_in_range( fVoltageInmV, ISL68221_POSITIVE_MIN_TEMP, ISL68221_POSITIVE_MAX_TEMP );

    /* Test 1 NULL parameter*/
    

    /*Test Function*/
    ucStatus = ucISL68221WriteRegister( ISL68221_I2CNUM, SLAVE_ADDRESS_ISL68221_0, ucRegisterAddress,NULL );
    assert_true( ucStatus == XST_FAILURE );

}

static void test_ucISL68221ReadRegister( void **state )
{
	( void ) state; /* unused */

    u8 ucStatus                     = 0;
	unsigned char pucReadBuff[2]    = {0};
	int i                           = 0;
    int j                           = 0;
    u8 ucRegisterContent            = 0;
    u8 ucRegisterAddress            = 0;

    /* Test 1 */
    set_Buffer( &pucReadBuff[0], 0, 1360, 2 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send_rs_recv, i2c_num, ISL68221_I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, SLAVE_ADDRESS_ISL68221_0 );
    expect_value( __wrap_i2c_send_rs_recv, write_length, ISL68221_WRITE_LEN_1 );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], ucRegisterAddress );
    expect_value( __wrap_i2c_send_rs_recv, read_length, ISL68221_READ_LEN );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 0 );

    /*Test Function*/
    ucStatus = ucISL68221ReadRegister( ISL68221_I2CNUM, SLAVE_ADDRESS_ISL68221_0, ucRegisterAddress, &ucRegisterContent );
    assert_true( ucStatus == XST_SUCCESS );

    
    /*Test Function*/
    ucStatus = ucISL68221ReadRegister( ISL68221_I2CNUM, SLAVE_ADDRESS_ISL68221_0, ucRegisterAddress, NULL );
    assert_true( ucStatus == XST_FAILURE );

}

static void test_ucISL68221ReadVoltage0Fail( void **state )
{
	( void ) state; /* unused */

    u8 ucStatus                     = 0;
	unsigned char pucReadBuff[2]    = {0};
    float fVoltageInmV              = 0;
    u8 ucChannel                    = 0;
    u8 ucRegisterContent            = 0;
    u8 ucRegisterAddress            = 0;

    /* 1. Test NULL pointer */
    ucStatus = ucISL68221ReadVoltage0( ISL68221_I2CNUM, SLAVE_ADDRESS_ISL68221_0, NULL );
    assert_true( ucStatus == XST_FAILURE );

    /* 2. Test failed i2c call  */
    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, ISL68221_I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS_ISL68221_0 );
    expect_value( __wrap_i2c_send, write_length, ISL68221_WRITE_LEN );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], ISL68221_PAGE_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 1 );

    ucStatus = ucISL68221ReadVoltage0( ISL68221_I2CNUM, SLAVE_ADDRESS_ISL68221_0, &fVoltageInmV );
    assert_true( ucStatus == XST_FAILURE );

    /* 3. Test failed i2c call  */
    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, ISL68221_I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS_ISL68221_0 );
    expect_value( __wrap_i2c_send, write_length, ISL68221_WRITE_LEN );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], ISL68221_PAGE_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    set_Buffer( &pucReadBuff[0], 0, 1360, 2 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send_rs_recv, i2c_num, ISL68221_I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, SLAVE_ADDRESS_ISL68221_0 );
    expect_value( __wrap_i2c_send_rs_recv, write_length, ISL68221_WRITE_LEN_1 );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], ISL68221_OUTPUT_VOLTAGE_REGISTER );
    expect_value( __wrap_i2c_send_rs_recv, read_length, ISL68221_READ_LEN );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 1 );

    ucStatus = ucISL68221ReadVoltage0( ISL68221_I2CNUM, SLAVE_ADDRESS_ISL68221_0, &fVoltageInmV );
    assert_true( ucStatus == XST_FAILURE );

}

static void test_ucISL68221ReadVoltage0( void **state )
{
	( void ) state; /* unused */

    u8 ucStatus                     = 0;
	unsigned char pucReadBuff[2]    = {0};
    float fVoltageInmV              = 0;
    u8 ucChannel                    = 0;
    u8 ucRegisterContent            = 0;
    u8 ucRegisterAddress            = 0;

    /* 1. Test  */
    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, ISL68221_I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS_ISL68221_0 );
    expect_value( __wrap_i2c_send, write_length, ISL68221_WRITE_LEN );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], ISL68221_PAGE_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 1 );

    ucStatus = ucISL68221ReadVoltage0( ISL68221_I2CNUM, SLAVE_ADDRESS_ISL68221_0, &fVoltageInmV );
    assert_true( ucStatus == XST_FAILURE );

    /* 2. Test failed i2c call  */
    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, ISL68221_I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS_ISL68221_0 );
    expect_value( __wrap_i2c_send, write_length, ISL68221_WRITE_LEN );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], ISL68221_PAGE_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    set_Buffer( &pucReadBuff[0], 0, 1360, 2 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send_rs_recv, i2c_num, ISL68221_I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, SLAVE_ADDRESS_ISL68221_0 );
    expect_value( __wrap_i2c_send_rs_recv, write_length, ISL68221_WRITE_LEN_1 );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], ISL68221_OUTPUT_VOLTAGE_REGISTER );
    expect_value( __wrap_i2c_send_rs_recv, read_length, ISL68221_READ_LEN );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 0 );

    ucStatus = ucISL68221ReadVoltage0( ISL68221_I2CNUM, SLAVE_ADDRESS_ISL68221_0, &fVoltageInmV );
    assert_true( ucStatus == XST_SUCCESS );

}


static void test_ucISL68221ReadVoltage1Fail( void **state )
{
	( void ) state; /* unused */

    u8 ucStatus                     = 0;
	unsigned char pucReadBuff[2]    = {0};
    float fVoltageInmV              = 0;
    u8 ucChannel                    = 0;
    u8 ucRegisterContent            = 0;
    u8 ucRegisterAddress            = 0;

    /* 1. Test NULL pointer */
    ucStatus = ucISL68221ReadVoltage1( ISL68221_I2CNUM, SLAVE_ADDRESS_ISL68221_0, NULL );
    assert_true( ucStatus == XST_FAILURE );


    /* 2. Test failed i2c call  */
    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, ISL68221_I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS_ISL68221_0 );
    expect_value( __wrap_i2c_send, write_length, ISL68221_WRITE_LEN );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], ISL68221_PAGE_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 1 );

    ucStatus = ucISL68221ReadVoltage1( ISL68221_I2CNUM, SLAVE_ADDRESS_ISL68221_0, &fVoltageInmV );
    assert_true( ucStatus == XST_FAILURE );

    /* 3. Test failed i2c call  */
    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, ISL68221_I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS_ISL68221_0 );
    expect_value( __wrap_i2c_send, write_length, ISL68221_WRITE_LEN );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], ISL68221_PAGE_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    set_Buffer( &pucReadBuff[0], 0, 1360, 2 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send_rs_recv, i2c_num, ISL68221_I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, SLAVE_ADDRESS_ISL68221_0 );
    expect_value( __wrap_i2c_send_rs_recv, write_length, ISL68221_WRITE_LEN_1 );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], ISL68221_OUTPUT_VOLTAGE_REGISTER );
    expect_value( __wrap_i2c_send_rs_recv, read_length, ISL68221_READ_LEN );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 1 );

    ucStatus = ucISL68221ReadVoltage1( ISL68221_I2CNUM, SLAVE_ADDRESS_ISL68221_0, &fVoltageInmV );
    assert_true( ucStatus == XST_FAILURE );

}

static void test_ucISL68221ReadVoltage1( void **state )
{
	( void ) state; /* unused */

    u8 ucStatus                     = 0;
	unsigned char pucReadBuff[2]    = {0};
    float fVoltageInmV              = 0;
    u8 ucChannel                    = 0;
    u8 ucRegisterContent            = 0;
    u8 ucRegisterAddress            = 0;

    /* 1. Test  */

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, ISL68221_I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS_ISL68221_0 );
    expect_value( __wrap_i2c_send, write_length, ISL68221_WRITE_LEN );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], ISL68221_PAGE_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 1 );

    ucStatus = ucISL68221ReadVoltage1( ISL68221_I2CNUM, SLAVE_ADDRESS_ISL68221_0, &fVoltageInmV );
    assert_true( ucStatus == XST_FAILURE );

    /* 3. Test failed i2c call  */
    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, ISL68221_I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS_ISL68221_0 );
    expect_value( __wrap_i2c_send, write_length, ISL68221_WRITE_LEN );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], ISL68221_PAGE_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    set_Buffer( &pucReadBuff[0], 0, 1360, 2 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send_rs_recv, i2c_num, ISL68221_I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, SLAVE_ADDRESS_ISL68221_0 );
    expect_value( __wrap_i2c_send_rs_recv, write_length, ISL68221_WRITE_LEN_1 );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], ISL68221_OUTPUT_VOLTAGE_REGISTER );
    expect_value( __wrap_i2c_send_rs_recv, read_length, ISL68221_READ_LEN );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 0 );

    ucStatus = ucISL68221ReadVoltage1( ISL68221_I2CNUM, SLAVE_ADDRESS_ISL68221_0, &fVoltageInmV );
    assert_true( ucStatus == XST_SUCCESS );

}

static void test_ucISL68221ReadVoltage2Fail( void **state )
{
	( void ) state; /* unused */

    u8 ucStatus                     = 0;
	unsigned char pucReadBuff[2]    = {0};
    float fVoltageInmV              = 0;
    u8 ucChannel                    = 0;
    u8 ucRegisterContent            = 0;
    u8 ucRegisterAddress            = 0;

    /* 1. Test NULL pointer */
    ucStatus = ucISL68221ReadVoltage2( ISL68221_I2CNUM, SLAVE_ADDRESS_ISL68221_0, NULL );
    assert_true( ucStatus == XST_FAILURE );

    /* 2. Test failed i2c call  */
    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, ISL68221_I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS_ISL68221_0 );
    expect_value( __wrap_i2c_send, write_length, ISL68221_WRITE_LEN );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], ISL68221_PAGE_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 1 );

    ucStatus = ucISL68221ReadVoltage2( ISL68221_I2CNUM, SLAVE_ADDRESS_ISL68221_0, &fVoltageInmV );
    assert_true( ucStatus == XST_FAILURE );

    /* 3. Test failed i2c call  */
    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, ISL68221_I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS_ISL68221_0 );
    expect_value( __wrap_i2c_send, write_length, ISL68221_WRITE_LEN );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], ISL68221_PAGE_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    set_Buffer( &pucReadBuff[0], 0, 1360, 2 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send_rs_recv, i2c_num, ISL68221_I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, SLAVE_ADDRESS_ISL68221_0 );
    expect_value( __wrap_i2c_send_rs_recv, write_length, ISL68221_WRITE_LEN_1 );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], ISL68221_OUTPUT_VOLTAGE_REGISTER );
    expect_value( __wrap_i2c_send_rs_recv, read_length, ISL68221_READ_LEN );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 1 );

    ucStatus = ucISL68221ReadVoltage2( ISL68221_I2CNUM, SLAVE_ADDRESS_ISL68221_0, &fVoltageInmV );
    assert_true( ucStatus == XST_FAILURE );

}

static void test_ucISL68221ReadVoltage2( void **state )
{
	( void ) state; /* unused */

    u8 ucStatus                     = 0;
	unsigned char pucReadBuff[2]    = {0};
    float fVoltageInmV              = 0;
    u8 ucChannel                    = 0;
    u8 ucRegisterContent            = 0;
    u8 ucRegisterAddress            = 0;

    /* 1. Test  */
    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, ISL68221_I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS_ISL68221_0 );
    expect_value( __wrap_i2c_send, write_length, ISL68221_WRITE_LEN );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], ISL68221_PAGE_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 1 );

    ucStatus = ucISL68221ReadVoltage2( ISL68221_I2CNUM, SLAVE_ADDRESS_ISL68221_0, &fVoltageInmV );
    assert_true( ucStatus == XST_FAILURE );

    /* 3. Test failed i2c call  */
    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, ISL68221_I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS_ISL68221_0 );
    expect_value( __wrap_i2c_send, write_length, ISL68221_WRITE_LEN );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], ISL68221_PAGE_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    set_Buffer( &pucReadBuff[0], 0, 1360, 2 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send_rs_recv, i2c_num, ISL68221_I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, SLAVE_ADDRESS_ISL68221_0 );
    expect_value( __wrap_i2c_send_rs_recv, write_length, ISL68221_WRITE_LEN_1 );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], ISL68221_OUTPUT_VOLTAGE_REGISTER );
    expect_value( __wrap_i2c_send_rs_recv, read_length, ISL68221_READ_LEN );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 0 );

    ucStatus = ucISL68221ReadVoltage2( ISL68221_I2CNUM, SLAVE_ADDRESS_ISL68221_0, &fVoltageInmV );
    assert_true( ucStatus == XST_SUCCESS );

}

static void test_ucISL68221ReadCurrent0Fail( void **state )
{
	( void ) state; /* unused */

    u8 ucStatus                     = 0;
	unsigned char pucReadBuff[2]    = {0};
    float fCurrentInmV              = 0;
    u8 ucChannel                    = 0;
    u8 ucRegisterContent            = 0;
    u8 ucRegisterAddress            = 0;

    /* 1. Test NULL pointer */
    ucStatus = ucISL68221ReadCurrent0( ISL68221_I2CNUM, SLAVE_ADDRESS_ISL68221_0, NULL );
    assert_true( ucStatus == XST_FAILURE );


    /* 2. Test failed i2c call  */
    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, ISL68221_I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS_ISL68221_0 );
    expect_value( __wrap_i2c_send, write_length, ISL68221_WRITE_LEN );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], ISL68221_PAGE_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 1 );

    ucStatus = ucISL68221ReadCurrent0( ISL68221_I2CNUM, SLAVE_ADDRESS_ISL68221_0, &fCurrentInmV );
    assert_true( ucStatus == XST_FAILURE );

    /* 3. Test failed i2c call  */
    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, ISL68221_I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS_ISL68221_0 );
    expect_value( __wrap_i2c_send, write_length, ISL68221_WRITE_LEN );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], ISL68221_PAGE_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    set_Buffer( &pucReadBuff[0], 0, 1360, 2 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send_rs_recv, i2c_num, ISL68221_I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, SLAVE_ADDRESS_ISL68221_0 );
    expect_value( __wrap_i2c_send_rs_recv, write_length, ISL68221_WRITE_LEN_1 );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], ISL68221_OUTPUT_CURRENT_REGISTER );
    expect_value( __wrap_i2c_send_rs_recv, read_length, ISL68221_READ_LEN );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 1 );

    ucStatus = ucISL68221ReadCurrent0( ISL68221_I2CNUM, SLAVE_ADDRESS_ISL68221_0, &fCurrentInmV );
    assert_true( ucStatus == XST_FAILURE );

}

static void test_ucISL68221ReadCurrent0( void **state )
{
	( void ) state; /* unused */

    u8 ucStatus                     = 0;
	unsigned char pucReadBuff[2]    = {0};
    float fCurrentInmV              = 0;
    u8 ucChannel                    = 0;
    u8 ucRegisterContent            = 0;
    u8 ucRegisterAddress            = 0;

    /* 1. Test  */
    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, ISL68221_I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS_ISL68221_0 );
    expect_value( __wrap_i2c_send, write_length, ISL68221_WRITE_LEN );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], ISL68221_PAGE_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 1 );

    ucStatus = ucISL68221ReadCurrent0( ISL68221_I2CNUM, SLAVE_ADDRESS_ISL68221_0, &fCurrentInmV );
    assert_true( ucStatus == XST_FAILURE );

    /* 3. Test failed i2c call  */
    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, ISL68221_I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS_ISL68221_0 );
    expect_value( __wrap_i2c_send, write_length, ISL68221_WRITE_LEN );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], ISL68221_PAGE_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    set_Buffer( &pucReadBuff[0], 0, 1360, 2 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send_rs_recv, i2c_num, ISL68221_I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, SLAVE_ADDRESS_ISL68221_0 );
    expect_value( __wrap_i2c_send_rs_recv, write_length, ISL68221_WRITE_LEN_1 );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], ISL68221_OUTPUT_CURRENT_REGISTER );
    expect_value( __wrap_i2c_send_rs_recv, read_length, ISL68221_READ_LEN );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 0 );

    ucStatus = ucISL68221ReadCurrent0( ISL68221_I2CNUM, SLAVE_ADDRESS_ISL68221_0, &fCurrentInmV );
    assert_true( ucStatus == XST_SUCCESS );

}


static void test_ucISL68221ReadCurrent1Fail( void **state )
{
	( void ) state; /* unused */

    u8 ucStatus                     = 0;
	unsigned char pucReadBuff[2]    = {0};
    float fCurrentInmV              = 0;
    u8 ucChannel                    = 0;
    u8 ucRegisterContent            = 0;
    u8 ucRegisterAddress            = 0;

    /* 1. Test NULL pointer */
    ucStatus = ucISL68221ReadCurrent1( ISL68221_I2CNUM, SLAVE_ADDRESS_ISL68221_0, NULL );
    assert_true( ucStatus == XST_FAILURE );

    /* 2. Test failed i2c call  */
    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, ISL68221_I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS_ISL68221_0 );
    expect_value( __wrap_i2c_send, write_length, ISL68221_WRITE_LEN );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], ISL68221_PAGE_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 1 );

    ucStatus = ucISL68221ReadCurrent1( ISL68221_I2CNUM, SLAVE_ADDRESS_ISL68221_0, &fCurrentInmV );
    assert_true( ucStatus == XST_FAILURE );

    /* 3. Test failed i2c call  */
    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, ISL68221_I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS_ISL68221_0 );
    expect_value( __wrap_i2c_send, write_length, ISL68221_WRITE_LEN );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], ISL68221_PAGE_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    set_Buffer( &pucReadBuff[0], 0, 1360, 2 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send_rs_recv, i2c_num, ISL68221_I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, SLAVE_ADDRESS_ISL68221_0 );
    expect_value( __wrap_i2c_send_rs_recv, write_length, ISL68221_WRITE_LEN_1 );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], ISL68221_OUTPUT_CURRENT_REGISTER );
    expect_value( __wrap_i2c_send_rs_recv, read_length, ISL68221_READ_LEN );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 1 );

    ucStatus = ucISL68221ReadCurrent1( ISL68221_I2CNUM, SLAVE_ADDRESS_ISL68221_0, &fCurrentInmV );
    assert_true( ucStatus == XST_FAILURE );

}

static void test_ucISL68221ReadCurrent1( void **state )
{
	( void ) state; /* unused */

    u8 ucStatus                     = 0;
	unsigned char pucReadBuff[2]    = {0};
    float fCurrentInmV              = 0;
    u8 ucChannel                    = 0;
    u8 ucRegisterContent            = 0;
    u8 ucRegisterAddress            = 0;

    /* 1. Test  */
    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, ISL68221_I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS_ISL68221_0 );
    expect_value( __wrap_i2c_send, write_length, ISL68221_WRITE_LEN );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], ISL68221_PAGE_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 1 );

    ucStatus = ucISL68221ReadCurrent1( ISL68221_I2CNUM, SLAVE_ADDRESS_ISL68221_0, &fCurrentInmV );
    assert_true( ucStatus == XST_FAILURE );

    /* 2. Test failed i2c call  */
    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, ISL68221_I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS_ISL68221_0 );
    expect_value( __wrap_i2c_send, write_length, ISL68221_WRITE_LEN );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], ISL68221_PAGE_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    set_Buffer( &pucReadBuff[0], 0, 1360, 2 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send_rs_recv, i2c_num, ISL68221_I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, SLAVE_ADDRESS_ISL68221_0 );
    expect_value( __wrap_i2c_send_rs_recv, write_length, ISL68221_WRITE_LEN_1 );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], ISL68221_OUTPUT_CURRENT_REGISTER );
    expect_value( __wrap_i2c_send_rs_recv, read_length, ISL68221_READ_LEN );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 0 );

    ucStatus = ucISL68221ReadCurrent1( ISL68221_I2CNUM, SLAVE_ADDRESS_ISL68221_0, &fCurrentInmV );
    assert_true( ucStatus == XST_SUCCESS );

}

static void test_ucISL68221ReadCurrent2Fail( void **state )
{
	( void ) state; /* unused */

    u8 ucStatus                     = 0;
	unsigned char pucReadBuff[2]    = {0};
    float fCurrentInmV              = 0;
    u8 ucChannel                    = 0;
    u8 ucRegisterContent            = 0;
    u8 ucRegisterAddress            = 0;

    /* 1. Test NULL pointer */
    ucStatus = ucISL68221ReadCurrent2( ISL68221_I2CNUM, SLAVE_ADDRESS_ISL68221_0, NULL );
    assert_true( ucStatus == XST_FAILURE );

    /* 2. Test failed i2c call  */
    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, ISL68221_I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS_ISL68221_0 );
    expect_value( __wrap_i2c_send, write_length, ISL68221_WRITE_LEN );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], ISL68221_PAGE_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 1 );

    ucStatus = ucISL68221ReadCurrent2( ISL68221_I2CNUM, SLAVE_ADDRESS_ISL68221_0, &fCurrentInmV );
    assert_true( ucStatus == XST_FAILURE );

    /* 3. Test failed i2c call  */
    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, ISL68221_I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS_ISL68221_0 );
    expect_value( __wrap_i2c_send, write_length, ISL68221_WRITE_LEN );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], ISL68221_PAGE_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    set_Buffer( &pucReadBuff[0], 0, 1360, 2 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send_rs_recv, i2c_num, ISL68221_I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, SLAVE_ADDRESS_ISL68221_0 );
    expect_value( __wrap_i2c_send_rs_recv, write_length, ISL68221_WRITE_LEN_1 );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], ISL68221_OUTPUT_CURRENT_REGISTER );
    expect_value( __wrap_i2c_send_rs_recv, read_length, ISL68221_READ_LEN );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 1 );

    ucStatus = ucISL68221ReadCurrent2( ISL68221_I2CNUM, SLAVE_ADDRESS_ISL68221_0, &fCurrentInmV );
    assert_true( ucStatus == XST_FAILURE );

}

static void test_ucISL68221ReadCurrent2( void **state )
{
	( void ) state; /* unused */

    u8 ucStatus                     = 0;
	unsigned char pucReadBuff[2]    = {0};
    float fCurrentInmV              = 0;
    u8 ucChannel                    = 0;
    u8 ucRegisterContent            = 0;
    u8 ucRegisterAddress            = 0;

    /* 1. Test  */
    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, ISL68221_I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS_ISL68221_0 );
    expect_value( __wrap_i2c_send, write_length, ISL68221_WRITE_LEN );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], ISL68221_PAGE_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 1 );

    ucStatus = ucISL68221ReadCurrent2( ISL68221_I2CNUM, SLAVE_ADDRESS_ISL68221_0, &fCurrentInmV );
    assert_true( ucStatus == XST_FAILURE );

    /* 2. Test failed i2c call  */
    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, ISL68221_I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS_ISL68221_0 );
    expect_value( __wrap_i2c_send, write_length, ISL68221_WRITE_LEN );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], ISL68221_PAGE_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    set_Buffer( &pucReadBuff[0], 0, 1360, 2 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send_rs_recv, i2c_num, ISL68221_I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, SLAVE_ADDRESS_ISL68221_0 );
    expect_value( __wrap_i2c_send_rs_recv, write_length, ISL68221_WRITE_LEN_1 );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], ISL68221_OUTPUT_CURRENT_REGISTER );
    expect_value( __wrap_i2c_send_rs_recv, read_length, ISL68221_READ_LEN );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 0 );

    ucStatus = ucISL68221ReadCurrent2( ISL68221_I2CNUM, SLAVE_ADDRESS_ISL68221_0, &fCurrentInmV );
    assert_true( ucStatus == XST_SUCCESS );

}

/*************************************/
static void test_ucISL68221ReadTemperature0Fail( void **state )
{
	( void ) state; /* unused */

    u8 ucStatus                     = 0;
	unsigned char pucReadBuff[2]    = {0};
    float fTemperature              = 0;
    u8 ucChannel                    = 0;
    u8 ucRegisterContent            = 0;
    u8 ucRegisterAddress            = 0;

    /* 1. Test NULL pointer */
    ucStatus = ucISL68221ReadTemperature0( ISL68221_I2CNUM, SLAVE_ADDRESS_ISL68221_0, NULL );
    assert_true( ucStatus == XST_FAILURE );

    /* 2. Test failed i2c call  */
    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, ISL68221_I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS_ISL68221_0 );
    expect_value( __wrap_i2c_send, write_length, ISL68221_WRITE_LEN );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], ISL68221_PAGE_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 1 );

    ucStatus = ucISL68221ReadTemperature0( ISL68221_I2CNUM, SLAVE_ADDRESS_ISL68221_0, &fTemperature );
    assert_true( ucStatus == XST_FAILURE );

    /* 3. Test failed i2c call  */
    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, ISL68221_I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS_ISL68221_0 );
    expect_value( __wrap_i2c_send, write_length, ISL68221_WRITE_LEN );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], ISL68221_PAGE_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    set_Buffer( &pucReadBuff[0], 0, 1360, 2 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send_rs_recv, i2c_num, ISL68221_I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, SLAVE_ADDRESS_ISL68221_0 );
    expect_value( __wrap_i2c_send_rs_recv, write_length, ISL68221_WRITE_LEN_1 );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], ISL68221_READ_TEMPERATURE_0 );
    expect_value( __wrap_i2c_send_rs_recv, read_length, ISL68221_READ_LEN );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 1 );

    ucStatus = ucISL68221ReadTemperature0( ISL68221_I2CNUM, SLAVE_ADDRESS_ISL68221_0, &fTemperature );
    assert_true( ucStatus == XST_FAILURE );

}

static void test_ucISL68221ReadTemperature0( void **state )
{
	( void ) state; /* unused */

    u8 ucStatus                     = 0;
	unsigned char pucReadBuff[2]    = {0};
    float fTemperature              = 0;
    u8 ucChannel                    = 0;
    u8 ucRegisterContent            = 0;
    u8 ucRegisterAddress            = 0;

    /* 1. Test  */
    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, ISL68221_I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS_ISL68221_0 );
    expect_value( __wrap_i2c_send, write_length, ISL68221_WRITE_LEN );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], ISL68221_PAGE_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 1 );

    ucStatus = ucISL68221ReadTemperature0( ISL68221_I2CNUM, SLAVE_ADDRESS_ISL68221_0, &fTemperature );
    assert_true( ucStatus == XST_FAILURE );

    /* 2. Test failed i2c call  */
    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, ISL68221_I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS_ISL68221_0 );
    expect_value( __wrap_i2c_send, write_length, ISL68221_WRITE_LEN );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], ISL68221_PAGE_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    set_Buffer( &pucReadBuff[0], 0, 1360, 2 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send_rs_recv, i2c_num, ISL68221_I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, SLAVE_ADDRESS_ISL68221_0 );
    expect_value( __wrap_i2c_send_rs_recv, write_length, ISL68221_WRITE_LEN_1 );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], ISL68221_READ_TEMPERATURE_0 );
    expect_value( __wrap_i2c_send_rs_recv, read_length, ISL68221_READ_LEN );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 0 );

    ucStatus = ucISL68221ReadTemperature0( ISL68221_I2CNUM, SLAVE_ADDRESS_ISL68221_0, &fTemperature );
    assert_true( ucStatus == XST_SUCCESS );

}


static void test_ucISL68221ReadTemperature1Fail( void **state )
{
	( void ) state; /* unused */

    u8 ucStatus                     = 0;
	unsigned char pucReadBuff[2]    = {0};
    float fTemperature              = 0;
    u8 ucChannel                    = 0;
    u8 ucRegisterContent            = 0;
    u8 ucRegisterAddress            = 0;

    /* 1. Test NULL pointer */
    ucStatus = ucISL68221ReadTemperature1( ISL68221_I2CNUM, SLAVE_ADDRESS_ISL68221_0, NULL );
    assert_true( ucStatus == XST_FAILURE );

    /* 2. Test failed i2c call  */
    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, ISL68221_I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS_ISL68221_0 );
    expect_value( __wrap_i2c_send, write_length, ISL68221_WRITE_LEN );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], ISL68221_PAGE_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 1 );

    ucStatus = ucISL68221ReadTemperature1( ISL68221_I2CNUM, SLAVE_ADDRESS_ISL68221_0, &fTemperature );
    assert_true( ucStatus == XST_FAILURE );

    /* 3. Test failed i2c call  */
    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, ISL68221_I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS_ISL68221_0 );
    expect_value( __wrap_i2c_send, write_length, ISL68221_WRITE_LEN );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], ISL68221_PAGE_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    set_Buffer( &pucReadBuff[0], 0, 1360, 2 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send_rs_recv, i2c_num, ISL68221_I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, SLAVE_ADDRESS_ISL68221_0 );
    expect_value( __wrap_i2c_send_rs_recv, write_length, ISL68221_WRITE_LEN_1 );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], ISL68221_READ_TEMPERATURE_1 );
    expect_value( __wrap_i2c_send_rs_recv, read_length, ISL68221_READ_LEN );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 1 );

    ucStatus = ucISL68221ReadTemperature1( ISL68221_I2CNUM, SLAVE_ADDRESS_ISL68221_0, &fTemperature );
    assert_true( ucStatus == XST_FAILURE );
}

static void test_ucISL68221ReadTemperature1( void **state )
{
	( void ) state; /* unused */

    u8 ucStatus                     = 0;
	unsigned char pucReadBuff[2]    = {0};
    float fTemperature              = 0;
    u8 ucChannel                    = 0;
    u8 ucRegisterContent            = 0;
    u8 ucRegisterAddress            = 0;

    /* 1. Test  */
    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, ISL68221_I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS_ISL68221_0 );
    expect_value( __wrap_i2c_send, write_length, ISL68221_WRITE_LEN );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], ISL68221_PAGE_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 1 );

    ucStatus = ucISL68221ReadTemperature1( ISL68221_I2CNUM, SLAVE_ADDRESS_ISL68221_0, &fTemperature );
    assert_true( ucStatus == XST_FAILURE );

    /* 2. Test failed i2c call  */
    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, ISL68221_I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS_ISL68221_0 );
    expect_value( __wrap_i2c_send, write_length, ISL68221_WRITE_LEN );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], ISL68221_PAGE_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    set_Buffer( &pucReadBuff[0], 0, 1360, 2 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send_rs_recv, i2c_num, ISL68221_I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, SLAVE_ADDRESS_ISL68221_0 );
    expect_value( __wrap_i2c_send_rs_recv, write_length, ISL68221_WRITE_LEN_1 );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], ISL68221_READ_TEMPERATURE_1 );
    expect_value( __wrap_i2c_send_rs_recv, read_length, ISL68221_READ_LEN );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 0 );

    ucStatus = ucISL68221ReadTemperature1( ISL68221_I2CNUM, SLAVE_ADDRESS_ISL68221_0, &fTemperature );
    assert_true( ucStatus == XST_SUCCESS );

}

static void test_ucISL68221ReadTemperature2Fail( void **state )
{
	( void ) state; /* unused */

    u8 ucStatus                     = 0;
	unsigned char pucReadBuff[2]    = {0};
    float fTemperature              = 0;
    u8 ucChannel                    = 0;
    u8 ucRegisterContent            = 0;
    u8 ucRegisterAddress            = 0;

    /* 1. Test NULL pointer */
    ucStatus = ucISL68221ReadTemperature2( ISL68221_I2CNUM, SLAVE_ADDRESS_ISL68221_0, NULL );
    assert_true( ucStatus == XST_FAILURE );

    /* 2. Test failed i2c call  */
    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, ISL68221_I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS_ISL68221_0 );
    expect_value( __wrap_i2c_send, write_length, ISL68221_WRITE_LEN );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], ISL68221_PAGE_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 1 );

    ucStatus = ucISL68221ReadTemperature2( ISL68221_I2CNUM, SLAVE_ADDRESS_ISL68221_0, &fTemperature );
    assert_true( ucStatus == XST_FAILURE );

    /* 3. Test failed i2c call  */
    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, ISL68221_I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS_ISL68221_0 );
    expect_value( __wrap_i2c_send, write_length, ISL68221_WRITE_LEN );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], ISL68221_PAGE_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    set_Buffer( &pucReadBuff[0], 0, 1360, 2 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send_rs_recv, i2c_num, ISL68221_I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, SLAVE_ADDRESS_ISL68221_0 );
    expect_value( __wrap_i2c_send_rs_recv, write_length, ISL68221_WRITE_LEN_1 );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], ISL68221_READ_TEMPERATURE_2 );
    expect_value( __wrap_i2c_send_rs_recv, read_length, ISL68221_READ_LEN );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 1 );

    ucStatus = ucISL68221ReadTemperature2( ISL68221_I2CNUM, SLAVE_ADDRESS_ISL68221_0, &fTemperature );
    assert_true( ucStatus == XST_FAILURE );

}

static void test_ucISL68221ReadTemperature2( void **state )
{
	( void ) state; /* unused */

    u8 ucStatus                     = 0;
	unsigned char pucReadBuff[2]    = {0};
    float fTemperature              = 0;
    u8 ucChannel                    = 0;
    u8 ucRegisterContent            = 0;
    u8 ucRegisterAddress            = 0;

    /* 1. Test  */

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, ISL68221_I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS_ISL68221_0 );
    expect_value( __wrap_i2c_send, write_length, ISL68221_WRITE_LEN );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], ISL68221_PAGE_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 1 );

    ucStatus = ucISL68221ReadTemperature2( ISL68221_I2CNUM, SLAVE_ADDRESS_ISL68221_0, &fTemperature );
    assert_true( ucStatus == XST_FAILURE );

    /* 3. Test failed i2c call  */
    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, ISL68221_I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, SLAVE_ADDRESS_ISL68221_0 );
    expect_value( __wrap_i2c_send, write_length, ISL68221_WRITE_LEN );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], ISL68221_PAGE_REGISTER );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, 0 );

    set_Buffer( &pucReadBuff[0], 0, 1360, 2 );

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send_rs_recv, i2c_num, ISL68221_I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, SLAVE_ADDRESS_ISL68221_0 );
    expect_value( __wrap_i2c_send_rs_recv, write_length, ISL68221_WRITE_LEN_1 );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], ISL68221_READ_TEMPERATURE_2 );
    expect_value( __wrap_i2c_send_rs_recv, read_length, ISL68221_READ_LEN );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 0 );

    ucStatus = ucISL68221ReadTemperature2( ISL68221_I2CNUM, SLAVE_ADDRESS_ISL68221_0, &fTemperature );
    assert_true( ucStatus == XST_SUCCESS );

}

int main( void ) 
{
	srand( time( 0 ) );
	const struct CMUnitTest tests[] = 
    {
        
        cmocka_unit_test( test_ucISL68221WriteRegister ),
        cmocka_unit_test( test_ucISL68221ReadRegister ),
	    cmocka_unit_test( test_ucISL68221ReadVoltage0Fail ),
        cmocka_unit_test( test_ucISL68221ReadVoltage0 ),
        cmocka_unit_test( test_ucISL68221ReadVoltage1Fail ),
        cmocka_unit_test( test_ucISL68221ReadVoltage1 ),
        cmocka_unit_test( test_ucISL68221ReadVoltage2Fail ),
        cmocka_unit_test( test_ucISL68221ReadVoltage2 ),
        cmocka_unit_test( test_ucISL68221ReadCurrent0Fail ),
        cmocka_unit_test( test_ucISL68221ReadCurrent0 ),
        cmocka_unit_test( test_ucISL68221ReadCurrent1Fail ),
        cmocka_unit_test( test_ucISL68221ReadCurrent1 ),
        cmocka_unit_test( test_ucISL68221ReadCurrent2Fail ),
        cmocka_unit_test( test_ucISL68221ReadCurrent2 ),
        cmocka_unit_test( test_ucISL68221ReadTemperature0Fail ),
        cmocka_unit_test( test_ucISL68221ReadTemperature0 ),
        cmocka_unit_test( test_ucISL68221ReadTemperature1Fail ),
        cmocka_unit_test( test_ucISL68221ReadTemperature1 ),
        cmocka_unit_test( test_ucISL68221ReadTemperature2Fail ),
        cmocka_unit_test( test_ucISL68221ReadTemperature2 )
	};

	return cmocka_run_group_tests( tests, NULL, NULL );
}
