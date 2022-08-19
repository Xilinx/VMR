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
#include "cl_config.h"

#define pdTICKS_TO_MS(n) (n * 1000 / configTICK_RATE_HZ)
#define pdTICKS_TO_S(n) (n / configTICK_RATE_HZ)

/**
 * Application type id for logging, mem signature etc.
 */
typedef enum app_type {
	APP_VMR = 0,
	APP_VMC,
} app_type_t;

/**
 * Note: please make sure index matches enum value of app_type
 */
__attribute__((unused))
static const char *app_type_name[] = {
	"VMR",
	"VMC",
};

typedef enum cl_log_level {
	CL_LOG_LEVEL_ERR = 0, /* Please do not change log level value, ERR is using 0 as default value */
	CL_LOG_LEVEL_LOG,
	CL_LOG_LEVEL_DBG,
	CL_LOG_LEVEL_PRNT, /*TODO: explain what is this level */
} cl_log_level_t;

typedef enum cl_uart_log_level {
	CL_UART_LOG_LEVEL_VERBOSE = 0,
	CL_UART_LOG_LEVEL_DEBUG, 
	CL_UART_LOG_LEVEL_INFO,
	CL_UART_LOG_LEVEL_WARN,
	CL_UART_LOG_LEVEL_ERROR,
	CL_UART_LOG_LEVEL_DEMO_MENU,
	CL_UART_LOG_LEVEL_NONE,        /* disable logging */
} cl_uart_log_level_t;

void cl_loglevel_set(uint8_t log_level);
uint8_t cl_loglevel_get(void);
/* default logging API */
void cl_printf(const char *name, uint32_t line, uint8_t log_level,
	const char *app_name, const char *fmt, ...);

/* ring buffer in memory for collecting performance sensitive logs */
void cl_log_collect(const char *name, uint32_t line, const char *fmt, ...);
void cl_log_dump();

/* UART specific API */
void cl_uart_printf(const char *name, uint32_t line, uint8_t log_level,
	const char *app_name, const char *fmt, ...);


#define CL_PRINT(level, app_name, fmt, arg...) \
	cl_printf(__FUNCTION__, __LINE__, level, app_name, fmt, ##arg)

#define CL_ERR(app, fmt, arg...) 	\
 	CL_PRINT(CL_LOG_LEVEL_ERR, app_type_name[app], fmt, ##arg)

#define CL_LOG(app, fmt, arg...)	\
 	CL_PRINT(CL_LOG_LEVEL_LOG, app_type_name[app], fmt, ##arg)

#define CL_DBG(app, fmt, arg...) 	\
 	CL_PRINT(CL_LOG_LEVEL_DBG, app_type_name[app], fmt, ##arg)

#define CL_DMO(app, fmt, arg...)
#define CL_PRNT(app, fmt, arg...)	\
	CL_PRINT(CL_LOG_LEVEL_PRNT, app_type_name[app], fmt, ##arg)

#define CL_COLLECT(fmt, arg...) cl_log_collect(__FUNCTION__, __LINE__, fmt, ##arg)
#define CL_DUMP() 		cl_log_dump()

#define CL_UART_PRINT(level, app_name, fmt, arg...) \
	cl_uart_printf(__FUNCTION__, __LINE__, level, app_name, fmt, ##arg)

#define CL_UART_ERR(app, fmt, arg...) 	\
	CL_UART_PRINT(CL_UART_LOG_LEVEL_ERROR, app_type_name[app], fmt, ##arg);
#define CL_UART_LOG(app, fmt, arg...)
#define CL_UART_DBG(app, fmt, arg...)
#define CL_UART_DMO(app, fmt, arg...)
#define CL_UART_PRNT(app, fmt, arg...)

#define VMR_ERR(fmt, arg...) \
        CL_ERR(APP_VMR, fmt, ##arg)
#define VMR_WARN(fmt, arg...) \
        CL_ERR(APP_VMR, fmt, ##arg)
#define VMR_LOG(fmt, arg...) \
        CL_LOG(APP_VMR, fmt, ##arg)
#define VMR_DBG(fmt, arg...) \
        CL_DBG(APP_VMR, fmt, ##arg)
#define VMR_PRNT(fmt, arg...) \
        CL_PRNT(APP_VMR, fmt, ##arg)

void cl_log_init();

#endif
