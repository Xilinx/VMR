/******************************************************************************
 *  * * Copyright (C) 2024 Advanced Micro Devices, Inc.    All rights reserved.
 *   * * SPDX-License-Identifier: MIT
 *    * *******************************************************************************/

#include "cl_i2c.h"
#include "ina3221.h"

#define SHUNT_RESISTANCE_VALUE (0.002)

u8 INA3221_ReadVoltage(u8 busnum, u8 slaveAddr, u8 channelNum, float *voltageInmV)
{
    u8 wbuf[8] = {0};
    u8 rbuf[8] = {0};
    u8 status = 0;
    u16 rddata = 0;

    if( NULL != voltageInmV )
    {
        wbuf[0] = INA3221_CH1_BUS_VOLTAGE;                

        if (channelNum == 1)
            wbuf[0] = INA3221_CH2_BUS_VOLTAGE;                
        if (channelNum == 2)
            wbuf[0] = INA3221_CH3_BUS_VOLTAGE;                

        status = i2c_send_rs_recv(busnum, slaveAddr, (u8 *)wbuf, 1, (u8 *)&rbuf[0], 2);
        if (0 == status)
        {
            rbuf[1] = rbuf[1] & ~0x7;
            rddata = ((u16)rbuf[1]) | ((u16)rbuf[0] << 8);
    //        rddata &= ~0x7;
            *voltageInmV = ((float) rddata);
        }

    }
    else
    {
        status = 1;
    }

    return status;

}
u8 INA3221_ReadCurrent(u8 busnum, u8 slaveAddr, u8 channelNum, float *currentInmA)
{
    u8 wbuf[8] = {0};
    u8 rbuf[8] = {0};
    u8 status = 0;
    u16 rddata = 0;

    if( NULL != currentInmA )
    {
        wbuf[0] = INA3221_CH1_SHUNT_VOLTAGE;                
        if (channelNum == 1)
            wbuf[0] = INA3221_CH2_SHUNT_VOLTAGE;           
        if (channelNum == 2)
            wbuf[0] = INA3221_CH3_SHUNT_VOLTAGE;

        status = i2c_send_rs_recv(busnum, slaveAddr, (u8 *)wbuf, 1, (u8 *)&rbuf[0], 2);
        if (0 == status)
        {
            rbuf[1] = rbuf[1] & ~0x7;
            rddata = ((u16) rbuf[1]) | ((u16)rbuf[0] << 8);
            //rddata &= ~0x7;

            float shuntVolt = (float) (rddata >> 3);
            shuntVolt *= (float) 0.00004;
            shuntVolt *= 1000;

            *currentInmA = shuntVolt / (float) SHUNT_RESISTANCE_VALUE;

        }
    }
    else
    {
        status = 1;
    }
    return status;
}
u8 INA3221_ReadPower(u8 busnum, u8 slaveAddr, u8 channelNum, float *powerInmW)
{
    u8 status = 0;
    float currentInMA = 0.0;
    float voltageInMV = 0.0;

    if( NULL != powerInmW )
    {
        if ( (status = INA3221_ReadCurrent(busnum,slaveAddr,channelNum,&currentInMA)) == 0)
        {
            if ( (status = INA3221_ReadVoltage(busnum,slaveAddr,channelNum,&voltageInMV)) == 0)
            {
                *powerInmW = (float) (((double)voltageInMV * (double)currentInMA) / 1000);
            }
        }
    }
    else
    {
        status = 1;
    }
    return status;
}
