# VMR

## System Requirements
    - Xlinx linux which can run 2021.2 Vitis software.
    - do not support hw emulation, like qemu.

## Build Instructions
    To build entire platform and applications:
      cd build

      ./build.sh -xsa /public/bugcases/CR/1105000-1105999/1105240/2021_10_14_Drop_16/xilinx_vck5000_gen4x8_xdma_base_1.xsa

      /* Older working xsa */
      ./build.sh -xsa /public/bugcases/CR/1086000-1086999/1086872/20210412/gen3x16.xsa


    To build entire application:
      cd build:
      ./build.sh -app

