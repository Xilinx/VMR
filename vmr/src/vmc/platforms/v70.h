/******************************************************************************
* Copyright (C) 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#ifndef INC_PLATFORMS_V70_H_
#define INC_PLATFORMS_V70_H_

#include "../vmc_asdm.h"

/*Power threshold limits for vck5000 */
#define POWER_12VPEX_CRITICAL_THRESHOLD 69.000
#define POWER_3V3PEX_CRITICAL_THRESHOLD 10.400

#define V70_TEMP_VCCINT_CRITICAL_THRESHOLD  110.000

u8 V70_Init(void);
s8 V70_Temperature_Read_Inlet(snsrRead_t *snsrData);
s8 V70_Temperature_Read_Outlet(snsrRead_t *snsrData);
s8 V70_Temperature_Read_Board(snsrRead_t *snsrData);
s32 V70_VMC_Fetch_BoardInfo(u8 *board_snsr_data);
s8 V70_Asdm_Read_Power(snsrRead_t *snsrData);

#endif
