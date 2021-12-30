/******************************************************************************
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/
/* FreeRTOS includes */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "cl_log.h"
#include "cl_io.h"
#include "xgq_cmd_vmr.h"
#include "vmr_common.h"

#define MAX_LOG_LEN 80 
#define MAX_BUF_LEN 256

#define CL_MSG_DBG_MAX_SIZE	96
#define CL_MSG_DBG_MAX_RECS	128

struct cl_msg_dbg {
	u32	tick;
	char	msg_dbg_buf[CL_MSG_DBG_MAX_SIZE];
};

/* semaphore lock should be inited by cl_main.c */
extern SemaphoreHandle_t cl_logbuf_lock;

static uint8_t log_level_gl = CL_LOG_LEVEL_DBG;
u32 cl_msg_idx = 0;
struct cl_msg_dbg msg_dbg_log[CL_MSG_DBG_MAX_RECS];

void cl_loglevel_set(uint8_t log_level)
{
	log_level_gl = log_level;
}

uint8_t cl_loglevel_get(void)
{
	return log_level_gl;
}

static void vmr_log_collect(uint32_t msg_index_addr, uint32_t msg_buf_addr,
	char *buf)
{
	struct vmr_log log;
	uint32_t log_idx = IO_SYNC_READ32(msg_index_addr);

	strncpy(log.log_buf, buf, sizeof(log.log_buf));

	/* update log into shared memory */
	cl_memcpy_toio32(msg_buf_addr + sizeof(log) * log_idx, &log, sizeof(log));

	log_idx = (log_idx + 1) % VMR_LOG_MAX_RECS;
	/* update log index into shared memory */
	IO_SYNC_WRITE32(log_idx, msg_index_addr);
}

static void cl_printf_impl(const char *name, uint32_t line, uint8_t log_level,
	const char *app_name, const char *fmt, va_list *argp)
{
	char buf[MAX_LOG_LEN] = { 0 };
	char log_buf[MAX_BUF_LEN] = { 0 };
	struct vmr_shared_mem mem = { 0 };

	/* DBG > LOG > ERR, default is ERR = 0 */
	if (log_level > log_level_gl)
		return;

	if (cl_logbuf_lock == NULL) {
		xil_printf("cl_logbuf_lock semaphore is not inited\r\n");
		return;
	}

	if (xSemaphoreTake(cl_logbuf_lock, portMAX_DELAY) == 0)
		return;
		
	/* assembile log message into log_buf char array */
	vsnprintf(buf, sizeof(buf), fmt, *argp);
	snprintf(log_buf, sizeof(log_buf), "[ %ld ] %s:%s:%ld: %s\r\n",
		xTaskGetTickCount(), app_name, name, line, buf);

	/* print to console */
	xil_printf("%s", log_buf);

	/* store to shared memory ring buffer */
	if (cl_memcpy_fromio32(RPU_SHARED_MEMORY_START, &mem,
		sizeof(mem)) != -1 && mem.vmr_magic_no == VMR_MAGIC_NO) {
		uint32_t index_addr = RPU_SHARED_MEMORY_START +
			offsetof(struct vmr_shared_mem, log_msg_index);
		uint32_t buf_addr = RPU_SHARED_MEMORY_START +
			mem.log_msg_buf_off;

		vmr_log_collect(index_addr, buf_addr, log_buf);
	}

	xSemaphoreGive(cl_logbuf_lock);
}

void cl_printf(const char *name, uint32_t line, uint8_t log_level, 
	const char *app_name, const char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	cl_printf_impl(name, line, log_level, app_name, fmt, &args);
	va_end(args);
}

/* collect log into ring buffer memory */
static void inline cl_msg_log_collect(const char *name, uint32_t line,
	const char *fmt, va_list *argp)
{
	struct cl_msg_dbg *log = &msg_dbg_log[cl_msg_idx];
	char buf[MAX_LOG_LEN] = { 0 };

	log->tick = xTaskGetTickCount();

	vsnprintf(buf, sizeof(buf), fmt, *argp);
	snprintf(log->msg_dbg_buf, CL_MSG_DBG_MAX_SIZE, "[ %ld ] %s:%ld: %s",
		log->tick, name, line, buf);

	cl_msg_idx = (cl_msg_idx + 1) % CL_MSG_DBG_MAX_RECS;
}

/* When serious condition occur, we can dump most recent collected logs */
void cl_log_dump()
{
	u32 idx, elem_idx = cl_msg_idx;

	xil_printf("=== start dumping log ===\r\n");
	for (idx = 0; idx < CL_MSG_DBG_MAX_RECS; idx++) {
		xil_printf("[ %d ] %s\r\n",
			msg_dbg_log[elem_idx].tick,
			msg_dbg_log[elem_idx].msg_dbg_buf);
		elem_idx = (elem_idx + 1) % CL_MSG_DBG_MAX_RECS;	
	}
	xil_printf("=== end dumping log ===\r\n");
}

void cl_log_collect(const char *name, uint32_t line, const char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);
	cl_msg_log_collect(name, line, fmt, &args);
	va_end(args);
}

/*TODO: to be implemented*/
void cl_uart_printf(const char *name, uint32_t line, uint8_t log_level,
	const char *app_name, const char *fmt, ...)
{
}
