#!/bin/bash

# TA will be passed through as an environment variable
#cd ../build && TA=$(jq -r '.CONF_BUILD_TA' build.json)
echo TA: ${TA}
cd ../build

#Install XRT and APU packages
sudo dpkg  -i /proj/xbuilds/${TA}/xbb/xrt/packages/xrt_*_20.04-amd64-xrt.deb
sudo dpkg -i /proj/xbuilds/${TA}/xbb/packages/internal_platforms/vck5000/gen4x8_qdma/*-dev/xilinx-vck5000-*_all.deb
sudo dpkg -i /proj/xbuilds/${TA}/xbb/packages/internal_platforms/vck5000/gen4x8_qdma/base/xilinx-vck5000*-base_2-*_all.deb
sudo dpkg -i /proj/xbuilds/${TA}/xbb/xrt/packages/apu_packages/xrt-apu-vck5000*all.deb

#Run VMR build
./build.sh
md5sum *.elf

#Source settings.sh
source /proj/xbuilds/${TA}/installs/lin64/Vitis/HEAD/settings64.sh

#Copy VMR.elf file to NFS location
mkdir -p /proj/xbuilds/VMR-ELF/${RELEASE}/${BUILD_NUMBER} && cp *.elf /proj/xbuilds/VMR-ELF/${RELEASE}/${BUILD_NUMBER}
