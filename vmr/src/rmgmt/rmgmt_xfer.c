/******************************************************************************
* Copyright (C) 2020-2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#include <stdbool.h>

#include "xilfpga.h"
#include "rmgmt_common.h"
#include "rmgmt_xfer.h"
#include "rmgmt_xclbin.h"
#include "rmgmt_fpt.h"

#include "cl_io.h"
#include "cl_msg.h"
#include "cl_flash.h"
#include "cl_rmgmt.h"

#define CLK_TYPE_DATA   0
#define CLK_TYPE_KERNEL 1
#define CLK_TYPE_SYSTEM 2
#define CLK_TYPE_MAX    4

int rmgmt_init_handler(struct rmgmt_handler *rh)
{
	rh->rh_data_base = 0;
	rh->rh_data_max_size = BITSTREAM_SIZE; /* 32M */
	rh->rh_log_max_size = LOG_MSG_SIZE; /* 4K */

	rh->rh_data_buffer = (u8 *)pvPortMalloc(rh->rh_data_max_size);
	if (rh->rh_data_buffer == NULL) {
		VMR_ERR("pvPortMalloc %d bytes for rh_data failed", rh->rh_data_max_size);
		return -ENOMEM;
	}

	rh->rh_log = (char *)pvPortMalloc(rh->rh_log_max_size);
	if (rh->rh_log == NULL) {
		vPortFree(rh->rh_data_buffer);
		VMR_ERR("pvPortMalloc %d bytes for rh_log failed", rh->rh_log_max_size);
		return -ENOMEM;
	}
	
	rh->rh_already_flashed = false;

	/* ospi flash should alreay be initialized */
	VMR_LOG("done");
	return 0;
}

int fpga_pdi_download_workaround(UINTPTR data, UINTPTR size, int has_pl)
{
	int ret;
	XFpga XFpgaInstance = { 0U };

	ret = XFpga_Initialize(&XFpgaInstance);
	if (ret != XST_SUCCESS) {
		VMR_DBG("FPGA init failed %d\r\n", ret);
		return ret;
	}

	if (has_pl) {
		axigate_freeze();
		ucs_stop();
	}

	IO_SYNC_WRITE32(0x30701, 0xff3f0440);
	IO_SYNC_WRITE32(0xf, 0xff3f0444);
	IO_SYNC_WRITE32(0x0, 0xff3f0448);
	IO_SYNC_WRITE32(data, 0xff3f044c);
	IO_SYNC_WRITE32(0x2, 0xff330000);
	/* wait for async operation done in case of firewall trip */
	MDELAY(1000);

	if (has_pl) {
		ucs_start();
		MDELAY(10);
		axigate_free();
	}

	VMR_LOG("ret: %d \r\n", ret);
	return ret;
}

static int sensor_task_request(cl_msg_t *msg)
{
	int ret = 0;

	ret = cl_send_to_queue(msg, CL_QUEUE_SENSOR_REQ);
	if (ret)
		return ret;

	ret = cl_recv_from_queue(msg, CL_QUEUE_SENSOR_RESP);
	if (ret)
		return ret;

	return cl_msg_get_rcode(msg);
}

static int disable_kernel_clock()
{
	cl_msg_t msg = { 0 };

	msg.hdr.type = CL_MSG_CLK_DISABLE;
	return sensor_task_request(&msg);
}

static int freq_scaling_internal(unsigned short *freqs, int num_freqs)
{	
	cl_msg_t msg = { 0 };

	/* This is an internal clock scaling command */
	msg.clock_payload.ocl_req_type = CL_CLOCK_SCALE_INTERNAL;
	msg.clock_payload.ocl_req_num = num_freqs;

	for (int i = 0; i < num_freqs; i++)
		msg.clock_payload.ocl_req_freq[i] = freqs[i];

	return cl_rmgmt_clock(&msg);	
}

static int reconfig_clock(const char *clock, int clock_size)
{
	struct clock_freq_topology *topo;
	struct clock_freq *freq = NULL;
	int data_clk_count = 0;
	int kernel_clk_count = 0;
	int system_clk_count = 0;
	int clock_type_count = 0;
	int i = 0;
	unsigned short target_freqs[CLK_TYPE_MAX] = { 0 };

	if (clock == NULL || clock_size == 0) {
		VMR_WARN("skip config");
		return 0;
	}

	topo = (struct clock_freq_topology *)clock;

	if (topo->m_count > CLK_TYPE_MAX) {
		VMR_ERR("More than %d clocks found in colock topology", CLK_TYPE_MAX);
		return -EINVAL;
	}

        for (i = 0; i < topo->m_count; i++) {
                freq = &(topo->m_clock_freq[i]);
                if (freq->m_type == CT_DATA)
                        data_clk_count++;
                if (freq->m_type == CT_KERNEL)
                        kernel_clk_count++;
                if (freq->m_type == CT_SYSTEM)
                        system_clk_count++;
        }

        if (data_clk_count != 1) {
                VMR_ERR("Data clock not found in clock topology");
                return -EINVAL;
        }
        if (kernel_clk_count != 1) {
                VMR_ERR("Kernel clock not found in clock topology");
                return -EINVAL;
        }
        if (system_clk_count > 2) {
                VMR_ERR("More than 2 system clocks found in clock topology");
                return -EINVAL;
        }

	/* system clock start index */
        clock_type_count = CLK_TYPE_SYSTEM;
        for (i = 0; i < topo->m_count; i++) {
                freq = &(topo->m_clock_freq[i]);
		switch (freq->m_type) {
		case CT_DATA:
                        target_freqs[CLK_TYPE_DATA] = freq->m_freq_Mhz;
			break;
		case CT_KERNEL:
                        target_freqs[CLK_TYPE_KERNEL] = freq->m_freq_Mhz;
			break;
		case CT_SYSTEM:
                        target_freqs[clock_type_count++] = freq->m_freq_Mhz;
			break;
		default:
			VMR_ERR("Unknown type: %d", freq->m_type);
			return -EINVAL;
		}
        }

        VMR_LOG("set %lu freq, data: %d, kernel: %d, sys: %d, sys1: %d",
            ARRAY_SIZE(target_freqs), target_freqs[0], target_freqs[1],
            target_freqs[2], target_freqs[3]);
	
	return freq_scaling_internal(target_freqs, ARRAY_SIZE(target_freqs));
}

static int enable_kernel_clock()
{
	cl_msg_t msg = { 0 };

	msg.hdr.type = CL_MSG_CLK_ENABLE;
	return sensor_task_request(&msg);
}

int fpga_pdi_download(UINTPTR data, UINTPTR size, const char *clock, int clock_size, int has_pl)
{
	int ret;
	XFpga XFpgaInstance = { 0U };
	UINTPTR KeyAddr = (UINTPTR)NULL;

	ret = XFpga_Initialize(&XFpgaInstance);
	if (ret != XST_SUCCESS) {
		VMR_ERR("FPGA init failed %d\r\n", ret);
		return ret;
	}

	if (has_pl) {
		ret = disable_kernel_clock();
		if (ret) {
			VMR_ERR("disable kernel clock failed %d\r\n", ret);
			return ret;
		}
		axigate_freeze();
		ucs_stop();
	}

	ret = XFpga_BitStream_Load(&XFpgaInstance, data, KeyAddr, size, PDI_LOAD);

	if (has_pl) {
		reconfig_clock(clock, clock_size);
		ucs_start();
		MDELAY(10);
		axigate_free();
		enable_kernel_clock();
	}

	VMR_LOG("ret: %d", ret);
	return ret;
}

static inline int pdi_download(UINTPTR data, UINTPTR size, const char *clock,
	int clock_size, int has_pl)
{
	return fpga_pdi_download(data, size, clock, clock_size, has_pl);
}

static int rmgmt_fpga_download(struct rmgmt_handler *rh, u32 len)
{
	int ret = 0;
	struct axlf *axlf = (struct axlf *)rh->rh_data_base;
	uint64_t offset = 0;
	uint64_t size = 0;
	char *partial_pdi = NULL;
	char *pdi = NULL;
	char *xclbin_topo = NULL;
	u32 partial_pdi_size = 0;
	u32 pdi_size = 0;
	u32 xclbin_topo_size = 0;

	/* Sync data from cache to memory */
	Xil_DCacheFlush();

	ret = rmgmt_xclbin_section_info(axlf, CLOCK_FREQ_TOPOLOGY, &offset, &size);
	if (ret || size == 0) {
		VMR_LOG("no CLOCK TOPOLOGY from xclbin: %d", ret);
	} else {
		xclbin_topo = (char *)axlf + offset;
		xclbin_topo_size = size;
	}

	ret = rmgmt_xclbin_section_info(axlf, BITSTREAM_PARTIAL_PDI, &offset, &size);
	if (ret || size == 0) {
		VMR_WARN("no PARTIAL PDI from xclbin: %d", ret);
	} else {
		partial_pdi = (char *)axlf + offset;
		partial_pdi_size = size;
		VMR_LOG("PARTIAL PDI from xclbin size:%d", partial_pdi_size);

		ret = pdi_download((UINTPTR)partial_pdi, (UINTPTR)partial_pdi_size,
			xclbin_topo, xclbin_topo_size, 1);
		
		if (ret)
			goto done;
	}

	ret = rmgmt_xclbin_section_info(axlf, PDI, &offset, &size);
	if (ret || size == 0) {
		VMR_LOG("no PDI from xclbin: %d", ret);
	} else {
		pdi = (char *)axlf + offset;
		pdi_size = size;
		VMR_LOG("PDI from xclbin size:%d", pdi_size);

		ret = pdi_download((UINTPTR)pdi, (UINTPTR)pdi_size,
			xclbin_topo, xclbin_topo_size, 1);

		if (ret)
			goto done;
	}

	ret = rmgmt_apu_download_xclbin(rh);
	if (ret == -ENODEV) {
		VMR_WARN("skip apu download xclbin ret: %d", ret);
		ret = 0;
	} else if (ret) {
		VMR_ERR("failed apu download xclbin ret: %d", ret);
	}

done:
	VMR_LOG("FPGA load pdi ret: %d", ret);
	return ret;
}

int rmgmt_load_apu(struct rmgmt_handler *rh)
{
	int ret = 0;
	u32 size;
	u8 pdiHeader[OSPI_VERSAL_PAGESIZE] = { 0 };

	ret = ospi_flash_read(CL_FLASH_APU, pdiHeader, 0, OSPI_VERSAL_PAGESIZE);
	if (*(u32 *)pdiHeader != MAGIC_NUM32) {
		VMR_LOG("WARN: skip loading APU, magic 0x%x is not 0x%x",
			*(u32 *)pdiHeader, MAGIC_NUM32);
		return 0;
	}

	size = *(((u32 *)pdiHeader) + 1);
	if (size == 0) {
		VMR_LOG("ERR: pdi size is 0.\r\n");
		return -1;
	}
	VMR_DBG("APU PDI size: %d\r\n", size);

	ret = ospi_flash_read(CL_FLASH_APU, (u8 *)rh->rh_data_base, OSPI_VERSAL_PAGESIZE, size);
	if (ret)
		return ret;

	/* Sync data from cache to memory */
	Xil_DCacheFlush();

	ret = pdi_download((UINTPTR)rh->rh_data_base, (UINTPTR)size, NULL, 0, 0);

	return ret;
}

static int rmgmt_ospi_rpu_download(struct rmgmt_handler *rh, u32 len)
{
	int ret;
	/* The base will be based on FPT table */
	u32 base = 0x0;

	/* Sync data from cache to memory */
	Xil_DCacheFlush();

	/* erase */
	ret = ospi_flash_erase(CL_FLASH_BOOT, base, len);
	if (ret) {
		VMR_LOG("OSPI fails to load pdi %d", ret);
		goto out;
	}

	/* write */
	ret = ospi_flash_write(CL_FLASH_BOOT, (u8 *)rh->rh_data_base, base, len);
	if (ret) {
		VMR_LOG("OSPI fails to load pdi %d", ret);
		goto out;
	}

	/*TODO: need authentication */
out:
	return ret;
}

static int rmgmt_ospi_apu_download(struct rmgmt_handler *rh, u32 len)
{
	int ret = 0;
	struct axlf *axlf = (struct axlf *)rh->rh_data_base;
	uint64_t offset = 0;
	uint64_t size = 0;
	cl_msg_t msg = { 0 };
	u32 dtb_offset = 0;
	u32 dtb_size = 0;

	if (cl_rmgmt_apu_is_ready()) {
		VMR_WARN("apu is ready, no need to re-download");
		return 0;
	}

	/*
	 * The systemdtb has been loaded when shell starts.
	 * We just to verify if there is valid systemdtb before loading
	 * apu pdi.
	 */
	if (rmgmt_fpt_get_systemdtb(&msg, &dtb_offset, &dtb_size)) {
		VMR_ERR("get system.dtb failed");
		return -1;
	}

	ret = rmgmt_xclbin_section_info(axlf, PDI, &offset, &size);
	if (ret) {
		VMR_LOG("get PDI failed %d", ret);
		return ret;
	}
	Xil_DCacheFlush();

	/*
	 * when loading an APU or any other PDI type which does not change the ULP.
	 * set ulp_changed to false.
	 * APU PDI doesn't have clock topology, thus set to NULL.
	 */
	ret = pdi_download((UINTPTR)((const char *)axlf + offset), (UINTPTR)size,
		NULL, 0, 0);

	VMR_LOG("FPGA load pdi ret: %d", ret);
	return ret;
}

int rmgmt_download_xclbin(struct rmgmt_handler *rh)
{
	return rmgmt_fpga_download(rh, rh->rh_data_size);
}

int rmgmt_download_rpu_pdi(struct rmgmt_handler *rh)
{
	return rmgmt_ospi_rpu_download(rh, rh->rh_data_size);
}

int rmgmt_download_apu_pdi(struct rmgmt_handler *rh)
{
	return rmgmt_ospi_apu_download(rh, rh->rh_data_size);
}
