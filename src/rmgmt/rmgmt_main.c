/******************************************************************************
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#include "rmgmt_common.h"
#include "rmgmt_xfer.h"
#include "cl_msg.h"

static TaskHandle_t xR5Task;
static TaskHandle_t xXGQTask;
static struct rmgmt_handler rh = { 0 };
msg_handle_t *xclbin_hdl;
msg_handle_t *af_hdl;
int xgq_xclbin_flag = 0;
int xgq_af_flag = 0;

int xgq_firewall_cid = -1;

static int xgq_xclbin_cb(cl_msg_t *msg, void *arg)
{
	RMGMT_DBG("cb to complete msg id%d", msg->pkt.head.cid);
	/*TODO: shared DDR not working, we still use xfer to transfer data */
	msg->pkt.head.rcode = 0;
	cl_msg_handle_complete(msg);
	return 0;
}

static int xgq_af_cb(cl_msg_t *msg, void *arg)
{
	RMGMT_DBG("cb to enable firewall checking for id%d", msg->pkt.head.cid);
	xgq_firewall_cid = msg->pkt.head.cid;
}

static void check_firewall()
{
	cl_msg_t msg;

	if (xgq_firewall_cid == -1) {
		return;
	}

	RMGMT_DBG("async check firewall on cid %d", xgq_firewall_cid);
	msg.pkt.head.cid = xgq_firewall_cid;
	msg.pkt.head.rcode = 1;

	cl_msg_handle_complete(&msg);
	xgq_firewall_cid = -1;
}

static void pvXGQTask( void *pvParameters )
{
	const TickType_t x1second = pdMS_TO_TICKS( 1000*1 );
	int cnt = 0;

	RMGMT_DBG("->");
	for ( ;; )
	{
		vTaskDelay( x1second );
		cnt++;

		//RMGMT_DBG("hb %d.", cnt++);
		if (xgq_xclbin_flag == 0 &&
		    cl_msg_handle_init(&xclbin_hdl, CL_MSG_XCLBIN, xgq_xclbin_cb, NULL) == 0) {
			RMGMT_LOG("init xclbin download handle done.");
			xgq_xclbin_flag = 1;
		}

		if (xgq_af_flag == 0 &&
		    cl_msg_handle_init(&af_hdl, CL_MSG_AF, xgq_af_cb, NULL) == 0) {
			RMGMT_LOG("init firewall handle done.");
			xgq_af_flag = 1;
		} else if ((xgq_af_flag == 1) && (cnt % 5 == 0)) {
			check_firewall();
		}
	}
	RMGMT_DBG("<-");
}

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
			//IO_SYNC_WRITE32(cnt++, RMGMT_HEARTBEAT_REG);
			if (++cnt % 100 == 0)
				RMGMT_DBG("heartbeat %d", cnt);
			vTaskDelay( x1second );
			continue;
		}

		CL_LOG(APP_RMGMT, "get new pkt, processing ... \r\n");
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
			CL_LOG(APP_RMGMT, "WARN: Unknown packet type: %d\r\n", pkt_type);
			break;
		}

		RMGMT_LOG("Re-start for next pkt ...\r\n");
	}

	RMGMT_LOG("FATAL: should never be here!\r\n");
}

static int rmgmt_create_tasks(void)
{
	if (xTaskCreate( pvR5Task,
		 ( const char * ) "R5-0-xfer",
		 2048,
		 NULL,
		 tskIDLE_PRIORITY + 1,
		 &xR5Task
		 ) != pdPASS) {
		RMGMT_LOG("FATAL: pvR5Task creation failed.\r\n");
		return -1;
	}

	if (xTaskCreate( pvXGQTask,
		 ( const char * ) "R5-0-XGQ",
		 configMINIMAL_STACK_SIZE,
		 NULL,
		 tskIDLE_PRIORITY + 1,
		 &xXGQTask) != pdPASS) {

		RMGMT_LOG("FATAL: pvXGQTask creation failed.\r\n");
		return -1;
	}

	return 0;
}

int RMGMT_Launch( void )
{	
	int ret = 0;

	ret = rmgmt_init_handler(&rh);
	if (ret != 0) {
		RMGMT_LOG("FATAL: init rmgmt handler failed.\r\n");
		return ret;
	}

#if 0
	rmgmt_load_apu(&rh);
#endif

	ret = rmgmt_create_tasks();
	if (ret != 0)
		return ret;

	RMGMT_LOG("succeeded.\r\n");
	return 0;
}
