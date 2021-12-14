/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#ifndef COMMON_LOG_H
#define COMMON_LOG_H

/* C includes */
#include "stdlib.h"
#include "stdio.h"

#include "cl_version.h"

//#define CL_VERBOSE
//#define VMC_DEBUG

/**
 * Application type id for logging, mem signature etc.
 */
typedef enum app_type {
	APP_MAIN = 0,
	APP_RMGMT,
	APP_VMC,
} app_type_t;


/**
 * Note: please make sure index matches enum value of app_type
 */
static const char *app_type_name[] = {
	"MAIN",
	"RMGMT",
	"VMC",
};

#define CL_ERR(app, fmt, arg...) 				\
({ 								\
	xil_printf("[ERROR]: %s:%s:" fmt "\r\n",		\
		app_type_name[app], __FUNCTION__, ##arg);	\
})

#define CL_LOG(app, fmt, arg...) 				\
({ 								\
	xil_printf("%s:%s:" fmt "\r\n",				\
		app_type_name[app], __FUNCTION__, ##arg);	\
})

#define CL_DMO(app, fmt, arg...)
#define CL_PRNT(app, fmt, arg...) 

#ifdef CL_VERBOSE
#define CL_DBG(app, fmt, arg...) 				\
({ 								\
	xil_printf("%s:%s:%d:%s:" fmt "\r\n",			\
		app_type_name[app], __FILE__, __LINE__,		\
		__FUNCTION__, ##arg);				\
})
#else
#define CL_DBG(app, fmt, arg...)
#endif

#endif
