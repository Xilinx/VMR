/******************************************************************************
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#include <stdbool.h>
#include <stdint.h>

#include "FreeRTOS.h"
#include "semphr.h"

#include "rmgmt_common.h"
#include "rmgmt_xfer.h"
#include "rmgmt_clock.h"
#include "rmgmt_fpt.h"

#include "cl_msg.h"
#include "cl_flash.h"
#include "cl_main.h"

static TaskHandle_t xXGQTask;
static struct rmgmt_handler rh = { 0 };
static bool rmgmt_is_ready = false;
msg_handle_t *pdi_hdl;
msg_handle_t *xclbin_hdl;
msg_handle_t *af_hdl;
msg_handle_t *clock_hdl;
msg_handle_t *apubin_hdl;
msg_handle_t *vmr_hdl;
int xgq_pdi_flag = 0;
int xgq_xclbin_flag = 0;
int xgq_log_page_flag = 0;
int xgq_clock_flag = 0;
int xgq_apubin_flag = 0;
int xgq_vmr_control_flag = 0;
int xgq_apu_control_flag = 0;
char log_msg[1024] = { 0 };

SemaphoreHandle_t xSemaDownload;

int32_t VMC_Start_SC_Update(void);

static struct vmr_endpoints vmr_eps[] = {
	{"VMR_EP_SYSTEM_DTB", VMR_EP_SYSTEM_DTB},
	{"VMR_EP_PR_ISOLATION", VMR_EP_PR_ISOLATION},
	{"VMR_EP_UCS_CONTROL", VMR_EP_UCS_CONTROL},
	{"VMR_EP_FIREWALL_USER_BASE", VMR_EP_FIREWALL_USER_BASE},
	{"VMR_EP_GAPPING_DEMAND", VMR_EP_GAPPING_DEMAND},
	{"VMR_EP_ACLK_KERNEL_0", VMR_EP_ACLK_KERNEL_0},
	{"VMR_EP_ACLK_KERNEL_1", VMR_EP_ACLK_KERNEL_1},
	{"VMR_EP_ACLK_FREQ_0", VMR_EP_ACLK_FREQ_0},
	{"VMR_EP_ACLK_FREQ_KERNEL_0", VMR_EP_ACLK_FREQ_KERNEL_0},
	{"VMR_EP_ACLK_FREQ_KERNEL_1", VMR_EP_ACLK_FREQ_KERNEL_1},
	{"VMR_EP_PLM_MULTIBOOT", VMR_EP_PLM_MULTIBOOT},
	{"VMR_EP_PMC_REG", VMR_EP_PMC_REG},
	{"VMR_EP_RPU_SHARED_MEMORY_START", VMR_EP_RPU_SHARED_MEMORY_START},
	{"VMR_EP_RPU_SHARED_MEMORY_END", VMR_EP_RPU_SHARED_MEMORY_END},
	{"VMR_EP_RPU_PRELOAD_FPT", VMR_EP_RPU_PRELOAD_FPT},
	{"VMR_EP_RPU_SQ_BASE", VMR_EP_RPU_SQ_BASE},
	{"VMR_EP_RPU_CQ_BASE", VMR_EP_RPU_CQ_BASE},
	{"VMR_EP_APU_SHARED_MEMORY_START", VMR_EP_APU_SHARED_MEMORY_START},
	{"VMR_EP_APU_SHARED_MEMORY_END", VMR_EP_APU_SHARED_MEMORY_END},
	{"VMR_EP_APU_SQ_BASE", VMR_EP_APU_SQ_BASE},
	{"VMR_EP_APU_CQ_BASE", VMR_EP_APU_CQ_BASE},
	{"VMR_EP_RPU_RING_BUFFER_BASE", VMR_EP_RPU_RING_BUFFER_BASE},
#ifdef CONFIG_FORCE_RESET
	{"VMR_EP_FORCE_RESET", VMR_EP_FORCE_RESET},
#endif
};

static bool acquire_download_sema()
{
	return xSemaphoreTake(xSemaDownload, (TickType_t)10) == pdTRUE;
}

static bool release_download_sema()
{
	return xSemaphoreGive(xSemaDownload) == pdTRUE;
}

/*
 * Block comment for validation functions:
 * We should always keep VMR running, any incoming request should be validated
 * in acceptable range, otherwise return -EINVAL.
 */
static int validate_clock_payload(struct xgq_vmr_clock_payload *payload)
{
	int ret = -EINVAL;

	if(!cl_xgq_pl_is_ready()) {
		RMGMT_ERR("pl is not ready.");
		return -EIO;
	}


	if (payload->ocl_region != 0) {
		RMGMT_ERR("ocl region %d is not 0", payload->ocl_region);
		return ret;
	}

	if (payload->ocl_req_type > CL_CLOCK_SCALE) {
		RMGMT_ERR("invalid req_type %d", payload->ocl_req_type);
		return ret;
	}

	if (payload->ocl_req_id > OCL_MAX_ID) {
		RMGMT_ERR("invalid req_id %d", payload->ocl_req_id);
		return ret;
	}

	if (payload->ocl_req_num > OCL_MAX_NUM) {
		RMGMT_ERR("invalid req_num %d", payload->ocl_req_num);
		return ret;
	}

	/*TODO: check freq by comparing with cached xclbin */

	return 0;
}

static int validate_data_payload(struct xgq_vmr_data_payload *payload)
{
	int ret = -EINVAL;
	u32 address = RPU_SHARED_MEMORY_ADDR(payload->address);
	u32 size = payload->size;

	if (address + size >= VMR_EP_RPU_SHARED_MEMORY_END) {
		RMGMT_ERR("address overflow 0x%x", address);
		return ret;
	}

	if (size > rh.rh_max_size) {
		RMGMT_ERR("size 0x%x over max 0x%x", size, rh.rh_max_size);
		return ret;
	}

	/* TODO: add more checking based on addr_type in the future */

	return 0;
}

static int validate_log_payload(struct xgq_vmr_log_payload *payload, u32 size)
{
	int ret = -EINVAL;
	u32 address = RPU_SHARED_MEMORY_ADDR(payload->address);

	if (address >= VMR_EP_RPU_SHARED_MEMORY_END) {
		RMGMT_ERR("address overflow 0x%x", address);
		return ret;
	}

	if (payload->pid > CL_LOG_INFO) {
		RMGMT_ERR("invalid log type 0x%x", payload->pid);
		return ret;
	}

	if (payload->size < size) {
		RMGMT_ERR("request size 0x%x is smaller than result size 0x%x",
			payload->size, size);
		return ret;
	}
	/* TODO: add more checking based on addr_type in the future */

	return 0;
}

static int xgq_clock_cb(cl_msg_t *msg, void *arg)
{
	int ret = 0;
	uint32_t freq = 0;

	ret = validate_clock_payload(&msg->clock_payload);
	if (ret)
		goto done;

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
		RMGMT_ERR("ERROR: unknown req_type %d",
			msg->clock_payload.ocl_req_type);
		break;
	}

done:
	msg->hdr.rcode = ret;
	cl_msg_handle_complete(msg);
	RMGMT_DBG("complete msg id %d, ret %d", msg->hdr.cid, ret);
	return 0;
}

static int rmgmt_download_pdi(cl_msg_t *msg, bool is_rpu_pdi)
{
	u32 address = 0;
	u32 size = 0;
	int ret = 0;

	ret = validate_data_payload(&msg->data_payload);
	if (ret)
		return ret;

	address = RPU_SHARED_MEMORY_ADDR(msg->data_payload.address);
	size = msg->data_payload.size;

	/* prepare rmgmt handler */
	rh.rh_data_size = size;
	cl_memcpy_fromio8(address, rh.rh_data, size);

	if (is_rpu_pdi)
		ret = rmgmt_flash_rpu_pdi(&rh, msg);
	else
		ret = rmgmt_download_apu_pdi(&rh);

	return ret;
}

/* We acquire sema for pdi but not for apubin because the apubin can be
 * run in parallel with xclbin download and sc download.
 * The race between pdi download and apubin download is handled by host driver.
 */
static int xgq_pdi_cb(cl_msg_t *msg, void *arg)
{
	int ret = 0;

	if (acquire_download_sema()) {
		ret = rmgmt_download_pdi(msg, true);
		release_download_sema();
	} else {
		RMGMT_WARN("system busy, please try later");
		ret = -EBUSY;
	}

	msg->hdr.rcode = ret;
	RMGMT_DBG("complete msg id %d, ret %d", msg->hdr.cid, ret);
	cl_msg_handle_complete(msg);
	return 0;
}

static int xgq_apubin_cb(cl_msg_t *msg, void *arg)
{
	int ret = 0;

	ret = rmgmt_download_pdi(msg, false);

	msg->hdr.rcode = ret;
	RMGMT_DBG("complete msg id %d, ret %d", msg->hdr.cid, ret);
	cl_msg_handle_complete(msg);
	return 0;
}

static int xgq_xclbin_cb(cl_msg_t *msg, void *arg)
{
	u32 address = 0;
	u32 size = 0;
	int ret = 0;

	if(!cl_xgq_pl_is_ready()) {
		RMGMT_ERR("pl is not ready");
		ret = -EIO;
		goto done;
	}

	ret = validate_data_payload(&msg->data_payload);
	if (ret)
		goto done;

	address = RPU_SHARED_MEMORY_ADDR(msg->data_payload.address);
	size = msg->data_payload.size;

	/* prepare rmgmt handler */
	rh.rh_data_size = size;
	cl_memcpy_fromio8(address, rh.rh_data, size);

	if (acquire_download_sema()) {
		ret = rmgmt_download_xclbin(&rh);
		release_download_sema();
	} else {
		RMGMT_WARN("system busy, please try later");
		ret = -EBUSY;
	}

done:
	msg->hdr.rcode = ret;
	RMGMT_DBG("complete msg id%d, ret %d", msg->hdr.cid, ret);
	cl_msg_handle_complete(msg);
	return 0;
}

static u32 rmgmt_fpt_status_query(cl_msg_t *msg, char *buf, u32 size)
{
	u32 count = 0;

	rmgmt_fpt_query(msg);

	count = snprintf(buf, size, "default image offset: 0x%lx\n"
		"default image size: 0x%lx\ndefault image capacity: 0x%lx\n",
		msg->multiboot_payload.default_partition_offset,
		msg->multiboot_payload.pdimeta_size,
		msg->multiboot_payload.default_partition_size);
	if (count > size) {
		RMGMT_WARN("msg is truncated");
		return size;
	}
	
	count += snprintf(buf + count, size - count, "backup image offset: 0x%lx\n"
		"backup image size: 0x%lx\nbackup image capacity: 0x%lx\n",
		msg->multiboot_payload.backup_partition_offset,
		msg->multiboot_payload.pdimeta_backup_size,
		msg->multiboot_payload.backup_partition_size);
	if (count > size) {
		RMGMT_WARN("msg is truncated");
		return size;
	}

	count += snprintf(buf + count, size - count, "SC firmware size: 0x%lx\n",
		msg->multiboot_payload.scfw_size);
	if (count > size) {
		RMGMT_WARN("msg is truncated");
		return size;
	}

	return count;
}

static u32 rmgmt_endpoints_query(cl_msg_t *msg, char *buf, u32 size)
{
	u32 count = 0;

	for (int i = 0; i < ARRAY_SIZE(vmr_eps); i++) {
		count += snprintf(buf + count, size - count, "%s: 0x%lx\n",
			vmr_eps[i].vmr_ep_name, vmr_eps[i].vmr_ep_address);
		if (count > size) {
			RMGMT_WARN("msg is truncated at %d %s", i, vmr_eps[i].vmr_ep_name);
			return size;
		}
	}
	return count;
}

/*
 * shared memory is reserved (a semaphore is hold on host driver) for this op,
 * log_msg will not be touched by other op as well.
 */
static int vmr_verbose_info(cl_msg_t *msg)
{
	u32 safe_size = sizeof(log_msg);
	u32 dst_addr = RPU_SHARED_MEMORY_ADDR(msg->log_payload.address);
	u32 count = 0;
	u32 total_count = 0;

	count = cl_rpu_status_query(msg, log_msg, safe_size);
	if (msg->log_payload.size < total_count + count) {
		RMGMT_ERR("log buffer %d is too small, log message %s is truncated",
			msg->log_payload.size, log_msg);
		goto done;
	}		
	cl_memcpy_toio8(dst_addr + total_count, &log_msg, count);
	total_count += count;

	count = cl_apu_status_query(msg, log_msg, safe_size);
	if (msg->log_payload.size < total_count + count) {
		RMGMT_ERR("log buffer %d is too small, log message %s is truncated",
			msg->log_payload.size, log_msg);
		goto done;
	}
	cl_memcpy_toio8(dst_addr + total_count, &log_msg, count);
	total_count += count;

	count = rmgmt_fpt_status_query(msg, log_msg, safe_size);
	if (msg->log_payload.size < total_count + count) {
		RMGMT_ERR("log buffer %d is too small, log message %s is truncated",
			msg->log_payload.size, log_msg);
		goto done;
	}
	cl_memcpy_toio8(dst_addr + total_count, &log_msg, count);
	total_count += count;

done:
	/* set correct size in result payload */
	msg->log_payload.size = total_count;

	return 0;
}

static int vmr_endpoint_info(cl_msg_t *msg)
{
	u32 safe_size = sizeof(log_msg);
	u32 dst_addr = RPU_SHARED_MEMORY_ADDR(msg->log_payload.address);
	u32 count = 0;
	u32 total_count = 0;

	count = rmgmt_endpoints_query(msg, log_msg, safe_size);
	if (msg->log_payload.size < total_count + count) {
		RMGMT_ERR("log buffer %d is too small, log message %s is truncated",
			msg->log_payload.size, log_msg);
		goto done;
	}
	cl_memcpy_toio8(dst_addr + total_count, &log_msg, count);
	total_count += count;

done:
	/* set correct size in result payload */
	msg->log_payload.size = total_count;

	return 0;
}

static inline u32 read_firewall()
{
	return IO_SYNC_READ32(VMR_EP_FIREWALL_USER_BASE + FAULT_STATUS);
}

static inline void write_firewall_unblock(u32 val)
{
	return IO_SYNC_WRITE32(val, VMR_EP_FIREWALL_USER_BASE + UNBLOCK_CTRL);
}

static inline u32 check_firewall()
{
	return IS_FIRED(read_firewall());
}

int cl_xgq_pl_is_ready()
{
	return !check_firewall();
}

static int vmr_clear_firewall()
{
	int i = 0;

	if (!check_firewall()) {
		RMGMT_LOG("done");
		return 0;
	}
	
	for (i = 0; i < FIREWALL_RETRY_COUNT; i++) {
		if (read_firewall() & FIREWALL_STATUS_BUSY) {
			vTaskDelay(pdMS_TO_TICKS(BUSY_RETRY_INTERVAL));
			continue;
		} else {
			break;
		}
	}

	if (read_firewall() & FIREWALL_STATUS_BUSY) {
		RMGMT_ERR("firewall is busy, status: 0x%x", read_firewall());
		return -EINVAL;
	}

	/* now clear firewall */
	write_firewall_unblock(1);

	for (i = 0; i < FIREWALL_RETRY_COUNT; i++) {
		if (!check_firewall())
			return 0;

		vTaskDelay(pdMS_TO_TICKS(CLEAR_RETRY_INTERVAL));
	}

	RMGMT_ERR("failed clear firewall, status: 0x%x", read_firewall());
	return -EINVAL;
}

static u32 vmr_check_firewall(cl_msg_t *msg)
{
	u32 val = check_firewall();

	/*
	 * copy messge back from log_msg to shared memory
	 */
	if (val) {
		u32 safe_size = sizeof(log_msg);
		u32 dst_addr = RPU_SHARED_MEMORY_ADDR(msg->log_payload.address);
		u32 count = 0;
		
		RMGMT_ERR("tripped status: 0x%x", val);

		if (msg->log_payload.size < sizeof(log_msg)) {
			RMGMT_ERR("log buffer %d is too small, log message %d is trunked",
				msg->log_payload.size, sizeof(log_msg));
			safe_size = msg->log_payload.size;
		}

		count = snprintf(log_msg, safe_size,
			"AXI Firewall User is tripped, status: 0x%lx\n", val);
		cl_memcpy_toio8(dst_addr, &log_msg, safe_size);

		/* set correct size in result payload */
		msg->log_payload.size = count;
	}

	return val;
}

static int vmr_load_firmware(cl_msg_t *msg)
{
	u32 dst_addr = 0;
	u32 src_addr = 0;
	u32 fw_offset = 0;
	u32 fw_size = 0;
	int ret = 0;

	if (rmgmt_fpt_get_xsabin(msg, &fw_offset, &fw_size)) {
		RMGMT_ERR("get xsabin firmware failed");
		return -1;
	}

	ret = validate_log_payload(&msg->log_payload, fw_size);
	if (ret)
		return ret;

	dst_addr = RPU_SHARED_MEMORY_ADDR(msg->log_payload.address);
	src_addr = fw_offset;

	/*
	 * TODO: the dst size can be small, check size <= dst size and
	 * copy small truck back based on offset + dst size in the future */
	cl_memcpy(dst_addr, src_addr, fw_size);

	/* remaining cound is actually size for now,
	 * it can be entire size - offset in the future*/
	msg->log_payload.size = fw_size;

	return 0;
}

static int xgq_log_page_cb(cl_msg_t *msg, void *arg)
{
	int ret = 0;

	switch (msg->log_payload.pid) {
	case CL_LOG_AF_CHECK:
		ret = vmr_check_firewall(msg);
		break;
	case CL_LOG_AF_CLEAR:
		ret = vmr_clear_firewall();
		break;
	case CL_LOG_FW:
		ret = vmr_load_firmware(msg);
		break;
	case CL_LOG_INFO:
		ret = vmr_verbose_info(msg);
		break;
	case CL_LOG_ENDPOINT:
		ret = vmr_endpoint_info(msg);
		break;
	default:
		RMGMT_LOG("unsupported type %d", msg->log_payload.pid);
		ret = -EINVAL;
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
	u32 pmc_intr = VMR_EP_PMC_REG;

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
	u32 pmc_intr = VMR_EP_PMC_REG;

	IO_SYNC_WRITE32(PMC_POR_ENABLE_BIT, pmc_intr + PMC_REG_ACTION);
	IO_SYNC_WRITE32(PMC_POR_ENABLE_BIT, pmc_intr + PMC_REG_SRST);
}

static void rmgmt_set_multiboot(u32 offset)
{
	IO_SYNC_WRITE32(offset, VMR_EP_PLM_MULTIBOOT);
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
		break;
	case CL_PROGRAM_SC:
		/* calling into vmc_update APIs, we check progress separately */
		if (acquire_download_sema()) {
			RMGMT_LOG("start programing SC ...");
			VMC_Start_SC_Update();
			RMGMT_LOG("end programing SC ...");
			release_download_sema();
		} else {
			RMGMT_LOG("system busy, please try later");
			ret = -1;
		}
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

	xSemaDownload = xSemaphoreCreateMutex();
	configASSERT(xSemaDownload != NULL);
	
	/* try to clear any existign uncleared firewall before rmgmt launch */
	vmr_clear_firewall();

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
			/* all rmgmt callbacks are ready, sensor cb can be inited later */
			rmgmt_is_ready = true;
		}

		RMGMT_DBG("free heap %d", xPortGetFreeHeapSize());
	}
	/* should never been here */
	RMGMT_LOG("done");
}

int cl_rmgmt_is_ready()
{
	RMGMT_DBG("%s", rmgmt_is_ready ? "READY" : "NOT READY");
	return rmgmt_is_ready;
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
