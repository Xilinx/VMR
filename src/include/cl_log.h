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

//#define VMC_DEBUG

/*
 * The pdMS_TO_TICKS is defined as (xTimeInMs * configTICKRATEHZ) / 1000
 * so the pdTICKS_TO_MS can be the opposite way
 */
#define pdTICKS_TO_MS( xTicks ) \
	(( xTicks * 1000 ) / configTICKRATEHZ)

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

typedef enum cl_log_level {
	CL_LOG_LEVEL_ERR = 0,
	CL_LOG_LEVEL_LOG,
	CL_LOG_LEVEL_DBG,
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
#define CL_PRNT(app, fmt, arg...)

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

#endif
