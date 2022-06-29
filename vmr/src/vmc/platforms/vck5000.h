/******************************************************************************
* Copyright (C) 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#ifndef INC_PLATFORMS_VCK5000_H_
#define INC_PLATFORMS_VCK5000_H_

#include "../vmc_asdm.h"

//Slave Addresses
#define SLAVE_ADDRESS_SE98A_0 0x18
#define SLAVE_ADDRESS_SE98A_1 0x19
#define SLAVE_ADDRESS_MAX6639 0x2E

/*Power threshold limits for vck5000 */
#define POWER_12VPEX_CRITICAL_THRESHOLD 69.000
#define POWER_12VAUX_2X3_CRITICAL_THRESHOLD 78.000
#define POWER_12VAUX_2X4_CRITICAL_THRESHOLD 153.000

/* Temp threshold limits for vck5000 */
#define TEMP_VCCINT_CRITICAL_THRESHOLD  110.000
#define TEMP_QSFP_CRITICAL_THRESHOLD    85.000
#define TEMP_FPGA_CRITICAL_THRESHOLD    100.000

#define MAX6639_FAN_TACHO_TO_RPM(x) (8000*60)/(x)

u8 Vck5000_Init(void);

s8 Vck5000_Temperature_Read_Inlet(snsrRead_t *snsrData);
s8 Vck5000_Temperature_Read_Outlet(snsrRead_t *snsrData);
s8 Vck5000_Temperature_Read_Board(snsrRead_t *snsrData);
s8 Vck5000_Temperature_Read_QSFP(snsrRead_t *snsrData);
s8 Vck5000_Fan_RPM_Read(snsrRead_t *snsrData);

void se98a_monitor(void);
void max6639_monitor(void);
void qsfp_monitor(void);


#endif
