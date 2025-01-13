# SPDX-License-Identifier: MIT
# Copyright (C) 2024 Advanced Micro Devices, Inc.    All rights reserved.

setws .
puts "=== config app"
puts "=== add include"
app config -name vmr_app -add include-path ../src/include
puts "=== set warnings as errors"
app config -name vmr_app -add compiler-misc -Werror
puts "=== analyze stack usage per function"
app config -name vmr_app -add compiler-misc -fstack-usage
puts "=== set warnings if stack is above 4k"
app config -name vmr_app -add compiler-misc -Wstack-usage=4096
