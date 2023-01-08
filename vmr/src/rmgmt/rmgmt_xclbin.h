/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#ifndef RMGMT_XCLBIN_H
#define RMGMT_XCLBIN_H

#include "xclbin.h"

int rmgmt_xclbin_section_info(const struct axlf *xclbin, enum axlf_section_kind kind,
        uint64_t *offset, uint64_t *size);

int rmgmt_xclbin_get_section(const struct axlf *xclbin, enum axlf_section_kind kind,
	void **data, uint64_t *len);

void rmgmt_xclbin_section_remove(struct axlf *xclbin, enum axlf_section_kind kind);
#endif
