# SPDX-License-Identifier: Apache-2.0
# Copyright (C) 2018-2022 Xilinx, Inc. All rights reserved.

puts "=== set workspace"
setws . 

puts "=== create entire platform and vmr_app application"
app create -name vmr_app -hw vmr.xsa -proc blp_cips_pspmc_0_psv_cortexr5_0 -os freertos10_xilinx -template "Empty Application(C)"

if { [lindex $argv 0] == 1 } {

	puts "=== stdout to jtag"
	bsp config stdin blp_cips_pspmc_0_psv_sbsauart_1
	bsp config stdout blp_cips_pspmc_0_psv_sbsauart_1

} else {

	puts "=== stdout to uartlite"
	bsp config stdin blp_blp_logic_axi_uart_rpu
	bsp config stdout blp_blp_logic_axi_uart_rpu

}

puts "=== customize bsp libs"
bsp setlib xilfpga
bsp setlib xilmailbox
bsp getlibs
puts "=== customize FreeRTOS heap size 0x16000000 (352M)"
bsp config total_heap_size 0x16000000

platform write
platform active
