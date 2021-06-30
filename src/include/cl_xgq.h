/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#ifndef COMMON_XGQ_H
#define COMMON_XGQ_H

/* Xilinx includes */
#include "xgq_impl.h"

/**
 * Assume: XGQ is attached already. Internal xQueue and xHandleTask are
 *         also created.
 *
 * cl_xgq_handle_init();
 *
 * Note: details of unblocking RTOS task design by counting semaphore.
 * When there is incoming entry in XGQ, xgq_consume will be called
 *     by polling or interrupt mode. The new entry will be dispatched
 *     into xQueue by xQueueSend(FromISR). vTaskNotifyGive(FromISR) will
 *     be called to notify xHandleTask;
 *
 * The xHandleTask will block by ulTaskNotifyTake until there is notification
 *     from vTaskNodifyGive(FromISR). Based on msg_type, calling into
 *     registered callbacks for different process_msg_cb function.
 *
 * cl_xgq_complete() should be called when msg done;
 *
 * cl_xgq_handle_fini(xgq_hdl):
 *     remove msg_type from XGQ;
 *     Note: this is rare situation. Most of the time, xga_handler stays alive.
 *
 */
typedef enum cl_msg_type {
	CL_MSG_UNKNOWN = 0,
	CL_MSG_PDI_RPU,
	CL_MSG_PDI_APU,
	CL_MSG_XCLBIN,
	CL_MSG_VMC,
} cl_msg_type_t;

typedef int (*process_msg_cb)(void *);

typedef struct xgq_handle {
	u32 xgq_entry;
	cl_msg_type_t msg_type;
	process_msg_cb cb;
} xgq_handle_t;

int cl_xgq_handle_init(xgq_handle_t *xgq_hdl, cl_msg_type_t type, process_msg_cb cb);
int cl_xga_handle_complete(xgq_handle_t *xgq_hdl);
void cl_xgq_handle_fini(xga_handle_t *xgq_hdl);

#endif
