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

#ifndef INCREASE_LIMITS_FOR_TEST
/*Power threshold limits for vck5000 */
#define POWER_12VPEX_CRITICAL_THRESHOLD 69.000
#define POWER_12VAUX_2X3_CRITICAL_THRESHOLD 78.000
#define POWER_12VAUX_2X4_CRITICAL_THRESHOLD 153.000

#define POWER_CRITICAL_THRESHOLD	(POWER_12VPEX_CRITICAL_THRESHOLD + \
					 POWER_12VAUX_2X3_CRITICAL_THRESHOLD + \
					 POWER_12VAUX_2X4_CRITICAL_THRESHOLD)

/*Power and Temp Throttling limits for vck5000 */
#define POWER_12VPEX_THROTTLE_THRESHOLD 	66.000
#define POWER_12VAUX_2X3_THROTTLE_THRESHOLD 	75.000
#define POWER_12VAUX_2X4_THROTTLE_THRESHOLD 	150.000

#define VCK5000_POWER_THROTTLING_THRESHOLD_LIMIT		(POWER_12VPEX_THROTTLE_THRESHOLD + \
						POWER_12VAUX_2X3_THROTTLE_THRESHOLD + \
						POWER_12VAUX_2X4_THROTTLE_THRESHOLD)

#define VCK50000_TEMP_THROTTLING_THRESHOLD_LIMIT 95

/* Temp threshold limits for vck5000 */
#define TEMP_VCCINT_CRITICAL_THRESHOLD  110.000
#define TEMP_QSFP_CRITICAL_THRESHOLD    85.000
#define TEMP_FPGA_CRITICAL_THRESHOLD    100.000

#define VCK5000_FPGA_THROTTLING_TEMP_LIMIT    95
#define VCK5000_VCCINT_THROTTLING_TEMP_LIMIT 105

#else
#define INCREASE_Y_PERCENT(x)  (x * (0.25))

/*Power threshold limits for vck5000 */
#define POWER_12VPEX_CRITICAL_THRESHOLD (69.000 + INCREASE_Y_PERCENT(69.000))
#define POWER_12VAUX_2X3_CRITICAL_THRESHOLD (78.000 + INCREASE_Y_PERCENT(78.000))
#define POWER_12VAUX_2X4_CRITICAL_THRESHOLD (153.000 + INCREASE_Y_PERCENT(153.000))

#define POWER_CRITICAL_THRESHOLD	(POWER_12VPEX_CRITICAL_THRESHOLD + \
					 POWER_12VAUX_2X3_CRITICAL_THRESHOLD + \
					 POWER_12VAUX_2X4_CRITICAL_THRESHOLD)

/*Power and Temp Throttling limits for vck5000 */
#define POWER_12VPEX_THROTTLE_THRESHOLD 	(66.000 + INCREASE_Y_PERCENT(66.000))
#define POWER_12VAUX_2X3_THROTTLE_THRESHOLD 	(75.000 + INCREASE_Y_PERCENT(75.000))
#define POWER_12VAUX_2X4_THROTTLE_THRESHOLD 	(150.000 + INCREASE_Y_PERCENT(150.000))

#define VCK5000_POWER_THROTTLING_THRESHOLD_LIMIT		(POWER_12VPEX_THROTTLE_THRESHOLD + \
						POWER_12VAUX_2X3_THROTTLE_THRESHOLD + \
						POWER_12VAUX_2X4_THROTTLE_THRESHOLD)

#define VCK50000_TEMP_THROTTLING_THRESHOLD_LIMIT (95 + INCREASE_Y_PERCENT(95.000))

#define TEMP_VCCINT_CRITICAL_THRESHOLD  (110.000 + INCREASE_Y_PERCENT(110.000))
#define TEMP_QSFP_CRITICAL_THRESHOLD    (85.000 + INCREASE_Y_PERCENT(85.000))
#define TEMP_FPGA_CRITICAL_THRESHOLD    (100.000 + INCREASE_Y_PERCENT(100.000))

#define VCK5000_FPGA_THROTTLING_TEMP_LIMIT    (95 + INCREASE_Y_PERCENT(95))
#define VCK5000_VCCINT_THROTTLING_TEMP_LIMIT (105 + INCREASE_Y_PERCENT(105))

#endif

#define MAX6639_FAN_TACHO_TO_RPM(x) (8000*60)/(x)

u8 Vck5000_Init(void);

/*
 * VCK5000 Sensor IDs Fetched from SC
 */
typedef enum
{
    eSC_PEX_12V = 0,
    eSC_PEX_3V3,
    eSC_AUX_3V3,
    eSC_AUX_12V,
    eSC_AUX1_12V,

    eSC_VCCINT_I,
    eSC_PEX_12V_I_IN,
    eSC_V12_IN_AUX0_I,
    eSC_V12_IN_AUX1_I,

    eSC_VCCINT_TEMP,
    eSC_FAN_SPEED,
    eSC_BMC_VERSION,
    eSC_POWER_MODE,
    eSC_POWER_GOOD,
    eSC_SENSOR_ID_MAX

}vck5000_sensor_id_list;

s8 Vck5000_Temperature_Read_Inlet(snsrRead_t *snsrData);
s8 Vck5000_Temperature_Read_Outlet(snsrRead_t *snsrData);
s8 Vck5000_Temperature_Read_Board(snsrRead_t *snsrData);
s8 Vck5000_Temperature_Read_QSFP(snsrRead_t *snsrData);
s8 Vck5000_Fan_RPM_Read(snsrRead_t *snsrData);
s8 Vck5000_Asdm_Read_Power(snsrRead_t *snsrData);
s8 Vck5000_Temperature_Read_Vccint(snsrRead_t *snsrData);


void se98a_monitor(void);
void max6639_monitor(void);
void qsfp_monitor(void);

u8 Vck5000_Vmc_Sc_Comms(void);
s32 Vck5000_VMC_Fetch_BoardInfo(u8 *board_snsr_data);

s8 Vck5000_Get_QSFP_Name(u8 index, char8* snsrName, u8 *SC_sensorId,sensorMonitorFunc *sensor_handler);
s8 Vck5000_Get_Voltage_Names(u8 index, char8* snsrName, u8 *SC_sensorId,sensorMonitorFunc *sensor_handler);
s8 Vck5000_Get_Current_Names(u8 index, char8* snsrName, u8 *SC_sensorId,sensorMonitorFunc *sensor_handler);



#endif
