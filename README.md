# VMR

## System Requirements
    - Xlinx linux which can run 2021.2 Vitis software.
    - do not support hw emulation, like qemu.

## Build Instructions
    To build entire platform and applications:
      cd build
      ./build.sh -xsa /public/bugcases/CR/1105000-1105999/1105240/2021_11_24_Drop_20/xilinx_vck5000_gen4x8_xdma_base_1.xsa

    To build entire application:
      cd build:
      ./build.sh -app

## TODO:
    1. using xsa in TA.
