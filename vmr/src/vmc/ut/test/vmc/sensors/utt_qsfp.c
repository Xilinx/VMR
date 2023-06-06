
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
#include "qsfp.h"

/* I2C parameters for qsfp  */
#define QSFP_WRITE_LEN 1
#define QSFP_WRITE_BUF 0X05
#define QSFP_READ_LEN  2

#define QSFP_I2CNUM                     0x00
#define QSFP_SLAVE_ADDRESS              0x50
#define QSFP_IO_EXPANDER_ADDRESS        0x20
#define QSFP_MUX0_ADDRESS               0x70
#define QSFP_MUX1_ADDRESS               0x71
#define QSFP_IO_EXPANDER_INPUT_REG      ( 0 )
//#define QSFP_MODPRES_L_BIT              ( 8 )

/*Min & Max temperatures for se98a */
#define SE98A_POSITIVE_MIN_TEMP 0
#define SE98A_POSITIVE_MAX_TEMP 85
#define SE98A_NEGATIVE_MIN_TEMP -128
#define SE98A_NEGATIVE_MAX_TEMP -1

static void vTestQSFPI2CMuxReadTemperature( void **state )
{
    u8 ucSensorInstance                 = 0;
    u8 ucStatus                         = 0;
    float fTemperatureValue             = 0;
    float* pfTemperatureValue           = NULL;
    u8 ucMuxControlRegisterIOExpander   = 0;
    u8 ucMuxControlRegisterQSFP         = 0;
    u8 ucMuxAddress                     = 0;

    /* 1 Test function NULL pointer */
    ucStatus = ucQSFPI2CMuxReadTemperature( NULL, ucSensorInstance );
    assert_true( ucStatus == XST_FAILURE );

    /* 2 Test function Invalid Instance */
    ucSensorInstance                = 5;

    ucStatus = ucQSFPI2CMuxReadTemperature( &fTemperatureValue, ucSensorInstance );
    assert_true( ucStatus == XST_FAILURE );

    /* 3 Test function  */
    ucSensorInstance                = 0;
    ucMuxAddress                      = QSFP_MUX0_ADDRESS;
    ucMuxControlRegisterIOExpander    = 1 << 0;
    ucMuxControlRegisterQSFP          = 1 << 1;

    /*Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, QSFP_MUX0_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], ucMuxControlRegisterIOExpander );

    /*set return value for mock function*/
    will_return( __wrap_i2c_send, XST_FAILURE );

    ucStatus = ucQSFPI2CMuxReadTemperature( &fTemperatureValue, ucSensorInstance );
    assert_true( ucStatus == XST_FAILURE );
    
}

static void vTestQSFPI2CMuxReadTemperature0( void **state )
{
    u8 ucSensorInstance     = 0;
    u8 ucStatus             = 0;
    float fTemperatureValue = 0;
    unsigned char pucReadBuff[4] = {0};

    u8 ucMuxControlRegisterIOExpander     = 0;
    u8 ucMuxControlRegisterQSFP           = 0;
    u8 ucMuxAddress                       = 0;
    
    /* 3 i2c_send SUCCESS, i2c_send_rs_recv FAIL */
    ucMuxAddress = QSFP_MUX0_ADDRESS;
    ucMuxControlRegisterIOExpander = 1 << 0;
    ucMuxControlRegisterQSFP = 1 << 1;
    u8 MyValues[3] = {8, 0, 35};

    /* Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, QSFP_MUX0_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], ucMuxControlRegisterIOExpander );

    /* set return value for mock function*/
    will_return( __wrap_i2c_send, XST_SUCCESS );

    /* Check mock function parameters*/
    expect_value( __wrap_i2c_send_rs_recv, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, QSFP_IO_EXPANDER_ADDRESS );
    expect_value( __wrap_i2c_send_rs_recv, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], QSFP_IO_EXPANDER_INPUT_REG );
    expect_value( __wrap_i2c_send_rs_recv, read_length, 1 );
       
    /* set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 1 );

    ucStatus = ucQSFPI2CMuxReadTemperature( &fTemperatureValue, ucSensorInstance );
    assert_true( ucStatus == XST_FAILURE );

    /* 4 i2c_send SUCCESS, i2c_send_rs_recv SUCCESS, Module not present*/

    /* Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, QSFP_MUX0_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], ucMuxControlRegisterIOExpander );

    /* set return value for mock function*/
    will_return( __wrap_i2c_send, XST_SUCCESS );
    /* Check mock function parameters*/
    /* Set bit to make module appear missing */
    MyValues[0] = QSFP_MODPRES_L_BIT;
    vSetReadBuffer( &pucReadBuff[0], MyValues, 1 );

    expect_value( __wrap_i2c_send_rs_recv, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, QSFP_IO_EXPANDER_ADDRESS );
    expect_value( __wrap_i2c_send_rs_recv, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], QSFP_IO_EXPANDER_INPUT_REG );
    expect_value( __wrap_i2c_send_rs_recv, read_length, 1 );
       
    /* set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 0 );    

    ucStatus = ucQSFPI2CMuxReadTemperature( &fTemperatureValue, ucSensorInstance );
    assert_true( ucStatus == XST_DEVICE_NOT_FOUND );

    /* 5 i2c_send SUCCESS, i2c_send_rs_recv SUCCESS, Module present send fail */

    /* Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, QSFP_MUX0_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], ucMuxControlRegisterIOExpander );

    /* set return value for mock function*/
    will_return( __wrap_i2c_send, XST_SUCCESS );
    /* Check mock function parameters*/
    /* Set bit to make module appear missing */
    MyValues[0] = 5;
    vSetReadBuffer( &pucReadBuff[0], MyValues, 1 );

    expect_value( __wrap_i2c_send_rs_recv, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, QSFP_IO_EXPANDER_ADDRESS );
    expect_value( __wrap_i2c_send_rs_recv, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], QSFP_IO_EXPANDER_INPUT_REG );
    expect_value( __wrap_i2c_send_rs_recv, read_length, 1 );
       
    /* set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 0 );    

    /* Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, QSFP_MUX0_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], ucMuxControlRegisterQSFP );

    /* set return value for mock function*/
    will_return( __wrap_i2c_send, XST_FAILURE );

    ucStatus = ucQSFPI2CMuxReadTemperature( &fTemperatureValue, ucSensorInstance );
    assert_true( ucStatus == XST_FAILURE );

    /* 6 i2c_send SUCCESS, i2c_send_rs_recv SUCCESS, Module present send success  
        i2c_send_rs_recv fail*/

    /* Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, QSFP_MUX0_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], ucMuxControlRegisterIOExpander );

    /* set return value for mock function*/
    will_return( __wrap_i2c_send, XST_SUCCESS );
    /* Check mock function parameters*/
    /* Set bit to make module appear missing */
    MyValues[0] = 6;
    MyValues[1] = 1;
    vSetReadBuffer( &pucReadBuff[0], MyValues, 2 );

    expect_value( __wrap_i2c_send_rs_recv, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, QSFP_IO_EXPANDER_ADDRESS );
    expect_value( __wrap_i2c_send_rs_recv, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], QSFP_IO_EXPANDER_INPUT_REG );
    expect_value( __wrap_i2c_send_rs_recv, read_length, 1 );
       
    /* set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 0 );    

    /* Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, QSFP_MUX0_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], ucMuxControlRegisterQSFP );

    /* set return value for mock function*/
    will_return( __wrap_i2c_send, XST_SUCCESS );

    //vSetReadBuffer( &pucReadBuff[0], 0, 1 );

    expect_value( __wrap_i2c_send_rs_recv, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, QSFP_SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send_rs_recv, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], QSFP_LSB_TEMPERATURE_REG );
    expect_value( __wrap_i2c_send_rs_recv, read_length, 1 );
    
    /* set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 1 );

    ucStatus = ucQSFPI2CMuxReadTemperature( &fTemperatureValue, ucSensorInstance );
    assert_true( ucStatus == XST_FAILURE );

    /* 7 i2c_send SUCCESS, i2c_send_rs_recv SUCCESS, Module present send success  
        i2c_send_rs_recv SUCCESS i2c_send_rs_recv FAIL */

    /* Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, QSFP_MUX0_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], ucMuxControlRegisterIOExpander );

    /* set return value for mock function*/
    will_return( __wrap_i2c_send, XST_SUCCESS );
    /* Check mock function parameters*/
    /* Set bit to make module appear missing */
    MyValues[0] = 7;
    MyValues[1] = 2;
    MyValues[2] = 3;
    vSetReadBuffer( &pucReadBuff[0], MyValues, 3 );

    expect_value( __wrap_i2c_send_rs_recv, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, QSFP_IO_EXPANDER_ADDRESS );
    expect_value( __wrap_i2c_send_rs_recv, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], QSFP_IO_EXPANDER_INPUT_REG );
    expect_value( __wrap_i2c_send_rs_recv, read_length, 1 );
       
    /* set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 0 );    

    /* Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, QSFP_MUX0_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], ucMuxControlRegisterQSFP );

    /* set return value for mock function*/
    will_return( __wrap_i2c_send, XST_SUCCESS );

    //vSetReadBuffer( &pucReadBuff[0], 0, 1 );

    expect_value( __wrap_i2c_send_rs_recv, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, QSFP_SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send_rs_recv, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], QSFP_LSB_TEMPERATURE_REG );
    expect_value( __wrap_i2c_send_rs_recv, read_length, 1 );
    
    /* set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 0 );

    //vSetReadBuffer( &pucReadBuff[0], 0, 1 );

    expect_value( __wrap_i2c_send_rs_recv, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, QSFP_SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send_rs_recv, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], QSFP_MSB_TEMPERATURE_REG );
    expect_value( __wrap_i2c_send_rs_recv, read_length, 1 );
    
    /* set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 1 );

    ucStatus = ucQSFPI2CMuxReadTemperature( &fTemperatureValue, ucSensorInstance );
    assert_true( ucStatus == XST_FAILURE );

    /* 8 i2c_send SUCCESS, i2c_send_rs_recv SUCCESS, Module present send success  
        i2c_send_rs_recv SUCCESS i2c_send_rs_recv SUCCESS Negative temperature */
    ucMuxAddress = QSFP_MUX0_ADDRESS;
    ucMuxControlRegisterIOExpander = 1 << 0;
    ucMuxControlRegisterQSFP = 1 << 1;

    /* Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, QSFP_MUX0_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], ucMuxControlRegisterIOExpander );

    /* set return value for mock function*/
    will_return( __wrap_i2c_send, XST_SUCCESS );
    /* Check mock function parameters*/
    /* Set bit to make module appear missing */
    MyValues[0] = 6;
    MyValues[1] = 3;
    MyValues[2] = ( 0xA3 );
    vSetReadBuffer( &pucReadBuff[0], MyValues, 3 );

    expect_value( __wrap_i2c_send_rs_recv, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, QSFP_IO_EXPANDER_ADDRESS );
    expect_value( __wrap_i2c_send_rs_recv, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], QSFP_IO_EXPANDER_INPUT_REG );
    expect_value( __wrap_i2c_send_rs_recv, read_length, 1 );
       
    /* set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 0 );    

    /* Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, QSFP_MUX0_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], ucMuxControlRegisterQSFP );

    /* set return value for mock function*/
    will_return( __wrap_i2c_send, XST_SUCCESS );

    //vSetReadBuffer( &pucReadBuff[0], 0, 1 );

    expect_value( __wrap_i2c_send_rs_recv, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, QSFP_SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send_rs_recv, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], QSFP_LSB_TEMPERATURE_REG );
    expect_value( __wrap_i2c_send_rs_recv, read_length, 1 );
    
    /* set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 0 );

    //vSetReadBuffer( &pucReadBuff[0], ( 0x80 | 35 ), 1 );

    expect_value( __wrap_i2c_send_rs_recv, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, QSFP_SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send_rs_recv, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], QSFP_MSB_TEMPERATURE_REG );
    expect_value( __wrap_i2c_send_rs_recv, read_length, 1 );
    
    /* set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 0 );

    ucStatus = ucQSFPI2CMuxReadTemperature( &fTemperatureValue, ucSensorInstance );
    assert_true( ucStatus == XST_SUCCESS );
    assert_true( fTemperatureValue == ( float )-92.988281 );

    /* 9 i2c_send SUCCESS, i2c_send_rs_recv SUCCESS, Module present send success  
        i2c_send_rs_recv SUCCESS i2c_send_rs_recv SUCCESS Positive temperature */

    /* Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, QSFP_MUX0_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], ucMuxControlRegisterIOExpander );

    /* set return value for mock function*/
    will_return( __wrap_i2c_send, XST_SUCCESS );
    /* Check mock function parameters*/
    /* Set bit to make module appear missing */
    MyValues[0] = 5;
    MyValues[1] = 4;
    MyValues[2] = ( 35 );
    vSetReadBuffer( &pucReadBuff[0], MyValues, 3 );

    expect_value( __wrap_i2c_send_rs_recv, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, QSFP_IO_EXPANDER_ADDRESS );
    expect_value( __wrap_i2c_send_rs_recv, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], QSFP_IO_EXPANDER_INPUT_REG );
    expect_value( __wrap_i2c_send_rs_recv, read_length, 1 );
       
    /* set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 0 );    

    /* Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, QSFP_MUX0_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], ucMuxControlRegisterQSFP );

    /* set return value for mock function*/
    will_return( __wrap_i2c_send, XST_SUCCESS );

    //vSetReadBuffer( pucReadBuff, 0, 1 );

    expect_value( __wrap_i2c_send_rs_recv, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, QSFP_SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send_rs_recv, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], QSFP_LSB_TEMPERATURE_REG );
    expect_value( __wrap_i2c_send_rs_recv, read_length, 1 );
    
    /* set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 0 );

    //vSetReadBuffer( pucReadBuff, 35, 1 );    /* 35 degree*/

    expect_value( __wrap_i2c_send_rs_recv, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, QSFP_SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send_rs_recv, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], QSFP_MSB_TEMPERATURE_REG );
    expect_value( __wrap_i2c_send_rs_recv, read_length, 1 );
    
    /* set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 0 );

    ucStatus = ucQSFPI2CMuxReadTemperature( &fTemperatureValue, ucSensorInstance );
    assert_true( ucStatus == XST_SUCCESS );
    assert_true( fTemperatureValue == 35.015625 );
}

static void vTestQSFPI2CMuxReadTemperature1( void **state )
{
    u8 ucSensorInstance     = 1;
    u8 ucStatus             = 0;
    float fTemperatureValue = 0;
    unsigned char pucReadBuff[4] = {0};

    u8 ucMuxControlRegisterIOExpander     = 0;
    u8 ucMuxControlRegisterQSFP           = 0;
    u8 ucMuxAddress                       = 0;
    
    /* 3 i2c_send SUCCESS, i2c_send_rs_recv FAIL */
    ucMuxAddress = QSFP_MUX0_ADDRESS;
    ucMuxControlRegisterIOExpander = 1 << 2;
    ucMuxControlRegisterQSFP = 1 << 3;
    u8 MyValues[3] = {8, 0, 35};

    /* Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, ucMuxAddress );
    expect_value( __wrap_i2c_send, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], ucMuxControlRegisterIOExpander );

    /* set return value for mock function*/
    will_return( __wrap_i2c_send, XST_SUCCESS );

    /* Check mock function parameters*/
    expect_value( __wrap_i2c_send_rs_recv, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, QSFP_IO_EXPANDER_ADDRESS );
    expect_value( __wrap_i2c_send_rs_recv, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], QSFP_IO_EXPANDER_INPUT_REG );
    expect_value( __wrap_i2c_send_rs_recv, read_length, 1 );
       
    /* set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 1 );

    ucStatus = ucQSFPI2CMuxReadTemperature( &fTemperatureValue, ucSensorInstance );
    assert_true( ucStatus == XST_FAILURE );

    /* 4 i2c_send SUCCESS, i2c_send_rs_recv SUCCESS, Module not present*/

    /* Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, ucMuxAddress );
    expect_value( __wrap_i2c_send, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], ucMuxControlRegisterIOExpander );

    /* set return value for mock function*/
    will_return( __wrap_i2c_send, XST_SUCCESS );
    /* Check mock function parameters*/
    /* Set bit to make module appear missing */
    MyValues[0] = QSFP_MODPRES_L_BIT;
    vSetReadBuffer( &pucReadBuff[0], MyValues, 1 );

    expect_value( __wrap_i2c_send_rs_recv, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, QSFP_IO_EXPANDER_ADDRESS );
    expect_value( __wrap_i2c_send_rs_recv, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], QSFP_IO_EXPANDER_INPUT_REG );
    expect_value( __wrap_i2c_send_rs_recv, read_length, 1 );
       
    /* set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 0 );    

    ucStatus = ucQSFPI2CMuxReadTemperature( &fTemperatureValue, ucSensorInstance );
    assert_true( ucStatus == XST_DEVICE_NOT_FOUND );

    /* 5 i2c_send SUCCESS, i2c_send_rs_recv SUCCESS, Module present send fail */

    /* Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, ucMuxAddress );
    expect_value( __wrap_i2c_send, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], ucMuxControlRegisterIOExpander );

    /* set return value for mock function*/
    will_return( __wrap_i2c_send, XST_SUCCESS );
    /* Check mock function parameters*/
    /* Set bit to make module appear missing */
    MyValues[0] = 5;
    vSetReadBuffer( &pucReadBuff[0], MyValues, 1 );

    expect_value( __wrap_i2c_send_rs_recv, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, QSFP_IO_EXPANDER_ADDRESS );
    expect_value( __wrap_i2c_send_rs_recv, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], QSFP_IO_EXPANDER_INPUT_REG );
    expect_value( __wrap_i2c_send_rs_recv, read_length, 1 );
       
    /* set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 0 );    

    /* Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, ucMuxAddress );
    expect_value( __wrap_i2c_send, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], ucMuxControlRegisterQSFP );

    /* set return value for mock function*/
    will_return( __wrap_i2c_send, XST_FAILURE );

    ucStatus = ucQSFPI2CMuxReadTemperature( &fTemperatureValue, ucSensorInstance );
    assert_true( ucStatus == XST_FAILURE );

    /* 6 i2c_send SUCCESS, i2c_send_rs_recv SUCCESS, Module present send success  
        i2c_send_rs_recv fail*/

    /* Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, ucMuxAddress );
    expect_value( __wrap_i2c_send, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], ucMuxControlRegisterIOExpander );

    /* set return value for mock function*/
    will_return( __wrap_i2c_send, XST_SUCCESS );
    /* Check mock function parameters*/
    /* Set bit to make module appear missing */
    MyValues[0] = 6;
    MyValues[1] = 1;
    vSetReadBuffer( &pucReadBuff[0], MyValues, 2 );

    expect_value( __wrap_i2c_send_rs_recv, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, QSFP_IO_EXPANDER_ADDRESS );
    expect_value( __wrap_i2c_send_rs_recv, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], QSFP_IO_EXPANDER_INPUT_REG );
    expect_value( __wrap_i2c_send_rs_recv, read_length, 1 );
       
    /* set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 0 );    

    /* Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, ucMuxAddress );
    expect_value( __wrap_i2c_send, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], ucMuxControlRegisterQSFP );

    /* set return value for mock function*/
    will_return( __wrap_i2c_send, XST_SUCCESS );

    expect_value( __wrap_i2c_send_rs_recv, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, QSFP_SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send_rs_recv, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], QSFP_LSB_TEMPERATURE_REG );
    expect_value( __wrap_i2c_send_rs_recv, read_length, 1 );
    
    /* set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 1 );


    ucStatus = ucQSFPI2CMuxReadTemperature( &fTemperatureValue, ucSensorInstance );
    assert_true( ucStatus == XST_FAILURE );

    /* 7 i2c_send SUCCESS, i2c_send_rs_recv SUCCESS, Module present send success  
        i2c_send_rs_recv SUCCESS i2c_send_rs_recv FAIL */

    /* Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, ucMuxAddress );
    expect_value( __wrap_i2c_send, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], ucMuxControlRegisterIOExpander );

    /* set return value for mock function*/
    will_return( __wrap_i2c_send, XST_SUCCESS );
    /* Check mock function parameters*/
    /* Set bit to make module appear missing */
    MyValues[0] = 7;
    MyValues[1] = 2;
    MyValues[2] = 3;
    vSetReadBuffer( &pucReadBuff[0], MyValues, 3 );

    expect_value( __wrap_i2c_send_rs_recv, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, QSFP_IO_EXPANDER_ADDRESS );
    expect_value( __wrap_i2c_send_rs_recv, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], QSFP_IO_EXPANDER_INPUT_REG );
    expect_value( __wrap_i2c_send_rs_recv, read_length, 1 );
       
    /* set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 0 );    

    /* Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, ucMuxAddress );
    expect_value( __wrap_i2c_send, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], ucMuxControlRegisterQSFP );

    /* set return value for mock function*/
    will_return( __wrap_i2c_send, XST_SUCCESS );

    //vSetReadBuffer( &pucReadBuff[0], 0, 1 );

    expect_value( __wrap_i2c_send_rs_recv, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, QSFP_SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send_rs_recv, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], QSFP_LSB_TEMPERATURE_REG );
    expect_value( __wrap_i2c_send_rs_recv, read_length, 1 );
    
    /* set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 0 );

    //vSetReadBuffer( &pucReadBuff[0], 0, 1 );

    expect_value( __wrap_i2c_send_rs_recv, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, QSFP_SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send_rs_recv, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], QSFP_MSB_TEMPERATURE_REG );
    expect_value( __wrap_i2c_send_rs_recv, read_length, 1 );
    
    /* set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 1 );

    ucStatus = ucQSFPI2CMuxReadTemperature( &fTemperatureValue, ucSensorInstance );
    assert_true( ucStatus == XST_FAILURE );

    /* 8 i2c_send SUCCESS, i2c_send_rs_recv SUCCESS, Module present send success  
        i2c_send_rs_recv SUCCESS i2c_send_rs_recv SUCCESS Negative temperature */

    /* Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, ucMuxAddress );
    expect_value( __wrap_i2c_send, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], ucMuxControlRegisterIOExpander );

    /* set return value for mock function*/
    will_return( __wrap_i2c_send, XST_SUCCESS );
    /* Check mock function parameters*/
    /* Set bit to make module appear missing */
    MyValues[0] = 6;
    MyValues[1] = 3;
    MyValues[2] = ( 0xA3 );
    vSetReadBuffer( &pucReadBuff[0], MyValues, 3 );

    expect_value( __wrap_i2c_send_rs_recv, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, QSFP_IO_EXPANDER_ADDRESS );
    expect_value( __wrap_i2c_send_rs_recv, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], QSFP_IO_EXPANDER_INPUT_REG );
    expect_value( __wrap_i2c_send_rs_recv, read_length, 1 );
       
    /* set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 0 );    

    /* Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, ucMuxAddress );
    expect_value( __wrap_i2c_send, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], ucMuxControlRegisterQSFP );

    /* set return value for mock function*/
    will_return( __wrap_i2c_send, XST_SUCCESS );

    //vSetReadBuffer( &pucReadBuff[0], 0, 1 );

    expect_value( __wrap_i2c_send_rs_recv, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, QSFP_SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send_rs_recv, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], QSFP_LSB_TEMPERATURE_REG );
    expect_value( __wrap_i2c_send_rs_recv, read_length, 1 );
    
    /* set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 0 );

    //vSetReadBuffer( &pucReadBuff[0], ( 0x80 | 35 ), 1 );

    expect_value( __wrap_i2c_send_rs_recv, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, QSFP_SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send_rs_recv, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], QSFP_MSB_TEMPERATURE_REG );
    expect_value( __wrap_i2c_send_rs_recv, read_length, 1 );
    
    /* set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 0 );

    ucStatus = ucQSFPI2CMuxReadTemperature( &fTemperatureValue, ucSensorInstance );
    assert_true( ucStatus == XST_SUCCESS );
    assert_true( fTemperatureValue == ( float )-92.988281 );

    /* 9 i2c_send SUCCESS, i2c_send_rs_recv SUCCESS, Module present send success  
        i2c_send_rs_recv SUCCESS i2c_send_rs_recv SUCCESS Positive temperature */

    /* Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, ucMuxAddress );
    expect_value( __wrap_i2c_send, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], ucMuxControlRegisterIOExpander );

    /* set return value for mock function*/
    will_return( __wrap_i2c_send, XST_SUCCESS );
    /* Check mock function parameters*/
    /* Set bit to make module appear missing */
    MyValues[0] = 5;
    MyValues[1] = 4;
    MyValues[2] = ( 35 );
    vSetReadBuffer( &pucReadBuff[0], MyValues, 3 );

    expect_value( __wrap_i2c_send_rs_recv, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, QSFP_IO_EXPANDER_ADDRESS );
    expect_value( __wrap_i2c_send_rs_recv, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], QSFP_IO_EXPANDER_INPUT_REG );
    expect_value( __wrap_i2c_send_rs_recv, read_length, 1 );
       
    /* set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 0 );    

    /* Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, ucMuxAddress );
    expect_value( __wrap_i2c_send, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], ucMuxControlRegisterQSFP );

    /* set return value for mock function*/
    will_return( __wrap_i2c_send, XST_SUCCESS );

    //vSetReadBuffer( pucReadBuff, 0, 1 );

    expect_value( __wrap_i2c_send_rs_recv, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, QSFP_SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send_rs_recv, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], QSFP_LSB_TEMPERATURE_REG );
    expect_value( __wrap_i2c_send_rs_recv, read_length, 1 );
    
    /* set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 0 );

    //vSetReadBuffer( pucReadBuff, 35, 1 );    /* 35 degree*/

    expect_value( __wrap_i2c_send_rs_recv, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, QSFP_SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send_rs_recv, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], QSFP_MSB_TEMPERATURE_REG );
    expect_value( __wrap_i2c_send_rs_recv, read_length, 1 );
    
    /* set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 0 );

    ucStatus = ucQSFPI2CMuxReadTemperature( &fTemperatureValue, ucSensorInstance );
    assert_true( ucStatus == XST_SUCCESS );
    assert_true( fTemperatureValue == 35.015625 );
}

static void vTestQSFPI2CMuxReadTemperature2( void **state )
{
    u8 ucSensorInstance                 = 2;
    u8 ucStatus                         = 0;
    float fTemperatureValue             = 0;
    unsigned char pucReadBuff[4]        = {0};
    u8 ucMuxControlRegisterIOExpander   = 0;
    u8 ucMuxControlRegisterQSFP         = 0;
    u8 ucMuxAddress                     = 0;
    
    /* 3 i2c_send SUCCESS, i2c_send_rs_recv FAIL */
    ucMuxAddress = QSFP_MUX1_ADDRESS;
    ucMuxControlRegisterIOExpander = 1 << 0;
    ucMuxControlRegisterQSFP = 1 << 1;
    u8 MyValues[3] = {8, 0, 35};

    /* Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, ucMuxAddress );
    expect_value( __wrap_i2c_send, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], ucMuxControlRegisterIOExpander );

    /* set return value for mock function*/
    will_return( __wrap_i2c_send, XST_SUCCESS );

    /* Check mock function parameters*/
    expect_value( __wrap_i2c_send_rs_recv, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, QSFP_IO_EXPANDER_ADDRESS );
    expect_value( __wrap_i2c_send_rs_recv, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], QSFP_IO_EXPANDER_INPUT_REG );
    expect_value( __wrap_i2c_send_rs_recv, read_length, 1 );
       
    /* set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 1 );

    ucStatus = ucQSFPI2CMuxReadTemperature( &fTemperatureValue, ucSensorInstance );
    assert_true( ucStatus == XST_FAILURE );

    /* 4 i2c_send SUCCESS, i2c_send_rs_recv SUCCESS, Module not present*/

    /* Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, ucMuxAddress );
    expect_value( __wrap_i2c_send, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], ucMuxControlRegisterIOExpander );

    /* set return value for mock function*/
    will_return( __wrap_i2c_send, XST_SUCCESS );
    /* Check mock function parameters*/
    /* Set bit to make module appear missing */
    MyValues[0] = QSFP_MODPRES_L_BIT;
    vSetReadBuffer( &pucReadBuff[0], MyValues, 1 );

    expect_value( __wrap_i2c_send_rs_recv, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, QSFP_IO_EXPANDER_ADDRESS );
    expect_value( __wrap_i2c_send_rs_recv, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], QSFP_IO_EXPANDER_INPUT_REG );
    expect_value( __wrap_i2c_send_rs_recv, read_length, 1 );
       
    /* set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 0 );    

    ucStatus = ucQSFPI2CMuxReadTemperature( &fTemperatureValue, ucSensorInstance );
    assert_true( ucStatus == XST_DEVICE_NOT_FOUND );

    /* 5 i2c_send SUCCESS, i2c_send_rs_recv SUCCESS, Module present send fail */

    /* Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, ucMuxAddress );
    expect_value( __wrap_i2c_send, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], ucMuxControlRegisterIOExpander );

    /* set return value for mock function*/
    will_return( __wrap_i2c_send, XST_SUCCESS );
    /* Check mock function parameters*/
    /* Set bit to make module appear missing */
    MyValues[0] = 5;
    vSetReadBuffer( &pucReadBuff[0], MyValues, 1 );

    expect_value( __wrap_i2c_send_rs_recv, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, QSFP_IO_EXPANDER_ADDRESS );
    expect_value( __wrap_i2c_send_rs_recv, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], QSFP_IO_EXPANDER_INPUT_REG );
    expect_value( __wrap_i2c_send_rs_recv, read_length, 1 );
       
    /* set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 0 );    

    /* Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, ucMuxAddress );
    expect_value( __wrap_i2c_send, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], ucMuxControlRegisterQSFP );

    /* set return value for mock function*/
    will_return( __wrap_i2c_send, XST_FAILURE );

    ucStatus = ucQSFPI2CMuxReadTemperature( &fTemperatureValue, ucSensorInstance );
    assert_true( ucStatus == XST_FAILURE );

    /* 6 i2c_send SUCCESS, i2c_send_rs_recv SUCCESS, Module present send success  
        i2c_send_rs_recv fail*/

    /* Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, ucMuxAddress );
    expect_value( __wrap_i2c_send, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], ucMuxControlRegisterIOExpander );

    /* set return value for mock function*/
    will_return( __wrap_i2c_send, XST_SUCCESS );
    /* Check mock function parameters*/
    /* Set bit to make module appear missing */
    MyValues[0] = 6;
    MyValues[1] = 1;
    vSetReadBuffer( &pucReadBuff[0], MyValues, 2 );

    expect_value( __wrap_i2c_send_rs_recv, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, QSFP_IO_EXPANDER_ADDRESS );
    expect_value( __wrap_i2c_send_rs_recv, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], QSFP_IO_EXPANDER_INPUT_REG );
    expect_value( __wrap_i2c_send_rs_recv, read_length, 1 );
       
    /* set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 0 );    

    /* Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, ucMuxAddress );
    expect_value( __wrap_i2c_send, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], ucMuxControlRegisterQSFP );

    /* set return value for mock function*/
    will_return( __wrap_i2c_send, XST_SUCCESS );

    expect_value( __wrap_i2c_send_rs_recv, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, QSFP_SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send_rs_recv, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], QSFP_LSB_TEMPERATURE_REG );
    expect_value( __wrap_i2c_send_rs_recv, read_length, 1 );
    
    /* set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 1 );


    ucStatus = ucQSFPI2CMuxReadTemperature( &fTemperatureValue, ucSensorInstance );
    assert_true( ucStatus == XST_FAILURE );

    /* 7 i2c_send SUCCESS, i2c_send_rs_recv SUCCESS, Module present send success  
        i2c_send_rs_recv SUCCESS i2c_send_rs_recv FAIL */

    /* Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, ucMuxAddress );
    expect_value( __wrap_i2c_send, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], ucMuxControlRegisterIOExpander );

    /* set return value for mock function*/
    will_return( __wrap_i2c_send, XST_SUCCESS );
    /* Check mock function parameters*/
    /* Set bit to make module appear missing */
    MyValues[0] = 7;
    MyValues[1] = 2;
    MyValues[2] = 3;
    vSetReadBuffer( &pucReadBuff[0], MyValues, 3 );

    expect_value( __wrap_i2c_send_rs_recv, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, QSFP_IO_EXPANDER_ADDRESS );
    expect_value( __wrap_i2c_send_rs_recv, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], QSFP_IO_EXPANDER_INPUT_REG );
    expect_value( __wrap_i2c_send_rs_recv, read_length, 1 );
       
    /* set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 0 );    

    /* Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, ucMuxAddress );
    expect_value( __wrap_i2c_send, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], ucMuxControlRegisterQSFP );

    /* set return value for mock function*/
    will_return( __wrap_i2c_send, XST_SUCCESS );

    expect_value( __wrap_i2c_send_rs_recv, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, QSFP_SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send_rs_recv, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], QSFP_LSB_TEMPERATURE_REG );
    expect_value( __wrap_i2c_send_rs_recv, read_length, 1 );
    
    /* set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 0 );

    expect_value( __wrap_i2c_send_rs_recv, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, QSFP_SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send_rs_recv, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], QSFP_MSB_TEMPERATURE_REG );
    expect_value( __wrap_i2c_send_rs_recv, read_length, 1 );
    
    /* set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 1 );

    ucStatus = ucQSFPI2CMuxReadTemperature( &fTemperatureValue, ucSensorInstance );
    assert_true( ucStatus == XST_FAILURE );

    /* 8 i2c_send SUCCESS, i2c_send_rs_recv SUCCESS, Module present send success  
        i2c_send_rs_recv SUCCESS i2c_send_rs_recv SUCCESS Negative temperature */

    /* Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, ucMuxAddress );
    expect_value( __wrap_i2c_send, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], ucMuxControlRegisterIOExpander );

    /* set return value for mock function*/
    will_return( __wrap_i2c_send, XST_SUCCESS );
    /* Check mock function parameters*/
    /* Set bit to make module appear missing */
    MyValues[0] = 6;
    MyValues[1] = 3;
    MyValues[2] = ( 0xA3 );
    vSetReadBuffer( &pucReadBuff[0], MyValues, 3 );

    expect_value( __wrap_i2c_send_rs_recv, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, QSFP_IO_EXPANDER_ADDRESS );
    expect_value( __wrap_i2c_send_rs_recv, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], QSFP_IO_EXPANDER_INPUT_REG );
    expect_value( __wrap_i2c_send_rs_recv, read_length, 1 );
       
    /* set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 0 );    

    /* Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, ucMuxAddress );
    expect_value( __wrap_i2c_send, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], ucMuxControlRegisterQSFP );

    /* set return value for mock function*/
    will_return( __wrap_i2c_send, XST_SUCCESS );

    expect_value( __wrap_i2c_send_rs_recv, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, QSFP_SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send_rs_recv, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], QSFP_LSB_TEMPERATURE_REG );
    expect_value( __wrap_i2c_send_rs_recv, read_length, 1 );
    
    /* set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 0 );

    expect_value( __wrap_i2c_send_rs_recv, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, QSFP_SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send_rs_recv, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], QSFP_MSB_TEMPERATURE_REG );
    expect_value( __wrap_i2c_send_rs_recv, read_length, 1 );
    
    /* set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 0 );

    ucStatus = ucQSFPI2CMuxReadTemperature( &fTemperatureValue, ucSensorInstance );
    assert_true( ucStatus == XST_SUCCESS );
    assert_true( fTemperatureValue == ( float )-92.988281 );

    /* 9 i2c_send SUCCESS, i2c_send_rs_recv SUCCESS, Module present send success  
        i2c_send_rs_recv SUCCESS i2c_send_rs_recv SUCCESS Positive temperature */

    /* Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, ucMuxAddress );
    expect_value( __wrap_i2c_send, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], ucMuxControlRegisterIOExpander );

    /* set return value for mock function*/
    will_return( __wrap_i2c_send, XST_SUCCESS );
    /* Check mock function parameters*/
    /* Set bit to make module appear missing */
    MyValues[0] = 5;
    MyValues[1] = 4;
    MyValues[2] = ( 35 );
    vSetReadBuffer( &pucReadBuff[0], MyValues, 3 );

    expect_value( __wrap_i2c_send_rs_recv, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, QSFP_IO_EXPANDER_ADDRESS );
    expect_value( __wrap_i2c_send_rs_recv, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], QSFP_IO_EXPANDER_INPUT_REG );
    expect_value( __wrap_i2c_send_rs_recv, read_length, 1 );
       
    /* set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 0 );    

    /* Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, ucMuxAddress );
    expect_value( __wrap_i2c_send, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], ucMuxControlRegisterQSFP );

    /* set return value for mock function*/
    will_return( __wrap_i2c_send, XST_SUCCESS );

    expect_value( __wrap_i2c_send_rs_recv, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, QSFP_SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send_rs_recv, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], QSFP_LSB_TEMPERATURE_REG );
    expect_value( __wrap_i2c_send_rs_recv, read_length, 1 );
    
    /* set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 0 );

    expect_value( __wrap_i2c_send_rs_recv, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, QSFP_SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send_rs_recv, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], QSFP_MSB_TEMPERATURE_REG );
    expect_value( __wrap_i2c_send_rs_recv, read_length, 1 );
    
    /* set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 0 );

    ucStatus = ucQSFPI2CMuxReadTemperature( &fTemperatureValue, ucSensorInstance );
    assert_true( ucStatus == XST_SUCCESS );
    assert_true( fTemperatureValue == 35.015625 );
}

static void vTestQSFPI2CMuxReadTemperature3( void **state )
{
    u8 ucSensorInstance                 = 3;
    u8 ucStatus                         = 0;
    float fTemperatureValue             = 0;
    unsigned char pucReadBuff[4]        = {0};
    u8 ucMuxControlRegisterIOExpander   = 0;
    u8 ucMuxControlRegisterQSFP         = 0;
    u8 ucMuxAddress                     = 0;
    
    /* 3 i2c_send SUCCESS, i2c_send_rs_recv FAIL */
    ucMuxAddress = QSFP_MUX1_ADDRESS;
    ucMuxControlRegisterIOExpander = 1 << 2;
    ucMuxControlRegisterQSFP = 1 << 3;
    u8 MyValues[3] = {8, 0, 35};

    /* Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, ucMuxAddress );
    expect_value( __wrap_i2c_send, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], ucMuxControlRegisterIOExpander );

    /* set return value for mock function*/
    will_return( __wrap_i2c_send, XST_SUCCESS );

    /* Check mock function parameters*/
    expect_value( __wrap_i2c_send_rs_recv, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, QSFP_IO_EXPANDER_ADDRESS );
    expect_value( __wrap_i2c_send_rs_recv, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], QSFP_IO_EXPANDER_INPUT_REG );
    expect_value( __wrap_i2c_send_rs_recv, read_length, 1 );
       
    /* set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 1 );

    ucStatus = ucQSFPI2CMuxReadTemperature( &fTemperatureValue, ucSensorInstance );
    assert_true( ucStatus == XST_FAILURE );

    /* 4 i2c_send SUCCESS, i2c_send_rs_recv SUCCESS, Module not present*/

    /* Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, ucMuxAddress );
    expect_value( __wrap_i2c_send, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], ucMuxControlRegisterIOExpander );

    /* set return value for mock function*/
    will_return( __wrap_i2c_send, XST_SUCCESS );
    /* Check mock function parameters*/
    /* Set bit to make module appear missing */
    MyValues[0] = QSFP_MODPRES_L_BIT;
    vSetReadBuffer( &pucReadBuff[0], MyValues, 1 );

    expect_value( __wrap_i2c_send_rs_recv, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, QSFP_IO_EXPANDER_ADDRESS );
    expect_value( __wrap_i2c_send_rs_recv, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], QSFP_IO_EXPANDER_INPUT_REG );
    expect_value( __wrap_i2c_send_rs_recv, read_length, 1 );
       
    /* set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 0 );    

    ucStatus = ucQSFPI2CMuxReadTemperature( &fTemperatureValue, ucSensorInstance );
    assert_true( ucStatus == XST_DEVICE_NOT_FOUND );

    /* 5 i2c_send SUCCESS, i2c_send_rs_recv SUCCESS, Module present send fail */

    /* Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, ucMuxAddress );
    expect_value( __wrap_i2c_send, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], ucMuxControlRegisterIOExpander );

    /* set return value for mock function*/
    will_return( __wrap_i2c_send, XST_SUCCESS );
    /* Check mock function parameters*/
    /* Set bit to make module appear missing */
    MyValues[0] = 5;
    vSetReadBuffer( &pucReadBuff[0], MyValues, 1 );

    expect_value( __wrap_i2c_send_rs_recv, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, QSFP_IO_EXPANDER_ADDRESS );
    expect_value( __wrap_i2c_send_rs_recv, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], QSFP_IO_EXPANDER_INPUT_REG );
    expect_value( __wrap_i2c_send_rs_recv, read_length, 1 );
       
    /* set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 0 );    

    /* Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, ucMuxAddress );
    expect_value( __wrap_i2c_send, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], ucMuxControlRegisterQSFP );

    /* set return value for mock function*/
    will_return( __wrap_i2c_send, XST_FAILURE );

    ucStatus = ucQSFPI2CMuxReadTemperature( &fTemperatureValue, ucSensorInstance );
    assert_true( ucStatus == XST_FAILURE );

    /* 6 i2c_send SUCCESS, i2c_send_rs_recv SUCCESS, Module present send success  
        i2c_send_rs_recv fail*/

    /* Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, ucMuxAddress );
    expect_value( __wrap_i2c_send, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], ucMuxControlRegisterIOExpander );

    /* set return value for mock function*/
    will_return( __wrap_i2c_send, XST_SUCCESS );
    /* Check mock function parameters*/
    /* Set bit to make module appear missing */
    MyValues[0] = 6;
    MyValues[1] = 1;
    vSetReadBuffer( &pucReadBuff[0], MyValues, 2 );

    expect_value( __wrap_i2c_send_rs_recv, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, QSFP_IO_EXPANDER_ADDRESS );
    expect_value( __wrap_i2c_send_rs_recv, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], QSFP_IO_EXPANDER_INPUT_REG );
    expect_value( __wrap_i2c_send_rs_recv, read_length, 1 );
       
    /* set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 0 );    

    /* Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, ucMuxAddress );
    expect_value( __wrap_i2c_send, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], ucMuxControlRegisterQSFP );

    /* set return value for mock function*/
    will_return( __wrap_i2c_send, XST_SUCCESS );

    expect_value( __wrap_i2c_send_rs_recv, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, QSFP_SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send_rs_recv, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], QSFP_LSB_TEMPERATURE_REG );
    expect_value( __wrap_i2c_send_rs_recv, read_length, 1 );
    
    /* set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 1 );


    ucStatus = ucQSFPI2CMuxReadTemperature( &fTemperatureValue, ucSensorInstance );
    assert_true( ucStatus == XST_FAILURE );

    /* 7 i2c_send SUCCESS, i2c_send_rs_recv SUCCESS, Module present send success  
        i2c_send_rs_recv SUCCESS i2c_send_rs_recv FAIL */

    /* Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, ucMuxAddress );
    expect_value( __wrap_i2c_send, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], ucMuxControlRegisterIOExpander );

    /* set return value for mock function*/
    will_return( __wrap_i2c_send, XST_SUCCESS );
    /* Check mock function parameters*/
    /* Set bit to make module appear missing */
    MyValues[0] = 7;
    MyValues[1] = 2;
    MyValues[2] = 3;
    vSetReadBuffer( &pucReadBuff[0], MyValues, 3 );

    expect_value( __wrap_i2c_send_rs_recv, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, QSFP_IO_EXPANDER_ADDRESS );
    expect_value( __wrap_i2c_send_rs_recv, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], QSFP_IO_EXPANDER_INPUT_REG );
    expect_value( __wrap_i2c_send_rs_recv, read_length, 1 );
       
    /* set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 0 );    

    /* Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, ucMuxAddress );
    expect_value( __wrap_i2c_send, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], ucMuxControlRegisterQSFP );

    /* set return value for mock function*/
    will_return( __wrap_i2c_send, XST_SUCCESS );

    expect_value( __wrap_i2c_send_rs_recv, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, QSFP_SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send_rs_recv, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], QSFP_LSB_TEMPERATURE_REG );
    expect_value( __wrap_i2c_send_rs_recv, read_length, 1 );
    
    /* set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 0 );

    expect_value( __wrap_i2c_send_rs_recv, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, QSFP_SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send_rs_recv, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], QSFP_MSB_TEMPERATURE_REG );
    expect_value( __wrap_i2c_send_rs_recv, read_length, 1 );
    
    /* set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 1 );

    ucStatus = ucQSFPI2CMuxReadTemperature( &fTemperatureValue, ucSensorInstance );
    assert_true( ucStatus == XST_FAILURE );

    /* 8 i2c_send SUCCESS, i2c_send_rs_recv SUCCESS, Module present send success  
        i2c_send_rs_recv SUCCESS i2c_send_rs_recv SUCCESS Negative temperature */

    /* Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, ucMuxAddress );
    expect_value( __wrap_i2c_send, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], ucMuxControlRegisterIOExpander );

    /* set return value for mock function*/
    will_return( __wrap_i2c_send, XST_SUCCESS );
    /* Check mock function parameters*/
    /* Set bit to make module appear missing */
    MyValues[0] = 6;
    MyValues[1] = 3;
    MyValues[2] = ( 0xA3 );
    vSetReadBuffer( &pucReadBuff[0], MyValues, 3 );

    expect_value( __wrap_i2c_send_rs_recv, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, QSFP_IO_EXPANDER_ADDRESS );
    expect_value( __wrap_i2c_send_rs_recv, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], QSFP_IO_EXPANDER_INPUT_REG );
    expect_value( __wrap_i2c_send_rs_recv, read_length, 1 );
       
    /* set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 0 );    

    /* Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, ucMuxAddress );
    expect_value( __wrap_i2c_send, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], ucMuxControlRegisterQSFP );

    /* set return value for mock function*/
    will_return( __wrap_i2c_send, XST_SUCCESS );

    expect_value( __wrap_i2c_send_rs_recv, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, QSFP_SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send_rs_recv, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], QSFP_LSB_TEMPERATURE_REG );
    expect_value( __wrap_i2c_send_rs_recv, read_length, 1 );
    
    /* set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 0 );

    expect_value( __wrap_i2c_send_rs_recv, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, QSFP_SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send_rs_recv, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], QSFP_MSB_TEMPERATURE_REG );
    expect_value( __wrap_i2c_send_rs_recv, read_length, 1 );
    
    /* set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 0 );

    ucStatus = ucQSFPI2CMuxReadTemperature( &fTemperatureValue, ucSensorInstance );
    assert_true( ucStatus == XST_SUCCESS );
    assert_true( fTemperatureValue == ( float )-92.988281 );

    /* 9 i2c_send SUCCESS, i2c_send_rs_recv SUCCESS, Module present send success  
        i2c_send_rs_recv SUCCESS i2c_send_rs_recv SUCCESS Positive temperature */

    /* Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, ucMuxAddress );
    expect_value( __wrap_i2c_send, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], ucMuxControlRegisterIOExpander );

    /* set return value for mock function*/
    will_return( __wrap_i2c_send, XST_SUCCESS );
    /* Check mock function parameters*/
    /* Set bit to make module appear missing */
    MyValues[0] = 5;
    MyValues[1] = 4;
    MyValues[2] = ( 35 );
    vSetReadBuffer( &pucReadBuff[0], MyValues, 3 );

    expect_value( __wrap_i2c_send_rs_recv, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, QSFP_IO_EXPANDER_ADDRESS );
    expect_value( __wrap_i2c_send_rs_recv, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], QSFP_IO_EXPANDER_INPUT_REG );
    expect_value( __wrap_i2c_send_rs_recv, read_length, 1 );
       
    /* set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 0 );    

    /* Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, ucMuxAddress );
    expect_value( __wrap_i2c_send, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], ucMuxControlRegisterQSFP );

    /* set return value for mock function*/
    will_return( __wrap_i2c_send, XST_SUCCESS );

    expect_value( __wrap_i2c_send_rs_recv, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, QSFP_SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send_rs_recv, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], QSFP_LSB_TEMPERATURE_REG );
    expect_value( __wrap_i2c_send_rs_recv, read_length, 1 );
    
    /* set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 0 );

    expect_value( __wrap_i2c_send_rs_recv, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, QSFP_SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send_rs_recv, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], QSFP_MSB_TEMPERATURE_REG );
    expect_value( __wrap_i2c_send_rs_recv, read_length, 1 );
    
    /* set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 0 );

    ucStatus = ucQSFPI2CMuxReadTemperature( &fTemperatureValue, ucSensorInstance );
    assert_true( ucStatus == XST_SUCCESS );
    assert_true( fTemperatureValue == 35.015625 );
}


static void vTestQSFPReadTemperatureFail( void **state )
{
    u8 ucSensorInstance     = 0;
    u8 ucStatus             = 0;
    float fTemperatureValue = 0;


    u8 ucMuxControlRegisterIOExpander     = 0;
    u8 ucMuxControlRegisterQSFP           = 0;
    u8 ucMuxAddress                       = 0;

    /* 1 Test function NULL pointer */
    ucStatus = ucQSFPReadTemperature( NULL, ucSensorInstance );
    assert_true( ucStatus == XST_FAILURE );
}

static void vTestQSFPReadTemperature0( void **state )
{
    u8 ucSensorInstance                 = 0;
    u8 ucStatus                         = 0;
    float fTemperatureValue             = 0;
    unsigned char pucReadBuff[4]        = {0};
    u8 ucMuxControlRegisterIOExpander   = 0;
    u8 ucMuxControlRegisterQSFP         = 0;
    u8 ucMuxAddress                     = 0;
    
    /* 3 i2c_send SUCCESS, i2c_send_rs_recv FAIL */
    ucMuxAddress = QSFP_MUX0_ADDRESS;
    ucMuxControlRegisterIOExpander = 1 << 0;
    ucMuxControlRegisterQSFP = 1 << 1;
    u8 MyValues[3] = {8, 0, 35};

    /* Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, QSFP_MUX0_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], ucMuxControlRegisterIOExpander );

    /* set return value for mock function*/
    will_return( __wrap_i2c_send, XST_SUCCESS );

    /* Check mock function parameters*/
    expect_value( __wrap_i2c_send_rs_recv, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, QSFP_IO_EXPANDER_ADDRESS );
    expect_value( __wrap_i2c_send_rs_recv, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], QSFP_IO_EXPANDER_INPUT_REG );
    expect_value( __wrap_i2c_send_rs_recv, read_length, 1 );
       
    /* set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 1 );

    ucStatus = ucQSFPReadTemperature( &fTemperatureValue, ucSensorInstance );
    assert_true( ucStatus == XST_FAILURE );

    /* 4 i2c_send SUCCESS, i2c_send_rs_recv SUCCESS, Module not present*/

    /* Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, QSFP_MUX0_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], ucMuxControlRegisterIOExpander );

    /* set return value for mock function*/
    will_return( __wrap_i2c_send, XST_SUCCESS );
    /* Check mock function parameters*/
    /* Set bit to make module appear missing */
    MyValues[0] = QSFP_MODPRES_L_BIT;
    vSetReadBuffer( &pucReadBuff[0], MyValues, 1 );

    expect_value( __wrap_i2c_send_rs_recv, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, QSFP_IO_EXPANDER_ADDRESS );
    expect_value( __wrap_i2c_send_rs_recv, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], QSFP_IO_EXPANDER_INPUT_REG );
    expect_value( __wrap_i2c_send_rs_recv, read_length, 1 );
       
    /* set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 0 );    

    ucStatus = ucQSFPReadTemperature( &fTemperatureValue, ucSensorInstance );
    assert_true( ucStatus == XST_DEVICE_NOT_FOUND );

    /* 5 i2c_send SUCCESS, i2c_send_rs_recv SUCCESS, Module present send fail */

    /* Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, QSFP_MUX0_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], ucMuxControlRegisterIOExpander );

    /* set return value for mock function*/
    will_return( __wrap_i2c_send, XST_SUCCESS );
    /* Check mock function parameters*/
    /* Set bit to make module appear missing */
    MyValues[0] = 5;
    vSetReadBuffer( &pucReadBuff[0], MyValues, 1 );

    expect_value( __wrap_i2c_send_rs_recv, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, QSFP_IO_EXPANDER_ADDRESS );
    expect_value( __wrap_i2c_send_rs_recv, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], QSFP_IO_EXPANDER_INPUT_REG );
    expect_value( __wrap_i2c_send_rs_recv, read_length, 1 );
       
    /* set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 0 );    

    /* Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, QSFP_MUX0_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], ucMuxControlRegisterQSFP );

    /* set return value for mock function*/
    will_return( __wrap_i2c_send, XST_FAILURE );

    ucStatus = ucQSFPReadTemperature( &fTemperatureValue, ucSensorInstance );
    assert_true( ucStatus == XST_FAILURE );

    /* 6 i2c_send SUCCESS, i2c_send_rs_recv SUCCESS, Module present send success  
        i2c_send_rs_recv fail*/

    /* Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, QSFP_MUX0_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], ucMuxControlRegisterIOExpander );

    /* set return value for mock function*/
    will_return( __wrap_i2c_send, XST_SUCCESS );
    /* Check mock function parameters*/
    /* Set bit to make module appear missing */
    MyValues[0] = 6;
    MyValues[1] = 1;
    vSetReadBuffer( &pucReadBuff[0], MyValues, 2 );

    expect_value( __wrap_i2c_send_rs_recv, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, QSFP_IO_EXPANDER_ADDRESS );
    expect_value( __wrap_i2c_send_rs_recv, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], QSFP_IO_EXPANDER_INPUT_REG );
    expect_value( __wrap_i2c_send_rs_recv, read_length, 1 );
       
    /* set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 0 );    

    /* Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, QSFP_MUX0_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], ucMuxControlRegisterQSFP );

    /* set return value for mock function*/
    will_return( __wrap_i2c_send, XST_SUCCESS );

    expect_value( __wrap_i2c_send_rs_recv, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, QSFP_SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send_rs_recv, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], QSFP_LSB_TEMPERATURE_REG );
    expect_value( __wrap_i2c_send_rs_recv, read_length, 1 );
    
    /* set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 1 );

    ucStatus = ucQSFPReadTemperature( &fTemperatureValue, ucSensorInstance );
    assert_true( ucStatus == XST_FAILURE );

    /* 7 i2c_send SUCCESS, i2c_send_rs_recv SUCCESS, Module present send success  
        i2c_send_rs_recv SUCCESS i2c_send_rs_recv FAIL */

    /* Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, QSFP_MUX0_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], ucMuxControlRegisterIOExpander );

    /* set return value for mock function*/
    will_return( __wrap_i2c_send, XST_SUCCESS );
    /* Check mock function parameters*/
    /* Set bit to make module appear missing */
    MyValues[0] = 7;
    MyValues[1] = 2;
    MyValues[2] = 3;
    vSetReadBuffer( &pucReadBuff[0], MyValues, 3 );

    expect_value( __wrap_i2c_send_rs_recv, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, QSFP_IO_EXPANDER_ADDRESS );
    expect_value( __wrap_i2c_send_rs_recv, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], QSFP_IO_EXPANDER_INPUT_REG );
    expect_value( __wrap_i2c_send_rs_recv, read_length, 1 );
       
    /* set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 0 );    

    /* Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, QSFP_MUX0_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], ucMuxControlRegisterQSFP );

    /* set return value for mock function*/
    will_return( __wrap_i2c_send, XST_SUCCESS );

    expect_value( __wrap_i2c_send_rs_recv, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, QSFP_SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send_rs_recv, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], QSFP_LSB_TEMPERATURE_REG );
    expect_value( __wrap_i2c_send_rs_recv, read_length, 1 );
    
    /* set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 0 );

    expect_value( __wrap_i2c_send_rs_recv, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, QSFP_SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send_rs_recv, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], QSFP_MSB_TEMPERATURE_REG );
    expect_value( __wrap_i2c_send_rs_recv, read_length, 1 );
    
    /* set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 1 );

    ucStatus = ucQSFPReadTemperature( &fTemperatureValue, ucSensorInstance );
    assert_true( ucStatus == XST_FAILURE );

    /* 8 i2c_send SUCCESS, i2c_send_rs_recv SUCCESS, Module present send success  
        i2c_send_rs_recv SUCCESS i2c_send_rs_recv SUCCESS Negative temperature */
    ucMuxAddress = QSFP_MUX0_ADDRESS;
    ucMuxControlRegisterIOExpander = 1 << 0;
    ucMuxControlRegisterQSFP = 1 << 1;

    /* Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, QSFP_MUX0_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], ucMuxControlRegisterIOExpander );

    /* set return value for mock function*/
    will_return( __wrap_i2c_send, XST_SUCCESS );
    /* Check mock function parameters*/
    /* Set bit to make module appear missing */
    MyValues[0] = 6;
    MyValues[1] = 3;
    MyValues[2] = ( 0xA3 );
    vSetReadBuffer( &pucReadBuff[0], MyValues, 3 );

    expect_value( __wrap_i2c_send_rs_recv, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, QSFP_IO_EXPANDER_ADDRESS );
    expect_value( __wrap_i2c_send_rs_recv, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], QSFP_IO_EXPANDER_INPUT_REG );
    expect_value( __wrap_i2c_send_rs_recv, read_length, 1 );
       
    /* set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 0 );    

    /* Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, QSFP_MUX0_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], ucMuxControlRegisterQSFP );

    /* set return value for mock function*/
    will_return( __wrap_i2c_send, XST_SUCCESS );

    expect_value( __wrap_i2c_send_rs_recv, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, QSFP_SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send_rs_recv, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], QSFP_LSB_TEMPERATURE_REG );
    expect_value( __wrap_i2c_send_rs_recv, read_length, 1 );
    
    /* set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 0 );

    expect_value( __wrap_i2c_send_rs_recv, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, QSFP_SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send_rs_recv, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], QSFP_MSB_TEMPERATURE_REG );
    expect_value( __wrap_i2c_send_rs_recv, read_length, 1 );
    
    /* set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 0 );

    ucStatus = ucQSFPReadTemperature( &fTemperatureValue, ucSensorInstance );
    assert_true( ucStatus == XST_SUCCESS );
    assert_true( fTemperatureValue == ( float )-92.988281 );

    /* 9 i2c_send SUCCESS, i2c_send_rs_recv SUCCESS, Module present send success  
        i2c_send_rs_recv SUCCESS i2c_send_rs_recv SUCCESS Positive temperature */

    /* Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, QSFP_MUX0_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], ucMuxControlRegisterIOExpander );

    /* set return value for mock function*/
    will_return( __wrap_i2c_send, XST_SUCCESS );
    /* Check mock function parameters*/
    /* Set bit to make module appear missing */
    MyValues[0] = 5;
    MyValues[1] = 4;
    MyValues[2] = ( 35 );
    vSetReadBuffer( &pucReadBuff[0], MyValues, 3 );

    expect_value( __wrap_i2c_send_rs_recv, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, QSFP_IO_EXPANDER_ADDRESS );
    expect_value( __wrap_i2c_send_rs_recv, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], QSFP_IO_EXPANDER_INPUT_REG );
    expect_value( __wrap_i2c_send_rs_recv, read_length, 1 );
       
    /* set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 0 );    

    /* Check mock function parameters*/
    expect_value( __wrap_i2c_send, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send, i2c_addr, QSFP_MUX0_ADDRESS );
    expect_value( __wrap_i2c_send, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send, i2c_write_buff[0], ucMuxControlRegisterQSFP );

    /* set return value for mock function*/
    will_return( __wrap_i2c_send, XST_SUCCESS );

    expect_value( __wrap_i2c_send_rs_recv, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, QSFP_SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send_rs_recv, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], QSFP_LSB_TEMPERATURE_REG );
    expect_value( __wrap_i2c_send_rs_recv, read_length, 1 );
    
    /* set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 0 );

    expect_value( __wrap_i2c_send_rs_recv, i2c_num, QSFP_I2CNUM );
    expect_value( __wrap_i2c_send_rs_recv, i2c_addr, QSFP_SLAVE_ADDRESS );
    expect_value( __wrap_i2c_send_rs_recv, write_length, QSFP_WRITE_LEN );
    expect_value( __wrap_i2c_send_rs_recv, i2c_write_buff[0], QSFP_MSB_TEMPERATURE_REG );
    expect_value( __wrap_i2c_send_rs_recv, read_length, 1 );
    
    /* set return value for mock function*/
    will_return( __wrap_i2c_send_rs_recv, 0 );

    ucStatus = ucQSFPReadTemperature( &fTemperatureValue, ucSensorInstance );
    assert_true( ucStatus == XST_SUCCESS );
    assert_true( fTemperatureValue == 35.015625 );
}

int main( void ) 
{
    srand( time( 0 ) );
    const struct CMUnitTest tests[] = 
    {
            cmocka_unit_test( vTestQSFPI2CMuxReadTemperature ),
            cmocka_unit_test( vTestQSFPI2CMuxReadTemperature0 ),
            cmocka_unit_test( vTestQSFPI2CMuxReadTemperature1 ),
            cmocka_unit_test( vTestQSFPI2CMuxReadTemperature2 ),
            cmocka_unit_test( vTestQSFPI2CMuxReadTemperature3 ),
            cmocka_unit_test( vTestQSFPReadTemperatureFail )
    };

    return cmocka_run_group_tests( tests, NULL, NULL );

}