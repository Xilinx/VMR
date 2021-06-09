/******************************************************************************
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#include "rmgmt_main.h"
#include "rmgmt_ospi.h"
#include "rmgmt_xfer.h"

static void pvCMCTask( void *pvParameters );
static void pvR5Task( void *pvParameters );
static TaskHandle_t xCMCTask;
static TaskHandle_t xR5Task;

/*
 * Task on R5 (same as zocl_ov_thread)
 */
static void pvR5Task( void *pvParameters )
{
	const TickType_t x1second = pdMS_TO_TICKS( 1000*1 );
	struct rmgmt_handler rh = { 0 };
	u8 pkt_flags, pkt_type;
	int cnt = 0;

	if (rmgmt_init_handler(&rh) != 0) {
		RMGMT_LOG("FATAL: init rmgmt handler failed.\r\n");
		return;
	}

	for( ;; )
	{
		if (rmgmt_check_for_status(&rh, XRT_XFR_PKT_STATUS_IDLE)) {
			/* increment count every tick */
			//IO_SYNC_WRITE32(cnt++, RMGMT_HEARTBEAT_REG);
			if (++cnt % 10 == 0)
				RMGMT_DBG("heart beat %d\r\n", cnt);
			vTaskDelay( x1second );
			continue;
		}

		RMGMT_LOG("get new pkt, processing ... \r\n");
		pkt_flags = rmgmt_get_pkt_flags(&rh);
		pkt_type = pkt_flags >> XRT_XFR_PKT_TYPE_SHIFT & XRT_XFR_PKT_TYPE_MASK;

		switch (pkt_type) {
		case XRT_XFR_PKT_TYPE_PDI:
			/* pass whole xsabin instead of pdi for authentication */
			rmgmt_download_xsabin(&rh);
			break;
		case XRT_XFR_PKT_TYPE_XCLBIN:
			rmgmt_download_xclbin(&rh);
			break;
		default:
			RMGMT_LOG("WARN: Unknown packet type: %d\r\n", pkt_type);
			break;
		}

		RMGMT_LOG("Re-start for next pkt ...\r\n");
	}

	RMGMT_LOG("FATAL: should never be here!\r\n");
}


static void pvCMCTask( void *pvParameters )
{
	const TickType_t x1second = pdMS_TO_TICKS( 1000*1 );

	for( ;; )
	{
		xil_printf("Second thread tick ... \r\n");
		/* Delay for 1 second. */
		vTaskDelay( x1second * 5 );

	}
}

static void  testEndian() {
	int x = 1;
	char *c = (char *)&x;
	RMGMT_DBG("Endian is %s\r\n", (int)c[0] == 1 ? "Little" : "Big");
}

int main( void )
{

	testEndian();

	if (xTaskCreate( pvCMCTask, 	/* The function that implements the task. */
		( const char * ) "Host", 	/* Text name for the task, provided to assist debugging only. */
		//configMINIMAL_STACK_SIZE, 	/* The stack allocated to the task. */
		1024,
		NULL, 						/* The task parameter is not used, so set to NULL. */
		tskIDLE_PRIORITY,			/* The task runs at the idle priority. */
		&xCMCTask
		) != pdPASS) {
		xil_printf("pvHostTask fail \n");
	}

	if (xTaskCreate( pvR5Task,
		 ( const char * ) "R5-0",
		 2048,
		 NULL,
		 tskIDLE_PRIORITY,
		 &xR5Task
		 ) != pdPASS) {
		xil_printf("pvR5Task fail \n");
	}

	vTaskStartScheduler();

	xil_printf( "not enough memory\n" );
}
