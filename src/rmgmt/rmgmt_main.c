/******************************************************************************
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#include "rpu_main.h"
#include "rmgmt_util.h"
#include "rmgmt_ospi.h"
#include "rmgmt_xfer.h"

static TaskHandle_t xR5Task;
static struct rmgmt_handler rh = { 0 };

/*
 * Task on R5 (same as zocl_ov_thread)
 */
static void pvR5Task( void *pvParameters )
{
	const TickType_t x1second = pdMS_TO_TICKS( 1000*1 );
	u8 pkt_flags, pkt_type;
	int cnt = 0;

	for( ;; )
	{
		if (rmgmt_check_for_status(&rh, XRT_XFR_PKT_STATUS_IDLE)) {
			/* increment count every tick */
			IO_SYNC_WRITE32(cnt++, RMGMT_HEARTBEAT_REG);
			//if (++cnt % 10 == 0)
			//	RMGMT_DBG("1 heartbeat %d\r\n", cnt);
			vTaskDelay( x1second );
			continue;
		}

		RMGMT_LOG("get new pkt, processing ... \r\n");
		pkt_flags = rmgmt_get_pkt_flags(&rh);
		pkt_type = pkt_flags >> XRT_XFR_PKT_TYPE_SHIFT & XRT_XFR_PKT_TYPE_MASK;

		switch (pkt_type) {
		case XRT_XFR_PKT_TYPE_PDI:
			/* pass whole xsabin instead of pdi for authentication */
			rmgmt_download_rpu_pdi(&rh);
			break;
		case XRT_XFR_PKT_TYPE_XCLBIN:
			FreeRTOS_ClearTickInterrupt();
			rmgmt_download_xclbin(&rh);
			FreeRTOS_SetupTickInterrupt();
			break;
		case XRT_XFR_PKT_TYPE_APU_PDI:
			rmgmt_download_apu_pdi(&rh);
			break;
		default:
			RMGMT_LOG("WARN: Unknown packet type: %d\r\n", pkt_type);
			break;
		}

		RMGMT_LOG("Re-start for next pkt ...\r\n");
	}

	RMGMT_LOG("FATAL: should never be here!\r\n");
}

void RMGMT_Launch( void )
{	
	if (rmgmt_init_handler(&rh) != 0) {
		RMGMT_LOG("FATAL: init rmgmt handler failed.\r\n");
		return;
	}

	rmgmt_load_apu(&rh);

	if (xTaskCreate( pvR5Task,
		 ( const char * ) "R5-0",
		 2048,
		 NULL,
		 tskIDLE_PRIORITY + 1,
		 &xR5Task
		 ) != pdPASS) {
		xil_printf("pvR5Task fail \r\n");
	}
}
