cmake_minimum_required(VERSION 3.5.0)

project(VMR)

#Fetch the cmocka src files from the remote repository
execute_process(COMMAND git submodule update --init cmocka
                WORKING_DIRECTORY ../../../../../
                ERROR_QUIET)

set(CMAKE_BUILD_TYPE Debug)
set(DEV xilinx_v70_gen5x8_qdma_3_202310_1)

include_directories(./mocks/common
                    ../
                    ../sensors/inc 
                    ../platforms
                    ../../include
                    ../../../../build/build_dir/${DEV}/export/${DEV}/sw/config0_0/freertos10_xilinx_domain/bspinclude/include)

add_subdirectory(../../../../cmocka ./cmocka-1.1.5)
add_subdirectory(./test/vmc)

