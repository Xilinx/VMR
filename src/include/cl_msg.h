/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#ifndef COMMON_MSG_H
#define COMMON_MSG_H

typedef enum cl_msg_type {
	CL_MSG_UNKNOWN = 0,
	CL_MSG_PDI,
	CL_MSG_XCLBIN,
	CL_MSG_AF,
	CL_MSG_CLOCK,
	CL_MSG_VMC,
} cl_msg_type_t;

struct xgq_vmr_data_payload {
	u32 address;
	u32 size;
};

struct xgq_vmr_head {
	u16 version;
	u16 type;
	u16 cid;
	u16 rcode;
};

/* TODO: add payload if necessary */
typedef struct cl_msg {
	struct xgq_vmr_head hdr;
	union {
		struct xgq_vmr_data_payload data_payload;
	};
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
