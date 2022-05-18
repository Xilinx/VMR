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
    echo "Script for upgrading vmr.elf"
    echo
    echo "Usage: $0 [-h] -v <vmr.elf>"
    echo "options:"
    echo "-h                print this help"
    echo "-v    <vmr.elf>   (MUST BE SPECIFIED)"
    echo 
}

############################################################
############################################################
# Main program                                             #
############################################################
############################################################

# script vars (change if required)
VMR=""
TOOL_VERSION="2022.1"
DEFAULT_VITIS="/proj/xbuilds/${TOOL_VERSION}_daily_latest/installs/lin64/Vitis/HEAD/settings64.sh"

############################################################
# Process the input options. 
############################################################

# Get the options
while getopts ":hv:" option; do
   case $option in
      h) # display Help
         Help
         exit;;
      v) # VMR.elf location
         VMR=$OPTARG
         echo "  vmr chosen: $VMR";;
     \?) # Invalid option
         echo "ERROR: Invalid option. See help below..."
         Help
         exit 1;;
   esac
done


############################################################
# check necessary files and tools exist
############################################################

if [[ ! -e "$VMR" ]]; then
    echo "ERROR: Cannot find the vmr elf file at \"$VMR\", -v option must be specified - see $0 -h"
    exit 1
fi

# check for tool dependencies, expect them to be installed
if ! command -v xsdb &> /dev/null
then
    echo "no xsdb, using version: $TOOL_VERSION"
    . ${DEFAULT_VITIS}
    which xsdb
fi
XSDB_VERSION=`which xsdb|rev|cut -f3 -d"/"|rev`
echo "Tools version: ${XSDB_VERSION}"


############################################################
# Generating plm.elf for xsa...
############################################################

# create folder for scripts
rm -rf /tmp/scripts
mkdir /tmp/scripts

# generate tcl script to upgrading vmr.elf
printf "%s\n" '
set i 0; foreach n $argv {set [incr i] $n}
puts "upgrading vmr.elf from $1"
puts "connect to the targets"
conn
puts "list all targets"
tar
puts "go to target R5-0"
tar -set -filter {name =~ "Cortex-R5 #0*"}
puts "reset target"
rst -proc
puts "download $1 onto target"
dow $1
puts "restart vmr.elf on target"
con

' > /tmp/scripts/vmr.tcl

# stop XRT drivers
rmmod xocl;rmmod xclmgmt

# upgrading vmr.elf
printf "\nupgrading vmr.elf via jtag ...\n"
xsdb /tmp/scripts/vmr.tcl $VMR
ERR=$?
if [ $ERR -ne 0 ];then
    echo "upgrade vmr error: $ERR"
    exit 1;
fi
echo "upgrade vmr done, reload XRT drivers..."

modprobe xocl;modprobe xclmgmt

#clean up
rm -rf /tmp/scripts

printf "\n $0 complete.\n"

