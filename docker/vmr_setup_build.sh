#!/bin/bash

#Setup TA
#cd ../build && TA=$(jq -r '.CONF_BUILD_TA' build.json)
cd ../build
TA=2022.2_daily_latest

#Install XRT and APU packages
sudo dpkg  -i /proj/xbuilds/${TA}/xbb/xrt/packages/xrt_*_20.04-amd64-xrt.deb
sudo dpkg -i /proj/xbuilds/${TA}/xbb/packages/internal_platforms/vck5000/gen4x8_qdma/*-dev/xilinx-vck5000-*_all.deb
#Use previous stable base pkg
#sudo dpkg -i /proj/xbuilds/${TA}/xbb/packages/internal_platforms/vck5000/gen4x8_qdma/base/xilinx-vck5000-*_all.deb
sudo dpkg -i /proj/xbuilds/2022.2_1102_1/xbb/packages/internal_platforms/vck5000/gen4x8_qdma/2-202220-1-dev/xilinx-vck5000-gen4x8-qdma-2-202220-1-dev_1-20221024_all.deb
sudo dpkg -i /proj/sdxbf/xrt-test/2022.2/xilinx_vck5000_gen4x8_qdma_2_202220_1/last_stable/dsabin/xilinx-vck5000-*_all.deb
sudo dpkg -i /proj/xbuilds/2022.2_daily_latest/xbb/xrt/packages/apu_packages/xrt-apu-vck5000*all.deb

#Run VMR build
./build.sh

#Source settings.sh
source /proj/xbuilds/${TA}/installs/lin64/Vitis/HEAD/settings64.sh

#Copy VMR.elf file to NFS location
mkdir -p /proj/xbuilds/VMR-ELF/${RELEASE}/${BUILD_NUMBER} && cp *.elf /proj/xbuilds/VMR-ELF/${RELEASE}/${BUILD_NUMBER}
