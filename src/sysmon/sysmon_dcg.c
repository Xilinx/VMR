/******************************************************************************
* Copyright (C) 2016 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file sysmon_dcg.c
* @addtogroup sysmonpsv
*
* Functions in this file are api's required for the DCG usecase..
*
* @note		None.
*
* <pre>
*
* MODIFICATION HISTORY:
*
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "sysmon.h"
#include "errno.h"

/******************************************************************************/
/**
 * This function reads temperature and returns value in degree celsius.
 *
 * @param	Pointer Instance to XSysMonPsv.
 * @param	address offset.
 * @param	float value in degree celsius.
 *
 * @return	- -EINVAL if error
 * 			- SUCCESS if successful..
 *
 * @note	None.
 *
*******************************************************************************/
int XSysMonPsv_ReadTempProcessed(XSysMonPsv *InstancePtr, XSysMonPsv_TempType Type, float *Val)
{
	u32 offset, regval;
	int val1, val2;

	if (!InstancePtr)
		return -EINVAL;

	offset = XSysMonPsv_TempOffset(Type);
	XSysMonPsv_ReadReg(InstancePtr, offset, &regval);
	XSysMonPsv_Q8P7ToCelsius(regval, &val1, &val2);
	*Val = (float)val1/(float)val2;

	return SUCCESS;
}

/******************************************************************************/
/**
 * This function reads temperature and returns value in raw format.
 *
 * @param	Pointer Instance to XSysMonPsv.
 * @param	address offset.
 * @param	int value in raw format.
 *
 * @return	- -EINVAL if error
 * 			- SUCCESS if successful..
 *
 * @note	None.
 *
*******************************************************************************/
int XSysMonPsv_ReadTempRaw(XSysMonPsv *InstancePtr, XSysMonPsv_TempType Type, u32 *Val)
{
	u32 offset;

	if (!InstancePtr)
		return -EINVAL;

	offset = XSysMonPsv_TempOffset(Type);
	XSysMonPsv_ReadReg(InstancePtr, offset, Val);

	return SUCCESS;
}

/******************************************************************************/
/**
 * This function reads temperature of each satellite in degree celsius.
 *
 * @param	Pointer Instance to XSysMonPsv.
 * @param	address offset.
 * @param	float value in raw format
 *
 * @return	- -EINVAL if error
 * 			- SUCCESS if successful..
 *
 * @note	None.
 *
*******************************************************************************/
int XSysMonPsv_ReadTempProcessedSat(XSysMonPsv *InstancePtr, int SatId, float *Val)
{
	u32 offset, regval;
	int val1, val2;

	if (!InstancePtr)
		return -EINVAL;

	offset = SYSMON_NODE_OFFSET + SatId * 4;
	XSysMonPsv_ReadReg(InstancePtr, offset, &regval);
	XSysMonPsv_Q8P7ToCelsius(regval, &val1, &val2);
	*Val = (float)val1/(float)val2;

	return SUCCESS;
}

/******************************************************************************/
/**
 * This function reads temperature of each satellite in raw format.
 *
 * @param	Pointer Instance to XSysMonPsv.
 * @param	address offset.
 * @param	float value in raw format
 *
 * @return	- -EINVAL if error
 * 			- SUCCESS if successful..
 *
 * @note	None.
 *
*******************************************************************************/
int XSysMonPsv_ReadTempRawSat(XSysMonPsv *InstancePtr, int SatId, int *Val)
{
	u32 offset;

	if (!InstancePtr)
		return -EINVAL;

	offset = SYSMON_NODE_OFFSET + SatId * 4;
	XSysMonPsv_ReadReg(InstancePtr, offset, Val);

	return SUCCESS;
}

/******************************************************************************/
/**
 * This function sets temperature upper threshold.
 *
 * @param	Pointer Instance to XSysMonPsv.
 * @param	address offset.
 * @param	threshold value
 *
 * @return	- -EINVAL if error
 * 			- SUCCESS if successful..
 *
 * @note	None.
 *
*******************************************************************************/
int XSysMonPsv_SetTempThresholdUpper(XSysMonPsv *InstancePtr, XSysMonPsv_TempEvent Event, int Val)
{
	u32 offset;

	if (!InstancePtr)
		return -EINVAL;

	if (Event == SYSMONPSV_TEMP_EVENT) {
		offset = SYSMON_TEMP_TH_UP;
	} else if (Event == SYSMONPSV_OT_EVENT) {
		offset = SYSMON_OT_TH_UP;
	} else
		return -EINVAL;

	XSysMonPsv_WriteReg(InstancePtr, offset, Val);

	return SUCCESS;
}

/******************************************************************************/
/**
 * This function sets temperature lower threshold.
 *
 * @param	Pointer Instance to XSysMonPsv.
 * @param	address offset.
 * @param	threshold value
 *
 * @return	- -EINVAL if error
 * 			- SUCCESS if successful..
 *
 * @note	None.
 *
*******************************************************************************/
int XSysMonPsv_SetTempThresholdLower(XSysMonPsv *InstancePtr, XSysMonPsv_TempEvent Event, int Val)
{
	u32 offset;

	if (!InstancePtr)
		return -EINVAL;

	if (Event == SYSMONPSV_TEMP_EVENT) {
		offset = SYSMON_TEMP_TH_LOW;
	} else if (Event == SYSMONPSV_OT_EVENT) {
		offset = SYSMON_OT_TH_LOW;
	} else
		return -EINVAL;

	XSysMonPsv_WriteReg(InstancePtr, offset, Val);

	return SUCCESS;
}

/******************************************************************************/
/**
 * This function registers temperature callback function for a region.  This is wrapper
 * function for the corresponding function in service layer.
 *
 * @param	Pointer Instance to XSysMonPsv.
 * @param	callback function.
 * @param	callback data.
 * @param	Region ID
 *
 * @return	- -EINVAL if error
 * 			- SUCCESS if successful..
 *
 * @note	None.
 *
*******************************************************************************/
int XSysMonPsv_RegisterRegionTempOps(XSysMonPsv *InstancePtr, void (*cb)(void *data, struct regional_node *node), void *data, XSysMonPsv_Region RegionId)
{
	int ret;

	if (!cb || !InstancePtr)
		return -EINVAL;

	ret = XSysMonPsv_RegisterRegionTempCallback(InstancePtr, cb, data, RegionId);

	return ret;
}


/******************************************************************************/
/**
 * This function unregister callback function for a region.  This is wrapper
 * function for the corresponding function in service layer.
 *
 * @param	Pointer Instance to XSysMonPsv.
 * @param	callback function.
 * @param	callback data.
 * @param	Region ID
 *
 * @return	- -EINVAL if error
 * 			- SUCCESS if successful..
 *
 * @note	None.
 *
*******************************************************************************/
int XSysMonPsv_UnregisterRegionTempOps(XSysMonPsv *InstancePtr, XSysMonPsv_Region RegionId)
{
	int ret;

	if (!InstancePtr)
		return -EINVAL;
	//Removed mutex lock ==> TBD

	ret = XSysMonPsv_UnregisterRegionTempCallback(InstancePtr, RegionId);

	return ret;
}


/******************************************************************************/
/**
 * This function registers callback function for the device.  This is wrapper
 * function for the corresponding function in service layer.
 *
 * @param	Pointer Instance to XSysMonPsv.
 * @param	callback function.
 * @param	callback data.
 * @param	Region ID
 *
 * @return	- -EINVAL if error
 * 			- SUCCESS if successful..
 *
 * @note	None.
 *
*******************************************************************************/
int XSysMonPsv_RegisterDeviceTempOps(XSysMonPsv *InstancePtr, void (*cb)(void *data), void *data)
{
	int ret;

	if (!cb || !InstancePtr)
		return -EINVAL;

	ret = XSysMonPsv_RegisterDeviceTempCallback(InstancePtr, cb, data);

	return ret;
}

/******************************************************************************/
/**
 * This function unregisters callback function for the device. This is wrapper
 * function for the corresponding function in service layer.
 *
 * @param	Pointer Instance to XSysMonPsv.
 * @param	callback function.
 * @param	callback data.
 * @param	Region ID
 *
 * @return	- -EINVAL if error
 * 			- SUCCESS if successful..
 *
 * @note	None.
 *
*******************************************************************************/
int XSysMonPsv_UnregisterDeviceTempOps(XSysMonPsv *InstancePtr)
{
	if (!InstancePtr)
		return -EINVAL;

	XSysMonPsv_UnregisterDeviceTempCallback(InstancePtr);

	return SUCCESS;
}
