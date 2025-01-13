#!/bin/bash

# SPDX-License-Identifier: MIT
# Copyright (C) 2024 Advanced Micro Devices, Inc.    All rights reserved.

############################################################
# Help                                                     #
############################################################
Help()
{
    # Display Help
    echo
    echo "Script for Regeneration of v70 vmr PDI"
    echo
    echo "Usage: $0 [-h] -v <vmr.elf>"
    echo "options:"
    echo "-h                print this help"
    echo "-a                build AIE2 shell"
    echo "-b                build AIE2 PQ2 shell"
    echo "-v    <vmr.elf>   fw file to use in PDI (MUST BE SPECIFIED)"
    echo
    echo "Script expects VITIS to be installed, this can be achieved by sourcing install script, e.g."
    echo "  source /proj/xbuilds/2022.2_daily_latest/installs/lin64/Vitis/2022.1/setup.sh"
    echo "  source /proj/xbuilds/2022.2_daily_latest/installs/lin64/Vitis/2022.1/setup.csh"
    echo
    echo 
}

############################################################
############################################################
# Main program                                             #
############################################################
############################################################

# script vars (change if required)
vmr=""
AIE2=0

############################################################
# Process the input options. 
############################################################

# Get the options
while getopts ":habv:" option; do
   case $option in
      h) # display Help
         Help
         exit;;
      a) # AIE2 V70
         echo "AIE2 platform"
	 AIE2=1;;
      b) # AIE2PQ2 V70
         echo "AIE2PQ2 platform"
	 AIE2PQ2=1;;
      v) # VMR.elf location
         vmr=$OPTARG
         echo "  vmr chosen: $vmr";;
     \?) # Invalid option
         echo "ERROR: Invalid option. See help below..."
         Help
         exit 1;;
   esac
done


############################################################
# check necessary files and tools exist
############################################################
if [[ ! -e "$vmr" ]]; then
    echo "ERROR: Cannot find the vmr file at \"$vmr\", -v option must be specified - see $0 -h"
    exit 1
fi

# check for tool dependencies, expect them to be installed
if ! command -v bootgen &> /dev/null
then
    echo -e "ERROR: bootgen required but could not be found. Install VITIS. Aborting"
    echo -e "       tip: source /proj/xbuilds/2022.2_daily_latest/installs/lin64/Vitis/2022.1/settings64.sh"
    exit 1
fi


############################################################
# Generating vmr.pdi from vmr.elf
############################################################

# create folder for scripts
rm -rf scripts
mkdir scripts

############################################################
# (Re)Generating Boot PDI with new VMR...
############################################################

# create BIF file to vmr partial PDI
printf "%s\n" 'vmr_bif:
{
 id_code = 0x14ca8093
 extended_id_code = 0x01
 id = 0x2
 image
 {
  name = rpu_test, id = 0x1c000000
  { core = r5-0, file = _VMR_FILE_ }
 }
}' > scripts/vmr.bif

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
}' > scripts/vmr_v70.bif

printf "%s\n" 'vmr_bif:
{
 id_code = 0x14cd7093
 extended_id_code = 0x01
 id = 0x2
 image
 {
  name = rpu_test, id = 0x1c000000
  { core = r5-0, file = _VMR_FILE_ }
 }
}' > scripts/vmr_v70pq2.bif

vmr_bif=scripts/vmr.bif
if [ $AIE2 == "1" ];then
	echo "aie2 using v70.bif"
	vmr_bif=scripts/vmr_v70.bif
elif [ $AIE2PQ2 == "1" ];then
	echo "aie2pq2 using v70.bif"
	vmr_bif=scripts/vmr_v70pq2.bif
fi

# replace _VMR_FILE_ with -v script argument
echo " using $vmr_bif"
sed -i 's,_VMR_FILE_,'$vmr',' $vmr_bif

# rebuild the pdi
rm -f vmr.pdi
printf "\nRebuilding vmr.pdi partial PDI...\n"
bootgen -arch versal -image $vmr_bif -w -o vmr.pdi

printf "\nScript complete....\n"
printf "  Generated the following files:\n"
realpath vmr.pdi
