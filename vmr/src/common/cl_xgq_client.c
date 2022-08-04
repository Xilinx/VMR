/******************************************************************************
* Copyright (C) 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/
/* FreeRTOS includes */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
#include "semphr.h"

#include <stdbool.h>

#include "cl_log.h"
#include "cl_msg.h"
#include "cl_main.h"
#include "cl_rmgmt.h"
#include "xgq_cmd_vmr.h"
#include "cl_xgq_client_plat.h"
#include "vmr_common.h"

#define APU_MAGIC_ID	0x4150 /* AP uint16_t */
#define APU_XGQ_TIMEOUT	15 /* seconds */

struct apu_shared_mem {
	u32	apu_channel_ready;
	u32	apu_xgq_ring_buffer;
};

static SemaphoreHandle_t semaData;
static const TickType_t xBlockTime = pdMS_TO_TICKS(1000);
static struct apu_shared_mem mem = { 0 };
static bool xgq_apu_ready = false;
static struct xgq apu_xgq;

static void read_completion(struct xgq_cmd_cq *cq_cmd, uint64_t addr)
{
	cl_memcpy_fromio(addr, cq_cmd, sizeof(*cq_cmd));

	// write 0 to first word to make sure the cmd state is not NEW 
	xgq_reg_write32(0, addr, 0x0);
}

static bool shm_acquire_data(u32 *addr_off, u32 *size)
{
	if (xSemaphoreTake(semaData, portMAX_DELAY) != pdTRUE) {
		VMR_ERR("semaData lock is busy");
		return false;
	}

	*addr_off = mem.apu_xgq_ring_buffer + APU_RING_BUFFER_SIZE;
	*size = VMR_EP_APU_SHARED_MEMORY_END - APU_SHARED_MEMORY_ADDR(*addr_off) + 1;

	return true;
}

static bool shm_release_data()
{
	return xSemaphoreGive(semaData) == pdTRUE;	
}

static inline int cq_cmd_complete_check(struct xgq_cmd_cq *cq_cmd)
{
	if (cq_cmd->hdr.cid != APU_MAGIC_ID) {
		VMR_ERR("unknown completion xgq cmd");
		return -EINVAL;
	}

	if (cq_cmd->rcode != 0) {
		VMR_ERR("rcode: %d", cq_cmd->rcode);
		return -EIO;
	}

	return 0;
}

static u32 xclbin_cmd_complete(struct xgq_cmd_cq *cq_cmd)
{
	struct xgq_cmd_cq_data_payload *payload =
		(struct xgq_cmd_cq_data_payload *)&cq_cmd->cq_xclbin_payload;

	if (cq_cmd_complete_check(cq_cmd))
		return 0;

	VMR_LOG("count %d", payload->count);
	
	return payload->count;
}

u32 cl_xgq_apu_download_trunk(char *data, u32 trunk_size, u32 remain_size,
	u32 base_off, void *priv)
{
	int rval = 0;
	uint64_t slot_addr = 0;
	struct xgq_cmd_sq sq_cmd;
	struct xgq_cmd_cq cq_cmd;
	struct xgq_cmd_data_payload *payload = NULL;
	u32 count = 0;

	if (!cl_rmgmt_apu_is_ready()) {
		VMR_ERR("apu is not ready.");
		return -ENODEV;
	}

	payload = &sq_cmd.xclbin_payload;
	payload->address = base_off;
	payload->size = trunk_size;
	payload->remain_size = remain_size;
	payload->priv = priv ? *(uint64_t *)priv : 0;

	VMR_LOG("send trunk size %d, remain %d, base %d, priv %llu",
		trunk_size, remain_size, base_off, payload->priv);

	cl_memcpy_toio(APU_SHARED_MEMORY_ADDR(base_off), data, trunk_size);

	sq_cmd.hdr.opcode = XGQ_CMD_OP_LOAD_XCLBIN;
	sq_cmd.hdr.state = XGQ_SQ_CMD_NEW;
	sq_cmd.hdr.count = sizeof(*payload);
	sq_cmd.hdr.cid = APU_MAGIC_ID;

	rval = xgq_produce(&apu_xgq, &slot_addr);
	if (rval) {
		VMR_ERR("xgq_produce failed %d", rval);
		return 0;
	}

	VMR_LOG("send to peer");
	cl_memcpy_toio((u32)slot_addr, &sq_cmd, sizeof(sq_cmd));
	xgq_notify_peer_produced(&apu_xgq);

	for (int i = 0; i < APU_XGQ_TIMEOUT; i++) {
		vTaskDelay(xBlockTime);

		rval = xgq_consume(&apu_xgq, &slot_addr);
		if (rval)
			continue;

		read_completion(&cq_cmd, slot_addr);
		count = xclbin_cmd_complete(&cq_cmd);
		xgq_notify_peer_consumed(&apu_xgq);

		VMR_LOG("done count %d", count);
		return count;
	}

	VMR_ERR("request timeout, please retry");
	return 0;
}

int rmgmt_apu_download_xclbin(struct rmgmt_handler *rh)
{
	u32 base_off = 0;
	u32 data_size = 0;
	u32 remain_size = 0;
	u32 offset = 0;
	u32 count = 0;
	char *data = NULL;
	u32 size = 0;
	void *priv = NULL;

	if (!cl_rmgmt_apu_is_ready())
		return -ENODEV;

	data = (char *)rh->rh_data;
	size = rh->rh_data_size;
	priv = rh->rh_data_priv;
	remain_size = size;
	if (data == NULL || size == 0) {
		VMR_ERR("invalid request, data is NULL or size %d is 0", size);
		return -EINVAL;
	}

	if (!shm_acquire_data(&base_off, &data_size))
		return -EBUSY;

	VMR_LOG("base 0x%x, data size %d, total size %d", base_off, data_size, size);

	/* set to 1M for test only */
	//data_size = 0x100000;

	while (remain_size > 0) {
		u32 trunk_size = remain_size > data_size ? data_size : remain_size;
		remain_size = remain_size - trunk_size;
		
		count = cl_xgq_apu_download_trunk(data + offset, trunk_size,
			remain_size, base_off, priv);
		if (count != trunk_size) {
			VMR_ERR("failed count %d != trunk_size %d", count, trunk_size);
			shm_release_data();
			return -EIO;
		}

		offset += trunk_size;
	}

	shm_release_data();

	VMR_LOG("return 0");
	return 0;
}

static int identify_cmd_complete(struct xgq_cmd_cq *cq_cmd, struct xgq_vmr_cmd_identify *id_cmd)
{
	struct xgq_cmd_resp_identify *resp_id = (struct xgq_cmd_resp_identify *)cq_cmd;
	int ret = 0;

	ret = cq_cmd_complete_check(cq_cmd);
	if (ret)
		return ret;

	id_cmd->major = resp_id->major;
	id_cmd->minor = resp_id->minor;

	VMR_LOG("major.minor %d.%d", resp_id->major, resp_id->minor);

	return 0;
}

int rmgmt_apu_identify(struct xgq_vmr_cmd_identify *id_cmd)
{
	int rval = 0;
	uint64_t slot_addr = 0;
	struct xgq_cmd_sq sq_cmd;
	struct xgq_cmd_cq cq_cmd;

	if (!cl_rmgmt_apu_is_ready())
		return -ENODEV;

	sq_cmd.hdr.opcode = XGQ_CMD_OP_IDENTIFY;
	sq_cmd.hdr.state = XGQ_SQ_CMD_NEW;
	sq_cmd.hdr.count = 0; /* no payload for identify */
	sq_cmd.hdr.cid = APU_MAGIC_ID;

	rval = xgq_produce(&apu_xgq, &slot_addr);
	if (rval) {
		VMR_ERR("xgq_produce failed %d", rval);
		return rval;
	}

	VMR_LOG("send to peer");
	cl_memcpy_toio((u32)slot_addr, &sq_cmd, sizeof(sq_cmd));
	xgq_notify_peer_produced(&apu_xgq);

	for (int i = 0; i < APU_XGQ_TIMEOUT; i++) {
		vTaskDelay(xBlockTime);

		rval = xgq_consume(&apu_xgq, &slot_addr);
		if (rval)
			continue;

		read_completion(&cq_cmd, slot_addr);
		rval = identify_cmd_complete(&cq_cmd, id_cmd);
		xgq_notify_peer_consumed(&apu_xgq);

		return rval;
	}

	VMR_ERR("identify request timeout, please retry");
	return -EBUSY;
}

static int log_page_cmd_complete(struct xgq_cmd_cq *cq_cmd, u32 address, u32 log_size,
	char *buf, u32 buf_size)
{
	struct xgq_cmd_cq_log_page_payload *payload =
		(struct xgq_cmd_cq_log_page_payload *)&cq_cmd->cq_log_payload;
	u32 size = 0;

	if (cq_cmd_complete_check(cq_cmd))
		return 0;

	size = payload->count;
	if (size > log_size) {
		VMR_WARN("return size %d is larger than log page size %d",
			size, log_size);
		/* only accept data  within log_size */
		size = log_size;
	}
	if (size > buf_size) {
		VMR_WARN("return size %d is larger than buffer size %d",
			size, buf_size);
		/* only accept data within buf_size */
		size = buf_size;
	}

	return (cl_memcpy_fromio(APU_SHARED_MEMORY_ADDR(address), buf, size) != size) ? 0 : size;
}

int rmgmt_apu_info(char *buf, u32 size)
{
	int rval = 0;
	uint64_t slot_addr = 0;
	struct xgq_cmd_sq sq_cmd = { 0 };
	struct xgq_cmd_cq cq_cmd = { 0 };
	struct xgq_cmd_log_payload *payload = NULL;
	u32 log_address = 0;
	u32 log_size = 0;

	if (!cl_rmgmt_apu_is_ready()) {
		VMR_WARN("apu is not ready.");
		return 0;
	}

	if (!shm_acquire_data(&log_address, &log_size)) {
		VMR_WARN("shared memory is busy");
		return 0;
	}

	payload = &sq_cmd.log_payload;
	payload->address = log_address;
	payload->size = log_size;
	payload->offset = 0;
	payload->pid = XGQ_CMD_LOG_INFO;

	sq_cmd.hdr.opcode = XGQ_CMD_OP_GET_LOG_PAGE;
	sq_cmd.hdr.state = XGQ_SQ_CMD_NEW;
	sq_cmd.hdr.count = sizeof(*payload);
	sq_cmd.hdr.cid = APU_MAGIC_ID;

	rval = xgq_produce(&apu_xgq, &slot_addr);
	if (rval) {
		VMR_ERR("xgq_produce failed %d", rval);
		rval = 0;
		goto out;
	}

	VMR_LOG("send to peer");
	cl_memcpy_toio((u32)slot_addr, &sq_cmd, sizeof(sq_cmd));
	xgq_notify_peer_produced(&apu_xgq);

	for (int i = 0; i < APU_XGQ_TIMEOUT; i++) {
		vTaskDelay(xBlockTime);

		rval = xgq_consume(&apu_xgq, &slot_addr);
		if (rval)
			continue;

		read_completion(&cq_cmd, slot_addr);
		rval = log_page_cmd_complete(&cq_cmd, log_address, log_size, buf, size);
		xgq_notify_peer_consumed(&apu_xgq);

		VMR_LOG("done rval %d", rval);
		goto out;
	}

	VMR_WARN("apu is busy");
out:
	shm_release_data();

	return rval;
}

int cl_xgq_client_fini()
{
	/* reset the APU status back to init status */
	IO_SYNC_WRITE32(0, VMR_EP_APU_SHARED_MEMORY_START);
	return 0;
}

int cl_rmgmt_apu_channel_probe()
{
	uint64_t flags = 0;
	int ret = 0;
	struct xgq_vmr_cmd_identify id_cmd = { 0 };
	
	if (cl_rmgmt_apu_is_ready()) {
		VMR_WARN("dup probe, skip");
		return -ENODEV;
	}

	ret = cl_memcpy_fromio(VMR_EP_APU_SHARED_MEMORY_START, &mem, sizeof(mem));
	if (ret == -1) {
		VMR_ERR("read APU shared memory partition table failed");
		return -ENODEV;
	}
	if (mem.apu_channel_ready == 0) {
		VMR_DBG("apu channel is not ready yet");
		return -ENODEV;
	}

	semaData = xSemaphoreCreateMutex();
	if (semaData == NULL) {
		VMR_ERR("no more memory for semaData creation");
		return -ENOMEM;
	}

	VMR_WARN("APU up:%d, ring buffer off:0x%x", mem.apu_channel_ready, mem.apu_xgq_ring_buffer);

	/* APU channel is ready, attaching xgq */
	ret = xgq_attach(&apu_xgq, flags, 0, APU_SHARED_MEMORY_ADDR(mem.apu_xgq_ring_buffer),
		VMR_EP_APU_SQ_BASE, VMR_EP_APU_CQ_BASE);
	if (ret != 0) {
		VMR_ERR("xgq_attach failed: %d, please reset device", ret);
		ret = -EINVAL;
		goto failure;
	}

	/*
	 * After xgq_attach, we mark apu_ready to true.
	 * If identify or version is not supported, we should stop
	 * continue probing APU again
	 */
	xgq_apu_ready = true;

	ret = rmgmt_apu_identify(&id_cmd);
	if (ret != 0) {
		VMR_ERR("xgq identify failed: %d, please reset device", ret);
		ret = -EINVAL;
		goto failure;
	}
	if (id_cmd.major != 1 && id_cmd.minor != 0) {
		VMR_WARN("unsupported xgq major.minor %d.%d", id_cmd.major, id_cmd.minor);
		ret = -EINVAL;
		goto failure;
	}

	VMR_WARN("APU is ready.");
	return ret;
failure:
	vSemaphoreDelete(semaData);
	xgq_apu_ready = false;

	return ret;
}

int cl_rmgmt_apu_is_ready()
{
	VMR_DBG("%s", xgq_apu_ready ? "READY" : "NOT READY");
	return xgq_apu_ready;
}
