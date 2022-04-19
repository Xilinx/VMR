# VMR Code

## Code Review procedure

### 1. Code Review Checklist (for all developers who can submit PRs)

        1.1 Will this change potentially stop VMR running? 
            > If yes, please explain more details.

        1.2 Is coding style aligned with FreeRTOS code style?
            > https://www.freertos.org/FreeRTOS-Coding-Standard-and-Style-Guide.html#StyleGuide

        1.3 A CR or Story number is required for every PR. And PR # should be updated back to CR/Story.
            > PR reviewer should not apporve any PR without a tracking number

        1.4 Please address all code review comments prior to ask for code merge.

>       Failing to do so might end up with "git revert" due to "need more work".
	
	
### 2. Code Merge Checklist (for maintainers who can merge the code to gate)

        2.1 Is there a unit test report?
            > unit test should be performed on TA XRT + TA Shell + TA APU;

        2.2 Are there 2+ code reviewers approved the PR on github?

>       Failing to do so might end up with "git revert" if serious regression found. 

# === The internal links below will be removed before open source the VMR github ===

# Build VMR and shell

## Install all following pkgs from TA

	XRT: 		/proj/xbuilds/2022.1_daily_latest/xbb/xrt/packages/
	Shell-base: 	/proj/xbuilds/2022.1_daily_latest/xbb/packages/internal_platforms/vck5000/gen4x8_xdma/base/
	Shell-dev: 	/proj/xbuilds/2022.1_daily_latest/xbb/packages/internal_platforms/vck5000/gen4x8_xdma/1-202120-1-dev/
	APU: 		/proj/xbuilds/2022.1_daily_latest/internal_platforms/sw/versal/apu_packages/versal/

## Build VMR

### Build VMR against xsa from TA
    on the server with TA Shell-dev pkgs
        root# <VMR>/build/build.sh -xsa xsa=/opt/xilinx/platforms/xilinx_vck5000_gen4x8_xdma_2_202210_1/hw/xilinx_vck5000_gen4x8_xdma_2_202210_1.xsa

### Build VMR with stable cached BSP (experienced developer only)
    on any machine
        $ <VMR>build/build.sh -stable

## Build shell
    on the server with TA Shell-base, Shell-dev pkgs
        root# <VMR>build/regen_pdi.sh -v <vmr.elf>
