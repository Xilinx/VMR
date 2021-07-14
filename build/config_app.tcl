setws .
puts "config app"
set xrt $::env(XILINX_XRT)
app config -name rmgmt -add include-path ../src/include
app config -name rmgmt -add include-path $xrt/include
