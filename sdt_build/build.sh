#!/bin/bash

# SPDX-License-Identifier: Apache-2.0
# Copyright (C) 2024 Advanced Micro Devices, Inc. All rights reserved.


#usage:
#source build.sh hw-design.xsa

#setting up the environment
TOOL_VERSION="2024.2"
export XILINX_VITIS=/proj/xbuilds/${TOOL_VERSION}_daily_latest/installs/lin64/Vitis/${TOOL_VERSION}
export PYTHON_VER="python-3.8.3"
export CMAKE_VER="cmake-3.24.2"
export XBUILDS_CMAKE_PATH=${XILINX_VITIS}/tps/lnx64/${CMAKE_VER}
export XBUILDS_PYTHON_PATH=${XILINX_VITIS}/tps/lnx64/${PYTHON_VER}

# LD_LIBRARY_PATH is needed to add the linker libraries (e.g. libffi) to the path
export LD_LIBRARY_PATH=${XBUILDS_PYTHON_PATH}/lib:${XBUILDS_CMAKE_PATH}/libs/Ubuntu:${LD_LIBRARY_PATH}
export LD_LIBRARY_PATH=${XILINX_VITIS}/lib/lnx64.o/Ubuntu/20:$LD_LIBRARY_PATH

# Source Paths of different toolchains, python, cmake and lopper binary
export PATH=${XILINX_VITIS}/bin:${XILINX_VITIS}/gnu/microblaze/lin/bin:${XILINX_VITIS}/gnu/arm/lin/bin:${XILINX_VITIS}/gnu/aarch32/lin/gcc-arm-linux-gnueabi/bin:${XILINX_VITIS}/gnu/aarch32/lin/gcc-arm-none-eabi/bin:${XILINX_VITIS}/gnu/aarch64/lin/aarch64-none/bin:${XILINX_VITIS}/gnu/armr5/lin/gcc-arm-none-eabi/bin:${XBUILDS_PYTHON_PATH}/bin:${XBUILDS_CMAKE_PATH}/bin:$PATH

# VALIDATE_ARGS flag is needed to enable the validation of inputs. Set this flag to "" when
# using the GUI flow.
export VALIDATE_ARGS="True"

# OSF Flag is needed to be enabled for Open Source Flow.
export OSF="False"

# LOPPER_DTC_FLAGS is needed to generate device tree with symbols in it.
export LOPPER_DTC_FLAGS="-b 0 -@"
source ${XILINX_VITIS}/tps/lnx64/lopper-1.1.0/env/bin/activate


#cleaning before building
rm -rf *.yaml 
rm -rf *.cmake
rm -rf build
rm -rf CMakeLists.txt
rm -rf hw_artifacts
rm -rf include
rm -rf lib
rm -rf libsrc
rm -rf *.dtb *.pp *.spec
rm -rf src sdt_dir

#to store generated .dts/.dtsi files
mkdir -p sdt_dir
SDT_DIR="sdt_dir"
XSA=$1

#generating the SDT
#hardcoding it for now, need to clean the code
xsct sdt.tcl ${XSA} ${SDT_DIR}

#setting the embeddedsw repo
python3 ${XILINX_VITIS}/data/embeddedsw/scripts/pyesw/repo.py -st ${XILINX_VITIS}/data/embeddedsw/

#creating the bsp
python ${XILINX_VITIS}/data/embeddedsw/scripts/pyesw/create_bsp.py -p psv_cortexr5_0 -s ./${SDT_DIR}/system-top.dts -w ./ -o freertos -t empty_application

#configuring the bsp
python ${XILINX_VITIS}/data/embeddedsw/scripts/pyesw/config_bsp.py -d ./ -al xilmailbox
python ${XILINX_VITIS}/data/embeddedsw/scripts/pyesw/config_bsp.py -d ./ -al xilpm
python ${XILINX_VITIS}/data/embeddedsw/scripts/pyesw/config_bsp.py -d ./ -al xilfpga
python ${XILINX_VITIS}/data/embeddedsw/scripts/pyesw/config_bsp.py -d ./ -st freertos freertos_generate_runtime_stats:0x1
python ${XILINX_VITIS}/data/embeddedsw/scripts/pyesw/config_bsp.py -d ./ -st freertos freertos_total_heap_size:369098752
python ${XILINX_VITIS}/data/embeddedsw/scripts/pyesw/config_bsp.py -d ./ -st freertos freertos_stream_buffer:true
python ${XILINX_VITIS}/data/embeddedsw/scripts/pyesw/config_bsp.py -d ./ -st freertos freertos_stdin:blp_cips_pspmc_0_psv_sbsauart_0
python ${XILINX_VITIS}/data/embeddedsw/scripts/pyesw/config_bsp.py -d ./ -st freertos freertos_stdout:blp_cips_pspmc_0_psv_sbsauart_0

#building the bsp
python ${XILINX_VITIS}/data/embeddedsw/scripts/pyesw/build_bsp.py -d ./

#creating empty application
python ${XILINX_VITIS}/data/embeddedsw/scripts/pyesw/create_app.py -d ./ -t empty_application

#copying the src to the empty application
cp -r ../vmr/src/* src/
cp lscript_rave.ld src/lscript.ld

#building the app
python ${XILINX_VITIS}/data/embeddedsw/scripts/pyesw/build_app.py -s src/




