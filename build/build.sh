#!/bin/bash

STABLE_VITIS=/proj/xbuilds/2020.2_daily_latest/installs/lin64/Vitis/HEAD/settings64.sh

default_env() {
	echo -ne "no xsct, using default stable version: "
	. ${STABLE_VITIS}
	which xsct
}

build_clean() {
	echo "=== Remove build directories ==="
	rm -r xsa .metadata rmgmt_platform rmgmt_system rmgmt
}

build_app() {
	if [ ! -d "rmgmt" ];then
		echo "no application found"
		exit 1;
	fi
	xsct ./make_app.tcl
}

usage() {
    echo "Usage:"
    echo 
    echo "-clean                     Remove build directories"  
    echo "-xsa                       XSA file"  
    echo "-app                       Build Application only"  
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
fi

if [[ $BUILD_CLEAN == 1 ]];then
	build_clean
	exit 0;
fi

if [[ $BUILD_APP == 1 ]];then
	build_app
	exit 0;
fi

echo "=== Build BSP ==="
# always perform build clean for a clean build env
build_clean
mkdir xsa
if [ -z $BUILD_XSA ];then
	cp ../xsa/gen3x16.xsa xsa/gen3x16.xsa
else
	cp $BUILD_XSA xsa/gen3x16.xsa
	if [[ $? -ne 0 ]];then
		invalid $BUILD_XSA
		exit 1;
	fi
fi


xsct ./create_bsp.tcl

echo "=== Build APP ==="
xsct ./create_app.tcl
cp -r ../src rmgmt
xsct ./config_app.tcl
xsct ./make_app.tcl
