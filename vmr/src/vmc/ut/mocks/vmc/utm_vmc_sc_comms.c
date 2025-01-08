/******************************************************************************
* Copyright (C) 2024 Advanced Micro Devices, Inc.    All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <cmocka.h>

#include "FreeRTOS.h"
#include "task.h"
#include "cl_mem.h"
#include "vmc_sc_comms.h"
#include "vmc_update_sc.h"

static u8 platform_specific_msgid_count = 0;
SC_VMC_Data sc_vmc_data;

Fetch_BoardInfo_Func fetch_boardinfo_ptr = NULL;
msg_id_ptr msg_id_handler_ptr = { 0 };

/*****************************Real function definitions*******************************/
/*This definition is same as real implementation.
 * It does not need mock features, since it is very simple definition*/
void set_total_req_size(u8 value)
{
	platform_specific_msgid_count = value;
}
