/******************************************************************************
* Copyright (C) 2016 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file sysmon.c
* @addtogroup sysmonpsv
*
* Functions in this file are basic driver function.
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
#include "xparameters.h"
#include "xil_io.h"
#include "sysmon_list.h"
#include <errno.h>

/* For Now we are using very limited features in sysmon
 * commented un-used code to avoid compiler warnings.
 * */
#if 0
static const struct sysmon_event_spec InstancePtremp_events[] = {
	{
		.type = SYSMON_EV_TYPE_THRESH,
		.dir = SYSMON_EV_DIR_RISING,
		.mask_separate = BIT(SYSMON_EV_INFO_VALUE),
	},
	{
		.type = SYSMON_EV_TYPE_THRESH,
		.dir = SYSMON_EV_DIR_FALLING,
		.mask_separate = BIT(SYSMON_EV_INFO_VALUE),
	},
	{
		.type = SYSMON_EV_TYPE_THRESH,
		.dir = SYSMON_EV_DIR_EITHER,
		.mask_separate =
			BIT(SYSMON_EV_INFO_ENABLE) | BIT(SYSMON_EV_INFO_HYSTERESIS),
	},
};

/* Temperature channel attributes */
static const struct sysmon_chan_spec temp_channels[] = {
	SYSMON_CHAN_TEMP(SYSMONPSV_TEMP_MAX, "temp"),
//	SYSMON_CHAN_TEMP(TEMP_MIN, "min"),  //removed from support from production Silicon
	SYSMON_CHAN_TEMP(SYSMONPSV_TEMP_MAX_MAX, "max_max"),
	SYSMON_CHAN_TEMP(SYSMONPSV_TEMP_MIN_MIN, "min_min"),
};

/* Temperature event attributes */
static const struct sysmon_chan_spec temp_events[] = {
	SYSMON_CHAN_TEMP_EVENT(SYSMONPSV_TEMP_EVENT, "temp", InstancePtremp_events),
	SYSMON_CHAN_TEMP_EVENT(SYSMONPSV_OT_EVENT, "ot", InstancePtremp_events),
};
#endif

void XSysMonPsv_ReadReg(XSysMonPsv *InstancePtr, u32 offset, u32 *data)
{
	*data = Xil_In32(InstancePtr->base + offset);
}

void XSysMonPsv_WriteReg(XSysMonPsv *InstancePtr, u32 offset, u32 data)
{
	Xil_Out32(InstancePtr->base + offset, data);
}

void XSysMonPsv_UpdateReg(XSysMonPsv *InstancePtr, u32 offset,
				     u32 mask, u32 data)
{
	u32 val;

	XSysMonPsv_ReadReg(InstancePtr, offset, &val);
	XSysMonPsv_WriteReg(InstancePtr, offset, (val & ~mask) | (mask & data));
}

int XSysMonPsv_TempOffset(XSysMonPsv_TempType Type)
{
	switch (Type) {
	case SYSMONPSV_TEMP_MAX:
		return SYSMON_TEMP_MAX;
	case SYSMONPSV_TEMP_MIN:
		return SYSMON_TEMP_MIN;
	case SYSMONPSV_TEMP_MAX_MAX:
		return SYSMON_TEMP_MAX_MAX;
	case SYSMONPSV_TEMP_MIN_MIN:
		return SYSMON_TEMP_MIN_MIN;
	default:
		return -EINVAL;
	}
	return -EINVAL;
}

u32 XSysMonPsv_GetNodeValue(XSysMonPsv *InstancePtr, int sat_id)
{
	u32 raw;

	XSysMonPsv_ReadReg(InstancePtr, SYSMON_NODE_OFFSET + (sat_id*4), &raw);

	return raw;
}

#if 0
static u32 XSysMonPsv_TempThreshOffset(XSysMonPsv_TempEvent Event, enum sysmon_event_direction dir)
{
	switch (Event) {
	case SYSMONPSV_TEMP_EVENT:
		return (dir == SYSMON_EV_DIR_RISING) ? SYSMON_TEMP_TH_UP :
						    SYSMON_TEMP_TH_LOW;
	case SYSMONPSV_OT_EVENT:
		return (dir == SYSMON_EV_DIR_RISING) ? SYSMON_OT_TH_UP :
						    SYSMON_OT_TH_LOW;
	default:
		return -EINVAL;
	}
	return -EINVAL;
}
#endif

u32 XSysMonPsv_SupplyOffset(int address)
{
	return (address * 4) + SYSMON_SUPPLY_BASE;
}

int XSysMonPsv_SupplyThreshOffset(int address,
				       enum sysmon_event_direction dir)
{
	if (dir == SYSMON_EV_DIR_RISING)
		return (address * 4) + SYSMON_SUPPLY_TH_UP;
	else if (dir == SYSMON_EV_DIR_FALLING)
		return (address * 4) + SYSMON_SUPPLY_TH_LOW;

	return -EINVAL;
}

void XSysMonPsv_Q8P7ToCelsius(int raw_data, int *val, int *val2)
{
	*val = (raw_data & 0x8000) ? -(twoscomp(raw_data)) : raw_data;
	*val2 = 128;
}

void XSysMonPsv_CelsiusToQ8P7(u32 *raw_data, int val, int val2)
{
	int scale = 1 << 7;

	val2 = val2 / 1000;
	*raw_data = (val * scale) + ((val2 * scale) / 1000);
}

void XSysMonPsv_SupplyRawToProcessed(int raw_data, int *val, int *val2)
{
	int mantissa, format, exponent;

	mantissa = raw_data & SYSMON_MANTISSA_MASK;
	exponent = (raw_data & SYSMON_MODE_MASK) >> SYSMON_MODE_SHIFT;
	format = (raw_data & SYSMON_FMT_MASK) >> SYSMON_FMT_SHIFT;

	*val2 = 1 << (16 - exponent);
	*val = mantissa;
	if (format && (mantissa >> SYSMON_MANTISSA_SIGN_SHIFT))
		*val = (~(mantissa) & SYSMON_MANTISSA_MASK) * -1;
}

void XSysMonPsv_SupplyProcessedToRaw(int val, int val2, u32 reg_val, u32 *raw_data)
{
	int exponent = (reg_val & SYSMON_MODE_MASK) >> SYSMON_MODE_SHIFT;
	int format = (reg_val & SYSMON_FMT_MASK) >> SYSMON_FMT_SHIFT;
	int scale = 1 << (16 - exponent);
	int tmp;

	val2 = val2 / 1000;
	tmp = (val * scale) + ((val2 * scale) / 1000);

	/* Set out of bound values to saturation levels */
	if (format) {
		if (tmp > SYSMON_UPPER_SATURATION_SIGNED)
			tmp = 0x7fff;
		else if (tmp < SYSMON_LOWER_SATURATION_SIGNED)
			tmp = 0x8000;

	} else {
		if (tmp > SYSMON_UPPER_SATURATION)
			tmp = 0xffff;
		else if (tmp < SYSMON_LOWER_SATURATION)
			tmp = 0x0000;
	}

	*raw_data = tmp & 0xffff;
}

int XSysMonPsv_GetEventMask(XSysMonPsv_TempEvent Event)
{
	if (Event == SYSMONPSV_TEMP_EVENT)
		return _BIT(SYSMON_BIT_TEMP);
	else if (Event == SYSMONPSV_OT_EVENT)
		return _BIT(SYSMON_BIT_OT);

	/* return supply */
	return _BIT(Event / 32);
}

/****************************************************************************/
/**
*
* This function initializes basic hardware and software required for the
* device. It setup the ISR, read hardware information.
*
* @param	Instance to the structure InstancePtr.
* @param	Instance to the Interrupt controller.
*
* @return	- -EINVAL if error
* 			- SUCCESS if successful
*
* @note		None.
*
*****************************************************************************/
int XSysMonPsv_Init(XSysMonPsv *InstancePtr, XScuGic *IntcInst) {
  int Status;
  u32 Mask;

  if (!InstancePtr)
	return -EINVAL;

  InstancePtr->base = XPAR_BLP_CIPS_PSPMC_0_PSV_PMC_SYSMON_0_S_AXI_BASEADDR;

  INIT_LIST_HEAD(&InstancePtr->region_list);
  XSysMonPsv_UnlockRegspace(InstancePtr);
  XSysMonPsv_IntrDisable(InstancePtr, 0xFFFFFFFF, INTR_0);
  XSysMonPsv_IntrClear(InstancePtr, 0xFFFFFFFF);

  Status = XSysMonPsv_SetupIntrHandlr(IntcInst, InstancePtr, SYSMON_INTR_0_ID);
  if (Status != XST_SUCCESS)
    return Status;

  Mask = XSYSMON_IER0_OT_MASK | XSYSMON_IER0_TEMP_MASK;
  XSysMonPsv_IntrEnable(InstancePtr, Mask, INTR_0);
  return XST_SUCCESS;
}

