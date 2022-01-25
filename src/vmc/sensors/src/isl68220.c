/******************************************************************************
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#include "cl_i2c.h"
#include "unistd.h"
#include "../inc/isl68220.h"

#include "xil_types.h"
#include "../../vmc_api.h"


#define MAX_VOUT_MAX_DIRECT_VAL     0x076C
#define MIN_VOUT_MIN_DIRECT_VAL     0x0000

#define MAX_RETRY       3


/*----------------------------------------------------------------------------
 * pmbus_init_vccint_config  Set the VCCINT regulator to the configuration
 *                           whose ID is passed in
 *
 * Parameters:     ID of VCCINT config setting.
 * Return:         none
 *
 * Disable the VCCINT regulator and initiate a config recall. Then enable
 * the regulator. It is assumed the configuration passed in is valid for
 * the currently plugged in AUXilary power connectors.
 *
 * This only applies to the V350 board. All other boards to date configure
 * VCCINT without firmware assist.
 *---------------------------------------------------------------------------*/
// Change the VCCINT regulator to use the configuration whose ID is passed in.
bool ISL68220_set_vccint_config(u8 busnum, u8 SlaveAddr, u8 vccint_opt)
{
    u8  txbuf[2] = {0};
    bool  status = XST_FAILURE;

    // u8  operation_page0_save; // needed?
    // u8  operation_page1_save; // needed?

    /*  Change VCCINT regulator
     *
     *  Procedure: Disable each output (2) by writing to OPERATION reg
     *             Change configuration by writing to RESTORE_CONFIG reg
     *               (Restore of config will default to output enabled or
     *                disabled based on config setting, so no need to restore
     *                OPERATION reg after restore.)
     */

    // Set Page to Both Pages: PAGE          (0x00) = 0xff
    txbuf[0] = 0xFF;

    status = ISL68220_write_register(busnum, SlaveAddr, ISL68220_PAGE_REGISTER, &txbuf[0]);
    if (status != XST_SUCCESS)
    {
        return false;
    }
    // Disable VCCINT and VCC1v2: OPERATION        (0x01) &= 0x3F

 /*   // Read OPERATION Reg
    txbuf[0] = 0x01;            // Register to read: OPERATION
                                // txbuf[1] not used in this case
    //function puts content of OPERATION reg in txbuf[0]
    status = ISL68220_read_register(busnum, SlaveAddr, ISL68220_OPERATION_REGISTER, &txbuf[0]);
    if (status == XST_FAILURE)
    {
        return false;
    }
*/

    /*
    // Restore config ID: RESTORE_CONFIG (F2h) = vccint_opt
    // Change configuration by sending the config ID code (assumed to be 0-F)
    // to the RESTORE_CONFIG register.
    status = ISL68220_write_register(busnum, SlaveAddr, ISL68220_RESTORE_CONFIG_REGISTER, &vccint_opt);
    if (status != XST_SUCCESS)
    {
        return false;
    }
    */
/*
    // Set Page to VCCINT and VCC1v2: Page          (0x00)  = 0xff
    // Set Page to Both Pages: PAGE          (0x00) = 0xff
    txbuf[0] = 0xFF;            // Register to write: PAGE
    status = ISL68220_write_register(busnum, SlaveAddr, ISL68220_PAGE_REGISTER, &txbuf[0]);
    if (status == false)
    {
        return false;
    }

    // Enable VCCINT and VCC1v2: OPERATION          (0x08) |= 0x80
    // Write OPERATION reg to enable voltage for both pages (VCCINT & VCC1v2)
    txbuf[0] = 0x88; // Turn on bit 7: On in OPERATION reg

    status = ISL68220_write_register(busnum, SlaveAddr, ISL68220_OPERATION_REGISTER, &txbuf[0]);
    if (status == false)
    {
        return false;
    }
*/
    return status;
}

bool ISL68220_read_one_byte_register(u8 busnum, u8 SlaveAddr, u8 register_address, u8 *register_content)
{
    u8 retries = MAX_RETRY;
    bool Status = XST_FAILURE;

    u8 wbuf[8];
    u8 rbuf[8];

    wbuf[0] = register_address;
    rbuf[0] = 0;

    do
    {
        Status = i2c_send_rs_recv(busnum, SlaveAddr, (u8 *)wbuf, 1, (u8 *)&rbuf[0], 1);
        retries--;
        if(Status != XST_SUCCESS)
        {
            VMC_ERR("\r\nError in reading one byte register from ISL68220.. Retries  Left: %d\r\n", retries);
        }
    }while((Status != XST_SUCCESS)&&(retries!=0));
    retries = MAX_RETRY;

    return Status;
}


bool ISL68220_read_register(u8 busnum, u8 SlaveAddr, u8 register_address, u8 *register_content)
{
    u8 retries = MAX_RETRY;
    bool Status = XST_FAILURE;

    u8 wbuf[8];
	//u8 rbuf[8] = {0};

	wbuf[0] = register_address;

    do
    {
        Status = i2c_send_rs_recv(busnum, SlaveAddr, (u8 *)wbuf, 1, register_content, 2);
        retries--;
        if(Status != XST_SUCCESS)
        {
            VMC_ERR("\r\nError in reading two byte register from ISL68220.. Retries  Left: %d\r\n", retries);
        }
    }while((Status != XST_SUCCESS)&&(retries!=0));
    retries = MAX_RETRY;

    return Status;
}

bool ISL68220_VOUT_write(u8 busnum, u8 slave_addr, u8 *register_content)
{
    u8 retries = MAX_RETRY;
    bool Status = XST_FAILURE;

    u8 write_data[3] = {0};
    write_data[0] = ISL68220_VOUT_COMMAND;
    memcpy(&write_data[1], register_content, 2);

    do
    {
        Status = i2c_send(busnum, slave_addr, write_data, 3);
        retries--;
        if(Status != XST_SUCCESS)
        {
            VMC_ERR("\r\nError in writing VOUT to ISL68220.. Retries  Left: %d\r\n", retries);
        }
    }while((Status != XST_SUCCESS)&&(retries!=0));
    retries = MAX_RETRY;

    return Status;
}

bool ISL68220_VOUT_MARGIN_HIGH_write(u8 busnum, u8 slave_addr, u8 *register_content)
{
    u8 retries = MAX_RETRY;
    bool Status = XST_FAILURE;

    u8 write_data[3] = {0};
    write_data[0] = ISL68220_VOUT_MARGIN_HIGH;
    memcpy(&write_data[1], register_content, 2);

    do
    {
        Status = i2c_send(busnum, slave_addr, write_data, 3);
        retries--;
        if(Status != XST_SUCCESS)
        {
            VMC_ERR("\r\nError in writing VOUT_MARGIN to ISL68220.. Retries  Left: %d\r\n", retries);
        }
    }while((Status != XST_SUCCESS)&&(retries!=0));
    retries = MAX_RETRY;

    return Status;
}

bool ISL68220_VOUT_OV_FAULT_LIMIT_write(u8 busnum, u8 slave_addr, u8 *register_content)
{
    u8 retries = MAX_RETRY;
    bool Status = XST_FAILURE;

    u8 write_data[3] = {0};
    write_data[0] = ISL68220_VOUT_OV_FAULT_LIMIT;
    memcpy(&write_data[1], register_content, 2);

    do
    {
        Status = i2c_send(busnum, slave_addr, write_data, 3);
        retries--;
        if(Status != XST_SUCCESS)
        {
            VMC_ERR("\r\nError in writing VOUT_OV_FAULT_LIMIT to ISL68220.. Retries  Left: %d\r\n", retries);
        }
    }while((Status != XST_SUCCESS)&&(retries!=0));
    retries = MAX_RETRY;

    return Status;
}

bool ISL68220_VOUT_MARGIN_LOW_write(u8 busnum, u8 slave_addr, u8 *register_content)
{
    u8 retries = MAX_RETRY;
    bool Status = XST_FAILURE;

    u8 write_data[3] = {0};
    write_data[0] = ISL68220_VOUT_MARGIN_LOW;
    memcpy(&write_data[1], register_content, 2);

    do
    {
        Status = i2c_send(busnum, slave_addr, write_data, 3);
        retries--;
        if(Status != XST_SUCCESS)
        {
            VMC_ERR("\r\nError in writing VOUT_MARGIN_LOW to ISL68220.. Retries  Left: %d\r\n", retries);
        }
    }while((Status != XST_SUCCESS)&&(retries!=0));
    retries = MAX_RETRY;

    return Status;
}

bool ISL68220_VOUT_UV_FAULT_LIMIT_write(u8 busnum, u8 slave_addr, u8 *register_content)
{
    u8 retries = MAX_RETRY;
    bool Status = XST_FAILURE;

    u8 write_data[3] = {0};
    write_data[0] = ISL68220_VOUT_UV_FAULT_LIMIT;
    memcpy(&write_data[1], register_content, 2);

    do
    {
        Status = i2c_send(busnum, slave_addr, write_data, 3);
        retries--;
        if(Status != XST_SUCCESS)
        {
            VMC_ERR("\r\nError in writing VOUT_UV_FAULT_LIMIT to ISL68220.. Retries  Left: %d\r\n", retries);
        }
    }while((Status != XST_SUCCESS)&&(retries!=0));
    retries = MAX_RETRY;

    return Status;
}


bool ISL68220_set_vout_max(u8 busnum, u8 slave_addr)
{
    u8 retries = MAX_RETRY;
    bool Status = XST_FAILURE;

    u8 write_data[3] = {0};
    write_data[0] = ISL68220_VOUT_MAX;

    //LSB followed by MSB
    write_data[1] = (u8)(MAX_VOUT_MAX_DIRECT_VAL);
    write_data[2] = (u8)(MAX_VOUT_MAX_DIRECT_VAL>>8);

    do
    {
        Status = i2c_send(busnum, slave_addr, write_data, 3);
        retries--;
        if(Status != XST_SUCCESS)
        {
            VMC_ERR("\r\nError in writing ISL68220_set_vout_max to ISL68220.. Retries  Left: %d\r\n", retries);
        }
    }while((Status != XST_SUCCESS)&&(retries!=0));
    retries = MAX_RETRY;

    return Status;
}

bool ISL68220_set_vout_min(u8 busnum, u8 slave_addr)
{
    u8 retries = MAX_RETRY;
    bool Status = XST_FAILURE;

    u8 write_data[3] = {0};
    write_data[0] = ISL68220_VOUT_MIN;

    //LSB followed by MSB
    write_data[1] = (u8)(MIN_VOUT_MIN_DIRECT_VAL);
    write_data[2] = (u8)(MIN_VOUT_MIN_DIRECT_VAL>>8);

    do
    {
        Status = i2c_send(busnum, slave_addr, write_data, 3);
        retries--;
        if(Status != XST_SUCCESS)
        {
            VMC_ERR("\r\nError in writing ISL68220_set_vout_min to ISL68220.. Retries  Left: %d\r\n", retries);
        }
    }while((Status != XST_SUCCESS)&&(retries!=0));
    retries = MAX_RETRY;

    return Status;
}
bool ISL68220_write_register(u8 busnum, u8 SlaveAddr, u8 register_address, u8 *register_content)
{
    u8 retries = MAX_RETRY;
    bool Status = XST_FAILURE;

    u8 write_data[2]= {0};
    write_data[0] = register_address;
    write_data[1] = *register_content;

    do
    {
        Status = i2c_send(busnum, SlaveAddr, (u8 *)write_data, 2);
        retries--;
        if(Status != XST_SUCCESS)
        {
            VMC_ERR("\r\nError in writing ISL68220_set_vout_min to ISL68220.. Retries  Left: %d\r\n", retries);
        }
    }while((Status != XST_SUCCESS)&&(retries!=0));
    retries = MAX_RETRY;

    return Status;
}

bool ISL68220_Read_VCCINT_Voltage(u8 busnum, u8 SlaveAddr, u16 *RegisterValue)
{
    u8 retries = MAX_RETRY;
    bool RetVal = XST_FAILURE;
    u8 temp[2] = {0, 0};
    u16 temp_voltage = 0;
    u8 reg_address = ISL68220_SELECT_PAGE_VCCINT;

    do
    {
        RetVal = ISL68220_write_register(busnum, SlaveAddr, ISL68220_PAGE_REGISTER, &reg_address);
        retries--;
        if ( RetVal != XST_SUCCESS)
        {
            VMC_ERR("\r\nError in writing PAGE REGISTER to ISL68220.. Retries  Left: %d\r\n", retries);
        }
    }while(( RetVal != XST_SUCCESS)&&(retries!=0));
    retries = MAX_RETRY;

    //If the value returned is false, return.
    if( RetVal != XST_SUCCESS)
    {
        return RetVal;
    }

    do
    {
        RetVal = ISL68220_read_register(busnum, SlaveAddr, ISL68220_OUTPUT_VOLTAGE_REGISTER, temp);
        retries--;
        if ( RetVal != XST_SUCCESS)
        {
            VMC_ERR("\r\nError in reading VCCINT Voltage from ISL68220.. Retries  Left: %d\r\n", retries);
        }

    }while(( RetVal != XST_SUCCESS)&&(retries!=0));
    retries = MAX_RETRY;

    //If the value returned is false, return.
    if( RetVal != XST_SUCCESS)
    {
        return RetVal;
    }

   // LOG_INFO_MSG("register value %x %x \r\n",temp[0],temp[1]);

    temp_voltage = (temp[1] << 8) | temp[0];
    *RegisterValue = temp_voltage;

    return RetVal;
}

bool ISL68220_Read_VCCINT_Current(u8 busnum,u8 SlaveAddr, float *RegisterValue)
{
    u8 retries = MAX_RETRY;
    bool RetVal = XST_FAILURE;
    u8 temp[2] = {0, 0};
    u16 temp_current = 0;
    u8 reg_address = ISL68220_SELECT_PAGE_VCCINT;

    do
    {
        RetVal = ISL68220_write_register(busnum, SlaveAddr, ISL68220_PAGE_REGISTER, &reg_address);
        retries--;
        if ( RetVal != XST_SUCCESS)
        {
            VMC_ERR("\r\nError in writing PAGE REGISTER to ISL68220.. Retries  Left: %d\r\n", retries);
        }
    }while(( RetVal != XST_SUCCESS)&&(retries!=0));
    retries = MAX_RETRY;

    //If the value returned is false, return.
    if(RetVal != XST_SUCCESS)

    {
        return RetVal;
    }

    do
    {
        RetVal = ISL68220_read_register(busnum, SlaveAddr, ISL68220_OUTPUT_CURRENT_REGISTER, temp);
        retries--;
        if ( RetVal != XST_SUCCESS)
        {
            VMC_ERR("\r\nError in reading VCCINT Current from ISL68220.. Retries  Left: %d\r\n", retries);
        }
    }while(( RetVal != XST_SUCCESS)&&(retries!=0));
    retries = MAX_RETRY;

    //If the value returned is false, return.
    if(RetVal != XST_SUCCESS)
    {
        return RetVal;
    }

    temp_current = (temp[1] << 8) | temp[0];
    *RegisterValue = (float)temp_current/10;

    return RetVal;
}

bool ISL68220_Read_VCCINT_BRAM_Voltage(u8 busnum, u8 SlaveAddr, u16 *RegisterValue)
{
    u8 retries = MAX_RETRY;
    bool RetVal = XST_FAILURE;
    u8 temp[2] = {0, 0};
    u16 temp_voltage = 0;
    u8 reg_address = ISL68220_SELECT_PAGE_VCCINT_BRAM;

    do
    {
        RetVal = ISL68220_write_register(busnum, SlaveAddr, ISL68220_PAGE_REGISTER, &reg_address);
        retries--;
        if (RetVal != XST_SUCCESS)
        {
            VMC_ERR("\r\nError in writing PAGE REGISTER to ISL68220.. Retries  Left: %d\r\n", retries);
        }
    }while(( RetVal != XST_SUCCESS)&&(retries!=0));
    retries = MAX_RETRY;

    //If the value returned is false, return.
    if( RetVal != XST_SUCCESS)
    {
        return RetVal;
    }

    do
    {
        RetVal = ISL68220_read_register(busnum, SlaveAddr, ISL68220_OUTPUT_VOLTAGE_REGISTER, temp);
        retries--;
        if ( RetVal != XST_SUCCESS)
        {
            VMC_ERR("\r\nError in reading VCCINT BRAM voltage from ISL68220.. Retries  Left: %d\r\n", retries);
        }
    }while(( RetVal != XST_SUCCESS)&&(retries!=0));
    retries = MAX_RETRY;

    //If the value returned is false, return.
    if( RetVal != XST_SUCCESS)
    {
        return RetVal;
    }

    temp_voltage = (temp[1] << 8) | temp[0];
    *RegisterValue = temp_voltage;

    return RetVal;
}

bool ISL68220_Read_VCCINT_BRAM_Current(u8 busnum,u8 SlaveAddr, float *RegisterValue)
{
    u8 retries = MAX_RETRY;
    bool RetVal = XST_FAILURE;
    u8 temp[2] = {0, 0};
    u16 temp_current = 0;
    u8 reg_address = ISL68220_SELECT_PAGE_VCCINT_BRAM;

    do
    {
        RetVal = ISL68220_write_register(busnum, SlaveAddr, ISL68220_PAGE_REGISTER, &reg_address);
        retries--;
        if ( RetVal != XST_SUCCESS)
        {
            VMC_ERR("\r\nError in writing PAGE REGISTER to ISL68220.. Retries  Left: %d\r\n", retries);
        }
    }while(( RetVal != XST_SUCCESS)&&(retries!=0));
    retries = MAX_RETRY;

    //If the value returned is false, return.
    if( RetVal != XST_SUCCESS)
    {
        return RetVal;
    }

    do
    {
        RetVal = ISL68220_read_register(busnum, SlaveAddr, ISL68220_OUTPUT_CURRENT_REGISTER, temp);
        retries--;
        if ( RetVal != XST_SUCCESS)
        {
            VMC_ERR("\r\nError in reading VCCINT BRAM current from ISL68220.. Retries  Left: %d\r\n", retries);
        }
    }while(( RetVal != XST_SUCCESS)&&(retries!=0));
    retries = MAX_RETRY;

    //If the value returned is false, return.
    if( RetVal != XST_SUCCESS)
    {
        return RetVal;
    }

    temp_current = (temp[1] << 8) | temp[0];
    *RegisterValue = (float)temp_current/10;

    return RetVal;
}

bool ISL68220_Read_VCCINT_Temperature(u8 busnum,u8 SlaveAddr, float *RegisterValue)
{
    u8 retries = MAX_RETRY;
    bool RetVal = XST_FAILURE;
    u8 temp[2] = {0, 0};
    int16_t temperature = 0;

    u8 reg_address = ISL68220_SELECT_PAGE_VCCINT;


    do
    {
        RetVal = ISL68220_write_register(busnum, SlaveAddr, ISL68220_PAGE_REGISTER, &reg_address);
        retries--;
        if ( RetVal != XST_SUCCESS)
        {
            VMC_ERR("\r\nError in writing PAGE REGISTER to ISL68220.. Retries  Left: %d\r\n", retries);
        }
    }while(( RetVal != XST_SUCCESS)&&(retries!=0));
    retries = MAX_RETRY;

    //If the value returned is false, return.
    if( RetVal != XST_SUCCESS)
    {
        return RetVal;
    }

    do
    {
        RetVal = ISL68220_read_register(busnum, SlaveAddr, ISL68220_READ_POWERSTAGE_TEMPERATURE, temp);
        retries--;
        if ( RetVal != XST_SUCCESS)
        {
            VMC_ERR("\r\nError in reading VCCINTTemperature from ISL68220.. Retries  Left: %d\r\n", retries);
        }
    }while(( RetVal != XST_SUCCESS)&&(retries!=0));
    retries = MAX_RETRY;

    //If the value returned is false, return.
    if( RetVal != XST_SUCCESS)
    {
        return RetVal;
    }
	temperature = (temp[1] << 8) | temp[0];

    if (temperature > 200 || temperature < -100)    /* temp by MDH because returning all FF's after power on */
    {
        RetVal = XST_FAILURE;
        temperature = 25;
    }

    *RegisterValue = (float)temperature;

    return RetVal;
}

bool ISL68220_Read_VCCINT_BRAM_Temperature(u8 busnum,u8 SlaveAddr, float *RegisterValue)
{
    u8 retries = MAX_RETRY;
    bool RetVal = XST_FAILURE;
    u8 temp[2] = {0, 0};
    u16 temperature = 0;

    u8 reg_address = ISL68220_SELECT_PAGE_VCCINT_BRAM;


    do
    {
        RetVal = ISL68220_write_register(busnum, SlaveAddr, ISL68220_PAGE_REGISTER, &reg_address);
        retries--;
        if ( RetVal != XST_SUCCESS)
        {
            VMC_ERR("\r\nError in writing PAGE Register to ISL68220.. Retries  Left: %d\r\n", retries);
        }
    }while(( RetVal != XST_SUCCESS)&&(retries!=0));
    retries = MAX_RETRY;

    //If the value returned is false, return.
    if( RetVal != XST_SUCCESS)
    {
        return RetVal;
    }

    do
    {
        RetVal = ISL68220_read_register(busnum, SlaveAddr, ISL68220_READ_POWERSTAGE_TEMPERATURE, temp);
        retries--;
        if ( RetVal != XST_SUCCESS)
        {
            VMC_ERR("\r\nError in reading VCCINT Temperature from ISL68220.. Retries  Left: %d\r\n", retries);
        }
    }while(( RetVal != XST_SUCCESS)&&(retries!=0));
    retries = MAX_RETRY;

    //If the value returned is false, return.
    if( RetVal != XST_SUCCESS)
    {
        return RetVal;
    }

    temperature = (temp[1] << 8) | temp[0];
    *RegisterValue = (float)temperature;

    return RetVal;
}

bool ISL68220_Read_Temperature(u8 busnum,u8 SlaveAddr,u8 tempReg, float *RegisterValue)
{
    u8 retries = MAX_RETRY;
    bool RetVal = XST_FAILURE;
    u8 temp[2] = {0, 0};
    int16_t temperature = 0;

    u8 reg_address = ISL68220_SELECT_PAGE_VCCINT;


    do
    {
        RetVal = ISL68220_write_register(busnum, SlaveAddr, ISL68220_PAGE_REGISTER, &reg_address);
        retries--;
        if ( RetVal != XST_SUCCESS)
        {
            VMC_ERR("\r\nError in writing PAGE REGISTER to ISL68220.. Retries  Left: %d\r\n", retries);
        }
    }while(( RetVal != XST_SUCCESS)&&(retries!=0));
    retries = MAX_RETRY;

    //If the value returned is false, return.
    if( RetVal != XST_SUCCESS)
    {
        return RetVal;
    }

    do
    {
        RetVal = ISL68220_read_register(busnum, SlaveAddr, tempReg, temp);
        retries--;
        if ( RetVal != XST_SUCCESS)
        {
            VMC_ERR("\r\nError in reading VCCINTTemperature from ISL68220.. Retries  Left: %d\r\n", retries);
        }
    }while(( RetVal != XST_SUCCESS)&&(retries!=0));
    retries = MAX_RETRY;

    //If the value returned is false, return.
    if( RetVal != XST_SUCCESS)
    {
        return RetVal;
    }
	temperature = (temp[1] << 8) | temp[0];

    if (temperature > 200 || temperature < -100)    /* temp by MDH because returning all FF's after power on */
    {
        RetVal = XST_FAILURE;
        temperature = 25;
    }

    *RegisterValue = (float)temperature;

    return RetVal;
}



