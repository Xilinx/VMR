/******************************************************************************
 * Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
 * SPDX-License-Identifier: MIT
 *******************************************************************************/

#include "xil_types.h"

/**
 * @brief Registers Address List
 *
 * Refer to QSFP management standard here: SFF-8636
 * Register definitions are in page 26.
 */

#define QSFP_SLAVE_ADDRESS 0x50

#define QSFP_TEMPERATURE_SENSOR_NUM 2

/** @brief QSFP MSB temperature register
 * PMC GPIO Controller
 */

#define QSFP_LOW_SPEED_IO_READ_OFFSET  (u32)(0xF1020064)
#define QSFP_LOW_SPPED_IO_WRITE_OFFSET (u32)(0xF1020044)

/* @brief QSFP MSB temperature register */
#define QSFP_MSB_TEMPERATURE_REG    (22)

/* @brief QSFP LSB temperature register */
#define QSFP_LSB_TEMPERATURE_REG    (23)

#define QSFP_TEMPERATURE_RESOLUTION (1.0/256.0)

/* @brief Macro to define maximum possible positive temperature that can be read. */
#define QSFP_MAX_POSITIVE_TEMP      (0x7FFF)

/* @brief Macro to define mask for only the temperature bits discarding the signed bits. */
#define QSFP_TEMP_BIT_MASK          (0x7FFF)

/* @brief Macro to define maximum possible negative temperature that can be read */
#define QSFP_MAX_NEGETIVE_TEMP      (0x8000)


u8 QSFP_ReadTemperature(float *TemperatureValue, u8 sensorInstance);
