# SPDX-License-Identifier: MIT
# Copyright (C) 2024 Advanced Micro Devices, Inc.    All rights reserved.

setws .
puts "create app"
set xsaName $::env(XSA_PLATFORM_NAME)
puts "xsaName is: $xsaName"
app create -name vmr -platform ${xsaName} -domain freertos10_xilinx_domain -template "Empty Application(C)"

