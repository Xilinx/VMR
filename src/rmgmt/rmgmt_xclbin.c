/******************************************************************************
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#include "rmgmt_main.h"
#include "rmgmt_xclbin.h"

static const struct axlf_section_header *
rmgmt_xclbin_get_section_hdr(const struct axlf *xclbin,
		enum axlf_section_kind kind)
{
	int i = 0;

	RMGMT_DBG("looking for kind %d \r\n", kind);

	/* Sanity check. */
	if (xclbin->m_header.m_numSections > XCLBIN_MAX_NUM_SECTION)
		return NULL;

	for (i = 0; i < xclbin->m_header.m_numSections; i++) {
		RMGMT_DBG("find kind %d\r\n", xclbin->m_sections[i].m_sectionKind);
		if (xclbin->m_sections[i].m_sectionKind == kind)
			return &xclbin->m_sections[i];
	}

	return NULL;
}

static int
rmgmt_xclbin_check_section_hdr(const struct axlf_section_header *header,
        uint64_t xclbin_len)
{
        return (header->m_sectionOffset + header->m_sectionSize) > xclbin_len ?
                -1 : 0;
}

int
rmgmt_xclbin_section_info(const struct axlf *xclbin, enum axlf_section_kind kind,
        uint64_t *offset, uint64_t *size)
{
        const struct axlf_section_header *memHeader = NULL;
        uint64_t xclbin_len;
        int err = 0;

        RMGMT_DBG("magic %s\r\n", xclbin->m_magic);

        memHeader = rmgmt_xclbin_get_section_hdr(xclbin, kind);
        if (!memHeader) {
        	xil_printf("no section for kind %d\r\n", kind);
        	return -1;
        }

        xclbin_len = xclbin->m_header.m_length;
        err = rmgmt_xclbin_check_section_hdr(memHeader, xclbin_len);
        if (err) {
        	xil_printf("check section header failed, len%d\r\n", xclbin_len);
        	return err;
        }

        *offset = memHeader->m_sectionOffset;
        *size = memHeader->m_sectionSize;

        xil_printf("Found section @ 0x%llx, size %lld\r\n", *offset, *size);
        return 0;
}

/* caller should free the allocated memory for **data */
//int rmgmt_xclbin_get_section(const struct axlf *xclbin, enum axlf_section_kind kind,
//		void **data, uint64_t *len)
//{
//        void *section = NULL;
//        int err = 0;
//        uint64_t offset = 0;
//        uint64_t size = 0;
//
//        err = rmgmt_xclbin_section_info(xclbin, kind, &offset, &size);
//        if (err)
//                return err;
//
//        section = malloc(size);
//        if (section == NULL)
//                return -1;
//
//        memcpy(section, ((const char *)xclbin) + offset, size);
//
//        *data = section;
//        *len = size;
//
//        return 0;
//}

