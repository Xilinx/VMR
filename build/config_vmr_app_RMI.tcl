# SPDX-License-Identifier: Apache-2.0
# Copyright (C) 2024 Advanced Micro Devices, Inc.    All rights reserved.

set i 0; foreach n $argv {set [incr i] $n}
puts "add library and path: $1 $2 "

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
puts "=== Adding library to app config"
app config -name vmr_app -add library-search-path $1
app config -name vmr_app -add libraries $2
