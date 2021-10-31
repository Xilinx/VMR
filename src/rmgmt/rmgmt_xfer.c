/******************************************************************************
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#include "xilfpga.h"
#include "rmgmt_common.h"
#include "rmgmt_xfer.h"
#include "rmgmt_xclbin.h"

#include "cl_io.h"
#include "cl_flash.h"

extern int ospi_flash_erase(flash_area_t area, u32 offset, u32 len);

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
			RMGMT_DBG("max: %d M, received %d M\r\n",
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
			RMGMT_DBG("\r\n");
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
		RMGMT_LOG("xfer size: %d is not 32bit aligned\r\n", XRT_XFR_RES_SIZE);
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
		RMGMT_LOG("malloc %d bytes failed\r\n", rh->rh_data_size);
		return -1;
	}

	/* ospi flash should alreay be initialized */

	set_version(rh);
	set_status(rh, XRT_XFR_PKT_STATUS_IDLE);

	RMGMT_DBG("rmgmt_init_handler done\r\n");
	return 0;
}
#endif

int rmgmt_init_handler(struct rmgmt_handler *rh)
{
	//rh->rh_base = OSPI_VERSAL_BASE;

	rh->rh_max_size = BITSTREAM_SIZE; /* 32M */
	rh->rh_data = (u8 *)malloc(rh->rh_max_size);
	if (rh->rh_data == NULL) {
		RMGMT_LOG("malloc %d bytes failed\r\n", rh->rh_data_size);
		return -1;
	}

	/* ospi flash should alreay be initialized */
	RMGMT_LOG("done\r\n");
	return 0;
}

static int fpga_pdi_download(UINTPTR data, UINTPTR size)
{
	int ret;
	XFpga XFpgaInstance = { 0U };
	UINTPTR KeyAddr = (UINTPTR)NULL;

	ret = XFpga_Initialize(&XFpgaInstance);
	if (ret != XST_SUCCESS) {
		RMGMT_DBG("FPGA init failed %d\r\n", ret);
		return ret;
	}

	ret = XFpga_BitStream_Load(&XFpgaInstance, data, KeyAddr, size, PDI_LOAD);

	return ret;
}

static int fpga_pl_pdi_download(UINTPTR data, UINTPTR size)
{
	int ret;
	XFpga XFpgaInstance = { 0U };
	UINTPTR KeyAddr = (UINTPTR)NULL;

	ret = XFpga_Initialize(&XFpgaInstance);
	if (ret != XST_SUCCESS) {
		RMGMT_DBG("FPGA init failed %d\r\n", ret);
		return ret;
	}

	//FreeRTOS_ClearTickInterrupt();

	axigate_freeze();
	ucs_stop();

	//ret = XFpga_BitStream_Load(&XFpgaInstance, data, KeyAddr, size, PDI_LOAD);

	//RMGMT_LOG("0x%x", data);

	IO_SYNC_WRITE32(0x30701, 0xff3f0440);
	IO_SYNC_WRITE32(0xf, 0xff3f0444);
	IO_SYNC_WRITE32(0x0, 0xff3f0448);
	IO_SYNC_WRITE32(data, 0xff3f044c);
	IO_SYNC_WRITE32(0x2, 0xff330000);
	/* wait for async operation done in case of firewall trip */
	MDELAY(1000);

	ucs_start();
	MDELAY(10);

	axigate_free();
	//FreeRTOS_SetupTickInterrupt();

	return ret;
}

static int rmgmt_fpga_download(struct rmgmt_handler *rh, u32 len)
{
	int ret;
	struct axlf *axlf = (struct axlf *)rh->rh_data;
	uint64_t offset = 0;
	uint64_t size = 0;

	ret = rmgmt_xclbin_section_info(axlf, BITSTREAM_PARTIAL_PDI, &offset, &size);
	if (ret) {
		RMGMT_LOG("get PARTIAL PDI from xclbin failed %d", ret);
		goto out;
	}
	/* Sync data from cache to memory */
	Xil_DCacheFlush();

	ret = fpga_pl_pdi_download((UINTPTR)((const char *)axlf + offset),
		(UINTPTR)size);
	if (ret != XFPGA_SUCCESS) {
		RMGMT_LOG("FPGA load pdi failed %d\r\n", ret);
		goto out;
	}
	
	/*
	 * The xclbin might has PDI section for APU
	 */
	ret = rmgmt_xclbin_section_info(axlf, PDI, &offset, &size);
	if (ret) {
		RMGMT_LOG("no APU PDI in xclbin, skip. ret: %d", ret);
		ret = 0;
		goto out;
	}

	/*
	 * TODO: to make sure each time we can download new PDI successfully
	 * we need:
	 *   1) correct system.dtb or equivlent device config for APU PDI
	 *   2) reset the APU to the state that can be booted correctly
	 *   3) load the PDI and boot it successfully.
	 *
	 * BUT: for now just dowload the PDI, the system will boot to correct
	 *   status after first cold reboot.
	 */
	ret = fpga_pdi_download((UINTPTR)((const char *)axlf + offset),
		(UINTPTR)size);
	if (ret) {
		RMGMT_LOG("cannot download APU PDI, skip. ret: %d", ret);
		ret = 0;
		goto out;
	}

out:
	return ret;
}

int rmgmt_load_apu(struct rmgmt_handler *rh)
{
	int ret = 0;
	u32 size;
	u8 pdiHeader[OSPI_VERSAL_PAGESIZE] = { 0 };

	ret = ospi_flash_read(CL_FLASH_APU, pdiHeader, 0, OSPI_VERSAL_PAGESIZE);
	if (*(u32 *)pdiHeader != MAGIC_NUM32) {
		RMGMT_LOG("WARN: skip loading APU, magic %x is not %x\r\n",
			*(u32 *)pdiHeader, MAGIC_NUM32);
		return 0;
	}

	size = *(((u32 *)pdiHeader) + 1);
	if (size == 0) {
		RMGMT_LOG("ERR: pdi size is 0.\r\n");
		return -1;
	}
	RMGMT_DBG("APU PDI size: %d\r\n", size);

	ret = ospi_flash_read(CL_FLASH_APU, rh->rh_data, OSPI_VERSAL_PAGESIZE, size);
	if (ret)
		return ret;

	/* Sync data from cache to memory */
	Xil_DCacheFlush();

	ret = fpga_pl_pdi_download((UINTPTR)rh->rh_data, (UINTPTR)size);

	return ret;
}

static int rmgmt_ospi_rpu_download(struct rmgmt_handler *rh, u32 len)
{
	int ret;

	/* Sync data from cache to memory */
	Xil_DCacheFlush();

	/* erase */
	ret = ospi_flash_erase(CL_FLASH_BOOT, 0, len);
	if (ret) {
		RMGMT_LOG("OSPI fails to load pdi %d", ret);
		goto out;
	}

	/* write */
	ret = ospi_flash_write(CL_FLASH_BOOT, rh->rh_data, 0, len);
	if (ret) {
		RMGMT_LOG("OSPI fails to load pdi %d", ret);
		goto out;
	}

	/*TODO: need authentication */
out:
	return ret;
}

static int rmgmt_ospi_apu_download(struct rmgmt_handler *rh, u32 len)
{
	int ret = 0;

	Xil_DCacheFlush();

	ret = fpga_pl_pdi_download((UINTPTR)rh->rh_data, (UINTPTR)len);

	return ret;
}

/* Temporary disable old apu design flow */
#if 0
static int rmgmt_ospi_apu_download(struct rmgmt_handler *rh, u32 len)
{
	int ret;
	u8 pdiHeader[OSPI_VERSAL_PAGESIZE] = { 0 };

	RMGMT_LOG("-> rmgmt_ospi_apu_download\r\n");

	*((u32 *)pdiHeader) = MAGIC_NUM32;
	*((u32 *)pdiHeader + 1) = len;

	/* Sync data from cache to memory */
	Xil_DCacheFlush();

	ret = ospi_flash_erase(CL_FLASH_APU, 0, OSPI_VERSAL_PAGESIZE + len);
	if (ret) {
		set_status(rh, XRT_XFR_PKT_STATUS_FAIL);
		RMGMT_DBG("OSPI fails to load pdi %d\r\n", ret);
		goto out;
	}

	/* flash rpu base pdi from offset 0(RPU_PDI_ADDRES) */
	ret = ospi_flash_write(CL_FLASH_APU, pdiHeader,
		0, OSPI_VERSAL_PAGESIZE);
	if (ret) {
		set_status(rh, XRT_XFR_PKT_STATUS_FAIL);
		RMGMT_DBG("OSPI fails to load pdi %d\r\n", ret);
		goto out;
	}

	ret = ospi_flash_write(CL_FLASH_APU, rh->rh_data,
		OSPI_VERSAL_PAGESIZE, len);
	if (ret) {
		set_status(rh, XRT_XFR_PKT_STATUS_FAIL);
		RMGMT_DBG("OSPI fails to load pdi %d\r\n", ret);
		goto out;
	}

	set_status(rh, XRT_XFR_PKT_STATUS_DONE);
out:
	RMGMT_LOG("<- rmgmt_ospi_apu_download %d\r\n", ret);
	return ret;
}
#endif

static int rmgmt_recv_pkt(struct rmgmt_handler *rh, u32 *len)
{
	int ret;

	RMGMT_DBG("-> rmgmt_recv_pkt\r\n");

	ret = rmgmt_data_receive(rh, len);
	if (ret) {
		set_status(rh, XRT_XFR_PKT_STATUS_FAIL);
		RMGMT_DBG("Fail to receive pkt %d\r\n", ret);
		goto out;
	}

	/*TODO: verify signature */

	RMGMT_DBG("<- rmgmt_recv_pkt\r\n");
out:
	return ret;
}

static void rmgmt_done_pkt(struct rmgmt_handler *rh)
{
	wait_for_status(rh, XRT_XFR_PKT_STATUS_IDLE);

	set_version(rh);

	RMGMT_DBG("<-");
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
		RMGMT_DBG("rmgmt_ops cannot be NULL\r\n");
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
