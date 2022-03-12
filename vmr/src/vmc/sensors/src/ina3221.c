/******************************************************************************
 *  * * Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
 *   * * SPDX-License-Identifier: MIT
 *    * *******************************************************************************/

#include "cl_i2c.h"
#include "../inc/ina3221.h"


u8 INA3221_ReadVoltage(u8 busnum, u8 slaveAddr, u8 channelNum, float *voltageInmV)
{
    u8 wbuf[8];
    u8 rbuf[8];
    u8 status;
    u16 rddata;

    wbuf[0] = INA3221_CH1_BUS_VOLTAGE;                

    if (channelNum == 1)
        wbuf[0] = INA3221_CH2_BUS_VOLTAGE;                
    if (channelNum == 2)
        wbuf[0] = INA3221_CH3_BUS_VOLTAGE;                

    status = i2c_send_rs_recv(busnum, slaveAddr, (u8 *)wbuf, 1, (u8 *)&rbuf[0], 2);
    if (0 == status)
    {
        rddata = ((u16)rbuf[1]) | ((u16)rbuf[0] << 8);
        rddata &= ~0x7;                             
        *voltageInmV = ((float) rddata);
    }
    return status;

}
u8 INA3221_ReadCurrent(u8 busnum, u8 slaveAddr, u8 channelNum, float *currentInmA)
{
    uint8_t wbuf[8];
    uint8_t rbuf[8];
    uint8_t status;
    uint16_t rddata;

    wbuf[0] = INA3221_CH1_SHUNT_VOLTAGE;                
     if (channelNum == 1)
         wbuf[0] = INA3221_CH2_SHUNT_VOLTAGE;           
     if (channelNum == 2)
         wbuf[0] = INA3221_CH3_SHUNT_VOLTAGE;

    status = i2c_send_rs_recv(busnum, slaveAddr, (u8 *)wbuf, 1, (u8 *)&rbuf[0], 2);
    if (0 == status)
    {
        rddata = ((u16) rbuf[1]) | ((u16)rbuf[0] << 8);
        rddata &= ~0x7;                   

        float shuntVolt = (float) (rddata >> 3);
        shuntVolt *= (float) .00004;            
        shuntVolt *= 1000;

        *currentInmA = shuntVolt / (float) .005;

     }
    return status;
}
u8 INA3221_ReadPower(u8 busnum, u8 slaveAddr, u8 channelNum, float *powerInmW)
{
    u8 status;
    float currentInMA;
    float voltageInMV;

    if ( (status = INA3221_ReadCurrent(busnum,slaveAddr,channelNum,&currentInMA)) == 0)
    {
        if ( (status = INA3221_ReadVoltage(busnum,slaveAddr,channelNum,&voltageInMV)) == 0)
         {
            *powerInmW = (float) (((double)voltageInMV * (double)currentInMA) / 1000);
         }
    }
    return status;
}
