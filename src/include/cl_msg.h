/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#ifndef COMMON_MSG_H
#define COMMON_MSG_H

/**
 * TODO: refine and move this block comment close to real code implementation.
 *
 * Assume: XGQ is attached already. Internal xQueue and xHandleTask are
 *         also created.
 *
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
 *
 * cl_xgq_handle_init(); -> xgq_attach();
 *
 * new entry from SQ -> xgq_consume -> xQueueSend -> xgq_notify_consumed.
 *                                       +
 *                                       |-> vTaskNOtifyGive(xHandleTask)
 *
 * vHandleTask {
 * 	for (;;) {
 * 		ulNotifyValue = ulTaskNotifyTake();
 * 		if (ulNotifyValue > 0) {
 * 			xQueueReceive;
 * 		}
 * 	}
 * }
 *
 * cl_xgq_complete -> xgq_produce -> xgq_notify_produced
 *
 */

//#define XGQ_DEBUG_IP

typedef union {
        struct {
                u32 version:16;
                u32 type:16;
                u32 cid:16;
                u32 rcode:16;
        };
        u32 head[2];
} xgq_vmr_head_t;

struct xgq_vmr_payload_xclbin {
        u64 address;
        u32 size;
};

struct xgq_vmr_payload_af {
        u64 buffer_addr;
        u32 buffer_size;
};

struct xgq_vmr_pkt {
        xgq_vmr_head_t head;
        union {
                struct xgq_vmr_payload_xclbin payload_xclbin;
                struct xgq_vmr_payload_af payload_af;
        };
};

/* Note:
 * this exactly matches the xrt_xgq_pkt_type_t.
 * We might use xrt interface, but we cannot use cl in XRT code base.
 * When the interface is stable, we should push this into XRT/include that we get
 * xgq_cmd.h so that we don't have dup code.
 */
typedef enum cl_msg_type {
	CL_MSG_UNKNOWN = 0,
	CL_MSG_PDI,
	CL_MSG_XCLBIN,
	CL_MSG_AF,
} cl_msg_type_t;

typedef struct cl_msg {
	struct xgq_vmr_pkt pkt;
} cl_msg_t;

typedef int (*process_msg_cb)(cl_msg_t *msg, void *arg);

typedef struct msg_handle {
	cl_msg_type_t type;
	process_msg_cb msg_cb;
	void *arg;
} msg_handle_t;

int cl_msg_handle_init(msg_handle_t **hdl, cl_msg_type_t type,
	process_msg_cb cb, void *arg);
int cl_msg_handle_complete(cl_msg_t *msg);
void cl_msg_handle_fini(msg_handle_t *hdl);

#endif
