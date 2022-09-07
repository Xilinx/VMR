#!/bin/bash

# SPDX-License-Identifier: Apache-2.0
# Copyright (C) 2018-2022 Xilinx, Inc. All rights reserved.

TOOL_VERSION="2022.2"
DEFAULT_VITIS="/proj/xbuilds/${TOOL_VERSION}_daily_latest/installs/lin64/Vitis/HEAD/settings64.sh"
STDOUT_JTAG=2 # 0: uart0; 1: uart1; 2: default to uartlite
BUILD_XRT=0
ROOT_DIR=`pwd`
#REAL_BSP=`realpath ../bsp/2021.2_stable/bsp`
#REAL_VMR=`realpath ../vmr`
BUILD_CONF_FILE="build.json"
BUILD_DIR="build_dir"
BUILD_LOG="build.log"
REGEN_SHELL="regen_pdi.sh"
BUILD_CLEAN=0

check_result()
{
	typeset log="$1"
	typeset ret="$2"

	if [ $2 -ne 0 ];then
		cat $BUILD_LOG
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

	if [ ! -z $BUILD_APP ];then
		echo "=== build app, skip loading from $BUILD_CONF_FILE ==="
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

	CONF_BUILD_SHELL=`grep_file "CONF_BUILD_SHELL" ${BUILD_CONF_FILE}`
	if [ -z $BUILD_SHELL ];then
		BUILD_SHELL=$CONF_BUILD_SHELL
	fi

	CONF_BUILD_AIE2=`grep_file "CONF_BUILD_AIE2" ${BUILD_CONF_FILE}`
	if [ -z $CONF_BUILD_AIE2 ];then
		echo "not aie2 shell"
	else
		BUILD_AIE2=$CONF_BUILD_AIE2;
	fi

	CONF_BUILD_JTAG=`grep_file "CONF_BUILD_JTAG" ${BUILD_CONF_FILE}`
	if [ -z $CONF_BUILD_JTAG ];then
		echo "skip loading CONF_BUILD_JTAG"
	else
		# override STDOUT_JTAG
		STDOUT_JTAG=$CONF_BUILD_JTAG;
	fi

	echo "================================"
	echo "BUILD_TA: $BUILD_TA"
	echo "BUILD_XSA: $BUILD_XSA"
	echo "BUILD_XSABIN: $BUILD_XSABIN"
	echo "PLATFORM_FILE: $PLATFORM_FILE"
	echo "================================"
}

check_file_exists()
{
	typeset FILE="$1"
	if [ -z $FILE ] || [ ! -f $FILE ];then
		echo "ERROR: $FILE doesn't exist";exit 1;
	fi
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
		check_file_exists "${BUILD_VERSION_FILE}"
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
	check_file_exists "$CL_VERSION_H"

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
		check_file_exists "$PLATFORM_FILE"
		for MACRO in `grep_yes`
		do
			echo "===    enable: $MACRO"
			echo "#define $MACRO" >> $CL_VERSION_H
		done
	fi
	echo "#endif" >> $CL_VERSION_H
}

check_vmr()
{
	typeset C_DIR="$1"
	typeset VMR_FILE="${C_DIR}/Debug/vmr_app.elf"

	if [[ ! -f "${VMR_FILE}" ]];then
		echo "Build failed, cannot find $VMR_FILE"
		cat $BUILD_DIR/$BUILD_LOG
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

	echo "xsct: $BUILD_TA"
	echo "XSA: $BUILD_XSA"
	echo "XSA_PLATFORM_NAME: $XSA_PLATFORM_NAME"

	if [ ! -z $PLATFORM_FILE ];then
		echo "PLATFROM patch is: $PLATFORM_FILE"
	fi

	if [ $STDOUT_JTAG == 1 ];then
		echo "STDOUT is JTAG1"
	elif [ $STDOUT_JTAG == 0 ];then
		echo "STDOUT is JTAG0"
	else
		echo "STDOUT is default UARTLite"
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

	cd $ROOT_DIR
	make_version_h "$BUILD_DIR/vmr_app"

	cd $BUILD_DIR
	xsct ./make_app.tcl >> $BUILD_LOG 2>&1

	cd $ROOT_DIR
	check_vmr "$BUILD_DIR/vmr_app"
}

build_vmr_source() {
	cd $ROOT_DIR/$BUILD_DIR/vmr_app/Debug
	if [ $? -ne 0 ];then
		echo "Please rebuild entire project"
		exit 1;
	fi
	
	start_seconds=$SECONDS

	# copy new source file
	cd $ROOT_DIR
	rsync -a ../vmr/src "$BUILD_DIR/vmr_app" --exclude cmc --exclude *.swp
	make_version_h "$BUILD_DIR/vmr_app"

	cd $ROOT_DIR/$BUILD_DIR/vmr_app/Debug
	make clean;make -j

	cd $ROOT_DIR
	check_vmr "$BUILD_DIR/vmr_app"

	echo "=== Make App Took: $((SECONDS - start_seconds)) S"
}

build_app_incremental() {
	cd $ROOT_DIR/$BUILD_DIR
	if [ $? -ne 0 ];then
		echo "Please rebuild entire project"
		exit 1;
	fi

	start_seconds=$SECONDS

	# Remove source and elf file
	rm -r vmr_app/src
	rm -r vmr_app/Debug/vmr_app.elf

	# copy new source file
	cd $ROOT_DIR
	rsync -a ../vmr/src "$BUILD_DIR/vmr_app" --exclude cmc --exclude *.swp
	make_version_h "$BUILD_DIR/vmr_app"

	cd $BUILD_DIR
	xsct ./make_app.tcl

	cd $ROOT_DIR
	check_vmr "$BUILD_DIR/vmr_app"

	echo "=== Make App Took: $((SECONDS - start_seconds)) S"
}

build_shell()
{
	cd $ROOT_DIR
	if [ -z $BUILD_SHELL ] || [ ! $BUILD_SHELL = "yes" ] || [ ! -f $REGEN_SHELL ];then
		echo "Skip Build Shell";exit 0;
	fi

	cp $REGEN_SHELL $BUILD_DIR
	cd $BUILD_DIR

	if [ -z $BUILD_AIE2 ] || [ ! $BUILD_AIE2 = "yes" ];then
		bash $REGEN_SHELL -v $ROOT_DIR/vmr.elf -x $BUILD_XSA -y $BUILD_XSABIN >> $BUILD_LOG 2>&1
	else
		bash $REGEN_SHELL -a -v $ROOT_DIR/vmr.elf -x $BUILD_XSA -y $BUILD_XSABIN >> $BUILD_LOG 2>&1
	fi

	ls rebuilt.xsabin
	if [ $? -eq 0 ];then
		realpath rebuilt.xsabin
	else
		echo "Build Shell failed."
		cat $BUILD_LOG
	fi
}

# Obsolated since 2022.1 #
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
    echo "Usage:"
    echo 
    echo "-config <config file.json> config json instead of using default build.json"  
    echo "-clean                     Remove build directories"  
    echo "-xsa <xsa file.xsa>        XSA file"  
    echo "-config_VMR                Update VMR project too edit in Vitis GUI"
    echo "-version                   version.json file"
    echo "-platform                  platform.json file for enable platform specific resources"
    echo "-jtag [0|1|2]              RPU console is on jtag uart 0, 1, or 2 for uartlite 0; APU is always on uartlite 1."
    echo "-config <json file>        build with pre configured options"
    echo "-app                       Re-build Vitis Application only"  
    echo "-vmr                       Re-build vmr source code only, fatest!!!"  
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
		-vmr)
			BUILD_VMR=1
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
		-version)
			shift
			BUILD_VERSION_FILE=$1
			;;
		-platform)
			shift
			PLATFORM_FILE=$1
			;;
		-jtag)
			shift
			STDOUT_JTAG=$1
			;;
		-stable)
			echo "bypass, obsolated"
			usage 1
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

echo "=== build log =="
echo "" > $ROOT_DIR/$BUILD_DIR/$BUILD_LOG
echo "tail -f $ROOT_DIR/$BUILD_DIR/$BUILD_LOG"

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

# only build vmr source code
if [[ $BUILD_VMR == 1 ]];then
	build_vmr_source
	exit 0;
fi

# only build app by vitis
if [[ $BUILD_APP == 1 ]];then
	build_app_incremental
	exit 0;
fi

# default build based on cached stable bsp
if [ -z $BUILD_XSA ] || [ $BUILD_XSA == "No" ];then
	echo "=== No XSA specified, build from stable BSP.";
	build_clean
	build_bsp_stable
	exit 0;
fi

#####################
# build entire BSP  #
#####################
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

echo "=== (4) Build shell "
start_seconds=$SECONDS
build_shell
echo "=== Build Shell Took: $((SECONDS - start_seconds)) S"
