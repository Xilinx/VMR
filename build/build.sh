#!/bin/bash

source ./utils.sh

TOOL_VERSION="2021.2"
DEFAULT_VITIS="/proj/xbuilds/${TOOL_VERSION}_daily_latest/installs/lin64/Vitis/HEAD/settings64.sh"
STDOUT_JTAG=0
NOT_STABLE=0
ROOT_DIR=`pwd`
BSP_DIR="$ROOT_DIR/vmr_platform/vmr_platform/psv_cortexr5_0/freertos10_xilinx_domain/bsp/"
REAL_BSP=`realpath ../bsp/2021.2_stable/bsp`

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
	if [ -z $TA ];then
		echo "$DEFAULT_VITIS"
	else
		echo "$TA"
		DEFAULT_VITIS="/proj/xbuilds/${TA}/installs/lin64/Vitis/HEAD/settings64.sh"
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
	rm -r xsa .metadata vmr_platform vmr_system vmr 
}

append_stable_bsp() {
	echo "=== Using bsp in:$REAL_BSP. \"diff -rw dir1 dir2\" to compare bsp source files "
	rsync -rlviI $REAL_BSP vmr_platform/vmr_platform/psv_cortexr5_0/freertos10_xilinx_domain/
	check_result "rsync" $?
}

grep_file()
{
	typeset name="$1"
	grep -w "$name" ${BUILD_VERSION_FILE} |grep -oP '".*?"'|tail -1|tr -d '"'
}

grep_yes()
{
	grep -w "yes" ${PLATFORM_FILE} |grep -oP '".*"'|cut -d ":" -f1|tr -d '"'
}

make_version_h()
{
	if [ -z $BUILD_VERSION_FILE ];then
		echo "=== WARN: No build version is specified, trying to load local git version."
		VMR_VERSION_HASH=`git rev-parse --verify HEAD`
		VMR_VERSION_HASH_DATE=`git log -1 --pretty=format:%cD`
		VMR_BUILD_VERSION="1.0.0"
		VMR_BUILD_VERSION_DATE=`date`
		VMR_BUILD_BRANCH=`git rev-parse --abbrev-ref HEAD`
	else
		echo "=== Loading version from ${BUILD_VERSION_FILE}"
		VMR_VERSION_HASH=`grep_file "VERSION_HASH"` 
		VMR_VERSION_HASH_DATE=`grep_file "VERSION_HASH_DATE"`
		VMR_BUILD_VERSION=`grep_file "BUILD_VERSION"`
		VMR_BUILD_VERSION_DATE=`grep_file "BUILD_VERSION_DATE"`
		VMR_BUILD_BRANCH=`grep_file "BUILD_BRANCH"`
	fi

	# NOTE: we only take git version, version date and branch for now

	CL_VERSION_H="vmr/src/include/cl_version.h"

	echo "#ifndef _VMR_VERSION_" >> $CL_VERSION_H
	echo "#define _VMR_VERSION_" >> $CL_VERSION_H
	echo "#define VMR_TOOL_VERSION "\""$XSCT_VERSION"\" >> $CL_VERSION_H
	echo "#define VMR_GIT_HASH "\""$VMR_VERSION_HASH"\" >> $CL_VERSION_H
	echo "#define VMR_GIT_BRANCH "\""$VMR_BUILD_BRANCH"\" >> $CL_VERSION_H
	echo "#define VMR_GIT_HASH_DATE "\""$VMR_VERSION_HASH_DATE"\" >> $CL_VERSION_H

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
	if [[ ! -f "vmr/Debug/vmr.elf" ]];then
		echo "Build failed, cannot find vmr.elf"
		exit 1
	fi

	echo "=== VMR github info ==="
	arm-none-eabi-strings vmr/Debug/vmr.elf |grep -E "VMR_GIT|VMR_TOOL"

	echo "=== Build env info ==="
	if [ NOT_STABLE == 1 ];then
		echo "BSP is daily latest"
	else
		echo "BSP is appended from $REAL_BSP"
	fi

	if [ STDOUT_JTAG == 1 ];then
		echo "STDOUT is JTAG"
	else
		echo "STDOUT is UARTLite"
	fi

	echo "=== Build done ==="
}

build_app_all() {
	xsct ./create_app.tcl
	rsync -av ../src vmr --exclude cmc
	xsct ./config_app.tcl
	make_version_h
	xsct ./make_app.tcl
	check_vmr
}

build_app_incremental() {
	rm -r vmr/src
	rm -r vmr/Debug/vmr.elf
	rsync -av ../src vmr --exclude cmc --exclude *.swp
	make_version_h
	xsct ./make_app.tcl
	check_vmr
}

usage() {
    echo "Usage:"
    echo 
    echo "-clean                     Remove build directories"  
    echo "-xsa                       XSA file"  
    echo "-app                       Re-build Application only"  
    echo "-config_VMR                Update VMR project too edit in Vitis GUI"
    echo "-XRT                       Build XRT only"
    echo "-TA                        TA exact version, default is [${TOOL_VERSION}_daily_latest]"
    echo "-version                   version.json file"
    echo "-platform                  platform.json file for enable platform specific resources"
    echo "-jtag                      build VMR stdout to jtag"
    echo "-daily_latest              build VMR from daily latest bsp, otherwise use stable bsp from this VMR repo"
    echo "-help"
    exit $1
}

while [ $# -gt 0 ];
do
        case "$1" in
                -help)
                        usage 0
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
			shift
			STDOUT_JTAG=1
			;;
		-daily_latest)
			shift
			NOT_STABLE=1
			;;
                * | --* | -*)
                        echo "Invalid argument: $1"
                        usage 1
                        ;;
        esac
	shift
done

## Build code starts here ###
if [[ $BUILD_CLEAN == 1 ]];then
	build_clean
	exit 0;
fi

which xsct
if [ $? -ne 0 ];then
	default_env
fi
XSCT_VERSION=`which xsct|rev|cut -f3 -d"/"|rev`
echo "using xsct: ${XSCT_VERSION} from env to build VMR"

if [[ $BUILD_APP == 1 ]];then
	build_app_incremental
	exit 0;
fi

echo "=== Build BSP ==="
if [ -z $BUILD_XSA ];then
	echo "ERROR: Building BSP requires xsa.";
	usage
	exit 1;
fi

ls $BUILD_XSA
if [ $? -ne 0 ];then
	echo "cannot find ${BUILD_XSA}"
	exit 1;
fi

#####################
# build starts here #
#####################

#
# (1) always perform build clean for a clean build env
#
build_clean
mkdir xsa
cp $BUILD_XSA xsa/vmr.xsa
check_result "copy $BUILD_XSA" $?

#
# (2) using xsct to create bsp with jtag flag
#
echo "=== Create vmr_platform ==="
xsct ./create_bsp.tcl $STDOUT_JTAG $NOT_STABLE
check_result "Create vmr_platform" $?

#
# (3) override freertos and recompile it
#
if [ $NOT_STABLE == 1 ];then
	echo "=== Build BSP with daily_latest"
else
	echo "=== Build BSP with stable version"
	append_stable_bsp
	xsct ./rebuild_bsp.tcl
	check_result "Rebuild BSP" $?
#	cd $BSP_DIR
#	set -x
#make -C psv_cortexr5_0/libsrc/freertos10_xilinx_v1_10/src -s libs  "SHELL=/bin/sh" "COMPILER=armr5-none-eabi-gcc" "ASSEMBLER=armr5-none-eabi-as" "ARCHIVER=armr5-none-eabi-ar" "COMPILER_FLAGS=  -O2 -c -mcpu=cortex-r5" "EXTRA_COMPILER_FLAGS=-g -DARMR5 -Wall -Wextra -mfloat-abi=hard -mfpu=vfpv3-d16 -fno-tree-loop-distribute-patterns -Dversal"
#	set +x
#	check_result "Recompile bsp" $?
#	cd $ROOT_DIR
fi

#
# (4) build vmr application after platform bsp is built done
#
echo "=== Build APP ==="
build_app_all
