puts "set workspace"
setws . 

puts "create platform as project_vck"
platform create -out rmgmt_platform -name "rmgmt_platform" -hw ../xsa/gen3x16.xsa -proc psv_cortexr5_0 -os freertos10_xilinx

#active platform
#platform active platform_vck

puts "customize ssw repo"
repo -set /proj/rdi/staff/davidzha/embeddedsw

puts "customize bsp libs"
bsp setlib xilfpga
bsp setlib xilmailbox
bsp getlibs

#repo add platform for apps
repo -add-platform rmgmt_platform
puts "repo list platforms"
repo -platforms

platform generate

puts "create app"
app create -name rmgmt -platform rmgmt_platform -domain freertos10_xilinx_domain -template "Empty Application"

puts "build app"
app build -name rmgmt 
