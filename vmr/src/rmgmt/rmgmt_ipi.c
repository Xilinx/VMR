/******************************************************************************
* Copyright (C) 2024 Advanced Micro Devices, Inc.    All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#include <stdbool.h>
#include <stdint.h>

#include "rmgmt_common.h"
#include "rmgmt_ipi.h"
#include "cl_msg.h"

#include "xipipsu.h"

#define TARGET_IPI_INT_MASK	XPAR_XIPIPS_TARGET_PSV_PMC_0_CH0_MASK
#define IPI_TIMEOUT		(0xFFFFFFFFU)
#define PDI_SRC_ADDR_LOW	(0x1000000U)
#define PDI_SRC_ADDR_HIGH	(0U)
#define XLOADER_PDI_SRC_DDR	(0xFU)
#define HEADER(Len, ModuleId, CmdId)	((Len << 16U) | (ModuleId << 8U) | CmdId)
#define LOAD_PDI_CMD_PAYLOAD_LEN	(3U)
#define XILLOADER_MODULE_ID		(7U)
#define XILLOADER_LOAD_PPDI_CMD_ID	(1U)

#define PDI_ID 0x0
#define PDI_SIZE_IN_WORDS 0
#define PDI_ADDRESS_LOW 0
#define PDI_ADDRESS_HIGH 0

static 	XIpiPsu IpiInst;

static int IpiInit(void)
{
	int Status = XST_FAILURE;
	XIpiPsu_Config *IpiCfgPtr;

	/* Look Up the config data */
	IpiCfgPtr = XIpiPsu_LookupConfig(XPAR_XIPIPSU_0_BASEADDR);
	if (NULL == IpiCfgPtr) {
		VMR_ERR("XIpiPsu_Lookup failed");
		goto END;
	}

	/* Initialize with the Cfg Data */
	Status = XIpiPsu_CfgInitialize(&IpiInst, IpiCfgPtr,
		IpiCfgPtr->BaseAddress);
	if (XST_SUCCESS != Status) {
		VMR_ERR("XIpiPsu_CfgInit failed");
		goto END;
	}

END:
	VMR_WARN("Status %d", Status);
	return Status;
}

/*
 * This command adds PDI address to the list of Image Store PDIs that are
 * maintained by PLM. During restore or reload of a image, PLM checks this
 * dynamically added list of PDIs first to get the required image and in case
 * of any failure, it falls back to next possible. If no valid entry is
 * present, it uses boot pdi, which is the first entry in the list. 
 */
int rmgmt_ipi_image_store(u32 pdi_address, u32 pdi_size)
{
	int Status = XST_FAILURE;
	u32 Response;

	/*
	 * Revisited @ 2023.2 interface changes.
	 *
	 * Command: Add ImageStore PDI
	 *   Reserved[31:25]=0	Security Flag[24]	Length[23:16]=4	XilLoader=7	CMD_ADD_IMG_STORE_PDI=9
	 *   PDI ID
	 *   High PDI Address
	 *   Low PDI Address
	 *   PDI Size ( In Words )
	 *
	 * See examples in SSW/xiloader/examples.
	 */
	u32 Payload[] = {0x040709, PDI_ID, PDI_ADDRESS_HIGH, PDI_ADDRESS_LOW, PDI_SIZE_IN_WORDS};

	VMR_WARN("addr 0x%x size %d", pdi_address, pdi_size);

	Status = IpiInit();
	if (Status != XST_SUCCESS)
		goto END;

	/* Note: set pdi_address into image store, size is not concerned for now */
	Payload[3] = pdi_address;
	Payload[4] = pdi_size;

	Xil_DCacheDisable();

	/**
	 * Send a Message to TEST_TARGET and WAIT for ACK
	 */
	Status = XIpiPsu_WriteMessage(&IpiInst, TARGET_IPI_INT_MASK, Payload,
		sizeof(Payload)/sizeof(u32), XIPIPSU_BUF_TYPE_MSG);
	if (Status != XST_SUCCESS) {
		VMR_ERR("Writing to IPI request buffer failed");
		goto END;
	}

	Status = XIpiPsu_TriggerIpi(&IpiInst, TARGET_IPI_INT_MASK);
	if (Status != XST_SUCCESS) {
		VMR_ERR("IPI trigger failed");
		goto END;
	}


	/* Wait until current IPI interrupt is handled by target module */
	Status = XIpiPsu_PollForAck(&IpiInst, TARGET_IPI_INT_MASK, IPI_TIMEOUT);
	if (XST_SUCCESS != Status) {
		VMR_ERR("IPI Timeout expired");
		goto END;
	}

	Status = XIpiPsu_ReadMessage(&IpiInst, TARGET_IPI_INT_MASK, &Response,
		sizeof(Response)/sizeof(u32), XIPIPSU_BUF_TYPE_RESP);
	if (XST_SUCCESS != Status) {
		VMR_ERR("Reading from IPI response buffer failed");
		goto END;
	}

	Status = (int)Response;

END:
	VMR_WARN("Status %d", Status);
	return Status;
}
