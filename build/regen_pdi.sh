#!/bin/bash

# SPDX-License-Identifier: Apache-2.0
# Copyright (C) 2018-2022 Xilinx, Inc. All rights reserved.

############################################################
# Help                                                     #
############################################################
Help()
{
    # Display Help
    echo
    echo "Script for Regeneration of vck5000 PDI"
    echo
    echo "Usage: $0 [-h] [-s] -v <vmr.elf>"
    echo "options:"
    echo "-h                print this help"
    echo "-s                skip plm.elf generation. Requires ./plm.elf to already exist"
    echo "-v    <vmr.elf>   fw file to use in PDI (MUST BE SPECIFIED)"
    echo
    echo "Script expects VITIS to be installed, this can be achieved by sourcing install script, e.g."
    echo "  source /proj/xbuilds/2022.1_daily_latest/installs/lin64/Vitis/2022.1/setup.sh"
    echo "  source /proj/xbuilds/2022.1_daily_latest/installs/lin64/Vitis/2022.1/setup.csh"
    echo
    echo "Script expects the deployment and development packages to be installed, e.g. "
    echo "  /proj/xbuilds/2022.1_daily_latest/xbb/packages/internal_platforms/vck5000/gen4x8_xdma/base/"
    echo "  /proj/xbuilds/2022.1_daily_latest/xbb/packages/internal_platforms/vck5000/gen4x8_xdma/2-202210-1-dev/"
    echo 
}

############################################################
############################################################
# Main program                                             #
############################################################
############################################################

# script vars (change if required)
#xsa=/opt/xilinx/platforms/xilinx_vck5000_gen4x8_xdma_2_202210_1/hw/xilinx_vck5000_gen4x8_xdma_2_202210_1.xsa
xsa=/opt/xilinx/platforms/xilinx_v70_gen5x8_qdma_2_202220_1/hw/xilinx_v70_gen5x8_qdma_2_202220_1.xsa
#xsabin=/opt/xilinx/firmware/vck5000/gen4x8-xdma/base/partition.xsabin
xsabin=/opt/xilinx/firmware/v70/gen5x8-qdma/base/partition.xsabin
vmr=""
skip_plm=0
AIE2=0


############################################################
# Process the input options. 
############################################################

# Get the options
while getopts ":hsav:x:y:" option; do
   case $option in
      h) # display Help
         Help
         exit;;
      a) # AIE2 V70 platform specific
         echo "AIE2 base pdi"
         AIE2=1;;
      s) # skip plm.elf generation
         echo "  skipping plm.elf generation, using ./plm.elf"    
         skip_plm=1;;
      v) # VMR.elf location
         vmr=$OPTARG
         echo "  vmr chosen: $vmr";;
      x)
         xsa=$OPTARG
	 echo " xsa: $xsa";;
      y)
         xsabin=$OPTARG
	 echo " xsabin: $xsabin";;
     \?) # Invalid option
         echo "ERROR: Invalid option. See help below..."
         Help
         exit 1;;
   esac
done


############################################################
# check necessary files and tools exist
############################################################

if [ $skip_plm -eq 1 ] && [[ ! -e "plm.elf" ]]; then
    echo "ERROR: -s option chosen but plm.elf does not exist, can only skip plm.elf generation if the plm.elf file exists"
    echo "       see $0 -h for [-s] option usage"
    exit 1
fi

if [[ ! -e "$xsa" ]]; then
    echo "ERROR: Cannot find the XSA file at $xsa - have you installed the platform development package e.g."
    echo "       sudo yum install /proj/xir_xbuilds/2022.1_daily_latest/xbb/packages/internal_platforms/vck5000/gen4x8_xdma/2-202210-1-dev/xilinx-vck5000-gen4x8-xdma-2-202210-1-dev-1-20220328.noarch.rpm"
    exit 1
fi

if [[ ! -e "$xsabin" ]]; then
    echo "ERROR: Cannot find the XSABIN file at $xsabin - have you installed the platform deployment package e.g."
    echo "       sudo yum install /proj/xir_xbuilds/2022.1_daily_latest/xbb/packages/internal_platforms/vck5000/gen4x8_xdma/base/xilinx-vck5000-gen4x8-xdma-base-2-20220328.noarch.rpm"
    exit 1
fi

if [[ ! -e "$vmr" ]]; then
    echo "ERROR: Cannot find the vmr file at \"$vmr\", -v option must be specified - see $0 -h"
    exit 1
fi

# check for tool dependencies, expect them to be installed
if ! command -v xclbinutil &> /dev/null
then
    echo -e "ERROR: xclbinutil required but could not be found. Install VITIS or XRT. Aborting"
    echo -e "       tip: source /proj/xbuilds/2022.1_daily_latest/installs/lin64/Vitis/2022.1/settings64.sh"
    exit 1
fi
if ! command -v bootgen &> /dev/null
then
    echo -e "ERROR: bootgen required but could not be found. Install VITIS. Aborting"
    echo -e "       tip: source /proj/xbuilds/2022.1_daily_latest/installs/lin64/Vitis/2022.1/settings64.sh"
    exit 1
fi


############################################################
# Generating plm.elf for xsa...
############################################################

# create folder for scripts
rm -rf scripts
mkdir scripts

# generate tcl script to generate plm.elf with uart swap
printf "%s\n" '
set i 0; foreach n $argv {set [incr i] $n}
puts "Generating vck5000 plm.elf for $1"
# Update the plm.elf file with swapped UARTs
# Unique to VCK5000
setws .
app create -name plm -template {versal PLM} -proc blp_cips_pspmc_0_psv_pmc_0 -hw $1 -os standalone
bsp config stdout blp_cips_pspmc_0_psv_sbsauart_1
bsp config stdin blp_cips_pspmc_0_psv_sbsauart_1
app build -name plm
file rename -force ./plm/Debug/plm.elf ./plm.elf
app remove plm
' > scripts/plm_gen.tcl

# generate tcl script to generate plm.elf with uart swap
printf "%s\n" '
set i 0; foreach n $argv {set [incr i] $n}
puts "Generating vck5000 plm.elf for $1"
# Update the plm.elf file with swapped UARTs
# Unique to V70
setws .
app create -name plm -template {versal PLM} -proc blp_cips_pspmc_0_psv_pmc_0 -hw $1 -os standalone
bsp config stdout blp_cips_pspmc_0_psv_sbsauart_0
bsp config stdin blp_cips_pspmc_0_psv_sbsauart_0
app build -name plm
file rename -force ./plm/Debug/plm.elf ./plm.elf
' > scripts/plm_gen_v70.tcl


# if not skipped (-s option) generate new plm.elf
if [ $skip_plm -eq 0 ]; then
    printf "\nGenerating plm.elf for xsa...\n"
    rm -f plm.elf
    rm -rf plm
    mkdir plm
    cd plm
    export FORCE_MARK_AS_EDGE_XSA=1

    if [ $AIE2 == "1" ];then
        xsct ../scripts/plm_gen_v70.tcl $xsa
    else
        xsct ../scripts/plm_gen.tcl $xsa
    fi

    cp plm.elf ../
    cd ..
fi


############################################################
# (Re)Generating Boot PDI with new VMR...
############################################################

rm -rf bins
mkdir bins
pdi=bins/base.pdi
vmr_bif=scripts/vmr.bif
rebuild_bif=scripts/rebuild.bif
aie2_rebuild_bif=scripts/aie2_rebuild.bif

# get PDI from XSABIN and disassemble into binaries
printf "\nDisassemblinng the PDI located here $xsabin...\n"
sleep 1
xclbinutil --input $xsabin --dump-section PDI:RAW:$pdi >> /dev/null
bootgen -arch versal -dump $pdi -dump_dir ./bins >> /dev/null

# check the contents of the PDI match the expected list
bin_list=`find bins/ -name '*.bin' -exec basename {} \; | sort`
golden_list="aie_subsys_7.bin
cpm_6.bin
ext_fpt_0.bin
fpd_8.bin
lpd_b.bin
lpd_c.bin
pl_cfi_3.bin
pl_cfi_5.bin
pmc_cdo_0.bin
pmc_subsys_0.bin
rpu_subsystem_0.1.bin
rpu_subsystem_0.bin"

aie2_golden_list="aie2_subsys_7.bin
cpm_14.bin
ext_fpt_0.bin
fpd_8.bin
lpd_b.bin
lpd_c.bin
pl_cfi_3.bin
pl_cfi_5.bin
pmc_cdo_0.bin
pmc_subsys_0.bin
rpu_subsystem_0.1.bin
rpu_subsystem_0.bin"

if [ $AIE2 == "1" ];then
	echo "using aie2_golden_list"
	golden_list=$aie2_golden_list
fi

if [ "$bin_list" != "$golden_list" ]; then
    echo "ERROR: disassembled PDI does not contain expected files, see bins/ folder. Differences.."
    diff  <(echo "$bin_list" ) <(echo "$golden_list")
    exit 1
fi

# create BIF file to vmr partial PDI
printf "%s\n" 'vmr_bif:
{
 id_code = 0x04cd7093
 extended_id_code = 0x01
 id = 0x2
 image
 {
  name = rpu_test, id = 0x1c000000
  { core = r5-0, file = _VMR_FILE_ }
 }
}' > $vmr_bif

# create BIF file to rebuild PDI - based on combination of BIF files used in original build process
printf "%s\n" 'new_bif:
{
 id_code = 0x14ca8093 
 extended_id_code = 0x01
 id = 0x2
 image
 {
  name = pmc_subsys, id = 0x1c000001
  partition { id = 0x01, type = bootloader, file = plm.elf }
  partition { id = 0x09, type = pmcdata, load = 0xf2000000, file = bins/pmc_cdo_0.bin }
 }
 image
 {
  name = lpd, id = 0x4210002 
  partition { id = 0x0C, type = cdo, file = bins/lpd_c.bin }
  partition { id = 0x0B, core = psm, file = _PSM_FILE_  }
 }
 image
 {
  name = pl_cfi, id = 0x18700000
  partition { id = 0x03, type = cdo, file = bins/pl_cfi_3.bin }
  partition { id = 0x05, type = cdo, file = bins/pl_cfi_5.bin }
 }
 image
 {
  name = cpm, id = 0x4218007
  partition { id = 0x06, type = cdo, file = bins/cpm_6.bin }
 }
 image
 {
  name = aie_subsys, id = 0x421c005
  partition { id = 0x07, type = cdo, file = bins/aie_subsys_7.bin }
 }
 image
 {
  name = fpd, id = 0x420c003
  partition { id = 0x08, type = cdo, file = bins/fpd_8.bin }
 }
 image 
 {
   name = rpu_subsystem, id = 0x1c000000, delay_handoff
   { core = r5-0, file = _VMR_FILE_ }
 }
 image
 {
   name = ext_fpt, id = 0x1c000000
   { type = cdo, file = bins/ext_fpt_0.bin }
 }
}' > $rebuild_bif

# aie2 version bif for
printf "%s\n" 'new_bif:
{
 id_code = 0x04cd7093
 extended_id_code = 0x01
 id = 0x2
 image
 {
  name = pmc_subsys, id = 0x1c000001
  partition { id = 0x01, type = bootloader, file = plm.elf }
  partition { id = 0x09, type = pmcdata, load = 0xf2000000, file = bins/pmc_cdo_0.bin }
 }
 image
 {
  name = lpd, id = 0x4210002 
  partition { id = 0x0C, type = cdo, file = bins/lpd_c.bin }
  partition { id = 0x0B, core = psm, file = _PSM_FILE_  }
 }
 image
 {
  name = pl_cfi, id = 0x18700000
  partition { id = 0x03, type = cdo, file = bins/pl_cfi_3.bin }
  partition { id = 0x05, type = cdo, file = bins/pl_cfi_5.bin }
 }
 image
 {
  name = cpm, id = 0x4218007
  partition { id = 0x06, type = cdo, file = bins/cpm_14.bin }
 }
 image
 {
  name = aie_subsys, id = 0x421c028
  partition { id = 0x07, type = cdo, file = bins/aie2_subsys_7.bin }
 }
 image
 {
  name = fpd, id = 0x420c003
  partition { id = 0x08, type = cdo, file = bins/fpd_8.bin }
 }
 image 
 {
   name = rpu_subsystem, id = 0x1c000000, delay_handoff
   { core = r5-0, file = _VMR_FILE_ }
 }
 image
 {
   name = ext_fpt, id = 0x1c000000
   { type = cdo, file = bins/ext_fpt_0.bin }
 }
}' > $aie2_rebuild_bif

# update paths in rebuild.bif
# TODO: rebuild BIF may be a bit brittle as tools could change the source that this was built on
#       may need to add a check to build flow to detect if the source top_wrapper.bif changes
printf "\nUpdate scripts/rebuild.bif with FW paths...\n"
sleep 1

if [ $AIE2 == "1" ];then
	echo "aie2 using $aie2_rebuild_bif"
	rebuild_bif=$aie2_rebuild_bif
fi
# find path to psm_fw.elf based on installed vitis path
# /proj/xbuilds/2022.1_daily_latest/installs/lin64/Vivado/2022.1/data/versal/flows/data_files/psm_fw.elf
psm_path=$(dirname `which bootgen`)/../data/versal/flows/data_files/psm_fw.elf
sed -i 's,_PSM_FILE_,'$psm_path',' $rebuild_bif

# replace _VMR_FILE_ with -v script argument
sed -i 's,_VMR_FILE_,'$vmr',' $rebuild_bif

# replace _VMR_FILE_ with -v script argument
sed -i 's,_VMR_FILE_,'$vmr',' $vmr_bif

# rebuild the pdi from the binary sources and repackage as xsabin as well
printf "\nRebuilding PDI and new XSABIN...\n"
sleep 1
rm -f rebuilt.*
rm -f vmr.pdi
bootgen -arch versal -image $rebuild_bif -w -o rebuilt.pdi
xclbinutil --input $xsabin --replace-section PDI:RAW:rebuilt.pdi --output rebuilt.xsabin

printf "\nRebuilding vmr.pdi partial PDI...\n"
bootgen -arch versal -image $vmr_bif -w -o vmr.pdi

printf "\nScript complete....\n"
printf "  Generated the following files:\n"
realpath vmr.pdi
realpath rebuilt.pdi
realpath rebuilt.xsabin
