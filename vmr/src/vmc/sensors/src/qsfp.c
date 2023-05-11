/******************************************************************************
* Copyright (C) 2023 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#include "xstatus.h"
#include "cl_i2c.h"
#include "cl_log.h"
#include "../inc/qsfp.h"
#include "../../vmc_api.h"

/* FreeRTOS includes */
#include <FreeRTOS.h>
#include <task.h>

u8 I2CNumPmc = 0;

u8 ucQSFPReadTemperature( float *pfTemperatureValue, u8 ucSensorInstance )
{
    u8 ucStatus                         = XST_FAILURE;
    const TickType_t xBlockTime         = pdMS_TO_TICKS( 10 );
    unsigned char ucLsbTempReg          = QSFP_LSB_TEMPERATURE_REG;
    unsigned char ucMsbTempReg          = QSFP_MSB_TEMPERATURE_REG;
    unsigned char ucTemperatureBuff[2]  = {0};
    u16 usTemperatureHexValue           = 0;
    long int ulI2CReadLen               = 1;
    u32 ulMIO                           = 0;
    u32 ulQSFPBits                      = 0;
    volatile u32  ulReadAddress         = QSFP_LOW_SPEED_IO_READ_OFFSET;
    volatile u32  ulWriteAddress        = QSFP_LOW_SPPED_IO_WRITE_OFFSET;

    ulMIO = Xil_In32( ulReadAddress );

    /*  IO pins : QSFP 0 is bits 14 - 18 , QSFP 1 is bits 19-23
        bit 14 MODSEL_L QSFP0
        bit 15 RESET_L  QSFP0
        bit 16 MODPRS_L QSFP0
        bit 17 INT_L    QSFP0
        bit 18 LPMODE   QSFP0

        bit 19 MODSEL_L QSFP1
        bit 20 RESET_L  QSFP1
        bit 21 MODPRS_L QSFP1
        bit 22 INT_L    QSFP1
        bit 23 LPMODE   QSFP1  */

    ulQSFPBits = ( ulMIO >> ( 14 + ( ucSensorInstance * 5 ) ) ) & 0x1F;

    /* check for QSFP module presence */

    if( !( ( ulQSFPBits & 0x4 ) >> 2) )
    {
        //CL_LOG (APP_VMC ,"QSFP_%d module present",i);

        ulMIO = ulMIO & ~( 1 << ( 14 + ( ucSensorInstance * 5 ) ) ); // clear MODSEL to select QSFP 0 or 1 based on QSFP index

        Xil_Out32( ulWriteAddress, ulMIO );

        vTaskDelay( xBlockTime ); // Required to update MODSEL status on MIO

        ucStatus = i2c_send_rs_recv( I2CNumPmc, QSFP_SLAVE_ADDRESS, &ucLsbTempReg, 1, &ucTemperatureBuff[0], ulI2CReadLen );
        if( XST_FAILURE == ucStatus )
        {
            CL_LOG( APP_VMC ,"failed to read QSFP LSB temperature register" );
            return ucStatus;
        }

        ucStatus = i2c_send_rs_recv(I2CNumPmc, QSFP_SLAVE_ADDRESS, &ucMsbTempReg, 1, &ucTemperatureBuff[1], ulI2CReadLen);
        if ( XST_FAILURE == ucStatus )
        {
            CL_LOG( APP_VMC ,"failed to read QSFP MSB temperature register" );
            return ucStatus;
        }

        /* Store MS byte f temperature. */
        usTemperatureHexValue = ( ucTemperatureBuff[1] << 8 ) | ucTemperatureBuff[0];

        /* Temperature reading is a signed 16 bit value with a resolution of 1/256 C.
         * The total range is from -128 to +128
         * MSB bits have the following weights:
         *  Bit 15: Sign bit
         *  Bit 14: 64 C (2^6)
         *  Bit 13: 32 C (2^5)
         *  Bit 12: 16 C (2^4)
         *  Bit 11: 8 C (2^3)
         *  Bit 10: 4 C (2^2)
         *  Bit 9:  2 C (2^1)
         *  Bit 8:  1 C (2^0)
         *
         * LSB bits have the following weights:
         *  Bit 7:  0.5 C (2^-1)
         *  Bit 6:  0.25 C (2^-2)
         *  Bit 5:  0.125 C (2^-3)
         *  Bit 4:  0.625 C (2^-4)
         *  Bit 3:  0.03125 C (2^-5)
         *  Bit 2:  0.015625 C (2^-6)
         *  Bit 1:  0.0078125 C (2^-7)
         *  Bit 0:  0.00390625 C (2^-8)
         * */


        /* From the above calculations, 0x7FFF is the largest temperature value that
         * can be read regardless of the signed bit.
         *
         * Since the 15th bit is the sign bit , if we did read a negative temperature,
         * then the value will be greater than 0x7FFF.
         *
         * For +ve temperature:
         * TempHexVal * (2^-8) = Actual Temperature
         *
         * For -ve Temperature:
         * Instead of taking the 2s complement, another approach to get the negative
         * temperature would be to subtract the read temperature from 2^15 and
         * then multiply with the resolution(00390625 C in out case) and add a -ve sign.
         * (0x8000 - TempHexVal) * (2^-8) * (-1) = Actual Temperature
         *
         */
        if( QSFP_MAX_POSITIVE_TEMP >= usTemperatureHexValue ) /* +ve Temperature */
        {
            *pfTemperatureValue = usTemperatureHexValue * QSFP_TEMPERATURE_RESOLUTION;
        }
        else /* -ve Temperature */
        {
            /* Ignore the signed bit here, since we have already determined that it is a
             * negative temperature */
            usTemperatureHexValue = usTemperatureHexValue & QSFP_TEMP_BIT_MASK;
            *pfTemperatureValue = ( QSFP_MAX_NEGATIVE_TEMP - usTemperatureHexValue ) * QSFP_TEMPERATURE_RESOLUTION * ( -1 );
        }

        ulMIO = ulMIO | ( 1 << ( 14 + ( ucSensorInstance * 5 ) ) ); //set MODSEL to de-select QSFP 0 or 1 based on QSFP index

        Xil_Out32( ulWriteAddress, ulMIO );
    }
    else
    {
        ucStatus = XST_DEVICE_NOT_FOUND;
    }
    return ucStatus;
}

u8 ucQSFPI2CMuxReadTemperature( float *pfTemperatureValue, u8 ucSensorInstance )
{
    u8 ucStatus                         = XST_SUCCESS;
/* TODO Remove the test code before merging back to VMR main */
#ifdef V80_ON_V70
    *pfTemperatureValue = 32+ucSensorInstance;
    if( 2 == ucSensorInstance )
    {
        ucStatus = XST_DEVICE_NOT_FOUND;
    }
    return ucStatus;
#else
    unsigned char ucQSFPControlRegister = QSFP_IO_EXPANDER_INPUT_REG;
    unsigned char ucQSFPControlInput    = 0;
    unsigned char ucLsbTempReg          = QSFP_LSB_TEMPERATURE_REG;
    unsigned char ucMsbTempReg          = QSFP_MSB_TEMPERATURE_REG;
    unsigned char ucTemperatureBuff[2]  = { 0 };
    u16 usTemperatureHexValue           = 0;
    long int ulI2CReadLen               = 1;

    /*  First check for QSFP module presence 
        Select Mux Address 0x70 for QSFPs 0 or 1 
        Select Mux Address 0x71 for QSFPs 2 or 3

        Next select the bit in the control register to pass through 
        the SCL and SDA signals to the correct IO Expander at address 0x20 
        Its bit 0 for QSFP 0 or 2, bit 2 for QSFP 1 or 3

        Read register 0 of the IO Expander, bit 4 is the Mod Present Line
        If present select the bit in the control register to pass through 
        the SCL and SDA signals to the correct QSFP at address 0x50 
        Its bit 1 for QSFP 0 or 2, bit 3 for QSFP 1 or 3

        Read the Temperature register

        Set Mux selection to none
    */
    u8 MuxControlRegisterIOExpander     = 0;
    u8 MuxControlRegisterQSFP           = 0;
    u8 MuxAddress                       = 0;
    switch( ucSensorInstance )
    {
        case 0:
            MuxAddress = QSFP_MUX0_ADDRESS;
            MuxControlRegisterIOExpander = 1 << 0;
            MuxControlRegisterQSFP = 1 << 1;
            break;

        case 1:
            MuxAddress = QSFP_MUX0_ADDRESS;
            MuxControlRegisterIOExpander = 1 << 2;
            MuxControlRegisterQSFP = 1 << 3;
            break;

        case 2:
            MuxAddress = QSFP_MUX1_ADDRESS;
            MuxControlRegisterIOExpander = 1 << 0;
            MuxControlRegisterQSFP = 1 << 1;
            break;

        case 3:
            MuxAddress = QSFP_MUX1_ADDRESS;
            MuxControlRegisterIOExpander = 1 << 2;
            MuxControlRegisterQSFP = 1 << 3;
            break;

        default:
            ucStatus = XST_FAILURE;
            break;
    }

    if( XST_SUCCESS == ucStatus )
    {
        /* Select the output to the IO Expander */
        ucStatus = i2c_send( I2CNumPmc, MuxAddress, &MuxControlRegisterIOExpander, 1 );
        if( XST_FAILURE == ucStatus )
        {
            CL_LOG ( APP_VMC ,"failed to set QSFP Mux control register" );
        }
        else
        {
            /* Read the MODPRES line to determine if the QSFP is present */ 
            ucStatus = i2c_send_rs_recv(I2CNumPmc, QSFP_IO_EXPANDER_ADDRESS, &ucQSFPControlRegister, 1, &ucQSFPControlInput, ulI2CReadLen);
            if(ucStatus == XST_FAILURE)
            {
                CL_LOG (APP_VMC ,"failed to read QSFP_IO_EXPANDER_ADDRESS input register");
            }
            else
            {
                /* If MODPRES is 0 the QSFP is present */
                if( !( ucQSFPControlInput & QSFP_MODPRES_L_BIT ) )
                {
                    /* QSFP is present so go ahead and read its temperature */
                    /* Select the output to the QSFP */
                    ucStatus = i2c_send( I2CNumPmc, MuxAddress, &MuxControlRegisterQSFP, 1 );
                    if( XST_FAILURE == ucStatus )
                    {
                        CL_LOG ( APP_VMC ,"failed to set QSFP Mux control register" );
                    }
                    else
                    {
                        ucStatus = i2c_send_rs_recv(I2CNumPmc, QSFP_SLAVE_ADDRESS, &ucLsbTempReg, 1, &ucTemperatureBuff[0], ulI2CReadLen);
                        if(ucStatus == XST_FAILURE)
                        {
                            CL_LOG (APP_VMC ,"failed to read QSFP LSB temperature register");
                        }
                        else
                        {
                            ucStatus = i2c_send_rs_recv(I2CNumPmc, QSFP_SLAVE_ADDRESS, &ucMsbTempReg, 1, &ucTemperatureBuff[1], ulI2CReadLen);
                            if(ucStatus == XST_FAILURE)
                            {
                                CL_LOG (APP_VMC ,"failed to read QSFP MSB temperature register");
                            }
                            else
                            {
                                /* Store MS byte f temperature. */
                                usTemperatureHexValue = ( ucTemperatureBuff[1] << 8 ) | ucTemperatureBuff[0];

                                /* Temperature reading is a signed 16 bit value with a resolution of 1/256 C.
                                * The total range is from -128 to +128
                                * MSB bits have the following weights:
                                *  Bit 15: Sign bit
                                *  Bit 14: 64 C (2^6)
                                *  Bit 13: 32 C (2^5)
                                *  Bit 12: 16 C (2^4)
                                *  Bit 11: 8 C (2^3)
                                *  Bit 10: 4 C (2^2)
                                *  Bit 9:  2 C (2^1)
                                *  Bit 8:  1 C (2^0)
                                *
                                * LSB bits have the following weights:
                                *  Bit 7:  0.5 C (2^-1)
                                *  Bit 6:  0.25 C (2^-2)
                                *  Bit 5:  0.125 C (2^-3)
                                *  Bit 4:  0.625 C (2^-4)
                                *  Bit 3:  0.03125 C (2^-5)
                                *  Bit 2:  0.015625 C (2^-6)
                                *  Bit 1:  0.0078125 C (2^-7)
                                *  Bit 0:  0.00390625 C (2^-8)
                                * */


                                /* From the above calculations, 0x7FFF is the largest temperature value that
                                * can be read regardless of the signed bit.
                                *
                                * Since the 15th bit is the sign bit , if we did read a negative temperature,
                                * then the value will be greater than 0x7FFF.
                                *
                                * For +ve temperature:
                                * TempHexVal * (2^-8) = Actual Temperature
                                *
                                * For -ve Temperature:
                                * Instead of taking the 2s complement, another approach to get the negative
                                * temperature would be to subtract the read temperature from 2^15 and
                                * then multiply with the resolution(00390625 C in out case) and add a -ve sign.
                                * (0x8000 - TempHexVal) * (2^-8) * (-1) = Actual Temperature
                                *
                                */
                                if( usTemperatureHexValue <= QSFP_MAX_POSITIVE_TEMP ) /* +ve Temperature */
                                {
                                    *pfTemperatureValue = usTemperatureHexValue * QSFP_TEMPERATURE_RESOLUTION;
                                }
                                else /* -ve Temperature */
                                {
                                    /* Ignore the signed bit here, since we have already determined that it is a
                                    * negative temperature */
                                    usTemperatureHexValue = usTemperatureHexValue & QSFP_TEMP_BIT_MASK;
                                    *pfTemperatureValue = ( QSFP_MAX_NEGATIVE_TEMP - usTemperatureHexValue ) * QSFP_TEMPERATURE_RESOLUTION * ( -1 );
                                }
                            }
                        }
                    }
                }
                else
                {
                    ucStatus = XST_DEVICE_NOT_FOUND;
                }
            }
        }
    }

    return ucStatus;
#endif
}