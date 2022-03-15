set log_file "build.log"
proc reopenStdout {file} {
	close stdout
	open $file w;
}

puts "=== set workspace"
setws . 

puts "=== create platform as vmr_platform"
platform create -out vmr_platform -name {vmr_platform} -hw xsa/vmr.xsa -proc {blp_cips_pspmc_0_psv_cortexr5_0} -os freertos10_xilinx

if { [lindex $argv 1] == 1 } {
	puts "=== use 2022.1 daily_lastest embeddedws"
	# comment out ssw patches
	#repo -set /proj/xsjhdstaff2/davidzha/ssw_2022.1/
	#repo -set /public/bugcases/CR/1105000-1105999/1105240/ssw_2022.1/
} else {
	puts "===  patching 2021.2 fixes for non-daily_latest stable builds"
	#repo -set /proj/rdi/staff/davidzha/embeddedsw/
	repo -set /public/bugcases/CR/1105000-1105999/1105240/embeddedsw/
}

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
puts "=== customize FreeRTOS heap size (65536 * 4 * 50) = 13107200 "
bsp config total_heap_size 13107200

#repo add platform for apps
repo -add-platform vmr_platform
puts "=== repo list platforms"
repo -platforms

platform write
platform active

#prior to build project, copy stable bsp
#if { [lindex $argv 1] == 1 } {
#	puts "=== use daily_latest bsp"
#} else {
#	puts "=== use stable bsp within this repo"
#	set shell_script " rsync -av ../bsp vmr_platform/vmr_platform/psv_cortexr5_0/freertos10_xilinx_domain/"
#	exec sh -c $shell_script
#}

puts "=== Start redirecting verbose log into: $log_file"
reopenStdout $log_file
platform generate
