#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <cmocka.h>

#include "FreeRTOS.h"
#include "task.h"
#include "cl_vmc.h"
#include "cl_log.h"
#include "vmc_api.h"
#include "vmc_sensors.h"
#include "vmc_asdm.h"
#include "vmr_common.h"
#include "vmc_sc_comms.h"
#include "vmc_update_sc.h"
#include "platforms/v80.h"

SemaphoreHandle_t sdr_lock = NULL;
static ePlatformType current_platform = eV80;

/*****************************Internal functions*******************************/
void vConfigureV80Platform()
{
	Temperature_Read_Board_Ptr  = scV80AsdmTemperatureReadBoard;
	Temperature_Read_QSFP_Ptr   = scV80AsdmTemperatureReadQSFP;
	Temperature_Read_VCCINT_Ptr = scV80AsdmTemperatureReadVccint;
	Power_Read_Ptr 	            = scV80AsdmReadPower;
	Voltage_Read_Ptr            = scV80AsdmGetVoltageNames;
	Current_Read_Ptr            = scV80AsdmGetCurrentNames;
    Temperature_Read_Ptr        = scV80AsdmGetTemperatureNames;
    QSFP_Read_Ptr               = scV80AsdmGetQSFPName;
}



/*****************************Real function definitions*******************************/
/*This definition is same as real implementation.
 * It does not need mock features, since it is very simple definition*/
ePlatformType xVmcGetPlatformType(void)
{
        return current_platform;
}
