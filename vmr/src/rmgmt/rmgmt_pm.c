/******************************************************************************
* Copyright (C) 2020-2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#include <stdbool.h>
#include <stdint.h>

#include "rmgmt_common.h"
#include "rmgmt_pm.h"
#include "cl_msg.h"
#include "cl_main.h"

#include "pm_api_sys.h"
#include "xpm_defs.h"
#include "gic_setup.h"
#include "ipi.h"
#include "pm_init.h"

/* RPU Private Persistant Area */
/* TODO: explian those fixed address which is from SSW code example */
#define RPU_PPA_BASE				0x3E500000
#define RPU_PPA_BAD_BOOT_OFFSET			0x10
#define RPU_BAD_BOOT_KEY			0x37232323
#define RPU_CLEAN_BOOT_KEY			0x0


static XIpiPsu IpiInst;

/*
 *  GetBootState :
 *  Returns 1: Good boot
 *  	    0: Bad boot
 *  	    2: Corrupted boot -- for debug purpose
 */
static u32 GetBootState()
{
	u32 State = 2; /* corrupted boot */
	u32 boot_status = Xil_In32(RPU_PPA_BASE + RPU_PPA_BAD_BOOT_OFFSET);
	if (boot_status == RPU_BAD_BOOT_KEY) {
		/* bad boot */
		State = 0;
	} else if (boot_status == RPU_CLEAN_BOOT_KEY) {
		/* Good boot */
		State = 1;
	} else {
		/* Corrupted boot state */
		State = 2;
		xil_printf("Received corrupted boot state=0x%X. Ignore for first boot\r\n", boot_status);
	}

	return State;
}

static void SetNextBootState(u32 state)
{
	Xil_Out32(RPU_PPA_BASE + RPU_PPA_BAD_BOOT_OFFSET, state);
	Xil_DCacheFlush();
}

int rmgmt_pm_init(void)
{
	int Status = XST_SUCCESS;

	VMR_WARN("pminit");

	Status = PmInit(NULL, &IpiInst, GetBootState());
	if (XST_SUCCESS != Status) {
		VMR_ERR("PM initialization failed ERROR=%d\r\n", Status);
	}
	/* Set Next boot state as clen */
	SetNextBootState(RPU_CLEAN_BOOT_KEY);

	VMR_WARN("done");
	return Status;
}

static int rmgmt_subsystem_fini()
{
	if (cl_xgq_receive_fini()) {
		VMR_ERR("xgq recevie cleanup failed, reset aborted!");
		return -EINVAL;
	}

	if (cl_xgq_client_fini()) {
		VMR_ERR("xgq client cleanup failed, reset aborted!");
		return -EINVAL;
	}

	return 0;
}

int rmgmt_pm_reset_rpu(struct cl_msg *msg)
{
	int Status = XST_FAILURE;

	VMR_WARN("start reset subsystem ...");
	vTaskDelay(pdMS_TO_TICKS(1000));

	/* wait for rpu app cleanup itself */
	Status = rmgmt_subsystem_fini();
	if (Status)
		return Status;

	VMR_WARN("xgq recevie cleanup successfully! reset continues ...");
	/*
	 * We don't need to reset whole system.
	 * Status = XPm_SystemShutdown(PM_SHUTDOWN_TYPE_RESET, PM_SHUTDOWN_SUBTYPE_RST_SYSTEM);
	 */
	Status = XPm_SystemShutdown(PM_SHUTDOWN_TYPE_RESET, PM_SHUTDOWN_SUBTYPE_RST_SUBSYSTEM);
	VMR_WARN("reset requested, wait for RPU subsystem reset.");

	if (XST_SUCCESS != Status) {
		VMR_ERR("Reset RPU subsystem error=%d", Status);
		goto done;
	}

	VMR_WARN("Reset RPU subsystem ... in progress");
	/* since we attempted to restart, hung here */
	while (1);

done:
	return Status;
}
