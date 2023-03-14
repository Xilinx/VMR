/******************************************************************************
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#include "rmgmt_common.h"
#include "rmgmt_xclbin.h"

static const struct axlf_section_header *
rmgmt_xclbin_get_section_hdr(const struct axlf *xclbin,
		enum axlf_section_kind kind)
{
	int i = 0;

	VMR_DBG("looking for kind %d \r\n", kind);

	/* Sanity check. */
	if (xclbin->m_header.m_numSections > XCLBIN_MAX_NUM_SECTION)
		return NULL;

	for (i = 0; i < xclbin->m_header.m_numSections; i++) {
		if (xclbin->m_sections[i].m_sectionKind == kind) {
			VMR_DBG("found kind[%d]= %d\r\n", i, kind);
			return &xclbin->m_sections[i];
		}
	}

	VMR_DBG("did not find kind %d from %d sections\r\n",
		kind, xclbin->m_header.m_numSections);
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

        VMR_DBG("magic %s\r\n", xclbin->m_magic);

        memHeader = rmgmt_xclbin_get_section_hdr(xclbin, kind);
        if (!memHeader)
        	return -1;

        xclbin_len = xclbin->m_header.m_length;
        err = rmgmt_xclbin_check_section_hdr(memHeader, xclbin_len);
        if (err) {
        	VMR_DBG("check section header failed, len%d\r\n", xclbin_len);
        	return err;
        }

        *offset = memHeader->m_sectionOffset;
        *size = memHeader->m_sectionSize;

        VMR_WARN("Found section offset: %lld, size: %lld\r\n", *offset, *size);
        return 0;
}

/* caller should free the allocated memory for **data */
int rmgmt_xclbin_get_section(const struct axlf *xclbin, enum axlf_section_kind kind,
		void **data, uint64_t *len)
{
        void *section = NULL;
        int err = 0;
        uint64_t offset = 0;
        uint64_t size = 0;

        err = rmgmt_xclbin_section_info(xclbin, kind, &offset, &size);
        if (err)
                return err;

        section = malloc(size);
        if (section == NULL) {
        	VMR_DBG("get section failed, no memory size%lld\r\n", size);
                return -1;
	}

        memcpy(section, ((const char *)xclbin) + offset, size);

        *data = section;
        *len = size;

        return 0;
}

void rmgmt_xclbin_section_remove(struct axlf *xclbin, enum axlf_section_kind kind)
{
	struct axlf_section_header *sectionHeaderArray = NULL;
	uint64_t bufferSize = 0;
	uint64_t startToOffset = 0;
	uint64_t startFromOffset = 0;
	uint64_t bytesToCopy = 0;
	uint64_t bytesRemoved = 0;
	void *ptrStartTo = NULL;
	void *ptrStartFrom = NULL;
	uint64_t bytesToShift = 0;

	for (uint64_t index = 0; index < xclbin->m_header.m_numSections; ++index) {

		sectionHeaderArray = &xclbin->m_sections[0];

		if (sectionHeaderArray[index].m_sectionKind != kind)
			continue;

		bufferSize = xclbin->m_header.m_length;
		startToOffset = sectionHeaderArray[index].m_sectionOffset;
		startFromOffset = ((index + 1) == xclbin->m_header.m_numSections) ?
			sectionHeaderArray[index].m_sectionOffset + sectionHeaderArray[index].m_sectionSize :
			sectionHeaderArray[index + 1].m_sectionOffset;
		bytesToCopy = bufferSize - startFromOffset;
		bytesRemoved = startFromOffset - startToOffset;

		VMR_WARN("bytesRemoved %d", (u32)bytesRemoved);

		if (bytesToCopy != 0) {
			memcpy((char *)xclbin + startToOffset, (char *)xclbin + startFromOffset, bytesToCopy);
		}
		
		/* clean up data structures */
		xclbin->m_header.m_length -= bytesRemoved;
		for (uint64_t idx = index + 1; idx < xclbin->m_header.m_numSections; ++idx) {
			sectionHeaderArray[idx].m_sectionOffset -= bytesRemoved;
		}
		/* handle last section */
		if (xclbin->m_header.m_numSections == 1) {
			xclbin->m_header.m_numSections = 0;
			sectionHeaderArray[0].m_sectionKind = 0;
			sectionHeaderArray[0].m_sectionOffset = 0;
			sectionHeaderArray[0].m_sectionSize = 0;
			continue;
		}
		/* remove the array entry */
		ptrStartTo = &sectionHeaderArray[index];
		ptrStartFrom = &sectionHeaderArray[index + 1];
		bytesToShift = xclbin->m_header.m_length - ((char *)ptrStartFrom - (char *)xclbin);
		memcpy((char *)ptrStartTo, (char *)ptrStartFrom, bytesToShift);

		/* update data elements */
		xclbin->m_header.m_numSections -= 1;
		xclbin->m_header.m_length -= sizeof(struct axlf_section_header);
		for (uint64_t idx = 0; idx < xclbin->m_header.m_numSections; ++idx) {
			sectionHeaderArray[idx].m_sectionOffset -= sizeof(struct axlf_section_header);
		}
	}
}
