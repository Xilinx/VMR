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
#include "xgq_cmd_vmr.h"
#include "cl_xgq_client_plat.h"
#include "vmr_common.h"

#define MSG_ERR(fmt, arg...) \
	CL_ERR(APP_XGQ, fmt, ##arg)
#define MSG_LOG(fmt, arg...) \
	CL_LOG(APP_XGQ, fmt, ##arg)
#define MSG_DBG(fmt, arg...) \
	CL_DBG(APP_XGQ, fmt, ##arg)

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
	cl_memcpy_fromio32(addr, cq_cmd, sizeof(*cq_cmd));

	// write 0 to first word to make sure the cmd state is not NEW 
	xgq_reg_write32(0, addr, 0x0);
}

static bool shm_acquire_data(u32 *addr_off, u32 *size)
{
	if (xSemaphoreTake(semaData, portMAX_DELAY) != pdTRUE)
		return false;

	*addr_off = mem.apu_xgq_ring_buffer + APU_RING_BUFFER_SIZE;
	*size = APU_SHARED_MEMORY_END - APU_SHARED_MEMORY_ADDR(*addr_off) + 1;

	return true;
}

static bool shm_release_data()
{
	return xSemaphoreGive(semaData) == pdTRUE;	
}

static u32 xclbin_cmd_complete(struct xgq_cmd_cq *cq_cmd)
{
	struct xgq_cmd_cq_data_payload *payload =
		(struct xgq_cmd_cq_data_payload *)&cq_cmd->cq_xclbin_payload;

	if (cq_cmd->hdr.cid != APU_MAGIC_ID) {
		MSG_ERR("unknown completion xgq cmd");
		return 0;
	}

	MSG_LOG("count %d", payload->count);
	
	return payload->count;
}

u32 cl_xgq_apu_download_trunk(char *data, u32 trunk_size, u32 remain_size,
	u32 base_off)
{
	int rval = 0;
	uint64_t slot_addr = 0;
	struct xgq_cmd_sq sq_cmd;
	struct xgq_cmd_cq cq_cmd;
	struct xgq_cmd_data_payload *payload = NULL;
	u32 count = 0;

	MSG_LOG("send trunk size %d, remain %d, base %d", trunk_size, remain_size, base_off);

	payload = &sq_cmd.xclbin_payload;
	payload->address = base_off;
	payload->size = trunk_size;
	payload->remain_size = remain_size;
	cl_memcpy_toio8(APU_SHARED_MEMORY_ADDR(base_off), data, trunk_size);

	sq_cmd.hdr.opcode = XGQ_CMD_OP_LOAD_XCLBIN;
	sq_cmd.hdr.state = XGQ_SQ_CMD_NEW;
	sq_cmd.hdr.count = sizeof(*payload);
	sq_cmd.hdr.cid = APU_MAGIC_ID;

	rval = xgq_produce(&apu_xgq, &slot_addr);
	if (rval) {
		MSG_ERR("xgq_produce failed %d", rval);
		return 0;
	}

	MSG_LOG("send to peer");
	cl_memcpy_toio32((u32)slot_addr, &sq_cmd, sizeof(sq_cmd));
	xgq_notify_peer_produced(&apu_xgq);

	for (int i = 0; i < APU_XGQ_TIMEOUT; i++) {
		vTaskDelay(xBlockTime);

		rval = xgq_consume(&apu_xgq, &slot_addr);
		if (rval)
			continue;

		read_completion(&cq_cmd, slot_addr);
		count = xclbin_cmd_complete(&cq_cmd);
		xgq_notify_peer_consumed(&apu_xgq);

		MSG_LOG("doen count %d", count);
		return count;
	}

	MSG_ERR("request timeout, please retry");
	return 0;
}

int cl_xgq_apu_download_xclbin(char *data, u32 size)
{
	u32 base_off = 0;
	u32 data_size = 0;
	u32 remain_size = size;
	u32 offset = 0;
	u32 count = 0;

	if (!cl_xgq_apu_is_ready())
		return -ENODEV;

	if (!shm_acquire_data(&base_off, &data_size))
		return -EBUSY;

	MSG_LOG("base off 0x%x, size %d", base_off, size);
	for (int i = 0; i < 8; i++)
		MSG_LOG("%x", data[i]);
	for (int i = size - 8; i < size; i++)
		MSG_LOG("%x", data[i]);

	/* set to 1M for test only */
	data_size = 0x100000;

	while (remain_size > 0) {
		u32 trunk_size = remain_size > data_size ? data_size : remain_size;
		remain_size = remain_size - trunk_size;
		
		count = cl_xgq_apu_download_trunk(data + offset, trunk_size,
			remain_size, base_off);
		if (count != trunk_size) {
			MSG_ERR("failed count %d != trunk_size %d", count, trunk_size);
			shm_release_data();
			return -EIO;
		}

		offset += trunk_size;
	}

	shm_release_data();

	MSG_LOG("return 0");
	return 0;
}

static void identify_cmd_complete(struct xgq_cmd_cq *cq_cmd)
{
	struct xgq_cmd_resp_identify *id_cmd = (struct xgq_cmd_resp_identify *)cq_cmd;

	if (cq_cmd->hdr.cid != APU_MAGIC_ID) {
		MSG_ERR("unknown completion xgq cmd");
		return;
	}

	MSG_LOG("major.minor %d.%d", id_cmd->major, id_cmd->minor);
}

int cl_xgq_apu_identify(struct cl_msg *msg)
{
	int rval = 0;
	uint64_t slot_addr = 0;
	struct xgq_cmd_sq sq_cmd;
	struct xgq_cmd_cq cq_cmd;

	if (!cl_xgq_apu_is_ready())
		return -ENODEV;

	sq_cmd.hdr.opcode = XGQ_CMD_OP_IDENTIFY;
	sq_cmd.hdr.state = XGQ_SQ_CMD_NEW;
	sq_cmd.hdr.count = 0; /* no payload for identify */
	sq_cmd.hdr.cid = APU_MAGIC_ID;

	rval = xgq_produce(&apu_xgq, &slot_addr);
	if (rval) {
		MSG_ERR("xgq_produce failed %d", rval);
		return rval;
	}

	MSG_LOG("send to peer");
	cl_memcpy_toio32((u32)slot_addr, &sq_cmd, sizeof(sq_cmd));
	xgq_notify_peer_produced(&apu_xgq);

	for (int i = 0; i < APU_XGQ_TIMEOUT; i++) {
		vTaskDelay(xBlockTime);

		rval = xgq_consume(&apu_xgq, &slot_addr);
		if (rval)
			continue;

		read_completion(&cq_cmd, slot_addr);
		identify_cmd_complete(&cq_cmd);
		xgq_notify_peer_consumed(&apu_xgq);

		return 0;
	}

	MSG_ERR("identify request timeout, please retry");
	return -1;
}

int cl_xgq_client_probe()
{
	uint64_t flags = 0;
	int ret = 0;

	if (cl_xgq_apu_is_ready()) {
		MSG_LOG("dup probe, skip");
		return -1;
	}

	ret = cl_memcpy_fromio32(APU_SHARED_MEMORY_START, &mem, sizeof(mem));
	if (ret == -1) {
		MSG_ERR("read APU shared memory partition table failed");
		return -1;
	}
	if (mem.apu_channel_ready == 0) {
		MSG_DBG("apu channel is not ready yet");
		return -1;
	}

	semaData = xSemaphoreCreateMutex();
	if (semaData == NULL) {
		MSG_ERR("no more memory for semaData creation");
		return -1;
	}

	MSG_DBG("mem %x %x", mem.apu_channel_ready, mem.apu_xgq_ring_buffer);

	/* APU channel is ready, attaching xgq */
	ret = xgq_attach(&apu_xgq, flags, 0, APU_SHARED_MEMORY_ADDR(mem.apu_xgq_ring_buffer),
		APU_SQ_BASE, APU_CQ_BASE);
	if (ret != 0) {
		MSG_ERR("xgq_attach failed: %d, please reset device", ret);
		return -1;
	}

	xgq_apu_ready = true;

	MSG_LOG("xgq_attach is done");
	return 0;
}

int cl_xgq_apu_is_ready()
{
	MSG_DBG("%s", xgq_apu_ready ? "READY" : "NOT READY");
	return xgq_apu_ready;
}
