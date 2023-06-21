#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <cmocka.h>

#include "clock_throttling.h"
#include "stdbool.h"
#include "vmc_sensors.h"
#include "vmc_sc_comms.h"
#include "vmr_common.h"
#include "vmc_main.h"

Clock_Throttling_Handle_t clock_throttling_std_algorithm;

/*****************************Mock functions *******************************/
void __wrap_ClockThrottling_Initialize(Clock_Throttling_Handle_t  *pContext, Clock_Throttling_Profile_t *pThrottling)
{
	/*Do Nothing for now*/
}

void __wrap_clock_throttling_algorithm_power(Clock_Throttling_Handle_t  *pContext )
{
	/*Do Nothing for now*/
}

void __wrap_clock_throttling_algorithm_temperature(Clock_Throttling_Handle_t  *pContext )
{
	/*Do Nothing for now*/
}
