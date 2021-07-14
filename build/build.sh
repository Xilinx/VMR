#!/bin/bash

TOOL_VERSION="2021.2"
STABLE_VITIS=/proj/xbuilds/${TOOL_VERSION}_daily_latest/installs/lin64/Vitis/HEAD/settings64.sh
STABLE_XRT=/proj/xbuilds/${TOOL_VERSION}_daily_latest/xbb/xrt/packages/setenv.sh

default_env() {
	echo -ne "no xsct, using default stable version: "
	. ${STABLE_VITIS}
	which xsct
}

build_clean() {
	echo "=== Remove build directories ==="
	rm -r xsa .metadata rmgmt_platform rmgmt_system rmgmt
}

build_app_all() {
	xsct ./create_app.tcl
	rsync -av ../src rmgmt --exclude cmc
	xsct ./config_app.tcl
	xsct ./make_app.tcl
}

build_app_incremental() {
	rm -r rmgmt/src
	rsync -av ../src rmgmt --exclude cmc
	xsct ./make_app.tcl
}

usage() {
    echo "Usage:"
    echo 
    echo "-clean                     Remove build directories"  
    echo "-xsa                       XSA file"  
    echo "-app                       Re-build Application only"  
    echo "-help"
    exit $1
}

while [ $# -gt 0 ] && [[ $1 == "-"* ]];
do
        opt="$1";
        shift;
        case "$opt" in
                -help)
                        usage 0
                        ;;
                -xsa)
                        BUILD_XSA=$1
                        ;;
                -clean)
                        BUILD_CLEAN=1
                        ;;
		-app)
			BUILD_APP=1
			;;
                *)
                        echo "Invalid argument: $1"
                        usage 1
                        ;;
        esac
done

which xsct
if [ $? -ne 0 ];then
	default_env
else
	version=$(xsct -eval "puts [version]" | awk 'NR==1{print $2}')
	if [ ${version} != ${TOOL_VERSION} ]
	then
		echo -ne "Detected xsct version: ${version}, Please use xsct from ${TOOL_VERSION} to build"
		exit
	fi
fi

if [ -z $XILINX_XRT ];then
	. ${STABLE_XRT}
	echo $XILINX_XRT
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
	echo "Building BSP requires xsa.";
	usage
	exit 1;
fi

ls $BUILD_XSA
if [[ $? -ne 0 ]];then
	invalid $BUILD_XSA
	exit 1;
fi

# always perform build clean for a clean build env
build_clean
mkdir xsa
cp $BUILD_XSA xsa/gen3x16.xsa
if [[ $? -ne 0 ]];then
	invalid $BUILD_XSA
	exit 1;
fi

xsct ./create_bsp.tcl

echo "=== Build APP ==="
build_app_all
