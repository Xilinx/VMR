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
    echo "-d    <bdf>       only provides bdf instead of whole string"
    echo "                  example: 0000:d8:00.0  only provides bdf like: -d d8"
    echo "-v    <vmr.elf>   vmr.elf location"
    echo "                  note: exclude with -p"
    echo "-p    <vmr.pdi>   vmr.pdi location"
    echo "                  note: exclude with -v"
    echo "examples:"
    echo "    When there is JTAG usb cable connected to only one card on host,  please use:"
    echo "    $0 -v vmr.elf"
    echo " "
    echo "    Without any usb cable connected to the card, please use:"
    echo "    $0 -p vmr.pdi -d bdf"
    echo 
}

############################################################
############################################################
# Main program                                             #
############################################################
############################################################

# script vars (change if required)
VMR=""
PDI=""
BDF=""
TOOL_VERSION="2022.2"
DEFAULT_VITIS="/proj/xbuilds/${TOOL_VERSION}_daily_latest/installs/lin64/Vitis/HEAD/settings64.sh"
JTAG=0

############################################################
# Process the input options. 
############################################################

# Get the options
while getopts ":hd:p:v:" option; do
   case $option in
      h) # display Help
         Help
         exit;;
      d) # card main bdf
         BDF=$OPTARG
	 echo " card selected: $BDF"
	 ;;
      v) # VMR.elf location
         VMR=$OPTARG
         echo "  vmr chosen: $VMR"
	 ;;
      p) # vmr.pdi location
         PDI=$OPTARG
	 echo " vmr.pdi chosen: $PDI"
	 ;;
      \?) # Invalid option
         echo "ERROR: Invalid option. See help below..."
         Help
         exit 1;;
   esac
done


############################################################
# check necessary files and tools exist
############################################################

if [[ -e "$VMR" && -e "$PDI" ]]; then
    echo "please either -v vmr.elf or -p vmr.pdi, -v and -p cannot be combined"
    exit 1
fi

upgrade_vmr_elf()
{
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

	echo "stop XRT drivers"
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
}

upgrade_vmr_pdi()
{
	source /opt/xilinx/xrt/setup.sh

	echo "stop XRT xocl drivers"
	rmmod xocl;

	echo "enable program_vmr flag"
	VMR_PROGRAM=`ls /sys/bus/pci/devices/0000:\$BDF\:00.0/xgq_vmr.m.*/program_vmr`
	if [[ $? -ne 0 ]];then
		echo "program_vmr doesn't exist"
		exit 1;
	fi
	# enable program vmr pdi
	echo 1 > $VMR_PROGRAM

	echo "VMR live upgrading ..."
	xbmgmt program -b shell --image $PDI -d $BDF:0 --force

	echo "Upgrade done, reload XRT drivers ..."
	modprobe xocl

	echo "check vmr is in good status"
	xbmgmt examine -d $BDF:0 -r vmr --verbose

	printf "\n $0 complete.\n"
}

############################################################
# script starts running from here
############################################################

if [[ -n "$VMR" ]];then
    upgrade_vmr_elf
fi

if [[ -n "$PDI" && -n "$BDF" ]];then
    upgrade_vmr_pdi
fi
