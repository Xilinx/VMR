puts "set workspace"
setws . 

puts "create platform as rmgmt_platform"
platform create -out rmgmt_platform -name "rmgmt_platform" -hw xsa/gen3x16.xsa -proc psv_cortexr5_0 -os freertos10_xilinx

puts "redirect output to 1"
bsp config stdin psv_sbsauart_1
bsp config stdout psv_sbsauart_1

#this is a temporary workaround to enable adding xilmailbox onto freerots, once Vitis has the fix, this line will be removed 
puts "customize ssw repo"
#repo -set /proj/rdi/staff/davidzha/embeddedsw
repo -set /public/bugcases/CR/1086000-1086999/1086872/embeddedsw

puts "customize bsp libs"
bsp setlib xilfpga
bsp setlib xilmailbox
bsp getlibs

#repo add platform for apps
repo -add-platform rmgmt_platform
puts "repo list platforms"
repo -platforms

platform generate
