/******************************************************************************
* Copyright (C) 2016 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file sysmon.h
* @addtogroup sysmonpsv_v2_0
*
* The SysMon driver supports the Xilinx System Monitor device on Versal
*
* The System Monitor device has the following features:
*		- Measure and monitor up to 160 voltages across the chip
*		- Automatic alarms based on user defined limis for the
*		  on-chip temperature.
*		- Optional interrupt request generation
*
*
* The user should refer to the hardware device specification for detailed
* information about the device.
*
* This header file contains the prototypes of driver functions that can
* be used to access the System Monitor device.
*
*
* <b> Initialization and Configuration </b>
*
* The device driver enables higher layer software (e.g., an application) to
* communicate to the System Monitor device.
*
* XSysMonPsv_Init() API is used to initialize the System Monitor
* device. The user needs to first call the XSysMonPsv_Init() API to further
* use user API's
*
*
* <b>Interrupts</b>
*
* The System Monitor device supports interrupt driven mode and the default
* operation mode is polling mode.
*
* This driver does not provide a Interrupt Service Routine (ISR) for the device.
* It is the responsibility of the application to provide one if needed. Refer to
* the interrupt example provided with this driver for details on using the
* device in interrupt mode.
*
* <pre>
* MODIFICATION HISTORY:
*
* </pre>
*
******************************************************************************/

#ifndef _SYSMON_H_
#ifdef __cplusplus
extern "C" {
#endif
#define _SYSMON_H_

/***************************** Include Files *********************************/
#include "sysmon_list.h"
#include "xil_types.h"
#include "xil_assert.h"
#include "xscugic.h"

/************************** Constant Definitions *****************************/
#define SUCCESS 0

/* Register Unlock Code */
#define NPI_UNLOCK	0xF9E8D7C6

/* Register Offsets */
#define SYSMON_NPI_LOCK		0x000C
#define SYSMON_ISR			0x0044
#define SYSMON_TEMP_MASK	0x300
#define SYSMON_IMR			0x0048
#define SYSMON_IER			0x004C
#define SYSMON_IDR			0x0050
#define SYSMON_ALARM_FLAG	0x1018
#define SYSMON_TEMP_MAX		0x1030
#define SYSMON_TEMP_MIN		0x1034
#define SYSMON_SUPPLY_BASE	0x1040
#define SYSMON_ALARM_REG	0x1940
#define SYSMON_TEMP_TH_LOW	0x1970
#define SYSMON_TEMP_TH_UP	0x1974
#define SYSMON_OT_TH_LOW	0x1978
#define SYSMON_OT_TH_UP		0x197C
#define SYSMON_SUPPLY_TH_LOW	0x1980
#define SYSMON_SUPPLY_TH_UP	0x1C80
#define SYSMON_TEMP_MAX_MAX	0x1F90
#define SYSMON_TEMP_MIN_MIN	0x1F8C
#define SYSMON_TEMP_EV_CFG	0x1F84
#define SYSMON_NODE_OFFSET	0x1FAC
#define SYSMON_STATUS_RESET	0x1F94

#define SYSMON_NO_OF_EVENTS	32

/* Supply Voltage Conversion macros */
#define SYSMON_MANTISSA_MASK		0xFFFF
#define SYSMON_FMT_MASK			0x10000
#define SYSMON_FMT_SHIFT		16
#define SYSMON_MODE_MASK		0x60000
#define SYSMON_MODE_SHIFT		17
#define SYSMON_MANTISSA_SIGN_SHIFT	15
#define SYSMON_UPPER_SATURATION_SIGNED	32767
#define SYSMON_LOWER_SATURATION_SIGNED	-32768
#define SYSMON_UPPER_SATURATION		65535
#define SYSMON_LOWER_SATURATION		0

#define _BIT(nr) (1UL << (nr))
#define _ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#define INTR_0 0U
#define INTR_1 1U
#define INTC_DEVICE_ID XPAR_SCUGIC_SINGLE_DEVICE_ID
#define SYSMON_INTR_0_ID (144U + 32U)
#define SYSMON_INTR_1_ID (145U + 32U)
#define LOCK_CODE 0xF9E8D7C6
/**
 * Register: XSYSMONPSV_PCSR_LOCK
 */
#define XSYSMON_PCSR_LOCK     			0X0000000CU

#define XSYSMON_ISR_OFFSET     			0X00000044U
#define XSYSMON_IMR_OFFSET     			0X00000048U

#define XSYSMON_PCSR_LOCK_STATE_SHIFT   0U
#define XSYSMON_PCSR_LOCK_STATE_WIDTH   1U
#define XSYSMON_PCSR_LOCK_STATE_MASK    0X00000001U

#define XSYSMON_IER0_TEMP_SHIFT   	9U
#define XSYSMON_IER0_TEMP_WIDTH   	1U
#define XSYSMON_IER0_TEMP_MASK    	0X00000200U

#define XSYSMON_IER0_OT_SHIFT   	8U
#define XSYSMON_IER0_OT_WIDTH   	1U
#define XSYSMON_IER0_OT_MASK    	0X00000100U

#define XSYSMON_ISR_OT_SHIFT   		8U
#define XSYSMON_ISR_OT_WIDTH   		1U
#define XSYSMON_ISR_OT_MASK    		0X00000100U

#define XSYSMON_IER0_OFFSET     	0X0000004CU
#define XSYSMON_IDR0_OFFSET     	0X00000050U

#define XSYSMON_INTR_OFFSET			0xCU

#define SYSMON_CHAN_TEMP_EVENT(_address, _ext, _events) { \
	.type = SYSMON_TEMP, \
	.indexed = 1, \
	.address = _address, \
	.channel = _address, \
	.event_spec = _events, \
	.num_event_specs = _ARRAY_SIZE(_events), \
	.scan_type = { \
		.sign = 's', \
		.realbits = 15, \
		.storagebits = 16, \
		.endianness = SYSMON_CPU, \
	}, \
	.extend_name = _ext, \
	}

#define SYSMON_CHAN_TEMP(_address, _ext) { \
	.type = SYSMON_TEMP, \
	.indexed = 1, \
	.address = _address, \
	.channel = _address, \
	.info_mask_separate = _BIT(SYSMON_CHAN_INFO_RAW) | \
		_BIT(SYSMON_CHAN_INFO_PROCESSED), \
	.scan_type = { \
		.sign = 's', \
		.realbits = 15, \
		.storagebits = 16, \
		.endianness = SYSMON_CPU, \
	}, \
	.extend_name = _ext, \
}

#define compare(val, thresh) (((val) & 0x8000) || ((thresh) & 0x8000) ? \
			      ((val) < (thresh)) : ((val) > (thresh)))

#define twoscomp(val) ((((val) ^ 0xFFFF) + 1) & 0x0000FFFF)
#define ALARM_REG(address) ((address) / 32)
#define ALARM_SHIFT(address) ((address) % 32)

enum sysmon_event_type {
	SYSMON_EV_TYPE_THRESH,
	SYSMON_EV_TYPE_MAG,
	SYSMON_EV_TYPE_ROC,
	SYSMON_EV_TYPE_THRESH_ADAPTIVE,
	SYSMON_EV_TYPE_MAG_ADAPTIVE,
	SYSMON_EV_TYPE_CHANGE,
};

enum sysmon_event_direction {
	SYSMON_EV_DIR_EITHER,
	SYSMON_EV_DIR_RISING,
	SYSMON_EV_DIR_FALLING,
	SYSMON_EV_DIR_NONE,
};

enum sysmon_alarm_bit {
	SYSMON_BIT_ALARM0 = 0,
	SYSMON_BIT_ALARM1 = 1,
	SYSMON_BIT_ALARM2 = 2,
	SYSMON_BIT_ALARM3 = 3,
	SYSMON_BIT_ALARM4 = 4,
	SYSMON_BIT_ALARM5 = 5,
	SYSMON_BIT_ALARM6 = 6,
	SYSMON_BIT_ALARM7 = 7,
	SYSMON_BIT_OT = 8,
	SYSMON_BIT_TEMP = 9,
};

enum sysmon_chan_info_enum {
	SYSMON_CHAN_INFO_RAW = 0,
	SYSMON_CHAN_INFO_PROCESSED,
};

enum sysmon_event_info {
	SYSMON_EV_INFO_ENABLE,
	SYSMON_EV_INFO_VALUE,
	SYSMON_EV_INFO_HYSTERESIS,
	SYSMON_EV_INFO_PERIOD,
	SYSMON_EV_INFO_HIGH_PASS_FILTER_3DB,
	SYSMON_EV_INFO_LOW_PASS_FILTER_3DB,
};

enum sysmon_chan_type {
	SYSMON_VOLTAGE,
	SYSMON_CURRENT,
	SYSMON_TEMP,
};

typedef enum {
	SYSMONPSV_TEMP_MAX,
	SYSMONPSV_TEMP_MIN,
	SYSMONPSV_TEMP_MAX_MAX,
	SYSMONPSV_TEMP_MIN_MIN,
} XSysMonPsv_TempType;

typedef enum {
	SYSMONPSV_TEMP_EVENT,
	SYSMONPSV_OT_EVENT,
} XSysMonPsv_TempEvent;

enum sysmon_endian {
	SYSMON_CPU,
	SYSMON_BE,
	SYSMON_LE,
};

typedef enum {
	REGION1,
	REGION2,
} XSysMonPsv_Region;

struct sysmon_chan_spec {
	enum sysmon_chan_type	type;
	int		channel;
	int		channel2;
	unsigned long	address;
	int		scan_index;
	struct {
		char	sign;
		u8	realbits;
		u8	storagebits;
		u8	shift;
		u8	repeat;
		enum sysmon_endian endianness;
	} scan_type;
	long	info_mask_separate;
	long	info_mask_separate_available;
	long	info_mask_shared_by_type;
	long	info_mask_shared_by_type_available;
	long	info_mask_shared_by_dir;
	long	info_mask_shared_by_dir_available;
	long	info_mask_shared_by_all;
	long	info_mask_shared_by_all_available;
	const struct sysmon_event_spec *event_spec;
	unsigned int	num_event_specs;
	//const struct sysmon_chan_spec_ext_info *ext_info;
	const char	*extend_name;
	const char	*datasheet_name;
	unsigned	modified:1;
	unsigned	indexed:1;
	unsigned	output:1;
	unsigned	differential:1;
	void (*cb)(void *data);
	void *data;
};

struct sysmon_event_spec {
	enum sysmon_event_type type;
	enum sysmon_event_direction dir;
	unsigned long mask_separate;
	unsigned long mask_shared_by_type;
	unsigned long mask_shared_by_dir;
	unsigned long mask_shared_by_all;
};

struct regions {

};

struct regional_node {
	int sat_id;
	int x;
	int y;
	u16 temp;
	struct list_head regional_node_list;
};

struct region_info {
	XSysMonPsv_Region id;
	void (*cb)(void *data, struct regional_node *node);
	void *data;
	struct list_head node_list;
	struct list_head list;
};

typedef struct {
	int num_channels;
	unsigned int base;
	int irq;
	struct list_head region_list;
	unsigned int masked_temp;
	unsigned int temp_mask;
	void (*temp_cb)(void *data);
	void *data;
} XSysMonPsv;

int XSysMonPsv_TempOffset(XSysMonPsv_TempType Type);
void XSysMonPsv_ReadReg(XSysMonPsv *InstancePtr, u32 offset, u32 *data);
void XSysMonPsv_WriteReg(XSysMonPsv *InstancePtr, u32 offset, u32 data);
void XSysMonPsv_UpdateReg(XSysMonPsv *InstancePtr, u32 offset, u32 mask, u32 data);
int XSysMonPsv_GetEventMask(XSysMonPsv_TempEvent Event);
void XSysMonPsv_Q8P7ToCelsius(int raw_data, int *val, int *val2);
void XSysMonPsv_CelsiusToQ8P7(u32 *raw_data, int val, int val2);
int XSysMonPsv_Init(XSysMonPsv *InstancePtr, XScuGic *IntcInst);
int XSysMonPsv_ReadTempProcessed(XSysMonPsv *InstancePtr, XSysMonPsv_TempType Type, float *Val);
u32 XSysMonPsv_GetNodeValue(XSysMonPsv *InstancePtr, int sat_id);
int XSysMonPsv_IntrEnable(XSysMonPsv *InstancePtr, u32 Mask, u8 IntrNum);
void XSysMonPsv_UnlockRegspace(XSysMonPsv *InstancePtr);
int XSysMonPsv_IntrDisable(XSysMonPsv *InstancePtr, u32 Mask, u8 IntrNum);
void XSysMonPsv_IntrClear(XSysMonPsv *InstancePtr, u32 Mask);
int XSysMonPsv_SetupIntrHandlr(XScuGic *IntcInstancePtr,XSysMonPsv *InstancePtr, u16 IntrId);
int XSysMonPsv_RegisterRegionTempCallback(XSysMonPsv *InstancePtr, void (*cb)(void *data, struct regional_node *node), void *data, XSysMonPsv_Region RegionId);
int XSysMonPsv_UnregisterRegionTempCallback(XSysMonPsv *InstancePtr, XSysMonPsv_Region RegionId);
int XSysMonPsv_RegisterDeviceTempCallback(XSysMonPsv *InstancePtr, void (*cb)(void *data), void *data);
void XSysMonPsv_UnregisterDeviceTempCallback(XSysMonPsv *InstancePtr);

#ifdef __cplusplus
}
#endif
#endif /* _SYSMON_H */
