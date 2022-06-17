#!/bin/bash

#Setup TA
cd ../build && TA=$(jq -r '.CONF_BUILD_TA' build.json)

#Install XRT and APU packages
sudo dpkg  -i /proj/xbuilds/${TA}/xbb/xrt/packages/xrt_*_20.04-amd64-xrt.deb
sudo dpkg -i /proj/xbuilds/2022.2_daily_latest/xbb/packages/internal_platforms/vck5000/gen4x8_xdma/*-dev/xilinx-vck5000-*_all.deb
sudo dpkg -i /proj/xbuilds/2022.2_daily_latest/xbb/packages/internal_platforms/vck5000/gen4x8_xdma/base/xilinx-vck5000-*_all.deb
sudo dpkg -i /proj/xbuilds/2022.2_daily_latest/xbb/packages/internal_platforms/vck5000/gen4x8_qdma/*-dev/xilinx-vck5000-*_all.deb
sudo dpkg -i /proj/xbuilds/2022.2_daily_latest/xbb/packages/internal_platforms/vck5000/gen4x8_qdma/base/xilinx-vck5000-*_all.deb
sudo dpkg -i /proj/xbuilds/${TA}/internal_platforms/sw/versal/apu_packages/versal/xrt-apu_*_all.deb


#Source settings.sh
source /proj/xbuilds/${TA}/installs/lin64/Vitis/HEAD/settings64.sh

#Run VMR build
./build.sh

#Copy VMR.elf file to NFS location
mkdir -p /proj/xbuilds/VMR-ELF/${RELEASE}/${BUILD_NUMBER} && cp *.elf /proj/xbuilds/VMR-ELF/${RELEASE}/${BUILD_NUMBER}
