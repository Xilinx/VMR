#ifndef __CL_XGQ_PLAT_H_
#define __CL_XGQ_PLAT_H_

#include "cl_io.h"
#include "cl_log.h"
#define ____cacheline_aligned_in_smp

static inline void xgq_mem_write32(u64 io_hdl, u64 addr, u32 val)
{
	IO_SYNC_WRITE32(val, addr);
}

static inline void xgq_reg_write32(u64 io_hdl, u64 addr, u32 val)
{
	IO_SYNC_WRITE32(val, addr);
}

static inline u32 xgq_mem_read32(u64 io_hdl, u64 addr)
{
	return IO_SYNC_READ32(addr);
}

static inline u32 xgq_reg_read32(u64 io_hdl, u64 addr)
{
	return IO_SYNC_READ32(addr);
}

#define XGQ_IMPL
#include "xgq_impl.h"

#endif
