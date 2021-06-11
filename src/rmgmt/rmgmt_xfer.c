/******************************************************************************
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#include "xilfpga.h"
#include "rmgmt_main.h"
#include "rmgmt_xclbin.h"
#include "rmgmt_xfer.h"
#include "rmgmt_ospi.h"

#define PR_ISOLATION_REG 0x80000000
#define PR_ISOLATION_FREEZE 0x0
#define PR_ISOLATION_UNFREEZE 0x3

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

static void print_pkg(u8 *data, int len) {

	for (int i = 0; i < len;) {
		RMGMT_DBG("%02x ", data[i]);
		if (++i % 16 == 0)
			RMGMT_DBG("\r\n");
	}
}

#if 0
static int zocl_ov_receive(struct zocl_ov_dev *ov)
{
	struct zocl_ov_pkt_node *node = ov->head;
	u32 *ov_base = (u32 *)ov->base;
	int len = 0, next = 0;
	int ret;
	int a = 1;

	RMGMT_DBG("-> zocl_ov_receive \r\n");
	for (;;) {
		u32 pkt_header;
		struct pdi_packet *pkt = (struct pdi_packet *)&pkt_header;
		struct zocl_ov_pkt_node *new;
		int lastpkt;


		/* Busy wait here until we get a new packet */
		pkt_header = wait_for_status(ov, XRT_XFR_PKT_STATUS_NEW);
		lastpkt = pkt->pkt_flags & XRT_XFR_PKT_FLAGS_LAST;

		new = malloc(sizeof (struct zocl_ov_pkt_node));
		if (!new) {
			RMGMT_DBG("pkt_node: no mem %d", sizeof (struct zocl_ov_pkt_node));
			ret = -1;
			break;
		}
		new->zn_datap = malloc(ov->size);
		if (!new->zn_datap) {
			//			free(new); memory leaks?
			free(new);
			RMGMT_DBG("zn_datap: no mem %d\r\n", ov->size);
			ret = -1;
			break;
		}
		new->zn_size = pkt->pkt_size;

		/* Read packet data payload on a 4 bytes base */
		read_data32(ov_base + (sizeof(struct pdi_packet)) / 4,
			new->zn_datap, (ov->size - sizeof(struct pdi_packet)) / 4);

		if (a) {
			RMGMT_DBG("pkt_size: %d \r\n", new->zn_size);
			print_pkg((u8 *)new->zn_datap, 8);
			a = 0;
		}
		/* Notify host that the data has been read */
		set_status(ov, XRT_XFR_PKT_STATUS_IDLE);

		len += ov->size;

		if ((len / 1000000) > next) {
			RMGMT_DBG("\r%d M", len / 1000000);
			fflush(stdout);
			next++;
		}

		/* Add packet data to linked list */
		if (node)
			node->zn_next = new;
		else
			ov->head = new;
		node = new;

		/* Bail out here if this is the last packet */
		if (lastpkt) {
			ret = 0;
			break;
		}
	}
	RMGMT_DBG("<- zocl_ov_receive \r\n");
	return ret;
}

static int zocl_ov_to_data(struct zocl_ov_dev *ov, u8 **data, u32 *length)
{
	struct zocl_ov_pkt_node *node = ov->head;
	u8 *xp;
	size_t len = 0;
	int flag = 1;

	RMGMT_DBG("-> zocl_ov_to_data \r\n");

	while (node) {
		len += node->zn_size;
		node = node->zn_next;
	}

	if (len == 0) {
		RMGMT_LOG("ERR: received 0 data.\r\n");
		return -1;
	}

	*data = malloc(len);
	if (!(*data)) {
		RMGMT_LOG("ERR: failed to allocate data buffer\r\n");
		return -1;
	}

	xp = *data;
	node = ov->head;
	while (node) {
		memcpy(xp, node->zn_datap, node->zn_size);
		xp += node->zn_size;
		node = node->zn_next;

		if (flag) {
			print_pkg((*data), 8);
			flag = 0;
		}
	}

	*length = len;

	RMGMT_DBG("<- zocl_ov_to_data len: %d \r\n", len);

	return 0;
}

static void* rtos_memcpy( void *pvDest, const void *pvSource, size_t ulBytes )
{
	unsigned char *pcDest = ( unsigned char * ) pvDest, *pcSource = ( unsigned char * ) pvSource;
	size_t x;

	for( x = 0; x < ulBytes; x++ )
	{
		*pcDest = *pcSource;
		pcDest++;
		pcSource++;
	}

	/* Sync data from cache to memory */
	Xil_DCacheFlush();

	RMGMT_LOG("x %lld, size %lld\r\n", x, ulBytes);

	return pvDest;
}
#endif



static int rmgmt_data_receive(struct rmgmt_handler *rh, u32 *len)
{
	u32 offset = 0, next = 0;
	int ret;
	int a = 0;

	RMGMT_DBG("-> rmgmt_data_receive \r\n");
	for (;;) {
		u32 pkt_header;
		struct pdi_packet *pkt = (struct pdi_packet *)&pkt_header;
		int lastpkt;

		/* Busy wait here until we get a new packet */
		pkt_header = wait_for_status(rh, XRT_XFR_PKT_STATUS_NEW);
		lastpkt = pkt->pkt_flags & XRT_XFR_PKT_FLAGS_LAST;

		if ((offset + pkt->pkt_size) > rh->rh_data_size) {
			RMGMT_LOG("max: %d M, received %d M\r\n",
				rh->rh_data_size / 0x100000,
				(offset + pkt->pkt_size) / 0x100000);
			return -1;
		}
		/* Read packet data payload on a 4 bytes base */
		read_data32((u32 *)rh->rh_base + (sizeof(struct pdi_packet)) / 4,
			(u32 *)(rh->rh_data) + offset / 4,
			pkt->pkt_size / 4);

		if (a == 0) {
			RMGMT_LOG("pkt_size %d, xfer size %d\r\n",
				pkt->pkt_size, (XRT_XFR_RES_SIZE - sizeof(struct pdi_packet)));
			a++;
		}
		/* Notify host that the data has been read */
		set_status(rh, XRT_XFR_PKT_STATUS_IDLE);

		/* Set len to next offset */
		offset += pkt->pkt_size;
		if ((offset / 0x100000) > next) {
			RMGMT_DBG("\r%d M", offset / 0x100000);
			fflush(stdout);
			next++;
		}

		/* Bail out here if this is the last packet */
		if (lastpkt) {
			RMGMT_DBG("\r\n");
			ret = 0;
			break;
		}
	}

	*len = offset;
	RMGMT_DBG("<- rmgmt_data_receive, len:%d\r\n", *len);
	return ret;
}

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

int rmgmt_init_handler(struct rmgmt_handler *rh)
{
	rh->rh_base = OSPI_VERSAL_BASE;

	if (rmgmt_init_xfer(rh) != 0)
		return -1;

	rh->rh_data_size = BITSTREAM_SIZE * 2; /* 32M */
	rh->rh_data = (u8 *)malloc(rh->rh_data_size);
	if (rh->rh_data == NULL) {
		RMGMT_LOG("malloc %d bytes failed\r\n", rh->rh_data_size);
		return -1;
	}

	set_version(rh);

	set_status(rh, XRT_XFR_PKT_STATUS_IDLE);

	RMGMT_DBG("rmgmt_init_handler done\r\n");
	return 0;
}

static int rmgmt_fpga_download(struct rmgmt_handler *rh, u32 len)
{
	XFpga XFpgaInstance = { 0U };
	int ret;
	struct axlf *axlf = (struct axlf *)rh->rh_data;
	uint64_t offset = 0;
	uint64_t size = 0;

	RMGMT_LOG("-> rmgmt_fpga_download\r\n");

	RMGMT_LOG(" head should be 0x78 0x63 0x6c 0x62 etc. \r\n");
	print_pkg(rh->rh_data, 16);

	ret = XFpga_Initialize(&XFpgaInstance);
	if (ret != XST_SUCCESS) {
		set_status(rh, XRT_XFR_PKT_STATUS_FAIL);
		RMGMT_DBG("FPGA init failed %d\r\n", ret);
		goto out;
	}

	ret = rmgmt_xclbin_section_info(axlf, BITSTREAM_PARTIAL_PDI, &offset, &size);
	if (ret) {
		set_status(rh, XRT_XFR_PKT_STATUS_FAIL);
		RMGMT_DBG("get PARTIAL PDI from xclbin failed %d\r\n", ret);
		goto out;
	}

	/* Sync data from cache to memory */
	Xil_DCacheFlush();

	/* isolate PR gate */
	//IO_SYNC_WRITE32(PR_ISOLATION_FREEZE, rh->rh_base + PR_ISOLATION_REG);

	ret = XFpga_PL_BitStream_Load(&XFpgaInstance,
		(UINTPTR)((const char *)axlf + offset), (UINTPTR)size, PDI_LOAD);

	//IO_SYNC_WRITE32(PR_ISOLATION_UNFREEZE, rh->rh_base + PR_ISOLATION_REG);

	if (ret != XFPGA_SUCCESS) {
		set_status(rh, XRT_XFR_PKT_STATUS_FAIL);
		RMGMT_DBG("FPGA load pdi failed %d\r\n", ret);
		goto out;
	}

	set_status(rh, XRT_XFR_PKT_STATUS_DONE);
out:

	RMGMT_LOG("<- rmgmt_fpga_download %d\r\n", ret);
	return ret;
}

static int rmgmt_ospi_download(struct rmgmt_handler *rh, u32 len)
{
	int retry = 0;
	int ret;
	struct axlf *axlf = (struct axlf *)rh->rh_data;
	uint64_t offset = 0;
	uint64_t size = 0;

	RMGMT_LOG("-> rmgmt_ospi_download\r\n");

	ret = rmgmt_xclbin_section_info(axlf, PDI, &offset, &size);
	if (ret) {
		set_status(rh, XRT_XFR_PKT_STATUS_FAIL);
		RMGMT_DBG("get PDI from xsabin failed %d\r\n", ret);
		goto out;
	}
	/* Sync data from cache to memory */
	//Xil_DCacheFlush();

	ret = ospi_flush_polled(rh->rh_data + offset, size);
	while (ret != 0 && retry++ < 10) {
		RMGMT_DBG("ospi_flush retrying... %d\r\n", retry);
		ret = ospi_flush_polled(rh->rh_data + offset, size);
	}

	if (ret) {
		set_status(rh, XRT_XFR_PKT_STATUS_FAIL);
		RMGMT_DBG("OSPI fails to load pdi %d\r\n", ret);
		goto out;
	}


	set_status(rh, XRT_XFR_PKT_STATUS_DONE);

out:
	RMGMT_LOG("<- rmgmt_ospi_download %d\r\n", ret);
	return ret;
}

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

	RMGMT_DBG("<- rmgmt_done_pkt\r\n");
}

struct rmgmt_ops {
	int (*rmgmt_recv_op)(struct rmgmt_handler *rh, u32 *len);
	int (*rmgmt_download_op)(struct rmgmt_handler *rh, u32 len);
	void (*rmgmt_done_op)(struct rmgmt_handler *rh);
};

static struct rmgmt_ops xsabin_ops = {
	.rmgmt_recv_op = rmgmt_recv_pkt,
	.rmgmt_download_op = rmgmt_ospi_download,
	.rmgmt_done_op = rmgmt_done_pkt,
};

static struct rmgmt_ops xclbin_ops = {
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

int rmgmt_download_xclbin(struct rmgmt_handler *rh)
{
	return rmgmt_xfer_download(rh, &xclbin_ops);
}

int rmgmt_download_xsabin(struct rmgmt_handler *rh)
{
	return rmgmt_xfer_download(rh, &xsabin_ops);
}
