#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <cmocka.h>

#include "xil_types.h"
#include "vmc_update_sc.h"

u8 fpt_sc_version[MAX_SC_VERSION_SIZE] = {0x00};
