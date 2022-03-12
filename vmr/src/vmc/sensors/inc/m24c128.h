/******************************************************************************
 * Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
 * SPDX-License-Identifier: MIT
 *******************************************************************************/

#include "xil_types.h"

#define SLAVE_ADDRESS_M24C128 0x52

/* Default register content in EEPROM if a particular register has not been programmed */
#define EEPROM_DEFAULT_VAL               0xFF

//The following size/offset are based on the v1.8 EEPROM spec
#define EEPROM_VERSION_OFFSET            0x0000
#define EEPROM_VERSION_SIZE              3

#define EEPROM_PRODUCT_NAME_OFFSET       0x0300
#define EEPROM_PRODUCT_NAME_SIZE         16

#define EEPROM_BOARD_REV_OFFSET          0x1300
#define EEPROM_BOARD_REV_SIZE            8

#define EEPROM_BOARD_SERIAL_OFFSET       0x1B00
#define EEPROM_BOARD_SERIAL_SIZE         14

#define EEPROM_BOARD_MAC_OFFSET          0x2900 // EEPROM offset for the first MAC address
#define EEPROM_BOARD_MAC_SIZE            6
#define EEPROM_BOARD_NUM_MAC             2    // The EEPROM has 4 MAC addresses

#define EEPROM_BOARD_ACT_PAS_OFFSET      0x4100
#define EEPROM_BOARD_ACT_PAS_SIZE        1

#define EEPROM_BOARD_CONFIG_MODE_OFFSET  0x4200
#define EEPROM_BOARD_CONFIG_MODE_SIZE    1

#define EEPROM_MFG_DATE_OFFSET           0x4300
#define EEPROM_MFG_DATE_SIZE             3

#define EEPROM_PART_NUM_OFFSET           0x4600
#define EEPROM_PART_NUM_SIZE             9

#define EEPROM_UUID_OFFSET               0x4F00
#define EEPROM_UUID_SIZE                 16

#define EEPROM_PCIE_INFO_OFFSET          0x5F00
#define EEPROM_PCIE_INFO_SIZE            8

#define EEPROM_MAX_POWER_MODE_OFFSET     0x6700
#define EEPROM_MAX_POWER_MODE_SIZE       1

#define EEPROM_DIMM_SIZE_OFFSET          0x6800
#define EEPROM_DIMM_SIZE_SIZE            4

#define EEPROM_OEMID_SIZE_OFFSET         0x6C00
#define EEPROM_OEMID_SIZE                4

u8 M24C128_ReadByte(u8 i2c_num, u8 slaveAddr, u16 *AddressOffset,u8 *RegisterValue);
