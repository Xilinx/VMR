/******************************************************************************
* Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#ifndef RMGMT_CLOCK_H
#define RMGMT_CLOCK_H

#define CLOCK_COUNTER_MAX_RES		4
#define CLOCK_WIZ_MAX_RES		4

#define OCL_CLKWIZ_STATUS_OFFSET        0x4
#define OCL_CLKWIZ_STATUS_MASK          0xffff
#define OCL_CLKWIZ_STATUS_MEASURE_START 0x1
#define OCL_CLKWIZ_STATUS_MEASURE_DONE  0x2
#define OCL_CLKWIZ_CONFIG_OFFSET(n)     (0x200 + 4 * (n))
#define OCL_CLK_FREQ_COUNTER_OFFSET     0x8
#define OCL_CLK_FREQ_V5_COUNTER_OFFSET  0x10
#define OCL_CLK_FREQ_V5_CLK0_ENABLED    0x10000

/* REGs for ACAP Versal */
#define OCL_CLKWIZ_INIT_CONFIG          0x14
#define OCL_CLKWIZ_DIVCLK               0x380
#define OCL_CLKWIZ_DIVCLK_TS            0x384
#define OCL_CLKWIZ_CLKFBOUT             0x330
#define OCL_CLKWIZ_CLKFBOUT_TS          0x334
#define OCL_CLKWIZ_CLKFBOUT_FRACT       0x3fc
#define OCL_CLKWIZ_CLKOUT0              0x338
#define OCL_CLKWIZ_CLKOUT0_TS           0x33c

#define CLK_MAX_VALUE           6400
#define CLK_SHUTDOWN_BIT        0x1
#define DEBUG_CLK_SHUTDOWN_BIT  0x2
#define VALID_CLKSHUTDOWN_BITS  (CLK_SHUTDOWN_BIT|DEBUG_CLK_SHUTDOWN_BIT)

#define CLK_ACAP_MAX_VALUE_FOR_O        4320
//#define CLK_ACAP_INPUT_FREQ             33.333
/* no float number in kernel, x/33.333 will be converted to x * 1000 / 33333) */
//#define CLK_ACAP_INPUT_FREQ_X_1000      33333
#define CLK_ACAP_INPUT_FREQ             100.000 
#define CLK_ACAP_INPUT_FREQ_X_1000      100000

#define CLK_TYPE_DATA   0
#define CLK_TYPE_KERNEL 1
#define CLK_TYPE_SYSTEM 2
#define CLK_TYPE_MAX    4

struct acap_clkfbout {
        u32 clkfbout_dt         :8;
        u32 clkfbout_edge       :1;
        u32 clkfbout_en         :1;
        u32 clkfbout_mx         :2;
        u32 clkfbout_prediv2    :1;
        u32 reserved            :19;
};

struct acap_clkfbout_ts {
        u32 clkfbout_lt         :8;
        u32 clkfbout_ht         :8;
        u32 reserved            :16;
};

struct acap_clkout0 {
        u32 clkout0_dt          :8;
        u32 clkout0_edge        :1;
        u32 clkout0_mx          :2;
        u32 clkout0_prediv2     :1;
        u32 clkout0_used        :1;
        u32 clkout0_p5en        :1;
        u32 clkout0_start_h     :1;
        u32 clkout0_p5_edge     :1;
        u32 reserved            :16;
};

struct acap_clkout0_ts {
        u32 clkout0_lt          :8;
        u32 clkout0_ht          :8;
        u32 reserved            :16;
};

struct acap_divclk {
        u32 deskew_dly_2nd      :6;
        u32 deskew_dly_en_2nd   :1;
        u32 deskew_dly_path_2nd :1;
        u32 deskew_en_2nd       :1;
        u32 direct_path_cntrl   :1;
        u32 divclk_edge         :1;
        u32 reserved            :21;
};

struct acap_divclk_ts {
        u32 divclk_lt           :8;
        u32 divclk_ht           :8;
        u32 reserved            :16;
};

struct acap_clkfbout_fract {
        u32 clkfbout_fract_alg  :1;
        u32 clkfbout_fract_en   :1;
        u32 clkfbout_fract_order:1;
        u32 clkfbout_fract_seed :2;
        u32 skew_sel            :6;
        u32 reserved            :21;
};

struct cl_msg;

enum clock_ip {
	RMGMT_CLOCK_WIZARD,
	RMGMT_CLOCK_COUNTER,
};

int rmgmt_clock_freq_scaling(struct cl_msg *msg);
uint32_t rmgmt_clock_get_freq(int req_id, enum clock_ip ip);
#endif
