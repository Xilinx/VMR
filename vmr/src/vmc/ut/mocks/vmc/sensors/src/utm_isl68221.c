#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <cmocka.h>

#include "cl_i2c.h"
#include "isl68221.h"

/*****************************Mock functions *******************************/
u8 __wrap_ISL68221_ReadVCCINT_Voltage(u8 busnum, u8 slaveAddr, float *voltageInmV)
{
	u8 status = 0;
        *voltageInmV = 12;

        return status;
}

u8 __wrap_ISL68221_ReadVCCINT_Current(u8 busnum, u8 slaveAddr, float *currentInA)
{
	u8 status = 0;
        *currentInA = 4;

        return status;
}

u8 __wrap_ISL68221_ReadVCCINT_Temperature(u8 busnum, u8 slaveAddr, float *temperature)
{
	u8 status = 0;
	*temperature = 60;

	return status;
}
