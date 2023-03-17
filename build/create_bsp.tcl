# SPDX-License-Identifier: Apache-2.0
# Copyright (C) 2018-2022 Xilinx, Inc. All rights reserved.

set i 0; foreach n $argv {set [incr i] $n}
puts "create with xsa: $1 jtag: $2"

puts "=== set workspace"
setws . 

puts "=== create entire platform and vmr_app application"
app create -name vmr_app -hw $1 -proc blp_cips_pspmc_0_psv_cortexr5_0 -os freertos10_xilinx -template "Empty Application(C)"

if { $2 == 0 } {
	puts "=== stdout to jtag0"
	bsp config stdin blp_cips_pspmc_0_psv_sbsauart_0
	bsp config stdout blp_cips_pspmc_0_psv_sbsauart_0
} elseif { $2 == 1 } {
	puts "=== stdout to jtag1"
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
bsp setlib xilpm
bsp getlibs
puts "=== customize FreeRTOS heap size 0x16000000 (352M)"
bsp config total_heap_size 0x16000000
puts "=== Configure Macro configGENERATE_RUN_TIME_STATS to 1 FreeRTOS Config"
bsp config generate_runtime_stats 1

puts "=== Configure Macro configSTREAM_BUFFER to true in FreeRTOS Config"
bsp config stream_buffer true

platform write
platform active
