/******************************************************************************
* Copyright (C) 2023 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/
#ifndef RMI_H
#define RMI_H

#include "vmc_asdm.h"
#include "RMI/rmi_api.h"

/* TODO Create instance of SDR_t here to copy sdrInfo found in vmc_asdm.c using
   stream buffer so as to be threadsafe */
extern SDR_t *sdrInfo;

/**
*  @brief Callback registered with RMI to respond to API requests
*  @param pucReq Request buffer - incoming API request from RMI
*  @param pusReq_size Size of request buffer
*  @param pucResp Response buffer - returned to RMI
*  @param pusResp_size Size of response buffer
*  @return eRMI_SUCCESS on success
*/
rmi_error_codes_t xRmi_Request_Handler(uint8_t* pucReq, uint16_t* pusReq_size, uint8_t** pucResp, uint16_t* pusResp_size);

rmi_error_codes_t get_SDR_API(uint8_t **pucBuffer, uint16_t *pusBufSize);
rmi_error_codes_t get_ALL_SENSOR_DATA_API(void);

#endif /* RMI_H */