/******************************************************************************
* Copyright (C) 2024 Advanced Micro Devices, Inc.    All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/


#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <cmocka.h>

#include "cl_mem.h"
#include "vmc_api.h"
#include "vmc_sensors.h"
#include "vmc_main.h"

Versal_BoardInfo board_info = {

.eeprom_version = "2.0",
.product_name   = "VCK5000-PP",
.board_rev      = "A",
.board_serial   = "XFL134CBTKZP",
.board_mac[0]   = {0x00, 0x0a, 0x35, 0x0c, 0xf6, 0x66},
.board_mac[1]   = {0x00, 0x0a, 0x35, 0x0c, 0xf6, 0x66},
.board_act_pas  = "A",
.board_config_mode = {0x08},
.board_mfg_date = {0xdd, 0x63, 0xcd},
.board_part_num = "05025-04",
.board_uuid     = {0xe2, 0x36, 0xf9, 0x4e, 0xcf, 0xf2, 0x43, 0xe8, 0x97, 0xd3, 0x04, 0x6c, 0x77, 0xc8, 0x93, 0xaf},
.board_pcie_info = {0x10, 0xee, 0x50, 0x48, 0x10, 0xee, 0x00, 0x0e},
.OEM_ID         = {0x00, 0x00, 0x10, 0xda},
};

/*****************************Mock functions *******************************/
void __wrap_VMC_Get_BoardInfo(Versal_BoardInfo *_board_info)
{
	/*Do Nothing for now*/
}

