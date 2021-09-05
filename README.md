# VMR

## System Requirements
    - Xlinx linux which can run 2021.2 Vitis software.
    - do not support hw emulation, like qemu.

## Build Instructions
    To build entire platform and applications:
      cd build
      ./build.sh -xsa /public/bugcases/CR/1086000-1086999/1086872/20210412/gen3x16.xsa

      Note: latest xsa only works with 2021.2 vitis, if you have your own vitis env, please set to 2021.2 or later version.


    To build entire application:
      cd build:
      ./build.sh -app

