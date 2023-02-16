#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <stdio.h>
#include <cmocka.h>

#include "vmc_sensors.h"
#include "vmc_asdm.h"
#include "cl_log.h"
#include "vmc_api.h"
#include "vmc_sc_comms.h"
#include "cl_mem.h"
#include <semphr.h>
#include "v70.h"

/*This enum and structure is created only for the purpose of asserting SDRs in UT*/
typedef enum
{
	eBoardInfoSDR_offset = 0,
	eTemperatureSDR_offset = 11,
	eVoltageSDR_offset = 14,
	eCurrentSDR_offset = 17,
	ePowerSDR_offset = 20
}Assert_SdrOffset_t;

typedef struct  Assert_Sdr_s
{
    u8 sensor_id;
    u8 sensor_name_type_length;
    char8 sensor_name[20];
    u8 sensor_value_type_length;
    u8 sensor_value[20];
    u8 sensor_base_unit_type_length;
    u8 sensor_base_unit[10];
    s8 sensor_unit_modifier_byte;
    u8 threshold_support_byte;
    u8 lower_fatal_limit[4];
    u8 lower_critical_limit[4];
    u8 lower_warning_limit[4];
    u8 upper_fatal_limit[4];
    u8 upper_critical_limit[4];
    u8 upper_warning_limit[4];
    u8 sensor_status;
    u8 sensorAverageValue[4];
    u8 sensorMaxValue[4];
}Assert_Sdr_t;
