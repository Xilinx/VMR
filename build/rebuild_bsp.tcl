# SPDX-License-Identifier: Apache-2.0
# Copyright (C) 2018-2022 Xilinx, Inc. All rights reserved.
# Copyright (C) 2022 Advanced Micro Devices, Inc. All rights reserved.

puts "=== generate platform "
setws . 

platform read {vmr_platform/vmr_platform/platform.spr}
platform active {vmr_platform}
platform generate
