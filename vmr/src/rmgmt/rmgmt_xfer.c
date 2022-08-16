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


static inline u32 wait_for_status(struct rmgmt_handler *rh, u8 status)
{
	u32 header;
	struct pdi_packet *pkt = (struct pdi_packet *)&header;

        for (;;) {
                header = IO_SYNC_READ32(rh->rh_base);
                if (pkt->pkt_status == status)
                        break;
        }

        return header;
}

u8 rmgmt_get_pkt_flags(struct rmgmt_handler *rh)
{
        struct pdi_packet *pkt;
        u32 header;

        pkt = (struct pdi_packet *)&header;
        header = IO_SYNC_READ32(rh->rh_base);
        return pkt->pkt_flags;
}

int rmgmt_check_for_status(struct rmgmt_handler *rh, u8 status)
{
        struct pdi_packet *pkt;
        u32 header;

        pkt = (struct pdi_packet *)&header;
        header = IO_SYNC_READ32(rh->rh_base);

        return (pkt->pkt_status == status);
}

static inline void set_flags(struct rmgmt_handler *rh, u8 flags)
{
        struct pdi_packet *pkt;
        u32 header;

        pkt = (struct pdi_packet *)&header;
        header = IO_SYNC_READ32(rh->rh_base);
        pkt->pkt_flags = flags;

        IO_SYNC_WRITE32(pkt->header, rh->rh_base);
}

static inline void set_version(struct rmgmt_handler *rh)
{
        set_flags(rh, XRT_XFR_VER <<  XRT_XFR_PKT_VER_SHIFT);
}

static inline void set_status(struct rmgmt_handler *rh, u8 status)
{
	volatile struct pdi_packet *pkt;
	volatile u32 header;

        header = IO_SYNC_READ32(rh->rh_base);
        pkt = (struct pdi_packet *)&header;
        pkt->pkt_status = status;

        IO_SYNC_WRITE32(pkt->header, rh->rh_base);
}

static inline void read_data32(u32 *addr, u32 *data, size_t sz)
{
	u32 i;

	for (i = 0; i < sz; ++i) {
		*(data + i) = IO_SYNC_READ32((UINTPTR)(addr + i));
	}
}

static int rmgmt_data_receive(struct rmgmt_handler *rh, u32 *len)
{
	u32 offset = 0;
	int ret;

	for (;;) {
		u32 pkt_header;
		struct pdi_packet *pkt = (struct pdi_packet *)&pkt_header;
		int lastpkt;

		/* Busy wait here until we get a new packet */
		pkt_header = wait_for_status(rh, XRT_XFR_PKT_STATUS_NEW);
		lastpkt = pkt->pkt_flags & XRT_XFR_PKT_FLAGS_LAST;

		if ((offset + pkt->pkt_size) > rh->rh_data_size) {
			VMR_DBG("max: %d M, received %d M\r\n",
				rh->rh_data_size / 0x100000,
				(offset + pkt->pkt_size) / 0x100000);
			return -1;
		}
		/* Read packet data payload on a 4 bytes base */
		read_data32((u32 *)rh->rh_base + (sizeof(struct pdi_packet)) / 4,
			(u32 *)(rh->rh_data) + offset / 4,
			pkt->pkt_size / 4);

		/* Notify host that the data has been read */
		set_status(rh, XRT_XFR_PKT_STATUS_IDLE);

		/* Set len to next offset */
		offset += pkt->pkt_size;

		/* Bail out here if this is the last packet */
		if (lastpkt) {
			VMR_DBG("\r\n");
			ret = 0;
			break;
		}
	}

	*len = offset;
	return ret;
}

#if 0
static int rmgmt_init_xfer(struct rmgmt_handler *rh)
{
	if (XRT_XFR_RES_SIZE & 0x3) {
		VMR_LOG("xfer size: %d is not 32bit aligned\r\n", XRT_XFR_RES_SIZE);
		return -1;
	}

	/* Special init BRAM from random data to all ZERO */
	for (int i = 0; i < XRT_XFR_RES_SIZE; i += 4) {
		IO_SYNC_WRITE32(0x0, rh->rh_base + i * 4);
	}

	return 0;
}

static int rmgmt_init_xfer_handler(struct rmgmt_handler *rh)
{
	rh->rh_base = OSPI_VERSAL_BASE;

	if (rmgmt_init_xfer(rh) != 0)
		return -1;

	rh->rh_max_size = BITSTREAM_SIZE; /* 32M */
	rh->rh_data = (u8 *)malloc(rh->rh_max_size);
	if (rh->rh_data == NULL) {
		VMR_LOG("malloc %d bytes failed\r\n", rh->rh_data_size);
		return -1;
	}

	/* ospi flash should alreay be initialized */

	set_version(rh);
	set_status(rh, XRT_XFR_PKT_STATUS_IDLE);

	VMR_DBG("rmgmt_init_handler done\r\n");
	return 0;
}
#endif

int rmgmt_init_handler(struct rmgmt_handler *rh)
{
	rh->rh_data_max_size = BITSTREAM_SIZE; /* 32M */
	rh->rh_log_max_size = LOG_MSG_SIZE; /* 4K */

	rh->rh_data = (u8 *)pvPortMalloc(rh->rh_data_max_size);
	if (rh->rh_data == NULL) {
		VMR_ERR("pvPortMalloc %d bytes for rh_data failed", rh->rh_data_max_size);
		return -ENOMEM;
	}

	rh->rh_log = (char *)pvPortMalloc(rh->rh_log_max_size);
	if (rh->rh_log == NULL) {
		vPortFree(rh->rh_data);
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

int fpga_pdi_download(UINTPTR data, UINTPTR size, int has_pl)
{
	int ret;
	XFpga XFpgaInstance = { 0U };
	UINTPTR KeyAddr = (UINTPTR)NULL;

	ret = XFpga_Initialize(&XFpgaInstance);
	if (ret != XST_SUCCESS) {
		VMR_DBG("FPGA init failed %d\r\n", ret);
		return ret;
	}

	if (has_pl) {
		axigate_freeze();
		ucs_stop();
	}

	ret = XFpga_BitStream_Load(&XFpgaInstance, data, KeyAddr, size, PDI_LOAD);

	if (has_pl) {
		ucs_start();
		MDELAY(10);
		axigate_free();
	}

	VMR_LOG("ret: %d \r\n", ret);
	return ret;
}

static inline int pdi_download(UINTPTR data, UINTPTR size, int has_pl)
{
	return fpga_pdi_download(data, size, has_pl);
}

static int rmgmt_fpga_download(struct rmgmt_handler *rh, u32 len)
{
	int ret = 0;
	struct axlf *axlf = (struct axlf *)rh->rh_data;
	uint64_t offset = 0;
	uint64_t size = 0;

	/* Sync data from cache to memory */
	Xil_DCacheFlush();

	ret = rmgmt_xclbin_section_info(axlf, BITSTREAM_PARTIAL_PDI, &offset, &size);
	if (ret || size == 0) {
		VMR_LOG("no PARTIAL PDI from xclbin: %d", ret);
	} else {
		ret = pdi_download((UINTPTR)((const char *)axlf + offset),
			(UINTPTR)size, 1);
		if (ret)
			goto done;
	}

	ret = rmgmt_xclbin_section_info(axlf, PDI, &offset, &size);
	if (ret || size == 0) {
		VMR_LOG("no PDI from xclbin: %d", ret);
	} else {
		ret = pdi_download((UINTPTR)((const char *)axlf + offset),
			(UINTPTR)size, 1);
		if (ret)
			goto done;
	}

	ret = cl_rmgmt_apu_download_xclbin((char *)rh->rh_data, rh->rh_data_size);
	if (ret == -ENODEV) {
		VMR_LOG("skip apu download xclbin ret: %d", ret);
		ret = 0;
	} else if (ret) {
		VMR_LOG("failed apu download xclbin ret: %d", ret);
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

	ret = ospi_flash_read(CL_FLASH_APU, rh->rh_data, OSPI_VERSAL_PAGESIZE, size);
	if (ret)
		return ret;

	/* Sync data from cache to memory */
	Xil_DCacheFlush();

	ret = pdi_download((UINTPTR)rh->rh_data, (UINTPTR)size, 0);

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
	ret = ospi_flash_write(CL_FLASH_BOOT, rh->rh_data, base, len);
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
	struct axlf *axlf = (struct axlf *)rh->rh_data;
	uint64_t offset = 0;
	uint64_t size = 0;
	cl_msg_t msg = { 0 };
	u32 dtb_offset = 0;
	u32 dtb_size = 0;

	if (cl_rmgmt_apu_is_ready()) {
		VMR_WARN("apu is ready, no need to re-download");
		return 0;
	}

	if (rmgmt_fpt_get_systemdtb(&msg, &dtb_offset, &dtb_size)) {
		VMR_ERR("get system.dtb failed");
		return -1;
	}
	cl_memcpy(VMR_EP_SYSTEM_DTB, dtb_offset, dtb_size);

	ret = rmgmt_xclbin_section_info(axlf, PDI, &offset, &size);
	if (ret) {
		VMR_LOG("get PDI failed %d", ret);
		return ret;
	}
	Xil_DCacheFlush();

	/*
	 * when loading an APU or any other PDI type which does not change the ULP.
	 * set ulp_changed to false.
	 */
	ret = pdi_download((UINTPTR)((const char *)axlf + offset),
		(UINTPTR)size, 0);

	VMR_LOG("FPGA load pdi ret: %d", ret);
	return ret;
}

/* Temporary disable old apu design flow */
#if 0
static int rmgmt_ospi_apu_download(struct rmgmt_handler *rh, u32 len)
{
	int ret;
	u8 pdiHeader[OSPI_VERSAL_PAGESIZE] = { 0 };

	VMR_LOG("-> rmgmt_ospi_apu_download\r\n");

	*((u32 *)pdiHeader) = MAGIC_NUM32;
	*((u32 *)pdiHeader + 1) = len;

	/* Sync data from cache to memory */
	Xil_DCacheFlush();

	ret = ospi_flash_erase(CL_FLASH_APU, 0, OSPI_VERSAL_PAGESIZE + len);
	if (ret) {
		set_status(rh, XRT_XFR_PKT_STATUS_FAIL);
		VMR_DBG("OSPI fails to load pdi %d\r\n", ret);
		goto out;
	}

	/* flash rpu base pdi from offset 0(RPU_PDI_ADDRES) */
	ret = ospi_flash_write(CL_FLASH_APU, pdiHeader,
		0, OSPI_VERSAL_PAGESIZE);
	if (ret) {
		set_status(rh, XRT_XFR_PKT_STATUS_FAIL);
		VMR_DBG("OSPI fails to load pdi %d\r\n", ret);
		goto out;
	}

	ret = ospi_flash_write(CL_FLASH_APU, rh->rh_data,
		OSPI_VERSAL_PAGESIZE, len);
	if (ret) {
		set_status(rh, XRT_XFR_PKT_STATUS_FAIL);
		VMR_DBG("OSPI fails to load pdi %d\r\n", ret);
		goto out;
	}

	set_status(rh, XRT_XFR_PKT_STATUS_DONE);
out:
	VMR_LOG("<- rmgmt_ospi_apu_download %d\r\n", ret);
	return ret;
}
#endif

static int rmgmt_recv_pkt(struct rmgmt_handler *rh, u32 *len)
{
	int ret;

	VMR_DBG("-> rmgmt_recv_pkt\r\n");

	ret = rmgmt_data_receive(rh, len);
	if (ret) {
		set_status(rh, XRT_XFR_PKT_STATUS_FAIL);
		VMR_DBG("Fail to receive pkt %d\r\n", ret);
		goto out;
	}

	/*TODO: verify signature */

	VMR_DBG("<- rmgmt_recv_pkt\r\n");
out:
	return ret;
}

static void rmgmt_done_pkt(struct rmgmt_handler *rh)
{
	wait_for_status(rh, XRT_XFR_PKT_STATUS_IDLE);

	set_version(rh);

	VMR_DBG("<-");
}

struct rmgmt_ops {
	int (*rmgmt_recv_op)(struct rmgmt_handler *rh, u32 *len);
	int (*rmgmt_download_op)(struct rmgmt_handler *rh, u32 len);
	void (*rmgmt_done_op)(struct rmgmt_handler *rh);
};

static struct rmgmt_ops xfer_rpu_ops = {
	.rmgmt_recv_op = rmgmt_recv_pkt,
	.rmgmt_download_op = rmgmt_ospi_rpu_download,
	.rmgmt_done_op = rmgmt_done_pkt,
};

static struct rmgmt_ops xfer_apu_ops = {
	.rmgmt_recv_op = rmgmt_recv_pkt,
	.rmgmt_download_op = rmgmt_ospi_apu_download,
	.rmgmt_done_op = rmgmt_done_pkt,
};

static struct rmgmt_ops xfer_xclbin_ops = {
	.rmgmt_recv_op = rmgmt_recv_pkt,
	.rmgmt_download_op = rmgmt_fpga_download,
	.rmgmt_done_op = rmgmt_done_pkt,
};

static int rmgmt_xfer_download(struct rmgmt_handler *rh, struct rmgmt_ops *ops)
{
	u32 len;
	int ret;

	if (ops == NULL) {
		VMR_DBG("rmgmt_ops cannot be NULL\r\n");
		return -1;
	}

	ret = ops->rmgmt_recv_op(rh, &len);
	if (ret)
		goto done;

	ret = ops->rmgmt_download_op(rh, len);

done:
	ops->rmgmt_done_op(rh);
	return ret;
}

int rmgmt_xfer_download_xclbin(struct rmgmt_handler *rh)
{
	return rmgmt_xfer_download(rh, &xfer_xclbin_ops);
}

int rmgmt_xfer_download_rpu_pdi(struct rmgmt_handler *rh)
{
	return rmgmt_xfer_download(rh, &xfer_rpu_ops);
}

int rmgmt_xfer_download_apu_pdi(struct rmgmt_handler *rh)
{
	return rmgmt_xfer_download(rh, &xfer_apu_ops);
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
