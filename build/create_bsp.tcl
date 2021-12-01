puts "set workspace"
setws . 

puts "create platform as vmr_platform"
platform create -out vmr_platform -name "vmr_platform" -hw xsa/gen3x16.xsa -proc psv_cortexr5_0 -os freertos10_xilinx

puts "redirect console to uart1 shared with APU temporary"
puts "TODO: redirect output to uart lite between rpu and pcie mgmt"
bsp config stdin blp_cips_pspmc_0_psv_sbsauart_1
bsp config stdout blp_cips_pspmc_0_psv_sbsauart_1
#bsp config stdin blp_blp_logic_axi_uart_rpu
#bsp config stdout blp_blp_logic_axi_uart_rpu

puts "customize bsp libs"
bsp setlib xilfpga
bsp setlib xilmailbox
bsp getlibs

#repo add platform for apps
repo -add-platform vmr_platform
puts "repo list platforms"
repo -platforms

platform generate
