#!/bin/bash

TOOL_VERSION="2021.2"
DEFAULT_VITIS="/proj/xbuilds/${TOOL_VERSION}_daily_latest/installs/lin64/Vitis/HEAD/settings64.sh"

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

make_version_h()
{
	CL_VERSION_H="vmr/src/include/cl_version.h"
	GIT_HASH=`git rev-parse --verify HEAD`
	GIT_BRANCH=`git rev-parse --abbrev-ref HEAD`
	GIT_BUILD_DATE=`git log -1 --pretty=format:%cD`

	echo "#ifndef _VMR_VERSION_" >> $CL_VERSION_H
	echo "#define _VMR_VERSION_" >> $CL_VERSION_H
	echo "#define VMR_GIT_HASH "\""$GIT_HASH"\" >> $CL_VERSION_H
	echo "#define VMR_GIT_BRANCH "\""$GIT_BRANCH"\" >> $CL_VERSION_H
	echo "#define VMR_GIT_BUILD_DATE "\""$GIT_BUILD_DATE"\" >> $CL_VERSION_H

	if [[ $BUILD_XRT == 1 ]];then
		echo "=== XRT only build ==="
		echo "#define VMR_BUILD_XRT_ONLY" >> $CL_VERSION_H
	else
		echo "=== Full build ==="
	fi
	echo "#endif" >> $CL_VERSION_H
}

check_vmr() {
	if [[ ! -f "vmr/Debug/vmr.elf" ]];then
		echo "build failed, cannot find vmr.elf"
		exit 1
	fi
	echo "=== VMR github info ==="
	arm-none-eabi-strings vmr/Debug/vmr.elf |grep VMR_GIT
	echo "=== VMR github info ==="
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
                * | --* | -*)
                        echo "Invalid argument: $1"
                        usage 1
                        ;;
        esac
	shift
done

which xsct
if [ $? -ne 0 ];then
	default_env
else
	version=$(xsct -eval "puts [version]" | awk 'NR==1{print $2}')
	echo "using current ${version} from env to build VMR"
fi

if [[ $BUILD_CLEAN == 1 ]];then
	build_clean
	exit 0;
fi

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

# always perform build clean for a clean build env
build_clean
mkdir xsa
cp $BUILD_XSA xsa/gen3x16.xsa
if [ $? -ne 0 ];then
	echo "cannot copy ${BUILD_XSA}"
	exit 1;
fi

xsct ./create_bsp.tcl

echo "=== Build APP ==="
build_app_all
