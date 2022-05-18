# SPDX-License-Identifier: Apache-2.0
# Copyright (C) 2018-2022 Xilinx, Inc. All rights reserved.

setws .
puts "=== config app"
puts "=== add include"
app config -name vmr -add include-path ../src/include
puts "=== set warnings as errors"
app config -name vmr -add compiler-misc -Werror
