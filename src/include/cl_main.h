/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#ifndef COMMON_MAIN_H
#define COMMON_MAIN_H

typedef int (*tasks_register_t)(void); 

int RMGMT_Launch(void);
int CMC_Launch(void);

#endif
