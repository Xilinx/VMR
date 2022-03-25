# VMR Code

## Code Review procedure

### 1. Code Review Checklist (for all developers who can submit PRs)

        1.1 Will this change potentially stop VMR running? 
            > If yes, please explain more details.

        1.2 Is coding style aligned with FreeRTOS code style?
            > https://www.freertos.org/FreeRTOS-Coding-Standard-and-Style-Guide.html#StyleGuide

        1.3 A CR or Story number is required for every PR. And PR # should be updated back to CR/Story.
            > PR reviewer should not apporve any PR without a tracking number

        1.4 Please address all code review commens prior to ask for code merge.

>       Failing to do so might end up with "git revert" due to "need more work".
	
	
### 2. Code Merge Checklist (for maintainers who can merge the code to gate)

        2.1 Is there a unit test report?
            > unit test should be performed on TA XRT + TA Shell + TA APU;

        2.2 Are there 2+ code reviewers approved the PR on github?

>       Failing to do so might end up with "git revert" if serious regression found. 

# === The following sections will be removed before open source the VMR github ===

# VMR Build

## System Requirements

	Xlinx linux which can run 2021.2 or 2022.1 Vitis software.
	do not support hw emulation, like qemu.

## TA location 2022.1

	XRT: 	/proj/xbuilds/2022.1_daily_latest/xbb/xrt/packages/
	Shell: 	/proj/xbuilds/2022.1_daily_latest/xbb/packages/internal_platforms/vck5000/gen4x8_xdma/base/
	XSA: 	/proj/xbuilds/2022.1_daily_latest/xbb/packages/internal_platforms/vck5000/gen4x8_xdma/1-202120-1-dev/
	APU: 	/proj/xbuilds/2022.1_daily_latest/internal_platforms/sw/versal/apu_packages/versal/

### 1. Preparation:

#### get xsa from TA

	rpm2cpio /proj/xbuilds/2022.1_daily_latest/xbb/packages/internal_platforms/vck5000/gen4x8_xdma/1-202120-1-dev/<xxx>.noarch.rpm |cpio -idmv
	cp ./opt/xilinx/platforms/xilinx_vck5000_gen4x8_xdma_1_202120_1/hw/xilinx_vck5000_gen4x8_xdma_1_202120_1.xsa <your own dir>

	Example:
	rpm2cpio /proj/xbuilds/2022.1_daily_latest/xbb/packages/internal_platforms/vck5000/gen4x8_xdma/1-202120-1-dev/xilinx-vck5000-gen4x8-xdma-1-202120-1-dev-1-3394490.noarch.rpm |cpio -idmv
	cp ./opt/xilinx/platforms/xilinx_vck5000_gen4x8_xdma_1_202120_1/hw/xilinx_vck5000_gen4x8_xdma_1_202120_1.xsa <your own dir>

### 2. Build:

#### build from xsa

	cd build
	export FORCE_MARK_AS_EDGE_XSA=1
	./build.sh -xsa <your own dir>/xilinx_vck5000_gen4x8_xdma_1_202120_1.xsa

 	Example:
 	./build.sh -xsa /proj/rdi/staff/davidzha/share/xgq/xilinx_vck5000_gen4x8_xdma_1_202120_1.xsa

#### build vmr.elf without rebuilding VITIS platform

	cd build
	./build.sh -app

## Build shell PDI 

### 1. Preparation:
### collect partition.pdi

	rpm2cpio /proj/xbuilds/2022.1_daily_latest/xbb/packages/internal_platforms/vck5000/gen4x8_xdma/base/<xxx>.noarch.rpm |cpio -idmv
	xclbinutil -i ./lib/firmware/xilinx/<uuid>/partition.xsabin --dump-section PDI:RAW:partition.pdi
	cp ./partition.pdi <your own dir>

	Example:
	rpm2cpio /proj/xbuilds/2022.1_daily_latest/xbb/packages/internal_platforms/vck5000/gen4x8_xdma/base/xilinx-vck5000-gen4x8-xdma-base-1-3394490.noarch.rpm |cpio -idmv
	xclbinutil -i ./lib/firmware/xilinx/3bc1b9162e6d8c76849419b33c259de3/partition.xsabin --dump-section PDI:RAW:partition.pdi
	cp ./partition.pdi <your own dir>

### 2. Build:
#### make rpu.bif

	$ cat rpu.bif 
	all:
	{
	    image {
		{ type=bootimage, file=partition.pdi }
	    }
	    image {
		id = 0x1c000000, name=rpu_subsystem
		{ core=r5-0, file=vmr.elf }
	    }
	}

#### generate rpu.pdi
	bootgen -arch versal -padimageheader=0 -log trace -w -o rpu.pdi -image rpu.bif
