#!/bin/bash

# SPDX-License-Identifier: Apache-2.0
# Copyright (C) 2018-2022 Xilinx, Inc. All rights reserved.

TOOL_VERSION="2022.1"
DEFAULT_VITIS="/proj/xbuilds/${TOOL_VERSION}_daily_latest/installs/lin64/Vitis/HEAD/settings64.sh"
STDOUT_JTAG=0
NOT_STABLE=1
BUILD_XRT=0
ROOT_DIR=`pwd`
#REAL_BSP=`realpath ../bsp/2021.2_stable/bsp`
#REAL_VMR=`realpath ../vmr`
BUILD_CONF_FILE="build.json"
BUILD_DIR="build_dir"
BUILD_LOG="build.log"

check_result()
{
	typeset log="$1"
	typeset ret="$2"

	if [ $2 -ne 0 ];then
		echo "$1 err: $2"
		exit 1;
	fi
}

default_env() {
	echo -ne "no xsct, using version: "
	if [ -z $BUILD_TA ];then
		echo "$DEFAULT_VITIS"
		BUILD_TA="${TOOL_VERSION}_daily_latest"
	else
		echo "$BUILD_TA"
		DEFAULT_VITIS="/proj/xbuilds/${BUILD_TA}/installs/lin64/Vitis/HEAD/settings64.sh"
	fi

	ls ${DEFAULT_VITIS}
	if [ $? -ne 0 ];then
		echo "cannot find ${DEFAULT_VITIS}"
		exit 1;
	fi

	. ${DEFAULT_VITIS}
	which xsct
}

build_clean() {
	echo "=== Remove build directories ==="
	rm -rf xsa .metadata vmr_platform vmr_system vmr 
	rm -rf build_tmp_dir
	rm -rf $BUILD_DIR
}

append_stable_bsp() {
	echo "=== Using bsp in:$REAL_BSP. \"diff -rw dir1 dir2\" to compare bsp source files "
	rsync -rlviI $REAL_BSP vmr_platform/vmr_platform/psv_cortexr5_0/freertos10_xilinx_domain/
	check_result "rsync" $?
}

grep_file()
{
	typeset name="$1"
	typeset file="$2"
	grep -w "$name" ${file} |grep -oP '".*?"'|tail -1|tr -d '"'
}

grep_yes()
{
	grep -w "yes" ${PLATFORM_FILE} |grep -oP '".*"'|cut -d ":" -f1|tr -d '"'
}

load_build_info()
{
	if [ ! -z "$1" ];then
		echo "=== set build conf file to $1"
		BUILD_CONF_FILE="$1"
	fi

	if [ -f $BUILD_CONF_FILE ];then
		echo "=== build conf file is $BUILD_CONF_FILE"
	else
		echo "=== cannot find $BUILD_CONF_FILE"
		exit 1	
	fi

	if [ ! -z $BUILD_XSA ];then
		echo "=== build from xsa, skip loading from $BUILD_CONF_FILE ==="
		return
	fi

	if [ $NOT_STABLE == 0 ];then
		echo "=== build from stable bsp, skip loading from $BUILD_CONF_FILE ==="
		return
	fi

	CONF_BUILD_TA=`grep_file "CONF_BUILD_TA" ${BUILD_CONF_FILE}`
	if [ -z $BUILD_TA ];then
		BUILD_TA=$CONF_BUILD_TA
	fi

	CONF_BUILD_XSA=`grep_file "CONF_BUILD_XSA" ${BUILD_CONF_FILE}`
	if [ -z $BUILD_XSA ];then
		BUILD_XSA=$CONF_BUILD_XSA
	fi

	CONF_BUILD_XSABIN=`grep_file "CONF_BUILD_XSABIN" ${BUILD_CONF_FILE}`
	if [ -z $BUILD_XSABIN ];then
		BUILD_XSABIN=$CONF_BUILD_XSABIN
	fi

	CONF_BUILD_PLATFORM=`grep_file "CONF_BUILD_PLATFORM" ${BUILD_CONF_FILE}`
	if [ -z $PLATFORM_FILE ];then
		PLATFORM_FILE=$CONF_BUILD_PLATFORM
	fi

	#enforce jtag mode for pipeline build
	STDOUT_JTAG=1

	echo "================================"
	echo "BUILD_TA: $BUILD_TA"
	echo "BUILD_XSA: $BUILD_XSA"
	echo "BUILD_XSABIN: $BUILD_XSABIN"
	echo "PLATFORM_FILE: $PLATFORM_FILE"
	echo "================================"
}

make_version_h()
{
	typeset C_DIR="$1"

	if [ -z $BUILD_VERSION_FILE ];then
		echo "=== WARN: No build version is specified, trying to load local git version."
		VMR_VERSION_HASH=`git rev-parse --verify HEAD`
		VMR_VERSION_HASH_DATE=`git log -1 --pretty=format:%cD`
		VMR_BUILD_BRANCH=`git rev-parse --abbrev-ref HEAD`
		VMR_VERSION_RELEASE="0"
		VMR_VERSION_MAJOR="0"
		VMR_VERSION_MINOR="0"
		VMR_VERSION_PATCH="0"
	else
		echo "=== Loading version from ${BUILD_VERSION_FILE}"
		VMR_VERSION_HASH=`grep_file "VERSION_HASH" ${BUILD_VERSION_FILE}` 
		VMR_VERSION_HASH_DATE=`grep_file "VERSION_HASH_DATE" ${BUILD_VERSION_FILE}`
		VMR_BUILD_BRANCH=`grep_file "BUILD_BRANCH" ${BUILD_VERSION_FILE}`
		VMR_VERSION_RELEASE=`grep_file "VMR_VERSION_RELEASE" ${BUILD_VERSION_FILE}`
		VMR_VERSION_MAJOR=`grep_file "VMR_VERSION_MAJOR" ${BUILD_VERSION_FILE}`
		VMR_VERSION_MINOR=`grep_file "VMR_VERSION_MINOR" ${BUILD_VERSION_FILE}`
		VMR_VERSION_PATCH=`grep_file "VMR_VERSION_PATCH" ${BUILD_VERSION_FILE}`

	fi
	VMR_BUILD_VERSION_DATE=`date`
	VMR_BUILD_VERSION="$VMR_VERSION_RELEASE.$VMR_VERSION_MAJOR.$VMR_VERSION_MINOR.$VMR_VERSION_PATCH"

	# NOTE: we only take git version, version date and branch for now

	CL_VERSION_H="${C_DIR}/src/include/cl_version.h"

	echo "#ifndef _VMR_VERSION_" >> $CL_VERSION_H
	echo "#define _VMR_VERSION_" >> $CL_VERSION_H
	echo "#define VMR_TOOL_VERSION "\""$BUILD_TA"\" >> $CL_VERSION_H
	echo "#define VMR_GIT_HASH "\""$VMR_VERSION_HASH"\" >> $CL_VERSION_H
	echo "#define VMR_GIT_BRANCH "\""$VMR_BUILD_BRANCH"\" >> $CL_VERSION_H
	echo "#define VMR_GIT_HASH_DATE "\""$VMR_VERSION_HASH_DATE"\" >> $CL_VERSION_H
	echo "#define VMR_BUILD_VERSION_DATE "\""$VMR_BUILD_VERSION_DATE"\" >> $CL_VERSION_H
	echo "#define VMR_BUILD_VERSION "\""$VMR_BUILD_VERSION"\" >> $CL_VERSION_H

	if [[ $BUILD_XRT == 1 ]];then
		echo "=== XRT only build ==="
		echo "#define VMR_BUILD_XRT_ONLY" >> $CL_VERSION_H
	else
		echo "=== Full build ==="
	fi

	echo "" >> $CL_VERSION_H

	# set platform specific config macros
	if [ -z $PLATFORM_FILE ];then
		echo "=== NOTE: No platform specific resources."
	else
		echo "=== NOTE: enable $PLATFORM_FILE specific resources."
		for MACRO in `grep_yes`
		do
			echo "===    enable: $MACRO"
			echo "#define $MACRO" >> $CL_VERSION_H
		done
	fi
	echo "#endif" >> $CL_VERSION_H
}

check_vmr() {
	typeset C_DIR="$1"
	typeset VMR_FILE="${C_DIR}/Debug/vmr_app.elf"

	if [[ ! -f "${VMR_FILE}" ]];then
		echo "Build failed, cannot find $VMR_FILE"
		exit 1
	fi

	#echo "=== VMR EndPoints info ==="
	#arm-none-eabi-strings $VMR_FILE |grep "^VMR_EP"

	echo "=== VMR github info ==="
	arm-none-eabi-strings $VMR_FILE |grep -E "VMR_GIT|VMR_TOOL"

	echo "=== Build version info ==="
	arm-none-eabi-strings $VMR_FILE |grep -E "VMR_BUILD_VERSION"

	echo "=== Build env info ==="

	if [ ! -z $CONF_BUILD_XSA ];then
		echo "BUILD from config file: $BUILD_CONF_FILE"
	else
		echo "BUILD from user specified config"
	fi

	if [ $NOT_STABLE == 1 ];then
		echo "xsct: $BUILD_TA"
		echo "XSA: $BUILD_XSA"
		echo "XSA_PLATFORM_NAME: $XSA_PLATFORM_NAME"
	else
		echo "BSP and XSA are copied from: $REAL_BSP"
	fi


	if [ ! -z $PLATFORM_FILE ];then
		echo "PLATFROM patch is: $PLATFORM_FILE"
	fi

	if [ $STDOUT_JTAG == 1 ];then
		echo "STDOUT is JTAG"
	else
		echo "STDOUT is default"
	fi

	if [ $BUILD_XRT == 1 ];then
		echo "XRT only build"
	else
		echo "Full Build"
	fi

	echo "=== Build vmr.elf ==="
	cp $VMR_FILE $ROOT_DIR/vmr.elf
	realpath $ROOT_DIR/vmr.elf
	echo "=== Build done. ==="
}

build_app_all() {
	cd $ROOT_DIR
	rsync -a ../vmr/src $BUILD_DIR/vmr_app --exclude cmc

	cp config_app.tcl $BUILD_DIR
	cp make_app.tcl $BUILD_DIR
	cd $BUILD_DIR

	xsct ./config_app.tcl >> $BUILD_LOG 2>&1
	make_version_h "vmr_app"

	xsct ./make_app.tcl >> $BUILD_LOG 2>&1

	check_vmr "vmr_app"
}

build_app_incremental() {
	cd $ROOT_DIR/$BUILD_DIR
	rm -r vmr_app/src
	rm -r vmr_app/Debug/vmr.elf

	rsync -av ../../vmr/src vmr_app --exclude cmc --exclude *.swp
	make_version_h "vmr_app"

	start_seconds=$SECONDS
	xsct ./make_app.tcl
	echo "=== Make App Took: $((SECONDS - start_seconds)) S"

	check_vmr "vmr_app"
}

build_bsp_stable() {
	echo "=== since 2022.1 release, do not support build -stable anymore, exit";exit 0;

	echo "make build_tmp_dir"
	mkdir build_tmp_dir
	echo "rsync and make BSP"
	rsync -aq $REAL_BSP build_tmp_dir
	cd build_tmp_dir/bsp 
	make clean;make all
	check_result "make stable BSP" $?
	cd $ROOT_DIR

	echo "rsync and make VMR"
	rsync -aq $REAL_VMR build_tmp_dir
	make_version_h "build_tmp_dir"
	cd build_tmp_dir/vmr/Debug
	make clean;make all
	check_result "make VMR" $?
	cd $ROOT_DIR
	check_vmr "build_tmp_dir"
}

usage() {
    echo "Instruction: PLEASE READ!!!"
    echo "Usage:"
    echo 
    echo "-clean                     Remove build directories"  
    echo "-xsa                       XSA file"  
    echo "-app                       Re-build Application only"  
    echo "-config_VMR                Update VMR project too edit in Vitis GUI"
    echo "-XRT                       Build XRT only"
    echo "-TA                        TA exact version, default is [${TOOL_VERSION}_daily_latest]."
    echo "                           Note: only take effect when env has no Vitis env"
    echo "-version                   version.json file"
    echo "-platform                  platform.json file for enable platform specific resources"
    echo "-jtag                      build VMR stdout to jtag"
    echo "-daily_latest              build VMR from daily latest bsp (this is enabled by default)"
    echo "-stable                    build VMR from cached stable bsp (this is not enabled by default)"
    echo "-help"
    exit $1
}

while [ $# -gt 0 ];
do
        case "$1" in
                -help)
                        usage 0
                        ;;

                -config)
			shift
                        BUILD_CONFIG=$1
                        ;;
                -xsa)
			shift
                        BUILD_XSA=$1
                        ;;
                -clean)
                        BUILD_CLEAN=1
                        ;;
		-app)
			BUILD_APP=1
			;;
		-config_VMR)
			if [ ! -f vmr/.project  ]; then
				echo "vmr project was not initialized. Please run ./build.sh -xsa <path to your xsa>"
				exit 1
			fi
			./update_VMR_vitis_project.sh
			;;
		-XRT)
			BUILD_XRT=1
			;;
		-TA)
			shift
			TA=$1
			;;
		-version)
			shift
			BUILD_VERSION_FILE=$1
			;;
		-platform)
			shift
			PLATFORM_FILE=$1
			;;
		-jtag)
			STDOUT_JTAG=1
			;;
		-daily_latest)
			NOT_STABLE=1
			;;
		-stable)
			NOT_STABLE=0
			;;
                * | --* | -*)
                        echo "Invalid argument: $1"
                        usage 1
                        ;;
        esac
	shift
done

#####################
# build starts here #
#####################

if [[ $BUILD_CLEAN == 1 ]];then
	build_clean
	exit 0;
fi

#####################
# load build.json to set options if the option is not explicitly set
# this will make sure pipeline automation script pick up correct TA
# submit PR to pipeline to move build.json forward.
#####################
load_build_info $BUILD_CONFIG

which xsct
if [ $? -ne 0 ];then
	default_env
else
	# use pre-sourced vitis TA, set to correct BUILD_TA
	BUILD_TA=`which xsct|cut -f5 -d"/"`
fi
XSCT_VERSION=`which xsct|rev|cut -f3 -d"/"|rev`
echo "Tools version: ${XSCT_VERSION}"

# option1: only build app by vitis
if [[ $BUILD_APP == 1 ]];then
	build_app_incremental
	exit 0;
fi

# option2: default build based on cached stable bsp
if [ -z $BUILD_XSA ] || [ $BUILD_XSA == "No" ];then
	echo "=== No XSA specified, build from stable BSP.";
	build_clean
	build_bsp_stable
	exit 0;
fi

#####################
# build entire BSP  #
#####################
echo "=== build log =="
echo "tail -f $ROOT_DIR/$BUILD_DIR/$BUILD_LOG"
echo "=== Build BSP from xsa: ==="
ls $BUILD_XSA
if [ $? -ne 0 ];then
	echo "cannot find ${BUILD_XSA}"
	exit 1;
fi

echo "=== (1) Build clean and preparation ..."
build_clean
mkdir $BUILD_DIR
cp $BUILD_XSA $BUILD_DIR/vmr.xsa
check_result "copy $BUILD_XSA" $?


echo "=== (2) Create entire project, including platform BSP and application  "
cp create_bsp.tcl $BUILD_DIR
cd $BUILD_DIR
start_seconds=$SECONDS
xsct ./create_bsp.tcl $STDOUT_JTAG > $BUILD_LOG 2>&1
check_result "Create vmr_platform" $?
echo "=== Create BSP Took: $((SECONDS - start_seconds)) S"


echo "=== (3) Build entire project "
start_seconds=$SECONDS
build_app_all
echo "=== Make App Took: $((SECONDS - start_seconds)) S"
