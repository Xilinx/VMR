/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#ifndef COMMON_XGQ_H
#define COMMON_XGQ_H

/* Xilinx includes */
#include "xgq_impl.h"

typedef struct xgq_handle {
	struct xgq *xgq;
	int (*queue_entry_op)(void *arg);
} xga_handle_t;

/**
 * Simplify server side xgq_attach
 */
int cl_xgq_init(xgq_hanlde_t *xgq);

/*
 * Simplify server side xgq_produce,and
 * fill-up-CQ-entry by queue_entry_op, then
 * call xqg_notify_peer_produced.
 */
int cl_xgq_send(xgq_handle_t *xgq);

/*
 * Simpify server side xgq_consume, and
 * process-SQ-entry by queue_entry_op, then
 * call xgq_notify_peer_consumed.
 */
int cl_xgq_recv(xgq_handle_t *xgq);

#endif
