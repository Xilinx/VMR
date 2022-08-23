/******************************************************************************
 *  * * Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
 *   * * SPDX-License-Identifier: MIT
 *    * *******************************************************************************/

#include "cl_i2c.h"
#include "../inc/isl68221.h"

#define STATUS_FAILURE		(1)
u8 ISL68221_write_register(u8 i2c_num, u8 SlaveAddr, u8 register_address, u8 *register_content)
{
    u8 status = 0;

    u8 write_data[2]= {0};
    write_data[0] = register_address;
    write_data[1] = *register_content;
    status = i2c_send(i2c_num, SlaveAddr, write_data, 2);

    return status;
}
u8 ISL68221_read_register(u8 i2c_num, u8 SlaveAddr, u8 register_address, u8 *register_content)
{
    u8 status = 0;

    status = i2c_send_rs_recv(i2c_num, SlaveAddr, &register_address,1, register_content, 2);

    return status;
}
u8 ISL68221_ReadVCCINT_Voltage(u8 busnum, u8 slaveAddr, float *voltageInmV)
{
    u8 wbuf[8] = {0};
    u8 rbuf[8]= {0};
    u8 status = 0;
    u16 rddata = 0;

    wbuf[0] = ISL68221_SELECT_PAGE_VCCINT;

    status = ISL68221_write_register(busnum, slaveAddr, ISL68221_PAGE_REGISTER,(u8 *)wbuf);
    if (STATUS_FAILURE == status)
    {
    	return status;
    }

    status = ISL68221_read_register(busnum, slaveAddr, ISL68221_OUTPUT_VOLTAGE_REGISTER,(u8 *)&rbuf[0]);
    if (STATUS_FAILURE == status)
    {
    	return status;
    }
    rddata = (rbuf[1] << 8) | rbuf[0];
    *voltageInmV = ((float) rddata);
    return status;

}
u8 ISL68221_ReadVCCINT_Current(u8 busnum, u8 slaveAddr, float *currentInA)
{
    u8 wbuf[8] = {0};
    u8 rbuf[8] = {0};
    u8 status = 0;
    u16 rddata = 0;

    wbuf[0] = ISL68221_SELECT_PAGE_VCCINT;

    status = ISL68221_write_register(busnum, slaveAddr, ISL68221_PAGE_REGISTER,(u8 *)wbuf);
    if (STATUS_FAILURE == status)
    {
    	return status;
    }

    status = ISL68221_read_register(busnum, slaveAddr, ISL68221_OUTPUT_CURRENT_REGISTER,(u8 *)&rbuf[0]);
    if (STATUS_FAILURE == status)
    {
    	return status;
    }
    rddata = (rbuf[1] << 8) | rbuf[0];
    *currentInA = ((float) rddata)/10;
    return status;

}
u8 ISL68221_ReadVCCINT_Temperature(u8 busnum, u8 slaveAddr, float *temperature)
{
    u8 wbuf[8] = {0};
    u8 rbuf[8] = {0};
    u8 status = 0;
    u16 rddata = 0;

    wbuf[0] = ISL68221_SELECT_PAGE_VCCINT;

    status = ISL68221_write_register(busnum, slaveAddr, ISL68221_PAGE_REGISTER,(u8 *)wbuf);
    if (STATUS_FAILURE == status)
    {
    	return status;
    }

    status = ISL68221_read_register(busnum, slaveAddr, ISL68221_READ_POWERSTAGE_TEMPERATURE,(u8 *)&rbuf[0]);
    if (STATUS_FAILURE == status)
    {
    	return status;
    }
    rddata = (rbuf[1] << 8) | rbuf[0];
    *temperature = ((float) rddata);
    return status;

}
