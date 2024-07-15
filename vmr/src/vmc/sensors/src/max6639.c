/******************************************************************************
 * * Copyright ( C ) 2021 Xilinx, Inc.  All rights reserved.
 * * SPDX-License-Identifier: MIT
 * *******************************************************************************/

#include "xil_types.h"
#include "xstatus.h"
#include "cl_i2c.h"
#ifndef SDT
#include "../inc/max6639.h"
#else
#include "max6639.h"
#endif

u8 max6639_write_register( u8 i2c_num, u8 SlaveAddr, u8 register_address, u8 *register_content )
{
    u8 status = XST_FAILURE;

    if( NULL != register_content )
    {
        u8 write_data[2]= {0};
        write_data[0] = register_address;
        write_data[1] = *register_content;
        status = i2c_send( i2c_num, SlaveAddr, write_data, 2 );
    }
    return status;
}


u8 max6639_read_register( u8 i2c_num, u8 SlaveAddr, u8 register_address, u8 *register_content )
{
    u8 status = XST_FAILURE;

    if( NULL != register_content )
    {
        status = i2c_send_rs_recv( i2c_num, SlaveAddr, &register_address,1, register_content, 1 );
    }
    return status;
}

u8 max6639_init( u8 i2c_num, u8 SlaveAddr )
{
    u8 status = XST_FAILURE;
    u8 register_value = 0x1A;

    /* Set RPM step size as 4,  Temp step size as 2 , PWM polarity as 0, and Minimum fan speed as 1 */
    register_value = 0x49;
    status = max6639_write_register( i2c_num, SlaveAddr, FAN1_CONFIGURATION2A_REGISTER, &register_value );
    if ( status == XST_FAILURE )
    {
        return XST_FAILURE;
    }
    status = max6639_write_register( i2c_num, SlaveAddr, FAN2_CONFIGURATION2A_REGISTER, &register_value );
    if ( status == XST_FAILURE )
    {
        return XST_FAILURE;
    }

    /* Spin-up disable -> 0
     * THERM to full-speed -> 1
     * Pulse stretching -> 1
     * Fan PWM frequency -> 0b11 */
    register_value = 0x63;
    status = max6639_write_register( i2c_num, SlaveAddr, FAN1_CONFIGURATION3_REGISTER, &register_value );
    if ( status == XST_FAILURE )
    {
        return XST_FAILURE;
    }
    status = max6639_write_register( i2c_num, SlaveAddr, FAN2_CONFIGURATION3_REGISTER, &register_value );
    if ( status == XST_FAILURE )
    {
        return XST_FAILURE;
    }

    /* RPM Step size B -> 0b1111
     * D[3:0] -> 0b1000 */
    register_value = 0xF8;
    status = max6639_write_register( i2c_num, SlaveAddr, FAN1_CONFIGURATION2B_REGISTER, &register_value );
    if ( status == XST_FAILURE )
    {

        return XST_FAILURE;
    }
    status = max6639_write_register( i2c_num, SlaveAddr, FAN2_CONFIGURATION2B_REGISTER, &register_value );
    if ( status == XST_FAILURE )
    {
        return XST_FAILURE;
    }

    /* Fan start Tach count = 0x34
     * Obtained by this formula:
     * Tachometer count value = ( ( ( internal clock frequency ) x 60 ) / actual RPM ) ( selected number of pulses per revolution / actual fan pulses )
     * selected number of pulses per revolution = 2 ( from register 24h D[7:6] )
     * actual fan pulses = 2 ( from fan Datasheet )
     * */
    register_value = 0x34;
    status = max6639_write_register( i2c_num, SlaveAddr, FAN1_START_TACH_COUNT_REGISTER, &register_value );
    if ( status == XST_FAILURE )
    {
        return XST_FAILURE;
    }
    status = max6639_write_register( i2c_num, SlaveAddr, FAN2_START_TACH_COUNT_REGISTER, &register_value );
    if ( status == XST_FAILURE )
    {
        return XST_FAILURE;
    }

    /* tachometer pulses per revolution D[7:6] -> 0b01
     * Min fan tach count D[5:0] -> 0b011000
     * Calculated using Tach count = ( ( ( internal clock frequency ) x 60 ) / actual RPM ) ( selected number of pulses per revolution / actual fan pulses )
     * and substituting RPM with max fan RPM( 20000 in this case ) */
    register_value = 0x58;
    status = max6639_write_register( i2c_num, SlaveAddr, FAN1_MIN_TACH_COUNT_REGISTER, &register_value );
    if ( status == XST_FAILURE )
    {
        return XST_FAILURE;
    }
    status = max6639_write_register( i2c_num, SlaveAddr, FAN2_MIN_TACH_COUNT_REGISTER, &register_value );
    if ( status == XST_FAILURE )
    {
        return XST_FAILURE;
    }
    /* set fan 1 start temperature as 40 C
     * Fan 1for FPGA temperature */
    register_value = 0x28;
    status = max6639_write_register( i2c_num, SlaveAddr, CHANNEL1_MIN_FAN_START_TEMP_REGISTER, &register_value );
    if ( status == XST_FAILURE )
    {
        return XST_FAILURE;
    }
    /* set fan 2 start temperature as 25 C.
     * fan 2 for local temperature
     * fan 2 start temperature reduced from 40C to 25C.
     * Because if local temperature goes below start temperature, fan oscillates for every 2 secs to adjust to fan start tach count value
     * */
    register_value = 0x19;
    status = max6639_write_register( i2c_num, SlaveAddr, CHANNEL2_MIN_FAN_START_TEMP_REGISTER, &register_value );
    if ( status == XST_FAILURE )
    {
        return XST_FAILURE;
    }

    /* PWM mode D[7]-> 0   ( Auto RPM Mode )
     * Rate of change D[6:4] -> 0b001
     * Fan channel 1 control D[3] -> 1
     * Fan channel 2 control D[2] -> 0
     * RPM range select D[1:0]-> 0b11 ( since our fan runs at 20000 RPM )
     * If two channels are selected, the fan goes to the higher of the two possible speeds.
     * */
    register_value = 0x1B;
    status = max6639_write_register( i2c_num, SlaveAddr, FAN1_CONFIGURATION1_REGISTER, &register_value );
    if ( status == XST_FAILURE )
    {
        return XST_FAILURE;
    }
    /* PWM mode D[7]-> 0   ( Auto RPM Mode )
     * Rate of change D[6:4] -> 0b001
     * Fan channel 1 control D[3] -> 0
     * Fan channel 2 control D[2] -> 1
     * RPM range select D[1:0]-> 0b11 ( since our fan runs at 20000 RPM )
     * If two channels are selected, the fan goes to the higher of the two possible speeds.*/
    register_value = 0x17;
    status = max6639_write_register( i2c_num, SlaveAddr, FAN2_CONFIGURATION1_REGISTER, &register_value );
    if ( status == XST_FAILURE )
    {
        return XST_FAILURE;
    }

    return XST_SUCCESS;
}


u8 max6639_ReadFPGATemperature( u8 i2c_num, u8 SlaveAddr, float *TemperatureReading )
{
    u8 msb_temperature = 0;
    u8 lsb_temperature = 0;
    u8 status = XST_FAILURE;
    u8 reg_addr = CHANNEL1_TEMPERATURE_REGISTER;

    if( NULL != TemperatureReading )
    {
        status = i2c_send_rs_recv( i2c_num, SlaveAddr, &reg_addr, 1, &msb_temperature, 1 );
        if( status == XST_SUCCESS )
        {
            reg_addr = CHANNEL1_EXTENDED_TEMPERATURE_REGISTER;
            status = i2c_send_rs_recv( i2c_num, SlaveAddr, &reg_addr, 1, &lsb_temperature, 1 );
            if( status == XST_SUCCESS )
            {
                *TemperatureReading = ( float )msb_temperature + ( ( ( lsb_temperature & 0x80 ) >> 7 ) * 0.5 ) + 
                                                             ( ( ( lsb_temperature & 0x40 ) >> 6 ) * 0.25 ) + 
                                                                ( ( ( lsb_temperature & 0x20 ) >> 5 ) * 0.125 );
            }
        }

    }    
    return status;
}

u8 max6639_ReadFanTach( u8 i2c_num, u8 SlaveAddr, u8 fanIndex, u8 *fanSpeed )
{
    u8 status = XST_FAILURE;
    u8 reg_addr;

    if( NULL != fanSpeed )
    {

        if( ( fanIndex != 0 ) && ( fanIndex > 2 ) )
        {
            return XST_FAILURE;
        }
        
        if( fanIndex == 1 )
        {
            reg_addr = FAN1_TACHOMETER_COUNT_REGISTER;
            status = i2c_send_rs_recv( i2c_num, SlaveAddr, &reg_addr, 1, fanSpeed, 1 );
            if( status == XST_FAILURE )
            {
            return XST_FAILURE;
            }
        }
        else
        {
            reg_addr = FAN2_TACHOMETER_COUNT_REGISTER;
            status = i2c_send_rs_recv( i2c_num, SlaveAddr, &reg_addr, 1, fanSpeed, 1 );
            if( status == XST_FAILURE )
            {
            return XST_FAILURE;
            }
        }
    }
    return status;
}

u8 max6639_ReadDDRTemperature( u8 i2c_num, u8 SlaveAddr, float *TemperatureReading )
{
    u8 msb_temperature = 0;
    u8 lsb_temperature = 0;
    u8 status = XST_FAILURE;
    u8 reg_addr = CHANNEL2_TEMPERATURE_REGISTER;

    if( NULL != TemperatureReading )
    {
        status = i2c_send_rs_recv( i2c_num, SlaveAddr, &reg_addr, 1, &msb_temperature, 1 );
        if( status == XST_SUCCESS )
        {
            reg_addr = CHANNEL2_EXTENDED_TEMPERATURE_REGISTER;
            status = i2c_send_rs_recv( i2c_num, SlaveAddr, &reg_addr, 1, &lsb_temperature, 1 );
            if( status == XST_SUCCESS )
            {
                *TemperatureReading = ( float )msb_temperature + ( float )( ( ( lsb_temperature & 0x80 ) >> 7 ) * 0.5 ) + 
                ( ( ( lsb_temperature & 0x40 ) >> 6 ) * 0.25 ) + ( ( ( lsb_temperature & 0x20 ) >> 5 ) * 0.125 );
            }
     
        }
    }
       
    return status;
}

