# XRT_RPU

## System Requirements
    - Build RPU platform and applications, linux with Xilinx Vitis software is recommended.
    - Run RPU applications, a MPSOC based platform like vck5000, zcu102 are recommended.
      Note: do not support hw emulation, like qemu.

## Build Instructions
    To build entire platform and applications:
      cd build
      ./build.sh -xsa /public/bugcases/CR/1086000-1086999/1086872/20210330/gen3x16.xsa

    To build entire application:
      cd build:
      ./build.sh -app

    To build


## Use Cases

    1. Install customized shell and XRT
    2. Flash shell
    3. Program xclbin
      
