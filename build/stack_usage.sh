#!/bin/bash

# SPDX-License-Identifier: Apache-2.0
# Copyright (C) 2024 Advanced Micro Devices, Inc.    All rights reserved.

TOOL_VERSION="2022.1"
DEFAULT_VITIS="/proj/xbuilds/${TOOL_VERSION}_daily_latest/installs/lin64/Vitis/HEAD/settings64.sh"

############################################################
# Help                                                     #
############################################################
Help()
{
    # Display Help
    echo
    echo "Script for upgrading vmr.elf"
    echo
    echo "Usage: $0 [-h] -v <src dir>"
    echo "options:"
    echo "-h                print this help"
    echo "-v    <src dir>   (MUST BE SPECIFIED)"
    echo 
}

############################################################
############################################################
# Main program                                             #
############################################################
############################################################

# script vars (change if required)
SRC=""
AVSTACK="./avstack.pl"

############################################################
# Process the input options. 
############################################################

# Get the options
while getopts ":hv:" option; do
   case $option in
      h) # display Help
         Help
         exit;;
      v) # src dir location
         SRC=$OPTARG
         echo "src is from : $SRC";;
     \?) # Invalid option
         echo "ERROR: Invalid option. See help below..."
         Help
         exit 1;;
   esac
done


############################################################
# check necessary files and tools exist
############################################################

if [[ ! -e "$SRC" ]]; then
    echo "ERROR: Cannot find the .su data at \"$VMR\", -v option must be specified - see $0 -h"
    exit 1
fi


# sourcing vitis tooling
. ${DEFAULT_VITIS}

printf "\nanalyzing stack from source $SRC ...\n"
O_FILE=`find $SRC -name *.o`
$AVSTACK $O_FILE
ERR=$?
if [ $ERR -ne 0 ];then
    echo "error: $ERR"
    exit 1;
fi

printf "\n $0 complete.\n"

