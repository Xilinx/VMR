/******************************************************************************
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#include "xilfpga.h"
#include "rmgmt_main.h"
#include "rmgmt_xclbin.h"
#include "rmgmt_xfer.h"
#include "rmgmt_ospi.h"

static inline u32 wait_for_status(struct zocl_ov_dev *ov, u8 status)
{
	u32 header;
	struct pdi_packet *pkt = (struct pdi_packet *)&header;

        for (;;) {
                header = IO_SYNC_READ32(ov->base);
                if (pkt->pkt_status == status)
                        break;
        }

        return header;
}

static inline u8 get_pkt_flags(struct zocl_ov_dev *ov)
{
        struct pdi_packet *pkt;
        u32 header;

        pkt = (struct pdi_packet *)&header;
        header = IO_SYNC_READ32(ov->base);
        return pkt->pkt_flags;
}

u8 rmgmt_get_pkt_flags(struct zocl_ov_dev *ov)
{
	return get_pkt_flags(ov);
}

static inline int check_for_status(struct zocl_ov_dev *ov, u8 status)
{
        struct pdi_packet *pkt;
        u32 header;

        pkt = (struct pdi_packet *)&header;
        header = IO_SYNC_READ32(ov->base);

        return (pkt->pkt_status == status);
}

int rmgmt_check_for_status(struct zocl_ov_dev *ov, u8 status)
{
	return check_for_status(ov, status);
}

static inline void set_flags(struct zocl_ov_dev *ov, u8 flags)
{
        struct pdi_packet *pkt;
        u32 header;

        pkt = (struct pdi_packet *)&header;
        header = IO_SYNC_READ32(ov->base);
        pkt->pkt_flags = flags;

        IO_SYNC_WRITE32(pkt->header, ov->base);
}

static inline void set_version(struct zocl_ov_dev *ov)
{
        set_flags(ov, XRT_XFR_VER <<  XRT_XFR_PKT_VER_SHIFT);
}

static inline void set_status(struct zocl_ov_dev *ov, u8 status)
{
		volatile struct pdi_packet *pkt;
		volatile u32 header;

        header = IO_SYNC_READ32(ov->base);
        pkt = (struct pdi_packet *)&header;
        pkt->pkt_status = status;

        IO_SYNC_WRITE32(pkt->header, ov->base);
}

static inline void read_data(u32 *addr, u32 *data, size_t sz)
{
	u32 i;

	for (i = 0; i < sz; ++i) {
		*(data + i) = IO_SYNC_READ32((INTPTR)(addr + i));
	}
}

static void print_pkg(u8 *data, int len) {

	for (int i = 0; i < len; i++)
		RMGMT_DBG("%d:[0x%02x] \r\n", i, data[i]);
}

static int zocl_ov_receive(struct zocl_ov_dev *ov)
{
	struct zocl_ov_pkt_node *node = ov->head;
	u32 *base = (u32 *)ov->base;
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
			xil_printf("pkt_node: no mem %d", sizeof (struct zocl_ov_pkt_node));
			ret = -1;
			break;
		}
		new->zn_datap = malloc(ov->size);
		if (!new->zn_datap) {
			//			free(new); memory leaks?
			xil_printf("zn_datap: no mem %d", ov->size);
			ret = -1;
			break;
		}
		new->zn_size = pkt->pkt_size;

		/* Read packet data payload on a 4 bytes base */
		read_data(base + (sizeof(struct pdi_packet)) / 4,
				new->zn_datap, (ov->size - sizeof(struct pdi_packet)) / 4);

		if (a) {
			xil_printf("pkt_size: %d \r\n", new->zn_size);
			print_pkg((u8 *)new->zn_datap, 8);
			a = 0;
		}
		/* Notify host that the data has been read */
		set_status(ov, XRT_XFR_PKT_STATUS_IDLE);

		len += ov->size;

		if ((len / 1000000) > next) {
			xil_printf("\r%d M", len / 1000000);
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

	return pvDest;
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

static void zocl_ov_clean(struct zocl_ov_dev *ov)
{
        struct zocl_ov_pkt_node *node;
        struct zocl_ov_pkt_node *pnode;

        node = ov->head;
        while (node != NULL) {
                pnode = node;
                node = pnode->zn_next;
                free(pnode->zn_datap);
                free(pnode);
        }

        ov->head = NULL;
}

int rmgmt_init_xfer(struct zocl_ov_dev *ov)
{
	ov->base = OSPI_VERSAL_BASE;
	ov->size = XRT_XFR_RES_SIZE;
	ov->head = NULL;

	if (ov->size & 0x3) {
		xil_printf("xfer size: %d is not 32bit aligned\r\n", ov->size);
		return -1;
	}

	for (int i = 0; i < ov->size; i += 4) {
		IO_SYNC_WRITE32(0x0, ov->base + i * 4);
	}

	set_version(ov);

	set_status(ov, XRT_XFR_PKT_STATUS_IDLE);

	return 0;
}

static int rmgmt_download_xclbin(struct zocl_ov_dev *ov, u8 *buf, u32 len)
{
	/*Note: we have to write PDI into this location then load */
	u32 addr = XFPGA_BASE_ADDRESS; 
	XFpga XFpgaInstance = { 0U };
	int ret;
	struct axlf *axlf = (struct axlf *)buf;
	uint64_t offset = 0;
	uint64_t size = 0;

	ret = rmgmt_xclbin_section_info(axlf, BITSTREAM_PARTIAL_PDI, &offset, &size);
	if (ret) {
		set_status(ov, XRT_XFR_PKT_STATUS_FAIL);
		xil_printf("get PDI from xsabin failed %d\r\n", ret);
		goto out;
	}

	rtos_memcpy((u8 *)addr, buf+offset, size);

	ret = XFpga_Initialize(&XFpgaInstance);
	if (ret != XST_SUCCESS) {
		set_status(ov, XRT_XFR_PKT_STATUS_FAIL);
		xil_printf("FPGA init failed %d\r\n", ret);
		goto out;
	}

	/*TODO: isolate PR gate */
	ret = XFpga_PL_BitStream_Load(&XFpgaInstance, addr, (UINTPTR)size, PDI_LOAD);
	if (ret != XFPGA_SUCCESS) {
		set_status(ov, XRT_XFR_PKT_STATUS_FAIL);
		xil_printf("FPGA load pdi failed %d\r\n", ret);
		goto out;
	}

	xil_printf("FPGA load pdi finished.\r\n");

	set_status(ov, XRT_XFR_PKT_STATUS_DONE);

out:
	return ret;
}

static int rmgmt_download_xsabin(struct zocl_ov_dev *ov, u8 *buf, u32 len)
{
	int retry = 0;
	int ret;
	struct axlf *axlf = (struct axlf *)buf;
	uint64_t offset = 0;
	uint64_t size = 0;

	RMGMT_LOG("-> rmgmt_download_xsabin\r\n");

	ret = rmgmt_xclbin_section_info(axlf, PDI, &offset, &size);
	if (ret) {
		set_status(ov, XRT_XFR_PKT_STATUS_FAIL);
		xil_printf("get PDI from xsabin failed %d\r\n", ret);
		goto out;
	}

	ret = ospi_flush_polled(buf + offset, size);
	while (ret != 0 && retry++ < 10) {
		xil_printf("ospi_flush retrying... %d\r\n", retry);
		ret = ospi_flush_polled(buf + offset, size);
	}

	if (ret) {
		set_status(ov, XRT_XFR_PKT_STATUS_FAIL);
		xil_printf("OSPI fails to load pdi %d\r\n", ret);
		goto out;
	}

	RMGMT_LOG("<- rmgmt_download_xsabin\r\n");

	set_status(ov, XRT_XFR_PKT_STATUS_DONE);

out:
	return ret;
}

static int rmgmt_recv_pkt(struct zocl_ov_dev *ov, u8 **buf, u32 *len)
{
	int ret;

	RMGMT_DBG("-> rmgmt_recv_pkt\r\n");

	ret = zocl_ov_receive(ov);
	if (ret) {
		set_status(ov, XRT_XFR_PKT_STATUS_FAIL);
		xil_printf("Fail to receive pkt %d\r\n", ret);
		goto out;
	}

	ret = zocl_ov_to_data(ov, buf, len);
	if (ret) {
		set_status(ov, XRT_XFR_PKT_STATUS_FAIL);
		xil_printf("Fail to copy pkt %d\r\n", ret);
		goto out;
	}

	print_pkg((*buf), 8);

	/*XXX verify signature */

	RMGMT_DBG("<- rmgmt_recv_pkt\r\n");

out:
	return ret;
}

static void rmgmt_done_pkt(struct zocl_ov_dev *ov, u8 *buf)
{
	zocl_ov_clean(ov);
	if (buf)
		free(buf);

	wait_for_status(ov, XRT_XFR_PKT_STATUS_IDLE);
	set_version(ov);

	xil_printf("rmgmt_done_pkt%d\r\n");
}

struct rmgmt_ops {
	int (*rmgmt_recv_op)(struct zocl_ov_dev *ov, u8 **buf, u32 *len);
	int (*rmgmt_download_op)(struct zocl_ov_dev *ov, u8 *buf, u32 len);
	void (*rmgmt_done_op)(struct zocl_ov_dev *ov, u8 *buf);
};

static struct rmgmt_ops xsabin_ops = {
	.rmgmt_recv_op = rmgmt_recv_pkt,
	.rmgmt_download_op = rmgmt_download_xsabin,
	.rmgmt_done_op = rmgmt_done_pkt,
};

static struct rmgmt_ops xclbin_ops = {
	.rmgmt_recv_op = rmgmt_recv_pkt,
	.rmgmt_download_op = rmgmt_download_xclbin,
	.rmgmt_done_op = rmgmt_done_pkt,
};

static int rmgmt_xfer_download(struct zocl_ov_dev *ov, struct rmgmt_ops *ops)
{
	u8 *buffer = NULL;
	u32 len;
	int ret;

	if (ops == NULL) {
		xil_printf("rmgmt_ops cannot be NULL\r\n");
		return -1;
	}

	ret = ops->rmgmt_recv_op(ov, &buffer, &len);
	if (ret)
		goto done;

	ret = ops->rmgmt_download_op(ov, buffer, len);

done:
	ops->rmgmt_done_op(ov, buffer);
	return ret;
}

int rmgmt_get_xclbin(struct zocl_ov_dev *ov)
{
	return rmgmt_xfer_download(ov, &xclbin_ops);
}

int rmgmt_get_xsabin(struct zocl_ov_dev *ov)
{
	return rmgmt_xfer_download(ov, &xsabin_ops);
}
