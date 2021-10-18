/******************************************************************************
 * * Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
 * * SPDX-License-Identifier: MIT
 * *******************************************************************************/

#include "xil_types.h"
#include "xstatus.h"
#include "cl_i2c.h"
#include "../inc/max6639.h"

u8 max6639_write_register(u8 i2c_num, u8 SlaveAddr, u8 register_address, u8 *register_content)
{
    u8 Status = XST_FAILURE;

    u8 write_data[2]= {0};
    write_data[0] = register_address;
    write_data[1] = *register_content;
    Status = i2c_send(i2c_num, SlaveAddr, write_data, 2);

    return Status;
}


u8 max6639_read_register(u8 i2c_num, u8 SlaveAddr, u8 register_address, u8 *register_content)
{
    u8 Status = XST_FAILURE;

    Status = i2c_send_rs_recv(i2c_num, SlaveAddr, &register_address,1, register_content, 1);

    return Status;
}

u8 max6639_init(u8 i2c_num, u8 SlaveAddr)
{
    u8 Status = XST_FAILURE;
    u8 register_value = 0x1A;

    /* Set RPM step size as 4,  Temp step size as 2 , PWM polarity as 0, and Minimum fan speed as 1 */
    register_value = 0x49;
    Status = max6639_write_register(i2c_num, SlaveAddr, FAN1_CONFIGURATION2A_REGISTER, &register_value);
    if (Status == XST_FAILURE)
    {
        return XST_FAILURE;
    }
    Status = max6639_write_register(i2c_num, SlaveAddr, FAN2_CONFIGURATION2A_REGISTER, &register_value);
    if (Status == XST_FAILURE)
    {
        return XST_FAILURE;
    }

    /* Spin-up disable -> 0
 *      * THERM to full-speed -> 1
 *           * Pulse stretching -> 1
 *                * Fan PWM frequency -> 0b11 */
    register_value = 0x63;
    Status = max6639_write_register(i2c_num, SlaveAddr, FAN1_CONFIGURATION3_REGISTER, &register_value);
    if (Status == XST_FAILURE)
    {
        return XST_FAILURE;
    }
    Status = max6639_write_register(i2c_num, SlaveAddr, FAN2_CONFIGURATION3_REGISTER, &register_value);
    if (Status == XST_FAILURE)
    {
        return XST_FAILURE;
    }

    /* RPM Step size B -> 0b1111
 *      * D[3:0] -> 0b1000 */
    register_value = 0xF8;
    Status = max6639_write_register(i2c_num, SlaveAddr, FAN1_CONFIGURATION2B_REGISTER, &register_value);
    if (Status == XST_FAILURE)
    {

        return XST_FAILURE;
    }
    Status = max6639_write_register(i2c_num, SlaveAddr, FAN2_CONFIGURATION2B_REGISTER, &register_value);
    if (Status == XST_FAILURE)
    {
        return XST_FAILURE;
    }

    /* Fan start Tach count = 0x34
 *      * Obtained by this formula:
 *           * Tach count = (Max6639 internal frequency * 60)/(RPM * (Number of pulses per rotation)) */
    register_value = 0x34;
    Status = max6639_write_register(i2c_num, SlaveAddr, FAN1_START_TACH_COUNT_REGISTER, &register_value);
    if (Status == XST_FAILURE)
    {
        return XST_FAILURE;
    }
    Status = max6639_write_register(i2c_num, SlaveAddr, FAN2_START_TACH_COUNT_REGISTER, &register_value);
    if (Status == XST_FAILURE)
    {
        return XST_FAILURE;
    }

    /*  tachometer pulses per revolution D[7:6] -> 0b01
 *       * Min fan tach count D[5:0] -> 0b001010
 *            * Calculated using Tach count = (Max6639 internal frequency * 60)/(RPM * (Number of pulses per rotation))
 *                 * and subsituting RPM with max fan RPM(23000 in this case) */
    register_value = 0x4A;
    Status = max6639_write_register(i2c_num, SlaveAddr, FAN1_MIN_TACH_COUNT_REGISTER, &register_value);
    if (Status == XST_FAILURE)
    {
        return XST_FAILURE;
    }
    Status = max6639_write_register(i2c_num, SlaveAddr, FAN2_MIN_TACH_COUNT_REGISTER, &register_value);
    if (Status == XST_FAILURE)
    {
        return XST_FAILURE;
    }
    /* set fan start temperature as 40 C */
    register_value = 0x28;
    Status = max6639_write_register(i2c_num, SlaveAddr, CHANNEL1_MIN_FAN_START_TEMP_REGISTER, &register_value);
    if (Status == XST_FAILURE)
    {
        return XST_FAILURE;
    }
    Status = max6639_write_register(i2c_num, SlaveAddr, CHANNEL2_MIN_FAN_START_TEMP_REGISTER, &register_value);
    if (Status == XST_FAILURE)
    {
        return XST_FAILURE;
    }

    /*  PWM mode -> 0
 *       * Rate of change D[6:4] -> 0b001
 *            * Fan channel 1 control -> 1
 *                 * Fan channel 2 control -> 1
 *                      * RPM range select -> 0b11 (since our fan runs at 23000 RPM)*/
    register_value = 0x9F;
    Status = max6639_write_register(i2c_num, SlaveAddr, FAN1_CONFIGURATION1_REGISTER, &register_value);
    if (Status == XST_FAILURE)
    {
        return XST_FAILURE;
    }
    Status = max6639_write_register(i2c_num, SlaveAddr, FAN2_CONFIGURATION1_REGISTER, &register_value);
    if (Status == XST_FAILURE)
    {
        return XST_FAILURE;
    }

    /*
 *      * Set CURRENT_DUTY_CYCLE_REGISTER to 120 to make the fan run at 100%.
 *           * NOTE: This is needed as a workaround for the HW bug.
 *                */
    register_value = 120;
    Status = max6639_write_register(i2c_num, SlaveAddr, FAN1_CURRENT_DUTY_CYCLE_REGISTER, &register_value);
    if (Status == XST_FAILURE)
    {
        return XST_FAILURE;
    }
    Status = max6639_write_register(i2c_num, SlaveAddr, FAN2_CURRENT_DUTY_CYCLE_REGISTER, &register_value);
    if (Status == XST_FAILURE)
    {
        return XST_FAILURE;    
    }

    return XST_SUCCESS;
}


u8 max6639_ReadFPGATemperature(u8 i2c_num, u8 SlaveAddr, float *TemperatureReading)
{
    u8 msb_temperature = 0;
    u8 lsb_temperature = 0;
    u8 Status = XST_FAILURE;
    u8 regAddr = CHANNEL1_TEMPERATURE_REGISTER;

    Status = i2c_send_rs_recv(i2c_num, SlaveAddr, &regAddr, 1, &msb_temperature, 1);
    if(Status == XST_FAILURE)
    {
       return XST_FAILURE;
    }

    regAddr = CHANNEL1_EXTENDED_TEMPERATURE_REGISTER;
    Status = i2c_send_rs_recv(i2c_num, SlaveAddr, &regAddr, 1, &lsb_temperature, 1);
    if(Status == XST_FAILURE)
    {
       return XST_FAILURE;
    }

    *TemperatureReading = (float)msb_temperature + (((lsb_temperature & 0x80) >> 7) * 0.5) + (((lsb_temperature & 0x40) >> 6) * 0.25) + (((lsb_temperature & 0x20) >> 5) * 0.125);
    return XST_SUCCESS;
}

u8 max6639_ReadFanTach(u8 i2c_num, u8 SlaveAddr, u8 fanIndex, u8 *fanSpeed)
{
    u8 Status = XST_FAILURE;
    u8 regAddr;

    if((fanIndex != 0) && (fanIndex > 2))
    {
        return XST_FAILURE;
    }
    
    if(fanIndex == 1)
    {
        regAddr = FAN1_TACHOMETER_COUNT_REGISTER;
        Status = i2c_send_rs_recv(i2c_num, SlaveAddr, &regAddr, 1, fanSpeed, 1);
        if(Status == XST_FAILURE)
        {
           return XST_FAILURE;
        }
    }
    else
    {
	regAddr = FAN2_TACHOMETER_COUNT_REGISTER;
        Status = i2c_send_rs_recv(i2c_num, SlaveAddr, &regAddr, 1, fanSpeed, 1);
        if(Status == XST_FAILURE)
        {
           return XST_FAILURE;
        }
    }

    return Status;
}

u8 max6639_ReadDDRTemperature(u8 i2c_num, u8 SlaveAddr, float *TemperatureReading)
{
    u8 msb_temperature = 0;
    u8 lsb_temperature = 0;
    u8 Status = XST_FAILURE;
    u8 regAddr = CHANNEL2_TEMPERATURE_REGISTER;

    Status = i2c_send_rs_recv(i2c_num, SlaveAddr, &regAddr, 1, &msb_temperature, 1);
    if(Status == XST_FAILURE)
    {
       return XST_FAILURE;
    }

    regAddr = CHANNEL2_EXTENDED_TEMPERATURE_REGISTER; 
    Status = i2c_send_rs_recv(i2c_num, SlaveAddr, &regAddr, 1, &lsb_temperature, 1);
    if(Status == XST_FAILURE)
    {
       return XST_FAILURE;
    }

    *TemperatureReading = (float)msb_temperature + (float)(((lsb_temperature & 0x80) >> 7) * 0.5) + (((lsb_temperature & 0x40) >> 6) * 0.25) + (((lsb_temperature & 0x20) >> 5) * 0.125);
    return XST_SUCCESS;
}

