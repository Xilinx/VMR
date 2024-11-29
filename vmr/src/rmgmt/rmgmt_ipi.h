/******************************************************************************
* Copyright (C) 2024 AMD, Inc.    All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#ifndef RMGMT_IPI_H
#define RMGMT_IPI_H

int rmgmt_ipi_image_store(u32 pdi_address, u32 pdi_size);
int rmgmt_ipi_load_pdi(u32 pdi_address, u32 pdi_size);

#endif
