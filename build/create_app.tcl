setws .
puts "create app"
set xsaName $::env(FORCE_MARK_XSA_NAME)
puts "xsaName is: $xsaName"
app create -name vmr -platform ${xsaName} -domain freertos10_xilinx_domain -template "Empty Application(C)"

