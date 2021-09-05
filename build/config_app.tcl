setws .
puts "config app"
set xrt $::env(XILINX_XRT)
app config -name vmr -add include-path ../src/include
app config -name vmr -add include-path $xrt/include
