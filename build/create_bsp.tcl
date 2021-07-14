puts "set workspace"
setws . 

puts "create platform as rmgmt_platform"
platform create -out rmgmt_platform -name "rmgmt_platform" -hw xsa/gen3x16.xsa -proc psv_cortexr5_0 -os freertos10_xilinx

puts "redirect output to 1"
# ES3 platform has new name for uart
bsp config stdin blp_cips_pspmc_0_psv_sbsauart_1
bsp config stdout blp_cips_pspmc_0_psv_sbsauart_1

#this is a temporary workaround to enable adding xilmailbox onto freerots, vitis fixed the issue in 2021.2.
#puts "customize ssw repo"
#repo -set /proj/rdi/staff/davidzha/embeddedsw
#repo -set /public/bugcases/CR/1086000-1086999/1086872/embeddedsw

puts "customize bsp libs"
bsp setlib xilfpga
bsp setlib xilmailbox
bsp getlibs

#repo add platform for apps
repo -add-platform rmgmt_platform
puts "repo list platforms"
repo -platforms

platform generate
