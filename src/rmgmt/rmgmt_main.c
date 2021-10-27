/******************************************************************************
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#include "rmgmt_common.h"
#include "rmgmt_xfer.h"
#include "rmgmt_clock.h"

#include "cl_msg.h"
#include "cl_flash.h"


//static TaskHandle_t xR5Task;
static TaskHandle_t xXGQTask;
static struct rmgmt_handler rh = { 0 };
msg_handle_t *pdi_hdl;
msg_handle_t *xclbin_hdl;
msg_handle_t *af_hdl;
msg_handle_t *clock_hdl;
int xgq_pdi_flag = 0;
int xgq_xclbin_flag = 0;
int xgq_af_flag = 0;
int xgq_sensor_flag = 0;
int xgq_clock_flag = 0;

static int xgq_clock_cb(cl_msg_t *msg, void *arg)
{
	int ret = 0;
	uint32_t freq = 0;

	switch (msg->clock_payload.ocl_req_type) {
	case CL_CLOCK_SCALE:
		ret = rmgmt_clock_freq_scaling(msg);
		break;
	case CL_CLOCK_WIZARD:
		freq = rmgmt_clock_get_freq(msg->clock_payload.ocl_req_id,
			RMGMT_CLOCK_WIZARD);
		msg->clock_payload.ocl_req_freq[0] = freq;
		break;
	case CL_CLOCK_COUNTER:
		freq = rmgmt_clock_get_freq(msg->clock_payload.ocl_req_id,
			RMGMT_CLOCK_COUNTER);
		msg->clock_payload.ocl_req_freq[0] = freq;
		break;
	default:
		RMGMT_LOG("ERROR: unknown req_type %d",
			msg->clock_payload.ocl_req_type);
		break;
	}

	msg->hdr.rcode = ret;
	cl_msg_handle_complete(msg);
	RMGMT_DBG("complete msg id %d, ret %d", msg->hdr.cid, ret);
	return 0;
}

static int xgq_pdi_cb(cl_msg_t *msg, void *arg)
{
	int ret = 0;

	u32 address = EP_RING_BUFFER_BASE + (u32)msg->data_payload.address;
	u32 size = msg->data_payload.size;
	if (size > rh.rh_max_size) {
		RMGMT_LOG("ERROR: size %d is too big", size);
		ret = 1;
	}

	/* prepare rmgmt handler */
	rh.rh_data_size = size;
	cl_memcpy_fromio(address, rh.rh_data, size);

	ret = rmgmt_download_rpu_pdi(&rh);
	//ret = rmgmt_download_apu_pdi(&rh);

	msg->hdr.rcode = ret;

	RMGMT_DBG("complete msg id%d, ret %d", msg->hdr.cid, ret);
	cl_msg_handle_complete(msg);
	return 0;
}

static int xgq_xclbin_cb(cl_msg_t *msg, void *arg)
{
	int ret = 0;

	u32 address = EP_RING_BUFFER_BASE + (u32)msg->data_payload.address;
	u32 size = msg->data_payload.size;
	if (size > rh.rh_max_size) {
		RMGMT_LOG("ERROR: size %d is too big", size);
		ret = 1;
	}

	/* prepare rmgmt handler */
	rh.rh_data_size = size;
	cl_memcpy_fromio(address, rh.rh_data, size);

	ret = rmgmt_download_xclbin(&rh);

	msg->hdr.rcode = ret;

	RMGMT_DBG("complete msg id%d, ret %d", msg->hdr.cid, ret);
	cl_msg_handle_complete(msg);
	return 0;
}

#define FAULT_STATUS            0x0
#define BIT(n) 			(1UL << (n))
#define READ_RESPONSE_BUSY      BIT(0)
#define WRITE_RESPONSE_BUSY     BIT(16)
#define FIREWALL_STATUS_BUSY    (READ_RESPONSE_BUSY | WRITE_RESPONSE_BUSY)
#define IS_FIRED(val) 		(val & ~FIREWALL_STATUS_BUSY)

static int check_firewall()
{
	u32 firewall = EP_FIREWALL_USER_BASE;
	u32 val;

	val = IO_SYNC_READ32(firewall);

	return IS_FIRED(val);
}

static int xgq_af_cb(cl_msg_t *msg, void *arg)
{
	msg->hdr.rcode = check_firewall();

	RMGMT_DBG("complete msg id %d, ret 0x%x", msg->hdr.cid, msg->hdr.rcode);

	/* workaround due to CU config will trigger this */
	msg->hdr.rcode = 0;

	cl_msg_handle_complete(msg);
	return 0;
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
		//xil_printf("%d  xgqTask\r", cnt);

		if (xgq_pdi_flag == 0 &&
		    cl_msg_handle_init(&pdi_hdl, CL_MSG_PDI, xgq_pdi_cb, NULL) == 0) {
			RMGMT_LOG("init pdi download handle done.");
			xgq_pdi_flag = 1;
		}

		if (xgq_xclbin_flag == 0 &&
		    cl_msg_handle_init(&xclbin_hdl, CL_MSG_XCLBIN, xgq_xclbin_cb, NULL) == 0) {
			RMGMT_LOG("init xclbin download handle done.");
			xgq_xclbin_flag = 1;
		}

		if (xgq_af_flag == 0 &&
		    cl_msg_handle_init(&af_hdl, CL_MSG_AF, xgq_af_cb, NULL) == 0) {
			RMGMT_LOG("init firewall handle done.");
			xgq_af_flag = 1;
		}

		if (xgq_clock_flag == 0 &&
		    cl_msg_handle_init(&clock_hdl, CL_MSG_CLOCK, xgq_clock_cb, NULL) == 0) {
			RMGMT_LOG("init sensor handle done.");
			xgq_clock_flag = 1;
		}
	}
	RMGMT_DBG("<-");
}

#if 0
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
#endif

static int rmgmt_create_tasks(void)
{
	/* Disabled xfer transfer thread
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
	*/

	if (xTaskCreate( pvXGQTask,
		 ( const char * ) "R5-0-XGQ",
		 configMINIMAL_STACK_SIZE,
		 NULL,
		 tskIDLE_PRIORITY + 1,
		 &xXGQTask) != pdPASS) {

		RMGMT_LOG("FATAL: pvXGQTask creation failed.");
		return -1;
	}

	return 0;
}

int RMGMT_Launch( void )
{	
	int ret = 0;

	ret = rmgmt_init_handler(&rh);
	if (ret != 0) {
		RMGMT_LOG("FATAL: init rmgmt handler failed.");
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
