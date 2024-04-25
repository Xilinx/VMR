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
#include "rmgmt_pm.h"
#include "rmgmt_ipi.h"

#include "cl_msg.h"
#include "cl_flash.h"
#include "cl_main.h"
#include "cl_rmgmt.h"
#include "cl_vmc.h"

/* rmgmt specific macros */
/* Firewall IPs */
/* version 1.0 registers */
#define FAULT_STATUS            		0x0
#define UNBLOCK_CTRL            		0x8
#define IP_VERSION				0x10
#define MAX_CONTINUOUS_RTRANSFERS_WAITS		0x30

/* version >= 1.1 registers */
#define SI_FAULT_STATUS                         0x100
#define SI_SOFT_CTRL                            0x104
#define SI_UNBLOCK_CTRL                         0x108
#define MAX_CONTINUOUS_WTRANSFERS_WAITS         0x130
#define ARADDR_LO				0x210
#define ARADDR_HI				0x214
#define AWADDR_LO				0x218
#define AWADDR_HI				0x21c
#define ARUSER					0x220
#define AWUSER					0x224

#define READ_RESPONSE_BUSY      BIT(0)
#define WRITE_RESPONSE_BUSY     BIT(16)
#define FIREWALL_STATUS_BUSY    (READ_RESPONSE_BUSY | WRITE_RESPONSE_BUSY)
#define IS_FIRED(val) 		(val & ~FIREWALL_STATUS_BUSY)
#define FIREWALL_RETRY_COUNT	10
#define	BUSY_RETRY_INTERVAL	100		/* ms */
#define	CLEAR_RETRY_INTERVAL	2		/* ms */

#define IP_VER_10		0
#define IP_VER_11		1

#define AF_READ32(base_addr, reg)				\
	IO_SYNC_READ32(base_addr + reg)
#define AF_WRITE32(base_addr, reg, val)				\
	IO_SYNC_WRITE32(val, base_addr + reg)

#define READ_ARADDR(base_addr) (((u64)(AF_READ32(base_addr, ARADDR_HI))) << 32 |	\
		(AF_READ32(base_addr, ARADDR_LO)))
#define READ_AWADDR(base_addr) (((u64)(AF_READ32(base_addr, AWADDR_HI))) << 32 |	\
		(AF_READ32(base_addr, AWADDR_LO)))
#define READ_ARUSER(base_addr) (AF_READ32(base_addr, ARUSER))
#define READ_AWUSER(base_addr) (AF_READ32(base_addr, AWUSER))

/* PMC IPs for host reset */
#define	PMC_ERR1_STATUS_MASK	(1 << 24)
#define	PMC_ERR_OUT1_EN_MASK	(1 << 24)
#define	PMC_POR1_EN_MASK	(1 << 24)
#define	PMC_POR_ENABLE_BIT	(1 << 24)
#define	PMC_REG_ERR_OUT1_MASK	0x20
#define	PMC_REG_ERR_OUT1_EN	0x24
#define	PMC_REG_POR1_MASK	0x40
#define	PMC_REG_POR1_EN		0x44
#define	PMC_REG_ACTION		0x48
#define	PMC_REG_SRST		0x84
#define	PL_TO_PMC_ERROR_SIGNAL_PATH_MASK	(1 << 0)


static struct rmgmt_handler rh = { 0 };
static bool rmgmt_is_ready_flag = false;

enum pdi_type {
	BASE_PDI = 0,
	APU_PDI,
	PL_XCLBIN,
	VMR_PDI,
};

static struct vmr_endpoints vmr_eps[] = {
	{"STDIN_BASEADDRESS", STDIN_BASEADDRESS},
	{"STDOUT_BASEADDRESS", STDOUT_BASEADDRESS},
	{"VMR_EP_SYSTEM_DTB", VMR_EP_SYSTEM_DTB},
	{"VMR_EP_PR_ISOLATION", VMR_EP_PR_ISOLATION},
	{"VMR_EP_UCS_SHUTDOWN", VMR_EP_UCS_SHUTDOWN},
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

static struct vmr_endpoints vmr_afs[] = {
	{"VMR_EP_FIREWALL_USER_BASE", VMR_EP_FIREWALL_USER_BASE},
	/* Comment out for test only now, we will have more firewall levels in the future
	{"VMR_EP_FIREWALL_TEMP_BASE", 0x80002000},
	*/
};

/*
 * Block comment for validation functions:
 * We should always keep VMR running, any incoming request should be validated
 * in acceptable range, otherwise return -EINVAL.
 */
static int validate_clock_payload(struct xgq_vmr_clock_payload *payload)
{
	int ret = -EINVAL;

	if(!cl_rmgmt_pl_is_ready()) {
		VMR_ERR("pl is not ready.");
		return -EIO;
	}


	if (payload->ocl_region != 0) {
		VMR_ERR("ocl region %d is not 0", payload->ocl_region);
		return ret;
	}

	if (payload->ocl_req_type > CL_CLOCK_SCALE_INTERNAL) {
		VMR_ERR("invalid req_type %d", payload->ocl_req_type);
		return ret;
	}

	if (payload->ocl_req_id > OCL_MAX_ID) {
		VMR_ERR("invalid req_id %d", payload->ocl_req_id);
		return ret;
	}

	if (payload->ocl_req_num > OCL_MAX_NUM) {
		VMR_ERR("invalid req_num %d", payload->ocl_req_num);
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

	/* check address is 32bit aligned */
	if (address & 0x3) {
		VMR_ERR("address 0x%x is not 32bit aligned", address);
		return -EINVAL;
	}

	if (address + size >= VMR_EP_RPU_SHARED_MEMORY_END) {
		VMR_ERR("address overflow 0x%x", address);
		return ret;
	}

	if (size > rh.rh_data_max_size) {
		VMR_ERR("size %d is over max %d", size, rh.rh_data_max_size);
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
		VMR_ERR("address overflow 0x%x", address);
		return ret;
	}

	if (payload->pid >= CL_LOG_MAX_TYPE) {
		VMR_ERR("invalid log type 0x%x", payload->pid);
		return ret;
	}

	if (payload->size < size) {
		VMR_ERR("request size 0x%x is smaller than result size 0x%x",
			payload->size, size);
		return ret;
	}
	/* TODO: add more checking based on addr_type in the future */

	return 0;
}

int cl_rmgmt_clock(cl_msg_t *msg)
{
	int ret = 0;
	uint32_t freq = 0;

	ret = validate_clock_payload(&msg->clock_payload);
	if (ret)
		goto done;

	switch (msg->clock_payload.ocl_req_type) {
	case CL_CLOCK_SCALE:
	case CL_CLOCK_SCALE_INTERNAL:
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
		VMR_ERR("ERROR: unknown req_type %d",
			msg->clock_payload.ocl_req_type);
		ret = -EINVAL;
		break;
	}

done:
	VMR_DBG("complete msg id %d, ret %d", msg->hdr.cid, ret);
	return ret;
}

static int rmgmt_download_vmr(struct rmgmt_handler *rh, cl_msg_t *msg)
{
	rmgmt_ipi_image_store((u32)rh->rh_data_base, rh->rh_data_size);
	return rmgmt_pm_reset_rpu(msg);
}

static int rmgmt_download_pdi(cl_msg_t *msg, enum pdi_type ptype)
{
	int ret = 0;

	ret = validate_data_payload(&msg->data_payload);
	if (ret)
		return ret;

	if (msg->data_payload.remain_size) {
		/* future improvment to copy over data into rh_data_buffer
		cl_memcpy_fromio(address, rh.rh_data, size);
		*/
		VMR_WARN("received unsupported request");
		return -EINVAL;
	}

	/* prepare rmgmt handler */
	rh.rh_data_size = msg->data_payload.size;
	rh.rh_data_priv = &msg->data_payload.priv;
	/* host driver holds this memory till download is completed */ 
	rh.rh_data_base = RPU_SHARED_MEMORY_ADDR(msg->data_payload.address);

	switch (ptype) {
	case BASE_PDI:
		return rmgmt_flash_rpu_pdi(&rh, msg);
	case APU_PDI:
		return rmgmt_download_apu_pdi(&rh);
	case PL_XCLBIN:
		return rmgmt_download_xclbin(&rh);
	case VMR_PDI:
		return rmgmt_download_vmr(&rh, msg);
	default:
		VMR_ERR("cannot handle pdi_type %d", ptype);
		break;
	}

	return -EINVAL;
}

int cl_rmgmt_program_base_pdi(cl_msg_t *msg)
{
	return rmgmt_download_pdi(msg, BASE_PDI);
}

int cl_rmgmt_program_apu_pdi(cl_msg_t *msg)
{
	return rmgmt_download_pdi(msg, APU_PDI);
}

int cl_rmgmt_program_xclbin(cl_msg_t *msg)
{
	return rmgmt_download_pdi(msg, PL_XCLBIN);
}

int cl_rmgmt_program_vmr(cl_msg_t *msg)
{
	return rmgmt_download_pdi(msg, VMR_PDI);
}

static u32 rmgmt_fpt_status_query(cl_msg_t *msg, char *buf, u32 size)
{
	u32 count = 0;
	struct fpt_sc_version version = { 0 };

	rmgmt_fpt_query(msg);

	count = snprintf(buf, size, "default image offset: 0x%lx\n"
		"default image size: 0x%lx\ndefault image capacity: 0x%lx\n",
		msg->multiboot_payload.default_partition_offset,
		msg->multiboot_payload.pdimeta_size,
		msg->multiboot_payload.default_partition_size);
	if (count > size) {
		VMR_WARN("msg is truncated");
		return size;
	}
	
	count += snprintf(buf + count, size - count, "backup image offset: 0x%lx\n"
		"backup image size: 0x%lx\nbackup image capacity: 0x%lx\n",
		msg->multiboot_payload.backup_partition_offset,
		msg->multiboot_payload.pdimeta_backup_size,
		msg->multiboot_payload.backup_partition_size);
	if (count > size) {
		VMR_WARN("msg is truncated");
		return size;
	}

	count += snprintf(buf + count, size - count, "SCFW image size: 0x%lx\n",
		msg->multiboot_payload.scfw_size);
	if (count > size) {
		VMR_WARN("msg is truncated");
		return size;
	}

	(void) cl_vmc_pdi_scfw_version(&version); 
	count += snprintf(buf + count, size - count, "SCFW image version: %d.%d.%d\n",
		version.fsv_major, version.fsv_minor, version.fsv_revision);
	if (count > size) {
		VMR_WARN("msg is truncated");
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
			VMR_WARN("msg is truncated at %d %s", i, vmr_eps[i].vmr_ep_name);
			return size;
		}
	}
	return count;
}

static u32 rmgmt_rpu_status_query(struct cl_msg *msg, char *buf, u32 size)
{
	u32 count = 0;

#ifdef VMR_BUILD_XRT_ONLY
	VMR_LOG("Build flags: -XRT");
	count = snprintf(buf, size, "Build flags: -XRT\n");
#else
	VMR_LOG("Build flags: default full build");
	count = snprintf(buf, size, "Build flags: default full build\n");
#endif

	if (count > size) {
		VMR_ERR("msg is truncated");
		return size;
	}

#ifdef _VMR_VERSION_
	VMR_LOG("vitis version: %s", VMR_TOOL_VERSION);
	VMR_LOG("git hash: %s", VMR_GIT_HASH);
	VMR_LOG("git branch: %s", VMR_GIT_BRANCH);
	VMR_LOG("git hash date: %s", VMR_GIT_HASH_DATE);
	VMR_LOG("vmr build date: %s", VMR_BUILD_VERSION_DATE);
	VMR_LOG("vmr build version: %s", VMR_BUILD_VERSION);

	count += snprintf(buf + count, size - count,
		"vitis version: %s\ngit hash: %s\ngit branch: %s\ngit hash date: %s\n",
		VMR_TOOL_VERSION, VMR_GIT_HASH, VMR_GIT_BRANCH, VMR_GIT_HASH_DATE);
	if (count > size) {
		VMR_ERR("msg is truncated");
		return size;
	}

	count += snprintf(buf + count, size - count,
		"vmr build date: %s\nvmr build version: %s\n",
		VMR_BUILD_VERSION_DATE, VMR_BUILD_VERSION);
	if (count > size) {
		VMR_ERR("msg is truncated");
		return size;
	}
#endif

	return count;
}

static u32 rmgmt_apu_status_query(char *buf, u32 size)
{
	u32 count = 0;
	struct xgq_vmr_cmd_identify id_cmd = { 0 };
	int apu_is_ready = cl_rmgmt_apu_is_ready();

	if (!apu_is_ready)
		goto done;

	if (rmgmt_apu_identify(&id_cmd) == 0){
		count = snprintf(buf, size, "APU XGQ Version: %d.%d\n",
				id_cmd.major,id_cmd.minor);
	}
	if (count > size) {
		VMR_ERR("msg is truncated");
		return size;
	}

	count += rmgmt_apu_info(buf + count, size - count);
	if (count > size) {
		VMR_ERR("msg is truncated");
		return size;
	}

done:
	return count;
}

/*
 * shared memory is reserved (a semaphore is hold on host driver) for this op,
 * rh_log will not be touched by other op as well.
 */
static int vmr_verbose_info(cl_msg_t *msg)
{
	u32 safe_size = rh.rh_log_max_size;
	u32 dst_addr = RPU_SHARED_MEMORY_ADDR(msg->log_payload.address);
	u32 count = 0;
	u32 total_count = 0;

	count = rmgmt_rpu_status_query(msg, rh.rh_log, safe_size);
	if (msg->log_payload.size < total_count + count) {
		VMR_ERR("log buffer %d is too small, log message %s is truncated",
			msg->log_payload.size, rh.rh_log);
		goto done;
	}		
	cl_memcpy_toio(dst_addr + total_count, rh.rh_log, count);
	total_count += count;

	count = rmgmt_apu_status_query(rh.rh_log, safe_size);
	if (msg->log_payload.size < total_count + count) {
		VMR_ERR("log buffer %d is too small, log message %s is truncated",
			msg->log_payload.size, rh.rh_log);
		goto done;
	}
	cl_memcpy_toio(dst_addr + total_count, rh.rh_log, count);
	total_count += count;

	count = rmgmt_fpt_status_query(msg, rh.rh_log, safe_size);
	if (msg->log_payload.size < total_count + count) {
		VMR_ERR("log buffer %d is too small, log message %s is truncated",
			msg->log_payload.size, rh.rh_log);
		goto done;
	}
	cl_memcpy_toio(dst_addr + total_count, rh.rh_log, count);
	total_count += count;

done:
	/* set correct size in result payload */
	msg->log_payload.size = total_count;

	return 0;
}

static int vmr_endpoint_info(cl_msg_t *msg)
{
	u32 safe_size = rh.rh_log_max_size;
	u32 dst_addr = RPU_SHARED_MEMORY_ADDR(msg->log_payload.address);
	u32 count = 0;
	u32 total_count = 0;

	count = rmgmt_endpoints_query(msg, rh.rh_log, safe_size);
	if (msg->log_payload.size < total_count + count) {
		VMR_ERR("log buffer %d is too small, log message %s is truncated",
			msg->log_payload.size, rh.rh_log);
		goto done;
	}
	cl_memcpy_toio(dst_addr + total_count, rh.rh_log, count);
	total_count += count;

done:
	/* set correct size in result payload */
	msg->log_payload.size = total_count;

	return 0;
}

static u32 read_firewall(u32 af_base)
{
	u32 version = 0;
	bool si_mode = false;

	version = IO_SYNC_READ32(af_base + IP_VERSION);
	if (version >= IP_VER_11 &&
		IO_SYNC_READ32(af_base + MAX_CONTINUOUS_WTRANSFERS_WAITS) != 0)
		si_mode = true;

	return si_mode ? IO_SYNC_READ32(af_base + SI_FAULT_STATUS) :
		IO_SYNC_READ32(af_base + FAULT_STATUS);
}

static inline void write_firewall_unblock(u32 val)
{
	return IO_SYNC_WRITE32(val, VMR_EP_FIREWALL_USER_BASE + UNBLOCK_CTRL);
}

static u32 check_firewall()
{
	u32 val = 0;

	for (int i = 0; i < ARRAY_SIZE(vmr_afs); i++) {
		val = IS_FIRED(read_firewall(vmr_afs[i].vmr_ep_address));
		/* Once one level firewall tripped, break and report */
		if (val)
			break;
	}

	return val;
}

int cl_rmgmt_pl_is_ready()
{
	return !check_firewall();
}

static int rmgmt_clear_firewall()
{
	u32 af_base = vmr_afs[0].vmr_ep_address;
	u32 val = 0;
	int i = 0;

	if (!check_firewall()) {
		VMR_LOG("DONE");
		return 0;
	}
	
	for (i  = 0; i < FIREWALL_RETRY_COUNT; i++) {
		val = read_firewall(af_base);
		if (val & FIREWALL_STATUS_BUSY) {
			vTaskDelay(pdMS_TO_TICKS(BUSY_RETRY_INTERVAL));
			continue;
		} else {
			break;
		}
	}

	if (val & FIREWALL_STATUS_BUSY) {
		VMR_ERR("firewall %s(0x%lx) is busy, status: 0x%x",
			vmr_afs[0].vmr_ep_name, af_base, val);
		return -EINVAL;
	}

	/*Note: only work for user firewall now */
	write_firewall_unblock(1);

	for (i = 0; i < FIREWALL_RETRY_COUNT; i++) {
		if (!check_firewall())
			return 0;

		vTaskDelay(pdMS_TO_TICKS(CLEAR_RETRY_INTERVAL));
	}

	VMR_ERR("failed clear firewall, status: 0x%x", read_firewall(af_base));
	return -EINVAL;
}

static u32 rmgmt_check_firewall(cl_msg_t *msg)
{
	u32 val = check_firewall();

	/*
	 * copy messge back from rh_log to shared memory
	 */
	if (val) {
		u32 safe_size = rh.rh_log_max_size;
		u32 dst_addr = RPU_SHARED_MEMORY_ADDR(msg->log_payload.address);
		u32 count = 0;
		
		VMR_ERR("tripped status: 0x%x", val);

		if (msg->log_payload.size < safe_size) {
			VMR_ERR("log buffer %d is too small, log message %d is trunked",
				msg->log_payload.size, rh.rh_log_max_size);
			safe_size = msg->log_payload.size;
		}

		count = snprintf(rh.rh_log, safe_size,
			"AXI Firewall User is tripped, status: 0x%lx. ", val);
		if (count > safe_size) 
			VMR_WARN("log msg is trunked");

		for (int i = 0; i < ARRAY_SIZE(vmr_afs); i++) {
			u32 af_base = vmr_afs[i].vmr_ep_address;
			u32 version = IO_SYNC_READ32(af_base + IP_VERSION);

			count += snprintf(rh.rh_log + count, safe_size - count,
				"Firewall IP Version: %ld, EP: 0x%lx. ",
				version, vmr_afs[i].vmr_ep_address);
			if (count >= safe_size) { 
				VMR_WARN("log msg is trunked");
				break;
			}

			if (version < IP_VER_11)
				continue;

			count += snprintf(rh.rh_log + count, safe_size - count,
				"ARADDR 0x%llx, AWADDR 0x%llx, ARUSER 0x%lx, AWUSER 0x%lx\n",
				READ_ARADDR(af_base), READ_AWADDR(af_base),
				READ_ARUSER(af_base), READ_AWUSER(af_base));
			if (count >= safe_size) { 
				VMR_WARN("log msg is trunked");
				break;
			}
		}

		cl_memcpy_toio(dst_addr, rh.rh_log, MIN(count, safe_size));

		/* set correct size in result payload */
		msg->log_payload.size = MIN(count, safe_size);
	}

	return val;
}

static u32 check_clock_shutdown_status(void)
{
	u32 shutdown_status = 0 ;

	//offset to read shutdown status
	shutdown_status = IO_SYNC_READ32(VMR_EP_GAPPING_DEMAND + VMR_EP_UCS_CHANNEL_2);

	return (shutdown_status & SHUTDOWN_LATCHED_STATUS);
}

static u32 rmgmt_log_clock_shutdown(cl_msg_t *msg)
{
	u32 val = check_clock_shutdown_status();

	/*
	 * copy message back from rh_log to shared memory
	 */
	if (val) {
		u32 safe_size = rh.rh_log_max_size; 
		u32 dst_addr = RPU_SHARED_MEMORY_ADDR(msg->log_payload.address);
		u32 count = 0;

		VMR_ERR("clock shutdown status: 0x%x", val);

		if (msg->log_payload.size < rh.rh_log_max_size) {
			VMR_ERR("log buffer %d is too small, log message will be trunked %d bytes ",
				msg->log_payload.size, rh.rh_log_max_size);
			safe_size = msg->log_payload.size;
		}

		count = snprintf(rh.rh_log, safe_size,
			"Clock shutdown due to power or temperature value reaching Critical threshold, status: 0x%lx\n", val);
		cl_memcpy_toio(dst_addr, rh.rh_log, safe_size);

		/* set correct size in result payload */
		if(safe_size > count)
		{
			msg->log_payload.size = count;
		}
		else
		{
			msg->log_payload.size = safe_size;
		}
	}

	return val;
}

static u8 rmgmt_log_clock_throttling_percentage(cl_msg_t *msg)
{
	u8 val = cl_clk_throttling_enabled_or_disabled();

	/*
	 * copy message back from rh_log to shared memory
	 */
	if (val) {
		u32 safe_size = rh.rh_log_max_size;
		u32 dst_addr = RPU_SHARED_MEMORY_ADDR(msg->log_payload.address);
		u32 count = 0;

		u8 max_gapping_demand_rate = MAX_GAPPING_DEMAND_RATE;
		u32 ep_gapping = IO_SYNC_READ32(VMR_EP_GAPPING_DEMAND);
		/* & with 0xFF to extract on 0-7 bits , these are for gapping demand rate */
		ep_gapping = ep_gapping & 0xFF;

		if (ep_gapping > MAX_GAPPING_DEMAND_RATE)
			ep_gapping = MAX_GAPPING_DEMAND_RATE;


		if (msg->log_payload.size < rh.rh_log_max_size) {
			VMR_ERR("log buffer %d is too small, log message will be trunked %d bytes ",
				msg->log_payload.size, rh.rh_log_max_size);
			safe_size = msg->log_payload.size;
		}
		/* To get % clock throttled = 100 - total clock running %
 		 * total clock running % = (current clock speed / maximum clock speed) * 100 	  
 		 */  
		count = snprintf(rh.rh_log, safe_size,"kernel clocks throttled at %lu%%.\n",MAX_CLOCK_SPEED_PERCENTAGE - ((ep_gapping * CONVERT_TO_PERCENTAGE)/max_gapping_demand_rate));
		cl_memcpy_toio(dst_addr, rh.rh_log, safe_size);

		/* set correct size in result payload */
		msg->log_payload.size = MIN(count, safe_size);
	} else {
		/* explicitly set log_payload.size back to 0 to avoid invalide dmesg  */
		msg->log_payload.size = 0;
	}

	return 0; /* we always return 0 because this is a warning only, no additional action is needed */
}

static int rmgmt_load_firmware(cl_msg_t *msg)
{
	u32 dst_addr = 0;
	u32 src_addr = 0;
	u32 fw_offset = 0;
	u32 fw_size = 0;
	int ret = 0;

	if (rmgmt_fpt_get_xsabin(msg, &fw_offset, &fw_size)) {
		VMR_ERR("get xsabin firmware failed");
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

static int rmgmt_load_fpt(cl_msg_t *msg, fpt_type_t type)
{
	u32 dst_addr = 0;
        int ret = 0;

	char *buf = (char *)pvPortMalloc(FW_COPY_SIZE_64_KB);
	if (!buf) {
		VMR_ERR("malloc failed no memory");
		return -ENOMEM;
	}

	bzero(buf, FW_COPY_SIZE_64_KB);

	if (type >= FPT_MAX_TYPES || type < FPT_TYPE_DEFAULT) {
		VMR_ERR("Invalid FPT type");
		vPortFree(buf);
		return -EINVAL;
	}

	if(type == FPT_TYPE_DEFAULT && rmgmt_fpt_get_data(msg, (u32 *)buf, FPT_TYPE_DEFAULT)) {
                VMR_ERR("get default fpt failed");
		vPortFree(buf);
                return -EINVAL;
        }
	else if (type == FPT_TYPE_BACKUP && rmgmt_fpt_get_data(msg, (u32 *)buf, FPT_TYPE_BACKUP)) {
		VMR_ERR("get backup fpt failed");
		vPortFree(buf);
                return -EINVAL;
	}

        ret = validate_log_payload(&msg->log_payload, FW_COPY_SIZE_64_KB);
        if (ret) {
		vPortFree(buf);
                return ret;
	}

        dst_addr = RPU_SHARED_MEMORY_ADDR(msg->log_payload.address);

        cl_memcpy(dst_addr, (u32)buf, FW_COPY_SIZE_64_KB);

        msg->log_payload.size = FW_COPY_SIZE_64_KB;
	vPortFree(buf);
        return 0;
}

static int rmgmt_load_shell_interface_uuid(cl_msg_t *msg)
{
	u32 dst_addr = 0;
	u32 fw_offset = 0;
	u32 fw_size = 0;
	u32 uuid_size = UUID_BYTES_LEN;
	char uuid[UUID_BYTES_LEN] = { 0 };
	int ret = 0;

	if (rmgmt_fpt_get_xsabin(msg, &fw_offset, &fw_size)) {
		VMR_ERR("get xsabin firmware failed");
		return -EINVAL;
	}

	ret = validate_log_payload(&msg->log_payload, fw_size);
	if (ret)
		return ret;

	ret = rmgmt_fdt_get_uuids(fw_offset, uuid, uuid_size);
	if (ret)
		return ret;

	dst_addr = RPU_SHARED_MEMORY_ADDR(msg->log_payload.address);

	cl_memcpy(dst_addr, (u32)&uuid, uuid_size);

	/* adjust result pay_load size */
	msg->log_payload.size = uuid_size;

	return 0;
}

static int rmgmt_load_system_dtb(cl_msg_t *msg)
{
	u32 dst_addr = 0;
	u32 src_addr = 0;
	u32 fw_offset = 0;
	u32 fw_size = 0;
	int ret = 0;

	if (rmgmt_fpt_get_systemdtb(msg, &fw_offset, &fw_size)) {
		VMR_ERR("get system dtb failed");
		return -1;
	}

	ret = validate_log_payload(&msg->log_payload, fw_size);
	if (ret)
		return ret;

	dst_addr = RPU_SHARED_MEMORY_ADDR(msg->log_payload.address);
	src_addr = fw_offset;

	cl_memcpy(dst_addr, src_addr, fw_size);

	msg->log_payload.size = fw_size;

	return 0;
}

static int rmgmt_sync_plm_data(cl_msg_t *msg)
{
	u32 dst_addr = 0;
	u32 src_addr = 0;
	u32 len = 0;
	int ret = 0;

	ret = validate_log_payload(&msg->log_payload, VMR_PLM_DATA_TOTAL_SIZE);
	if (ret)
		return ret;

	/*
	 * Note: to simplify the handling of offset and request size.
	 *    each time, we return entire plm_log (16k) back.
	 *    host code will handle copying out and avoid duplicate calls
	 *    on the same data.
	 */
	dst_addr = RPU_SHARED_MEMORY_ADDR(msg->log_payload.address);
	src_addr = VMR_PLM_DATA_START_ADDRESS;
	len = MIN(msg->log_payload.size, VMR_PLM_DATA_TOTAL_SIZE);

	VMR_WARN("dst %x src %x len %d", dst_addr, src_addr, len);

	cl_memcpy(dst_addr, src_addr, len);

	/* adjust payload size to MIN size */
	msg->log_payload.size = len;

	return 0;
}

static int rmgmt_load_apu_log(cl_msg_t *msg)
{
	u32 dst_addr = 0;
	u32 size = 0;
	u32 off = 0;
	int ret = 0;

	ret = validate_log_payload(&msg->log_payload, msg->log_payload.size);
	if (ret)
		return ret;

	dst_addr = RPU_SHARED_MEMORY_ADDR(msg->log_payload.address);
	size = msg->log_payload.size;
	off = msg->log_payload.offset;

	VMR_WARN("dst %x size %d off %d", dst_addr, size, off);

	msg->log_payload.size = rmgmt_apu_log((char *)dst_addr, off, size);

	return 0;
}

static char* eTaskStateName(eTaskState st)
{
	switch (st) {
	case eRunning:
		return "Running";
	case eReady:
		return "Ready";
	case eBlocked:
		return "Blocked";
	case eSuspended:
		return "Suspended";
	case eDeleted:
		return "Deleted";
	case eInvalid:
	default:
		break;
	}

	return "Invalid";
}

static int vmr_rtos_task_stats(cl_msg_t *msg)
{
	u32 safe_size = rh.rh_log_max_size;
	u32 dst_addr = RPU_SHARED_MEMORY_ADDR(msg->log_payload.address);
	u32 count = 0;
	TaskStatus_t *pxTaskStatusArray = NULL;
	volatile UBaseType_t uxArraySize, i;
	unsigned long totalruntime, percentage_time;
	
	uxArraySize = uxTaskGetNumberOfTasks();
	pxTaskStatusArray = pvPortMalloc( uxArraySize * sizeof(TaskStatus_t));
	if (pxTaskStatusArray == NULL)
		return -ENOMEM;

	count += snprintf(rh.rh_log + count, safe_size - count,
		"Name\t\tState\tPriority\tStack\tBase\t\tRun Time\n");    

	uxArraySize = uxTaskGetSystemState(pxTaskStatusArray, uxArraySize, &totalruntime);
	totalruntime /= 100UL;
	
	for (i = 0; i < uxArraySize; i++) {
		percentage_time = totalruntime > 0 ?
			pxTaskStatusArray[i].ulRunTimeCounter / totalruntime : 0;

		if(percentage_time > 0UL){
			count += snprintf(rh.rh_log + count, safe_size - count,
			"%-16s%-12s%-4ld\t%ld\t0x%lx\t%lu%%\n",
			pxTaskStatusArray[i].pcTaskName,
			eTaskStateName(pxTaskStatusArray[i].eCurrentState),
			pxTaskStatusArray[i].uxBasePriority,
			pxTaskStatusArray[i].usStackHighWaterMark,
			(u32)pxTaskStatusArray[i].pxStackBase,
			percentage_time);
		}
		else{
			count += snprintf(rh.rh_log + count, safe_size - count,
			"%-16s%-12s%-4ld\t%ld\t0x%lx\t<1%%\n",
			pxTaskStatusArray[i].pcTaskName,
			eTaskStateName(pxTaskStatusArray[i].eCurrentState),
			pxTaskStatusArray[i].uxBasePriority,
			pxTaskStatusArray[i].usStackHighWaterMark,
			(u32)pxTaskStatusArray[i].pxStackBase);
		}

		if (count >= safe_size) {
			VMR_WARN("log msg is trunked or Array Buffer Size Overflow");
			break;
		}
	}
	vPortFree(pxTaskStatusArray);
	cl_memcpy_toio(dst_addr, rh.rh_log, MIN(count, safe_size));
	msg->log_payload.size = MIN(count, safe_size);

	return 0;
}

static int vmr_rtos_mem_stats(cl_msg_t *msg)
{
	u32 safe_size = rh.rh_log_max_size;
	u32 dst_addr = RPU_SHARED_MEMORY_ADDR(msg->log_payload.address);
	u32 count = 0;
	HeapStats_t hp = {0};
	
	vPortGetHeapStats(&hp);

	count += snprintf(rh.rh_log + count, safe_size - count,
		"Available heap size: %d\n"
		"Max size of free blocks in bytes: %d\n"
		"Min size of free blocks in bytes: %d\n"
		"Num of free blocks: %d\n"
		"Min amount of free blocks since system booted: %d\n"
		"Num of successful pvPortMalloc: %d\n"
		"Num of successful pvPortFree: %d\n",
		hp.xAvailableHeapSpaceInBytes,
		hp.xSizeOfLargestFreeBlockInBytes,
		hp.xSizeOfSmallestFreeBlockInBytes,
		hp.xNumberOfFreeBlocks,
		hp.xMinimumEverFreeBytesRemaining,
		hp.xNumberOfSuccessfulAllocations,
		hp.xNumberOfSuccessfulFrees);
	if (count > safe_size) {
		VMR_WARN("log msg is trunked");
	}

	cl_memcpy_toio(dst_addr, rh.rh_log, MIN(count, safe_size));
	msg->log_payload.size = MIN(count, safe_size);

	return 0;
}

static int rmgmt_healthy_check(cl_msg_t *msg)
{
	int ret = 0;

	ret = rmgmt_log_clock_shutdown(msg);
	if(ret)
		return ret;

	ret = rmgmt_check_firewall(msg);
	if(ret)
		return ret;

	return rmgmt_log_clock_throttling_percentage(msg);
}

int cl_rmgmt_log_page(cl_msg_t *msg)
{
	int ret = 0;

	switch (msg->log_payload.pid) {
	case CL_LOG_AF_CHECK:
		ret = rmgmt_healthy_check(msg);
		break;
	case CL_LOG_AF_CLEAR:
		ret = rmgmt_clear_firewall();
		break;
	case CL_LOG_FW:
		ret = rmgmt_load_firmware(msg);
		break;
	case CL_LOG_INFO:
		ret = vmr_verbose_info(msg);
		break;
	case CL_LOG_ENDPOINT:
		ret = vmr_endpoint_info(msg);
		break;
	case CL_LOG_TASK_STATS:
		ret = vmr_rtos_task_stats(msg);
		break;
	case CL_LOG_MEM_STATS:
		ret = vmr_rtos_mem_stats(msg);
		break;
	case CL_LOG_SYSTEM_DTB:
		ret = rmgmt_load_system_dtb(msg);
		break;
	case CL_LOG_PLM_LOG:
		ret = rmgmt_sync_plm_data(msg);
		break;
	case CL_LOG_APU_LOG:
		ret = rmgmt_load_apu_log(msg);
		break;
	case CL_LOG_SHELL_INTERFACE_UUID:
		ret = rmgmt_load_shell_interface_uuid(msg);
		break;
	case CL_LOG_DEFAULT_FPT:
		ret = rmgmt_load_fpt(msg,FPT_TYPE_DEFAULT);
		break;
	case CL_LOG_BACKUP_FPT:
		ret = rmgmt_load_fpt(msg,FPT_TYPE_BACKUP);
		break;
	default:
		VMR_WARN("unsupported type %d", msg->log_payload.pid);
		ret = -EINVAL;
		break;
	}

	VMR_DBG("complete msg id %d, ret %d", msg->hdr.cid, ret);
	return ret;
}

static inline void rmgmt_enable_srst_por()
{
	u32 pmc_intr = VMR_EP_PMC_REG;

	IO_SYNC_WRITE32(PMC_POR_ENABLE_BIT, pmc_intr + PMC_REG_SRST);
}

static inline void rmgmt_set_multiboot(u32 offset)
{
	VMR_WARN("set to: 0x%x", offset);
	IO_SYNC_WRITE32(offset, VMR_EP_PLM_MULTIBOOT);
}

static void rmgmt_set_multiboot_to_default(cl_msg_t *msg)
{
	rmgmt_boot_fpt_query(msg);

	rmgmt_set_multiboot(MULTIBOOT_OFF(msg->multiboot_payload.default_partition_offset));
}

static void rmgmt_set_multiboot_to_backup(cl_msg_t *msg)
{
	rmgmt_boot_fpt_query(msg);

	rmgmt_set_multiboot(MULTIBOOT_OFF(msg->multiboot_payload.backup_partition_offset));
}

int rmgmt_enable_boot_default(cl_msg_t *msg)
{
	int ret = 0;

	rmgmt_enable_srst_por();
	rmgmt_set_multiboot_to_default(msg);

	ret = rmgmt_enable_pl_reset();
	if (ret && ret != -ENODEV) {
		VMR_ERR("request reset failed %d", ret);
		return -1;
	}

	VMR_LOG("DONE");
	return 0;
}

int rmgmt_enable_boot_backup(cl_msg_t *msg)
{
	int ret = 0;

	rmgmt_enable_srst_por();
	rmgmt_set_multiboot_to_backup(msg);

	ret = rmgmt_enable_pl_reset();
	if (ret && ret != -ENODEV) {
		VMR_ERR("request reset failed %d", ret);
		return -1;
	}

	VMR_LOG("DONE");
	return 0;
}

u32 rmgmt_boot_on_offset()
{
	return rh.rh_boot_on_offset;
}

int cl_rmgmt_fpt_query(cl_msg_t *msg)
{
	rmgmt_fpt_query(msg);
	return 0;
}

int rmgmt_fpt_debug(cl_msg_t *msg)
{
	return rmgmt_fpt_set_debug_type(msg);
}

int rmgmt_program_sc(cl_msg_t *msg)
{
	VMR_WARN("Obsolated cmd, ignore");
	return 0;
}

struct xgq_vmr_op {
	int 		op_req_type;
	const char 	*op_name;
	int (*op_handle)(cl_msg_t *msg);
} xgq_vmr_op_table[] = {
	{ CL_MULTIBOOT_DEFAULT, "MULTIBOOT_DEFAULT", rmgmt_enable_boot_default },
	{ CL_MULTIBOOT_BACKUP, "MULTIBOOT_BACKUP", rmgmt_enable_boot_backup },
	{ CL_VMR_QUERY, "VMR_QUERY", cl_rmgmt_fpt_query },
	{ CL_VMR_DEBUG, "VMR_DEBUG", rmgmt_fpt_debug },
	{ CL_PROGRAM_SC, "PROGRAM_SC", rmgmt_program_sc },
	{ CL_VMR_EEMI_SRST, "PMC SRST", rmgmt_eemi_pm_reset },
};

int cl_rmgmt_vmr_control(cl_msg_t *msg)
{
	int ret = 0;

	for (int i = 0; i < ARRAY_SIZE(xgq_vmr_op_table); i++) {
		if (xgq_vmr_op_table[i].op_req_type == msg->multiboot_payload.req_type) {
			ret = xgq_vmr_op_table[i].op_handle(msg);
			goto done;
		}
	}

	VMR_ERR("cannot handle type %d", msg->multiboot_payload.req_type);
	ret = -EINVAL;
done:
	VMR_DBG("msg cid %d, ret %d", msg->hdr.cid, ret);
	return ret;	
}

int rmgmt_is_ready()
{
	return rmgmt_is_ready_flag == true;
}

int cl_rmgmt_init( void )
{	
	int ret = 0;
	cl_msg_t msg = { 0 };
	u32 dtb_offset = 0;
	u32 dtb_size = 0;

	ret = ospi_flash_init();
	if (ret != XST_SUCCESS) {
		VMR_ERR("OSPI is not ready");
		return -EIO;
	}

	if (rmgmt_init_handler(&rh)) {
		VMR_ERR("FATAL: init rmgmt handler failed.");
		return -ENOMEM;
	}

	if (rmgmt_pm_init()) {
		VMR_ERR("WARN: rmgmt_pm init failed.");
		return -ENODEV;
	}

	/* try to clear any existign uncleared firewall before rmgmt launch */
	rmgmt_clear_firewall();

	rh.rh_boot_on_offset = IO_SYNC_READ32(VMR_EP_PLM_MULTIBOOT);
	VMR_WARN("boot on 0x%x", rh.rh_boot_on_offset);

	/* enforce next boot to default image */
	rmgmt_enable_boot_default(&msg);

	/* copy systemdtb to correct location */
	if (rmgmt_fpt_get_systemdtb(&msg, &dtb_offset, &dtb_size) == 0) {
		VMR_WARN("copy system.dtb off 0x%x size %d to 0x%x",
			dtb_offset, dtb_size, VMR_EP_SYSTEM_DTB);
		cl_memcpy(VMR_EP_SYSTEM_DTB, dtb_offset, dtb_size);
	}
	else
	{
        /* system.dtb is not exclusively included as part of FPT for all the platforms */
		VMR_WARN(" skip loading system.dtb at 0x%x ",VMR_EP_SYSTEM_DTB);
	}

	rmgmt_is_ready_flag = true;
	VMR_LOG("DONE. rmgmt is ready.");
	return 0;
}
