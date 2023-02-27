/******************************************************************************
 * * Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
 * * SPDX-License-Identifier: MIT
 * *******************************************************************************/

#include "cl_config.h"

#ifdef BUILD_FOR_RMI

/* FreeRTOS includes */
#include "FreeRTOS.h"
#include "task.h"

#include <stdio.h>
#include <stdlib.h>
#include "cl_rmi.h"
#include "RMI/rmi_api.h"

int cl_rmi_init(void){
    rmi_init();
    return 0;
}


/*
 * When task func started, all init works should be done already.
 */
void cl_rmi_func(void *task_args){

    while (1){

        /* Call RMI Task here. */
        rmi_task_func();

        /* every 100ms we should check hardware status */
        vTaskDelay(pdMS_TO_TICKS(100));
    }

}


#endif
