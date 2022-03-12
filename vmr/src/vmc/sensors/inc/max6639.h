

/******************************************************************************
 * * Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
 * * SPDX-License-Identifier: MIT
 * *******************************************************************************/


#ifndef MAX6639_H_
#define MAX6639_H_
 
#include "xil_types.h"

/**
 *  * @brief Registers Address List
 *   */
#define CHANNEL1_TEMPERATURE_REGISTER           0x00
#define CHANNEL2_TEMPERATURE_REGISTER           0x01
#define STATUS_REGISTER                         0x02
#define GLOBAL_CONFIGURATION_REGISTER           0x04
#define CHANNEL1_EXTENDED_TEMPERATURE_REGISTER  0x05
#define CHANNEL2_EXTENDED_TEMPERATURE_REGISTER  0x06
#define FAN1_CONFIGURATION1_REGISTER            0x10
#define FAN1_CONFIGURATION2A_REGISTER           0x11
#define FAN1_CONFIGURATION2B_REGISTER           0x12
#define FAN1_CONFIGURATION3_REGISTER            0x13
#define FAN2_CONFIGURATION1_REGISTER            0x14
#define FAN2_CONFIGURATION2A_REGISTER           0x15
#define FAN2_CONFIGURATION2B_REGISTER           0x16
#define FAN2_CONFIGURATION3_REGISTER            0x17
#define FAN1_TACHOMETER_COUNT_REGISTER          0x20
#define FAN2_TACHOMETER_COUNT_REGISTER          0x21
#define FAN1_START_TACH_COUNT_REGISTER          0x22
#define FAN2_START_TACH_COUNT_REGISTER          0x23
#define FAN1_MIN_TACH_COUNT_REGISTER            0x24
#define FAN2_MIN_TACH_COUNT_REGISTER            0x25
#define FAN1_CURRENT_DUTY_CYCLE_REGISTER        0x26
#define FAN2_CURRENT_DUTY_CYCLE_REGISTER        0x27
#define CHANNEL1_MIN_FAN_START_TEMP_REGISTER    0x28
#define CHANNEL2_MIN_FAN_START_TEMP_REGISTER    0x29

#define DEVICE_ID_REGISTER                      0x3D
#define MANUFACTURER_ID_REGISTER                0x3E
#define DEVICE_REVISION_REGISTER                0x3F

u8 max6639_init(u8 i2c_num, u8 SlaveAddr);

u8 max6639_read_register(u8 i2c_num, u8 SlaveAddr, u8 register_address, u8 *register_content);

u8 max6639_ReadFPGATemperature(u8 i2c_num, u8 SlaveAddr, float *TemperatureReading);

u8 max6639_ReadDDRTemperature(u8 i2c_num, u8 SlaveAddr, float *TemperatureReading);

u8 max6639_ReadFanTach(u8 i2c_num, u8 SlaveAddr, u8 fanIndex, u8 *fanSpeed);


#endif /* MAX6639_H_ */
