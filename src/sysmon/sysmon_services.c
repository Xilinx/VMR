/******************************************************************************
* Copyright (C) 2016 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file sysmon_services.c
* @addtogroup sysmonpsv
*
* Functions in this file are required for to provide service to the DCG layer.
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
#include "xscugic.h"
#include "xscugic_hw.h"
#include "sysmon.h"
#include "xparameters.h"
#include "errno.h"

/****************************************************************************/
/**
*
* This function enables the specified interrupts in the device.
*
* @param	Pointer Instance to XSysMonPsv.
* @param	Mask is the 32 bit-mask of the interrupts to be enabled.
*		Bit positions of 1 will be enabled. Bit positions of 0 will
*		keep the previous setting. This mask is formed by OR'ing
*		XSYSMON_IER_*  bits defined in InstancePtr.h.
* @param	IntrNum is the interrupt enable register to be used
*
* @return	- -EINVAL if error
* 			- SUCCESS if successful
*
* @note		None.
*
*****************************************************************************/
int XSysMonPsv_IntrEnable(XSysMonPsv *InstancePtr, u32 Mask, u8 IntrNum)
{
	u32 offset;

	if (!InstancePtr)
		return -EINVAL;

	/* Calculate the offset of the IER register to be written to */
	offset = (XSYSMON_IER0_OFFSET +
		  ((u32)IntrNum * XSYSMON_INTR_OFFSET));

	/* Enable the specified interrupts in the AMS Interrupt Enable Register. */
	XSysMonPsv_WriteReg(InstancePtr, offset, Mask);

	return SUCCESS;
}

/****************************************************************************/
/**
*
* This function disables the specified interrupts in the device.
*
* @param	Pointer Instance to XSysMonPsv.
* @param	Mask is the 32 bit-mask of the interrupts to be enabled.
*		Bit positions of 1 will be disabled. Bit positions of 0 will
*		keep the previous setting. This mask is formed by OR'ing
*		XSYSMON_IDR_*  bits defined in InstancePtr.h.
* @param	IntrNum is the interrupt disable register to be used
*
* @return	- -EINVAL if error
* 			- SUCCESS if successful
*
* @note		None.
*
*****************************************************************************/
int XSysMonPsv_IntrDisable(XSysMonPsv *InstancePtr, u32 Mask, u8 IntrNum)
{
	u32 offset;

	if (!InstancePtr)
		return -EINVAL;

	/* Calculate the offset of the IDR register to be written to */
	offset = (XSYSMON_IDR0_OFFSET +
		  ((u32)IntrNum * XSYSMON_INTR_OFFSET));

	/* Disable the specified interrupts in the AMS Interrupt Disable Register. */
	XSysMonPsv_WriteReg(InstancePtr, offset, Mask);

	return SUCCESS;
}

/****************************************************************************/
/**
*
* This function returns the interrupt status read from Interrupt Status
* Register(ISR). Use the XSYSMON_ISR* constants defined in InstancePtr.h
* to interpret the returned value.
*
* @param	Pointer Instance to XSysMonPsv.
*
* @return	A 32-bit value representing the contents of the Interrupt Status
*		Register (ISR).
*
* @note		None.
*
*****************************************************************************/
int XSysMonPsv_IntrGetStatus(XSysMonPsv *InstancePtr)
{
	u32 intr_status;

	if (!InstancePtr)
		return -EINVAL;

	/* Return the value read from the AMS ISR. */
	XSysMonPsv_ReadReg(InstancePtr, XSYSMON_ISR_OFFSET, &intr_status);
	return intr_status;
}

/****************************************************************************/
/**
*
* This function clears the specified interrupts in the Interrupt Status
* Register (ISR).
*
* @param	InstancePtr is a pointer to the struct InstancePtr.
* @param	Mask is the 32 bit-mask of the interrupts to be cleared.
*		Bit positions of 1 will be cleared. Bit positions of 0 will not
*		change the previous interrupt status.*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
void XSysMonPsv_IntrClear(XSysMonPsv *InstancePtr, u32 Mask)
{
	XSysMonPsv_WriteReg(InstancePtr, XSYSMON_ISR_OFFSET, Mask);
}

/****************************************************************************/
/**
*
* This function unlocks the register space of InstancePtr hardware.
*
* @param	Pointer Instance to XSysMonPsv.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
void XSysMonPsv_UnlockRegspace(XSysMonPsv *InstancePtr)
{
	XSysMonPsv_WriteReg(InstancePtr, XSYSMON_PCSR_LOCK, LOCK_CODE);
}

/****************************************************************************/
/**
*
* This function handles the event for different regions.
*
* @param	Pointer Instance to XSysMonPsv.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
static void XSysMonPsv_RegionEventHandler(XSysMonPsv *InstancePtr)
{
	struct region_info *region;
	struct regional_node *node, *eventnode;
	u32 regval, event = 0;
	u16 thresh_up, val;

	XSysMonPsv_ReadReg(InstancePtr, SYSMON_TEMP_TH_UP, &regval);
	thresh_up = (u16)regval;

	list_for_each_entry(region, &InstancePtr->region_list, list) {
		list_for_each_entry(node, &region->node_list,
				    regional_node_list) {

			val = XSysMonPsv_GetNodeValue(InstancePtr, node->sat_id);
			/* Find the highest value */
			if (compare(val, thresh_up)) {
				eventnode = node;
				eventnode->temp = val;
				thresh_up = val;
				event = 1;
				xil_printf("%s %d %d %d %x\n", __func__, __LINE__, region->id, node->sat_id, val);
			}
		}
		if (event && region->cb)
			region->cb(region->data, eventnode);
	}
}

/******************************************************************************/
/**
 * This function call the callback function registered with device when temp
 * event is triggered.
 *
 * @param	Pointer Instance to XSysMonPsv.
 *
 * @return	- -EINVAL if error
 * 			- SUCCESS if successful..
 *
 * @note	None.
 *
*******************************************************************************/
void XSysMonPsv_TempEventHandler(XSysMonPsv *InstancePtr)
{
	if(InstancePtr->temp_cb && InstancePtr->data)
		InstancePtr->temp_cb(InstancePtr->data);
}
/****************************************************************************/
/**
*
* This function handles the event for different different offsets.
*
* @param	Pointer Instance to XSysMonPsv.
* @param	Events that triggered the interrupt
*
* @return	None.
*
* @note		None.
*
***************************************************************************/
static void XSysMonPsv_HandleEvent(XSysMonPsv *InstancePtr, u32 Event)
{

	switch (Event) {
	case SYSMON_BIT_TEMP:
		xil_printf("SYSMON_BIT_TEMP\n");
		XSysMonPsv_TempEventHandler(InstancePtr);
		XSysMonPsv_WriteReg(InstancePtr, SYSMON_IDR, _BIT(SYSMON_BIT_TEMP));
		InstancePtr->masked_temp |= _BIT(SYSMON_BIT_TEMP);
		XSysMonPsv_RegionEventHandler(InstancePtr);
		break;

	case SYSMON_BIT_OT:
		xil_printf("SYSMON_BIT_OT\n");
		XSysMonPsv_TempEventHandler(InstancePtr);
		XSysMonPsv_WriteReg(InstancePtr, SYSMON_IDR, _BIT(SYSMON_BIT_OT));
		InstancePtr->masked_temp |= _BIT(SYSMON_BIT_OT);
		XSysMonPsv_RegionEventHandler(InstancePtr);
		break;
	case SYSMON_BIT_ALARM4:
	case SYSMON_BIT_ALARM3:
	case SYSMON_BIT_ALARM2:
	case SYSMON_BIT_ALARM1:
	case SYSMON_BIT_ALARM0:
		/* TBD */
		break;

	default:
		break;
	}
}

/****************************************************************************/
/**
*
* This function sends the event triggered bit by bit.
*
* @param	Pointer Instance to XSysMonPsv.
* @param	Events that is set in the ISR register.
*
* @return	None.
*
* @note		None.
*
***************************************************************************/
static void XSysMonPsv_HandleEvents(XSysMonPsv *InstancePtr,
				 unsigned int Events)
{
	int bit;

	for (bit = 0; bit < sizeof(Events)*8; bit++) {
		if(Events & (1 << bit)) {
			XSysMonPsv_HandleEvent(InstancePtr, bit);
		}
	}
}

/****************************************************************************/
/**
*
* This function is a interrupt handler.
*
* @param	data pointer to the iSR.
*
* @return	None.
*
* @note		None.
*
***************************************************************************/
static void XSysMonPsv_IntrHandler(void *data)
{
	u32 isr, imr;

	XSysMonPsv *InstancePtr = (XSysMonPsv *)data;

	XSysMonPsv_ReadReg(InstancePtr, SYSMON_ISR, &isr);
	XSysMonPsv_ReadReg(InstancePtr, SYSMON_IMR, &imr);

	/* only process alarm that are not masked */
	isr &= ~imr;

	/* clear interrupt */
	XSysMonPsv_WriteReg(InstancePtr, SYSMON_ISR, isr);

	if (isr) {
		XSysMonPsv_HandleEvents(InstancePtr, isr);
	}
}

/****************************************************************************/
/**
*
* This function registers ISR for driver instance
*
* @param	Interupt instance .
* @param	Pointer Instance to XSysMonPsv.
* @param	Interrupt ID.
*
* @return	None.
*
* @note		None.
*
***************************************************************************/
int XSysMonPsv_SetupIntrHandlr(XScuGic *IntcInstancePtr,
                                XSysMonPsv *InstancePtr, u16 IntrId) {
  int Status;
  XScuGic_Config *IntcConfig;

  /* Initialize the interrupt controller driver */
  IntcConfig = XScuGic_LookupConfig(INTC_DEVICE_ID);
  if (NULL == IntcConfig) {
	  return XST_FAILURE;
  }

  Status = XScuGic_CfgInitialize(IntcInstancePtr, IntcConfig,
                                 IntcConfig->CpuBaseAddress);
  if (Status != XST_SUCCESS) {
    return XST_FAILURE;
  }

  /*
   * Connect the interrupt controller interrupt handler to the
   * hardware interrupt handling logic in the processor.
   */
  Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
                               (Xil_ExceptionHandler)XScuGic_InterruptHandler,
                               IntcInstancePtr);
  /*
   * Connect a device driver handler that will be called when an
   * interrupt for the device occurs, the device driver handler
   * performs the specific interrupt processing for the device
   */

  Status = XScuGic_Connect(IntcInstancePtr, IntrId,
                           (Xil_ExceptionHandler)XSysMonPsv_IntrHandler,
                           (void *)InstancePtr);
  if (Status != XST_SUCCESS) {
    return XST_FAILURE;
  }

  /* Enable the interrupt for the device */
  XScuGic_Enable(IntcInstancePtr, IntrId);

  /*
   * Enable interrupts in the Processor.
   */
  Xil_ExceptionEnable();

  return XST_SUCCESS;
}

/******************************************************************************/
/**
 * This function registers temperature callback function for a region.
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

int XSysMonPsv_RegisterRegionTempCallback(XSysMonPsv *InstancePtr, void (*cb)(void *data, struct regional_node *node), void *data, XSysMonPsv_Region RegionId)
{
	struct region_info *region;
	int ret = SUCCESS, found = 0;

	if (!cb || !InstancePtr)
		return -EINVAL;

	//Removed mutex lock ==> TBD

	if (list_empty(&InstancePtr->region_list)) {
		xil_printf("Failed to set a callback.\n");
		ret = -EINVAL;
		goto exit;
	}

	list_for_each_entry(region, &InstancePtr->region_list, list) {
		if (region->id == RegionId) {
			found = 1;
			if (region->cb) {
				xil_printf("Error callback already set. Unregister the existing callback to set a new one.\n");
				ret = -EINVAL;
				goto exit;
			}
			region->cb = cb;
			region->data = data;
			xil_printf("Callback registered for region %d\n", RegionId);
			break;
		}
	}

	if (!found) {
		xil_printf("Error invalid region. Please select the correct region\n");
		ret = -EINVAL;
	}

exit:
	return ret;
}


/******************************************************************************/
/**
 * This function unregisters temperature callback function for a region.
 *
 * @param	Pointer Instance to XSysMonPsv.
 * @param	Region ID
 *
 * @return	- -EINVAL if error
 * 			- SUCCESS if successful..
 *
 * @note	None.
 *
*******************************************************************************/
int XSysMonPsv_UnregisterRegionTempCallback(XSysMonPsv *InstancePtr, XSysMonPsv_Region RegionId)
{
	struct region_info *region;
	int ret = SUCCESS, found = 0;

	if (!InstancePtr)
		return -EINVAL;
	//Removed mutex lock ==> TBD

	if (list_empty(&InstancePtr->region_list)) {
		xil_printf("Failed to set a callback.\n");
		ret = -EINVAL;
		goto exit;
	}

	list_for_each_entry(region, &InstancePtr->region_list, list) {
		if (region->id == RegionId) {
			found = 1;
			region->cb = NULL;
			region->data = NULL;
			xil_printf("Callback unregistered for region %d\n", RegionId);
			break;
		}
	}

	if (!found) {
		xil_printf("Error no such region. Please select the correct region\n");
		ret = -EINVAL;
	}

exit:
	return ret;
}

/******************************************************************************/
/**
 * This function registers temperature callback function for the complete device.
 *
 * @param	Pointer Instance to XSysMonPsv.
 * @param	callback function.
 * @param	callback data.
 *
 * @return	- -EINVAL if error
 * 			- SUCCESS if successful..
 *
 * @note	None.
 *
*******************************************************************************/
int XSysMonPsv_RegisterDeviceTempCallback(XSysMonPsv *InstancePtr, void (*cb)(void *data), void *data)
{
	//mutex lock ==> TBD

	if (InstancePtr->temp_cb) {
		xil_printf("Error callback already set. Unregister the existing callback to set a new one.\n");
		return -EINVAL;
	}
	InstancePtr->temp_cb = cb;
	InstancePtr->data = data;

	return SUCCESS;
}

/******************************************************************************/
/**
 * This function unregisters temperature callback function for the complete device.
 *
 * @param	Pointer Instance to XSysMonPsv.
 *
 * @return	- -EINVAL if error
 * 			- SUCCESS if successful..
 *
 * @note	None.
 *
*******************************************************************************/
void XSysMonPsv_UnregisterDeviceTempCallback(XSysMonPsv *InstancePtr)
{
	InstancePtr->temp_cb = NULL;
	InstancePtr->data = NULL;
}
