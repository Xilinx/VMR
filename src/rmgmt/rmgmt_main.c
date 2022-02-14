/******************************************************************************
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#include <stdbool.h>
#include <stdint.h>

#include "rmgmt_common.h"
#include "rmgmt_xfer.h"
#include "rmgmt_clock.h"
#include "rmgmt_fpt.h"

#include "cl_msg.h"
#include "cl_flash.h"
#include "cl_main.h"

static TaskHandle_t xXGQTask;
static struct rmgmt_handler rh = { 0 };
msg_handle_t *pdi_hdl;
msg_handle_t *xclbin_hdl;
msg_handle_t *af_hdl;
msg_handle_t *clock_hdl;
msg_handle_t *apubin_hdl;
msg_handle_t *vmr_hdl;
int xgq_pdi_flag = 0;
int xgq_xclbin_flag = 0;
int xgq_log_page_flag = 0;
int xgq_sensor_flag = 0;
int xgq_clock_flag = 0;
int xgq_apubin_flag = 0;
int xgq_vmr_control_flag = 0;
int xgq_apu_control_flag = 0;

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

static int rmgmt_download_pdi(cl_msg_t *msg, bool is_rpu_pdi)
{
	int ret = 0;

	u32 address = RPU_SHARED_MEMORY_ADDR(msg->data_payload.address);
	u32 size = msg->data_payload.size;
	if (size > rh.rh_max_size) {
		RMGMT_LOG("ERROR: size %d is too big", size);
		ret = 1;
	}

	/* prepare rmgmt handler */
	rh.rh_data_size = size;
	cl_memcpy_fromio8(address, rh.rh_data, size);

	if (is_rpu_pdi)
		ret = rmgmt_flush_rpu_pdi(&rh, msg);
	else
		ret = rmgmt_download_apu_pdi(&rh);

	msg->hdr.rcode = ret;

	RMGMT_DBG("complete msg id %d, ret %d", msg->hdr.cid, ret);
	cl_msg_handle_complete(msg);
	return 0;
}

static int xgq_pdi_cb(cl_msg_t *msg, void *arg)
{
	return rmgmt_download_pdi(msg, true);
}

static int xgq_apubin_cb(cl_msg_t *msg, void *arg)
{
	return rmgmt_download_pdi(msg, false);
}

static int xgq_xclbin_cb(cl_msg_t *msg, void *arg)
{
	int ret = 0;

	u32 address = RPU_SHARED_MEMORY_ADDR(msg->data_payload.address);
	u32 size = msg->data_payload.size;
	if (size > rh.rh_max_size) {
		RMGMT_LOG("ERROR: size %d is too big", size);
		ret = 1;
	}

	/* prepare rmgmt handler */
	rh.rh_data_size = size;
	cl_memcpy_fromio8(address, rh.rh_data, size);

	ret = rmgmt_download_xclbin(&rh);

	msg->hdr.rcode = ret;
	RMGMT_DBG("complete msg id%d, ret %d", msg->hdr.cid, ret);
	cl_msg_handle_complete(msg);
	return 0;
}

static int check_firewall()
{
	u32 firewall = EP_FIREWALL_USER_BASE;
	u32 val;

	val = IO_SYNC_READ32(firewall);

	return IS_FIRED(val);
}

static int load_firmware(cl_msg_t *msg)
{
	u32 dst_addr = 0;
	u32 src_addr = 0;
	u32 offset = 0;
	u32 size = 0;

	if (rmgmt_fpt_get_xsabin(msg, &offset, &size)) {
		RMGMT_ERR("get xsabin firmware failed");
		return -1;
	}

	dst_addr = RPU_SHARED_MEMORY_ADDR(msg->log_payload.address);
	src_addr = offset;

	/*
	 * TODO: the dst size can be small, check size <= dst size and
	 * copy small truck back based on offset + dst size in the future */
	cl_memcpy(dst_addr, src_addr, size);

	/* remaining cound is actually size for now,
	 * it can be entire size - offset in the future*/
	msg->log_payload.size = size;

	return 0;
}

static int xgq_log_page_cb(cl_msg_t *msg, void *arg)
{
	int ret = 0;

	switch (msg->log_payload.pid) {
	case CL_LOG_AF:
		ret = check_firewall();
		break;
	case CL_LOG_FW:
		ret = load_firmware(msg);
		break;
	case CL_LOG_DBG:
	default:
		RMGMT_LOG("unsupported type %d", msg->log_payload.pid);
		ret = -1;
		break;
	}

	msg->hdr.rcode = ret;
	cl_msg_handle_complete(msg);
	RMGMT_DBG("complete msg id %d, ret %d", msg->hdr.cid, ret);
	return 0;
}

static int rmgmt_init_pmc()
{
	u32 val = 0;
	u32 pmc_intr = EP_PMC_REG;

	val = IO_SYNC_READ32(pmc_intr);

	if (val & PMC_ERR1_STATUS_MASK) {
		val &= ~PMC_ERR1_STATUS_MASK;
		IO_SYNC_WRITE32(val, pmc_intr); 
	}

	IO_SYNC_WRITE32(PMC_ERR_OUT1_EN_MASK, pmc_intr + PMC_REG_ERR_OUT1_EN);
	val = IO_SYNC_READ32(pmc_intr + PMC_REG_ERR_OUT1_MASK);
	if (val & PMC_ERR_OUT1_EN_MASK) {
		RMGMT_LOG("mask 0x%x for PMC_REG_ERR_OUT1_MASK 0x%x "
		    "should be 0.\n", PMC_ERR_OUT1_EN_MASK, val);
		return -1;
	}

	IO_SYNC_WRITE32(PMC_POR1_EN_MASK, pmc_intr + PMC_REG_POR1_EN);
	val = IO_SYNC_READ32(pmc_intr + PMC_REG_POR1_MASK);
	if (val & PMC_POR1_EN_MASK) {
		RMGMT_LOG("mask 0x%x for PMC_REG_POR1_MASK 0x%x "
		    "should be 0.\n", PMC_POR1_EN_MASK, val);
		return -1;
	}

	RMGMT_LOG("done");
	return 0;
}

static int rmgmt_enable_boot_default()
{
	int ret = 0;

	if (rmgmt_init_pmc())
		return -1;

	ret = rmgmt_enable_pl_reset();
	if (ret && ret != -ENODEV) {
		RMGMT_ERR("request reset failed %d", ret);
		return -1;
	}

	RMGMT_LOG("done");
	return 0;
}

static void rmgmt_enable_srst_por()
{
	u32 pmc_intr = EP_PMC_REG;

	IO_SYNC_WRITE32(PMC_POR_ENABLE_BIT, pmc_intr + PMC_REG_ACTION);
	IO_SYNC_WRITE32(PMC_POR_ENABLE_BIT, pmc_intr + PMC_REG_SRST);
}

static void rmgmt_set_multiboot(u32 offset)
{
	IO_SYNC_WRITE32(offset, EP_PLM_MULTIBOOT);
}

static int rmgmt_enable_boot_backup()
{
	int ret = 0;

	if (rmgmt_init_pmc())
		return -1;
	
	rmgmt_enable_srst_por();
	rmgmt_set_multiboot(0xC00);

	ret = rmgmt_enable_pl_reset();
	if (ret && ret != -ENODEV) {
		RMGMT_ERR("request reset failed %d", ret);
		return -1;
	}

	RMGMT_LOG("done");
	return 0;
}

static int xgq_vmr_cb(cl_msg_t *msg, void *arg)
{
	int ret = 0;

	switch (msg->multiboot_payload.req_type) {
	case CL_MULTIBOOT_DEFAULT:
		ret = rmgmt_enable_boot_default();
		break;
	case CL_MULTIBOOT_BACKUP:
		ret = rmgmt_enable_boot_backup();
		break;
	case CL_VMR_QUERY:
		rmgmt_fpt_query(msg);
		cl_rpu_status_query(msg);
		cl_apu_status_query(msg);
		break;
	case CL_PROGRAM_SC:
		/* place holder for starting SC download */
		VMC_Start_SC_Update();
		break;
	default:
		RMGMT_LOG("unknown type %d", msg->multiboot_payload.req_type);
		ret = -1;
		break;
	}

	msg->hdr.rcode = ret;
	cl_msg_handle_complete(msg);
	RMGMT_DBG("complete msg id %d, ret %d", msg->hdr.cid, ret);
	return 0;
}

static int xgq_apu_channel_probe()
{
	if (cl_xgq_client_probe() != 0) {
		return -1;
	}

	return 0;
}

static void pvXGQTask( void *pvParameters )
{
	const TickType_t x1second = pdMS_TO_TICKS( 1000*1 );
	int cnt = 0;

	for ( ;; )
	{
		vTaskDelay( x1second );

		cnt++;
		//xil_printf("%d  pvXGQTask in main \r", cnt);

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

		if (xgq_log_page_flag == 0 &&
		    cl_msg_handle_init(&af_hdl, CL_MSG_LOG_PAGE, xgq_log_page_cb, NULL) == 0) {
			RMGMT_LOG("init log page handle done.");
			xgq_log_page_flag = 1;
		}

		if (xgq_clock_flag == 0 &&
		    cl_msg_handle_init(&clock_hdl, CL_MSG_CLOCK, xgq_clock_cb, NULL) == 0) {
			RMGMT_LOG("init sensor handle done.");
			xgq_clock_flag = 1;
		}

		if (xgq_apubin_flag == 0 &&
		    cl_msg_handle_init(&apubin_hdl, CL_MSG_APUBIN, xgq_apubin_cb, NULL) == 0) {
			RMGMT_LOG("init apubin handle done.");
			xgq_apubin_flag = 1;
		}

		if (xgq_vmr_control_flag == 0 &&
		    cl_msg_handle_init(&vmr_hdl, CL_MSG_VMR_CONTROL, xgq_vmr_cb, NULL) == 0) {
			RMGMT_LOG("init vmr control handle done.");
			xgq_vmr_control_flag = 1;
		}

		if (xgq_apu_control_flag == 0 && xgq_apu_channel_probe() == 0) {
			RMGMT_LOG("apu is ready.");
			xgq_apu_control_flag = 1;
		}

		RMGMT_ERR("free heap %d", xPortGetFreeHeapSize());
	}
	RMGMT_LOG("done");
}

static int rmgmt_create_tasks(void)
{
	if (xTaskCreate( pvXGQTask,
		 ( const char * ) "R5-0-XGQ",
		 TASK_STACK_DEPTH,
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

	RMGMT_LOG("done.");
	return 0;
}
