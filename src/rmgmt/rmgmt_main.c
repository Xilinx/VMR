/******************************************************************************
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#include "rmgmt_main.h"
#include "rmgmt_ospi.h"
#include "rmgmt_xfer.h"

//static void pvHostTask( void *pvParameters );
static void pvR5Task( void *pvParameters );
//static TaskHandle_t xHostTask;
static TaskHandle_t xR5Task;

/*
 * Task on R5 (same as zocl_ov_thread)
 */
static void pvR5Task( void *pvParameters )
{
	const TickType_t x1second = pdMS_TO_TICKS( 1000*1 );
	struct zocl_ov_dev ov = { 0 };
	u8 pkt_flags, pkt_type;
	int cnt = 0;

	if (rmgmt_init_xfer(&ov) != 0)
		return;
	RMGMT_LOG("Init ospi xfer done.\r\n");

	for( ;; )
	{
		if (rmgmt_check_for_status(&ov, XRT_XFR_PKT_STATUS_IDLE)) {
			/* increment count every tick */
			IO_SYNC_WRITE32(cnt++, RMGMT_HEARTBEAT_REG);
			vTaskDelay( x1second );
			continue;
		}
		RMGMT_LOG("get new pkt, processing ... \r\n");
		pkt_flags = rmgmt_get_pkt_flags(&ov);
		pkt_type = pkt_flags >> XRT_XFR_PKT_TYPE_SHIFT &
				XRT_XFR_PKT_TYPE_MASK;
		switch (pkt_type) {
		case XRT_XFR_PKT_TYPE_PDI:
			/* Note: for R5, we pass whole xsabin instead of pdi for authentication */
			rmgmt_get_xsabin(&ov);
			break;
		case XRT_XFR_PKT_TYPE_XCLBIN:
			rmgmt_get_xclbin(&ov);
			break;
		default:
			RMGMT_LOG("Unknown packet type: %d\r\n", pkt_type);
			break;
		}
		cnt = 0;
		RMGMT_LOG("processing done, wait for next pkt ...\r\n");
	}
}

//
//static void pvHostTask( void *pvParameters )
//{
//	const TickType_t x1second = pdMS_TO_TICKS( 1000*1 );
//
//	for( ;; )
//	{
//		xil_printf("Host sent \n");
//		/* Delay for 1 second. */
//		vTaskDelay( x1second );
//
//		xfer_versal_write();
//	}
//}

static void  testEndian() {
	int x = 1;
	char *c = (char *)&x;
	RMGMT_DBG("Endian is %s\r\n", (int)c[0] == 1 ? "Little" : "Big");
}

int main( void )
{

	testEndian();

//	if (xTaskCreate( pvHostTask, 	/* The function that implements the task. */
//		( const char * ) "Host", 	/* Text name for the task, provided to assist debugging only. */
//		configMINIMAL_STACK_SIZE, 	/* The stack allocated to the task. */
//		NULL, 						/* The task parameter is not used, so set to NULL. */
//		tskIDLE_PRIORITY,			/* The task runs at the idle priority. */
//		&xHostTask
//		) != pdPASS) {
//		xil_printf("pvHostTask fail \n");
//	}

	if (xTaskCreate( pvR5Task,
				 ( const char * ) "R5-0",
				 configMINIMAL_STACK_SIZE,
				 NULL,
				 tskIDLE_PRIORITY,
				 &xR5Task
				 ) != pdPASS) {
		xil_printf("pvR5Task fail \n");
	}

	vTaskStartScheduler();

	xil_printf( "not enough memory\n" );
}
