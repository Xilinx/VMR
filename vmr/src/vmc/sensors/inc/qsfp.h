/******************************************************************************
 * Copyright (C) 2024 AMD, Inc.    All rights reserved.
 * SPDX-License-Identifier: MIT
 *******************************************************************************/

#include "xil_types.h"

/**
 * @brief Registers Address List
 *
 * Refer to QSFP management standard here: SFF-8636
 * Register definitions are in page 26.
 */

#define QSFP_SLAVE_ADDRESS              0x50
#define QSFP_IO_EXPANDER_ADDRESS        0x20
#define QSFP_MUX0_ADDRESS               0x70
#define QSFP_MUX1_ADDRESS               0x71

#define QSFP_TEMPERATURE_SENSOR_NUM     4

/** @brief QSFP MSB temperature register
 * PMC GPIO Controller
 */

/* @brief QSFP LSB temperature register */
#define QSFP_IO_EXPANDER_INPUT_REG      ( 0 )
#define QSFP_MODSEL_L_BIT               ( 0x1 )
#define QSFP_RESET_L_BIT                ( 0x2 )
#define QSFP_LPMODE_BIT                 ( 0x4 )
#define QSFP_MODPRES_L_BIT              ( 0x8 )
#define QSFP_INTR_B_BIT                 ( 0x10 )
#define QSFP_LOW_SPEED_IO_READ_OFFSET   ( u32 )( 0xF1020064 )
#define QSFP_LOW_SPPED_IO_WRITE_OFFSET  ( u32 )( 0xF1020044 )

/* @brief QSFP MSB temperature register */
#define QSFP_MSB_TEMPERATURE_REG        ( 22 )

/* @brief QSFP LSB temperature register */
#define QSFP_LSB_TEMPERATURE_REG        ( 23 )

#define QSFP_TEMPERATURE_RESOLUTION     ( 1.0/256.0 )

/* @brief Macro to define maximum possible positive temperature that can be read. */
#define QSFP_MAX_POSITIVE_TEMP          ( 0x7FFF )

/* @brief Macro to define mask for only the temperature bits discarding the signed bits. */
#define QSFP_TEMP_BIT_MASK              ( 0x7FFF )

/* @brief Macro to define maximum possible negative temperature that can be read */
#define QSFP_MAX_NEGATIVE_TEMP          ( 0x8000 )

u8 ucQSFPReadTemperature( float *pfTemperatureValue, u8 ucSensorInstance );
u8 ucQSFPI2CMuxReadTemperature( float *pfTemperatureValue, u8 ucSensorInstance );
