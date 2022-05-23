# SPDX-License-Identifier: Apache-2.0
# Copyright (C) 2018-2022 Xilinx, Inc. All rights reserved.

setws .
puts "=== config app"
puts "=== add include"
app config -name vmr_app -add include-path ../src/include
puts "=== set warnings as errors"
app config -name vmr_app -add compiler-misc -Werror
