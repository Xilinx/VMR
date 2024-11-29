/******************************************************************************
* Copyright (C) 2024 AMD, Inc.    All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#include "FreeRTOS.h"
#include "cl_msg.h"
#include "rmgmt_clock.h"
#include "rmgmt_common.h"

u32 clock_wiz_bases[] = {
	VMR_EP_ACLK_KERNEL_0,
	VMR_EP_ACLK_KERNEL_1,
	VMR_EP_ACLK_HBM_0,
	VMR_EP_ACLK_SHUTDOWN_0,
};

u32 clock_counter_bases[] = {
	VMR_EP_ACLK_FREQ_KERNEL_0,
	VMR_EP_ACLK_FREQ_KERNEL_1,
	VMR_EP_ACLK_FREQ_HBM,
	VMR_EP_ACLK_FREQ_K1_K2,
};

static inline int clock_wiz_busy(int idx, int cycle, int interval)
{
        u32 val = 0;
        int count;

        val = IO_SYNC_READ32(clock_wiz_bases[idx] + OCL_CLKWIZ_STATUS_OFFSET);
        for (count = 0; val != 1 && count < cycle; count++) {
		MDELAY(interval);
                val = IO_SYNC_READ32(clock_wiz_bases[idx] + OCL_CLKWIZ_STATUS_OFFSET);
        }
        if (val != 1) {
                VMR_WARN("clockwiz(%d) is (%u) busy after %d ms",
                    idx, val, cycle * interval);
                return -1;
        }

        return 0;
}

static inline unsigned int floor_acap_o(int freq)
{
        return (CLK_ACAP_MAX_VALUE_FOR_O / freq);
}

/*
 * Kernel compiler even has disabled SSE(floating caculation) for preprocessor,
 * we need a simple math to count floor without losing too much accuracy.
 * formula: (O * freq / 33.333)
 */
static inline unsigned int floor_acap_m(int freq)
{
        return (floor_acap_o(freq) * freq * 1000 / CLK_ACAP_INPUT_FREQ_X_1000);
}

static int rmgmt_clock_freq_scaling_impl(struct cl_msg *msg, bool force)
{
        struct acap_divclk              *divclk;
        struct acap_divclk_ts           *divclk_ts;
        struct acap_clkfbout_fract      *fract;
        struct acap_clkfbout            *clkfbout;
        struct acap_clkfbout_ts         *clkfbout_ts;
        struct acap_clkout0             *clkout0;
        struct acap_clkout0_ts          *clkout0_ts;
        unsigned int M, O;
	u32 val;
	int err = 0;
	int i = 0;
	int num_clock = msg->clock_payload.ocl_req_num;

	for (i = 0; i < num_clock; i++) {
		u32 freq = msg->clock_payload.ocl_req_freq[i];
		if (freq == 0)
			continue;

		VMR_LOG("Clock: %d, force:%d, New: %d Mhz", i, force, freq);
		
		if (!force) {
			u32 cur_freq = rmgmt_clock_get_freq(i, RMGMT_CLOCK_COUNTER);
			VMR_LOG("current freq: %d Mhz", cur_freq);
			if (cur_freq == freq) {
				VMR_LOG("request is the same as current, skip.");
				continue;
			}
		}

                err = clock_wiz_busy(i, 20, 50);
                if (err)
                        break;
                /*
                 * Simplified formula for ACAP clock wizard.
                 * 1) Set DIVCLK_EDGE, DIVCLK_LT and DIVCLK_HT to 0;
                 * 2) Set CLKFBOUT_FRACT_EN to 0;
                 * 3) O = floor(4320/freq_req), M = floor((O*freq_req)/33.333);
                 * 4) CLKFBOUT_EDGE = if M%2 write 0x17, else write 0x16
                 * 5) CLKFBOUT_LT_HT = (M-M%2)/2_(M-M%2)/2
                 * 6) check CLKOUT0_PREDIV2, CLKOUT0_P5EN == 0
                 * 7) CLKOUT0_EDGE O%2 write 0x13, else write 0x12
                 * 8) CLKOUT0_LT_HT = (O-(O%2))/2
                 */
                /* Step 1) */
                val = IO_SYNC_READ32(clock_wiz_bases[i] + OCL_CLKWIZ_DIVCLK);
                divclk = (struct acap_divclk *)&val;
                divclk->divclk_edge = 0;
                IO_SYNC_WRITE32(val, clock_wiz_bases[i] + OCL_CLKWIZ_DIVCLK);

                val = IO_SYNC_READ32(clock_wiz_bases[i] + OCL_CLKWIZ_DIVCLK_TS);
                divclk_ts = (struct acap_divclk_ts *)&val;
                divclk_ts->divclk_lt = 0;
                divclk_ts->divclk_ht = 0;
                IO_SYNC_WRITE32(val, clock_wiz_bases[i] + OCL_CLKWIZ_DIVCLK_TS);

                /* Step 2) */
                val = IO_SYNC_READ32(clock_wiz_bases[i] + OCL_CLKWIZ_CLKFBOUT_FRACT);
                fract = (struct acap_clkfbout_fract *)&val;
                fract->clkfbout_fract_en = 0;
                IO_SYNC_WRITE32(val, clock_wiz_bases[i] + OCL_CLKWIZ_CLKFBOUT_FRACT);

                /* Step 3) */
                O = floor_acap_o(freq);
                M = floor_acap_m(freq);

                /* Step 4) */
                val = IO_SYNC_READ32(clock_wiz_bases[i] + OCL_CLKWIZ_CLKFBOUT);
                clkfbout = (struct acap_clkfbout *)&val;
                clkfbout->clkfbout_edge = (M % 2) ? 1 : 0;
                clkfbout->clkfbout_en = 1;
                clkfbout->clkfbout_mx = 1;
                clkfbout->clkfbout_prediv2 = 1;
                IO_SYNC_WRITE32(val, clock_wiz_bases[i] + OCL_CLKWIZ_CLKFBOUT);

                /* Step 5) */
                val = 0;
                clkfbout_ts = (struct acap_clkfbout_ts *)&val;
                clkfbout_ts->clkfbout_lt = (M - (M % 2)) / 2;
                clkfbout_ts->clkfbout_ht = (M - (M % 2)) / 2;
                IO_SYNC_WRITE32(val, clock_wiz_bases[i] + OCL_CLKWIZ_CLKFBOUT_TS);

                /* Step 6, 7) */
                val = IO_SYNC_READ32(clock_wiz_bases[i] + OCL_CLKWIZ_CLKOUT0);
                clkout0 = (struct acap_clkout0 *)&val;
                clkout0->clkout0_edge = (O % 2) ? 1 : 0;
                clkout0->clkout0_mx = 1;
                clkout0->clkout0_used = 1;
                clkout0->clkout0_prediv2 = 0;
                clkout0->clkout0_p5en = 0;
                IO_SYNC_WRITE32(val, clock_wiz_bases[i] + OCL_CLKWIZ_CLKOUT0);

                /* Step 8) */
                val = 0;
                clkout0_ts = (struct acap_clkout0_ts *)&val;
                clkout0_ts->clkout0_lt = (O - (O % 2)) / 2;
                clkout0_ts->clkout0_ht = (O - (O % 2)) / 2;
                IO_SYNC_WRITE32(val, clock_wiz_bases[i] + OCL_CLKWIZ_CLKOUT0_TS);

                /* init the freq change */
                IO_SYNC_WRITE32(0x3, clock_wiz_bases[i] + OCL_CLKWIZ_INIT_CONFIG);
                err = clock_wiz_busy(i, 100, 100);
                if (err)
                        break;
        }

        VMR_LOG("num_clock %d, returns %d", num_clock, err);
        return err;
}

int rmgmt_clock_freq_scaling(struct cl_msg *msg)
{
	int ret = 0;
	bool force = false;

	/* Internal scaling has gate freezed already, and force to config. */
	if (msg->clock_payload.ocl_req_type != CL_CLOCK_SCALE_INTERNAL)
		axigate_freeze();
	else
		force = true;

	ret = rmgmt_clock_freq_scaling_impl(msg, force);

	if (msg->clock_payload.ocl_req_type != CL_CLOCK_SCALE_INTERNAL)
		axigate_free();

	return ret;
}

uint32_t rmgmt_clock_get_freq(int idx, enum clock_ip ip)
{
        uint32_t freq = 0, status;
        int times = 10;

	/* Check array size, should never happen */
	if (idx > CLOCK_COUNTER_MAX_RES)
		return 0;

	/* For versal, we use counter for both wizard and counter freq */
	if (ip != RMGMT_CLOCK_WIZARD && ip != RMGMT_CLOCK_COUNTER)
		return 0;

	if (clock_counter_bases[idx]) {
		u32 base = clock_counter_bases[idx];
		/* start clock freq counter measurement */
		IO_SYNC_WRITE32(OCL_CLKWIZ_STATUS_MEASURE_START, base);

		while (times != 0) {
			VMR_DBG("times %d", times);
			status = IO_SYNC_READ32(base);
			if ((status & OCL_CLKWIZ_STATUS_MASK) ==
				OCL_CLKWIZ_STATUS_MEASURE_DONE)
				break;
			MDELAY(30);
			times--;
		};
		if ((status & OCL_CLKWIZ_STATUS_MASK) ==
			OCL_CLKWIZ_STATUS_MEASURE_DONE) {
			freq = (status & OCL_CLK_FREQ_V5_CLK0_ENABLED) ?
				IO_SYNC_READ32(base + OCL_CLK_FREQ_V5_COUNTER_OFFSET) :
				IO_SYNC_READ32(base + OCL_CLK_FREQ_COUNTER_OFFSET);
		}
	}

	VMR_LOG("idx %d freq %d times %d", idx, freq, times);
	return freq;
}
