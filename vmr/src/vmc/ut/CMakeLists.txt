cmake_minimum_required(VERSION 3.5.0)

project(VMR)

set(CMAKE_BUILD_TYPE Debug)
set(DEV xilinx_v70_gen5x8_qdma_2_202220_1)

include_directories(../ 
                    ../sensors/inc 
	            ../platforms
                    ../../include
	            ../../../../build/build_dir/${DEV}/export/${DEV}/sw/config0_0/freertos10_xilinx_domain/bspinclude/include)

add_subdirectory(../../../../../cmocka-1.1.5 ./cmocka-1.1.5)
add_subdirectory(./test/vmc)

