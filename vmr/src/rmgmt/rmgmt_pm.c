/******************************************************************************
* Copyright (C) 2024 AMD, Inc.    All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#include <stdbool.h>
#include <stdint.h>

#include "rmgmt_pm.h"
#include "cl_main.h"

#include "pm_api_sys.h"
#include "xpm_defs.h"
#include "gic_setup.h"
#include "ipi.h"
#include "pm_init.h"
#include "xloader_client.h"
#include "rmgmt_common.h"
#include "cl_msg.h"

/* RPU Private Persistant Area */
/* TODO: explian those fixed address which is from SSW code example */
#define RPU_PPA_BASE				0x3E500000
#define RPU_PPA_BAD_BOOT_OFFSET			0x10
#define RPU_BAD_BOOT_KEY			0x37232323
#define RPU_CLEAN_BOOT_KEY			0x0
#define NODE_ID						(0x18700000) /* Node to configure */


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
	int Status = XST_SUCCESS, uuid = 0;

	VMR_WARN("pminit");

	Status = PmInit(NULL, &IpiInst, GetBootState());
	if (XST_SUCCESS != Status) {
		VMR_ERR("PM initialization failed ERROR=%d\r\n", Status);
	}
	/* Set Next boot state as clen */
	SetNextBootState(RPU_CLEAN_BOOT_KEY);

#if defined(CONFIG_RAVE)
	/* Hardware ROM ID is eliminated on RAVE Platform, hence fetch it from PLM and
	 * write to AXI Register
	 */
	Status = rmgmt_plm_get_uid(&uuid);

	if(XST_SUCCESS != Status) {
		VMR_ERR("%s: PLM UUID Read Failed st=%d",__func__, Status);
	}
	else {
		/* Copy UUID to AXI UUID Register mapped to S1 AXI 0x20102002000 via qdma */
		cl_memcpy_toio32(XPAR_BLP_BLP_LOGIC_UUID_S0_AXI_ADDR, &uuid, sizeof(uint32_t));
	}
#endif
	VMR_WARN("DONE");
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

int rmgmt_eemi_pm_reset(struct cl_msg *msg)
{
	int Status = XST_FAILURE;

	VMR_WARN("start reset subsystem ...");
	vTaskDelay(pdMS_TO_TICKS(1000));

	/* wait for rpu app cleanup itself */
	Status = rmgmt_subsystem_fini();
	if (Status)
		goto done;

	VMR_WARN("xgq receive cleanup successfully! reset continues ...multiboot=%x req_type=%x", msg->multiboot_payload.boot_on_backup, msg->multiboot_payload.req_type);

	/* Enable multiboot based on the boot config from EEMI SRST XGQ command.
	 * NOTE: Here at this point this is not the config read from fpt query yet.
	 */
	if (msg->multiboot_payload.boot_on_backup == 1) {
		rmgmt_enable_boot_backup(msg);
	}
	else {
		rmgmt_enable_boot_default(msg);
	}
	/*  Reset whole system. */
	Status = XPm_SystemShutdown(PM_SHUTDOWN_TYPE_RESET, PM_SHUTDOWN_SUBTYPE_RST_SYSTEM);
	if (XST_SUCCESS != Status) {
		VMR_ERR("Reset system error=%d", Status);
		goto done;
	}
	VMR_WARN("Reset entire system ... in progress");

	/* since VMR attempted to restart, suspend here */
	while (1);

done:
	return Status;
}

int rmgmt_plm_get_uid(int* uuid)
{
	int Status = XST_SUCCESS;
	XMailbox MailboxInstance;
	XLoader_ClientInstance LoaderClientInstance;
	XLoader_ImageInfo ImageInfo;

#ifndef SDT
	Status = XMailbox_Initialize(&MailboxInstance, 0U);
#else
	Status = XMailbox_Initialize(&MailboxInstance, XPAR_XIPIPSU_0_BASEADDR);
#endif
	if (Status != XST_SUCCESS) {
		goto done;
	}

	Status = XLoader_ClientInit(&LoaderClientInstance, &MailboxInstance);
	if (Status != XST_SUCCESS) {
		goto done;
	}

	Status = XLoader_GetImageInfo(&LoaderClientInstance, NODE_ID, &ImageInfo);
	if (XST_SUCCESS != Status) {
		VMR_ERR("Reading Image info failed =%d uuid=%x", Status, ImageInfo.UID);
		goto done;
	}

	*uuid = ImageInfo.UID;
	return XST_SUCCESS;

done:
	VMR_ERR("PLM UUID Read Failed =%d uuid=%x", Status, ImageInfo.UID);
	return Status;
}

