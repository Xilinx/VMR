/******************************************************************************
* Copyright (C) 2024 Advanced Micro Devices, Inc.    All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/
#include "pm_api_sys.h"
#include "ipi.h"
#include "gic_setup.h"
#include "pm_init.h"

#include "rmgmt_common.h"

/* Select TTC for timer based on xparameters.h */
#if (SLEEP_TIMER_BASEADDR == XPAR_VERSAL_CIPS_0_PSPMC_0_PSV_TTC_0_BASEADDR)
	#define PM_DEV_TTC_FOR_TIMER PM_DEV_TTC_0
#elif (SLEEP_TIMER_BASEADDR  == XPAR_VERSAL_CIPS_0_PSPMC_0_PSV_TTC_3_BASEADDR)
	#define PM_DEV_TTC_FOR_TIMER PM_DEV_TTC_1
#elif (SLEEP_TIMER_BASEADDR  == XPAR_VERSAL_CIPS_0_PSPMC_0_PSV_TTC_6_BASEADDR)
	#define PM_DEV_TTC_FOR_TIMER PM_DEV_TTC_2
#elif (SLEEP_TIMER_BASEADDR  == XPAR_VERSAL_CIPS_0_PSPMC_0_PSV_TTC_9_BASEADDR)
	#define PM_DEV_TTC_FOR_TIMER PM_DEV_TTC_3
#endif



XStatus PmInit(XScuGic *const GicInst, XIpiPsu *const IpiInst, u32 FullBoot)
{
        int Status;

	/* GIC Initialize */
	if (NULL != GicInst) {
		VMR_WARN("gic setup");
		Status = GicSetupInterruptSystem(GicInst);
		if (Status != XST_SUCCESS) {
			VMR_WARN("GicSetupInterruptSystem() failed with error: %d\r\n", Status);
			goto done;
		}
	}

	/* IPI Initialize */
        Status = IpiInit(GicInst, IpiInst);
        if (XST_SUCCESS != Status) {
                VMR_WARN("IpiInit() failed with error: %d\r\n", Status);
                goto done;
        }

	/* XilPM Initialize */
        Status = XPm_InitXilpm(IpiInst);
        if (XST_SUCCESS != Status) {
                VMR_WARN("XPm_InitXilpm() failed with error: %d\r\n", Status);
                goto done;
        }
#if 0
	/* Request the required Nodes for the rpu application */
        Status = XPm_RequestNode(PM_DEV_UART_1, PM_CAP_ACCESS, 0, 0);
		if (XST_SUCCESS != Status) {
			VMR_ERR("XPm_RequestNode of TTC is failed with error: %d\r\n", Status);
			goto done;
		}

        Status = XPm_RequestNode(PM_DEV_TTC_FOR_TIMER, PM_CAP_ACCESS, 0, 0);
		if (XST_SUCCESS != Status) {
			VMR_ERR("XPm_RequestNode of TTC is failed with error: %d\r\n", Status);
			goto done;
		}

	Status = XPm_RequestNode(PM_DEV_SWDT_LPD, PM_CAP_ACCESS, 0, 0);
		if (XST_SUCCESS != Status) {
			VMR_ERR("XPm_RequestNode of TTC is failed with error: %d\r\n", Status);
			goto done;
		}
	if(FullBoot) {
		/* Finalize Initialization */
		VMR_WARN("Xpm init fullboot");
		Status = XPm_InitFinalize();
		if (XST_SUCCESS != Status) {
			VMR_WARN("XPm_initfinalize() failed\r\n");
			goto done;
		}
	}
	else {
		VMR_ERR("BAD Boot: Skipping XPm_InitFinalize\r\n");
	}
#endif

done:

	VMR_WARN("Status: %d", Status);
        return Status;
}
