#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <stdlib.h>
#include <xil_types.h>
#include <cmocka.h>

#include "./cl_io.h"

int __wrap_cl_memcpy_toio8(u32 dst, void *buf, size_t len)
{
    return 0;
}

void IO_SYNC_WRITE32(u32 val, u32 addr)
{
    return;
} 

u32 IO_SYNC_READ32(u32 addr)
{
    return 0x12345678;
} 
