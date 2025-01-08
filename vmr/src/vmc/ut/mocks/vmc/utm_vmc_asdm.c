/******************************************************************************
* Copyright (C) 2024 Advanced Micro Devices, Inc.    All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <cmocka.h>

#include "xil_types.h"
#include "vmc_api.h"

supported_sdr_info_ptr get_supported_sdr_info;
asdm_update_record_count_ptr asdm_update_record_count;

s8 __wrap_Asdm_Process_Sensor_Request(u8 *req, u8 *resp, u16 *respSize)
{
    return 0;
}


void __wrap_Asdm_Update_Sensors(void)
{

}
