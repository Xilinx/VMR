/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#ifndef COMMON_LOG_H
#define COMMON_LOG_H

/**
 * Comment VERBOSE out in production code
 */
#define CL_VERBOSE

/* C includes */
#include "stdlib.h"
#include "stdio.h"

#include "../vmc/vmc_api.h"


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

/**
 * Note: preprocessors are complier related...
 */

#define CL_ERR(app, fmt, arg...) 		\
	VMC_Printf(__FILENAME__, __LINE__, VMC_LOG_LEVEL_ERROR,"%s[ERROR]: %s" fmt "\r\n", 	\
			app_type_name[app], __FUNCTION__, ##arg)

#define CL_DMO(app, fmt, arg...) 		\
	VMC_Printf(__FILENAME__, __LINE__, VMC_LOG_LEVEL_DEMO_MENU, fmt,##arg)

#define CL_PRNT(app, fmt, arg...) 		\
	VMC_Printf(__FILENAME__, __LINE__, VMC_LOG_LEVEL_INFO, fmt,##arg)

#define CL_LOG(app, fmt, arg...) 		\
	VMC_Printf(__FILENAME__, __LINE__, VMC_LOG_LEVEL_INFO, "%s: %s " fmt "\r\n", 	\
			app_type_name[app], __FUNCTION__, ##arg)

#ifdef CL_VERBOSE

#define CL_DBG(app, fmt, arg...) 		\
	VMC_Printf(__FILENAME__, __LINE__, VMC_LOG_LEVEL_DEBUG,"%s[DEBUG]: %s:%d %s " fmt "\r\n", 	\
			app_type_name[app], __FILENAME__, __LINE__, __FUNCTION__, ##arg)

#else
#define CL_DBG(app, fmt, arg...)
#endif

#endif
