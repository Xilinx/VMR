#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <cmocka.h>

#include "cl_i2c.h"
#include "ina3221.h"

/*****************************Mock functions *******************************/
u8 __wrap_INA3221_ReadVoltage(u8 busnum, u8 slaveAddr, u8 channelNum, float *voltageInmV)
{
	u8 status = 0;

	if( channelNum == 0)
	{
        *voltageInmV = 12000;
	}
	else if(channelNum == 1)
	{
		*voltageInmV = 3000;
	}
        return status;
}

u8 __wrap_INA3221_ReadCurrent(u8 busnum, u8 slaveAddr, u8 channelNum, float *currentInmA)
{
	u8 status = 0;
	if( channelNum == 0)
	{
        *currentInmA = 5000;
	}
	else if(channelNum == 1)
	{
		*currentInmA = 4000;
	}
        return status;
}

u8 __wrap_INA3221_ReadPower(u8 busnum, u8 slaveAddr, u8 channelNum, float *powerInmW)
{
        u8 status = 0;
        *powerInmW = 75;

        return status;
}

