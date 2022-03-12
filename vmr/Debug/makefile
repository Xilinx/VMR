################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/vmc/sensors/src/subdir.mk
-include src/vmc/subdir.mk
-include src/tests/subdir.mk
-include src/sysmon/subdir.mk
-include src/rmgmt/subdir.mk
-include src/common/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
ELFSIZE += \
vmr.elf.size \


# All Target
all: vmr.elf secondary-outputs

# Tool invocations
vmr.elf: $(OBJS) ../src/lscript.ld $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: ARM R5 gcc linker'
	armr5-none-eabi-gcc -mcpu=cortex-r5  -mfloat-abi=hard  -mfpu=vfpv3-d16 -Wl,-T -Wl,../src/lscript.ld -L../../bsp/psv_cortexr5_0/lib -o "vmr.elf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

vmr.elf.size: vmr.elf
	@echo 'Invoking: ARM R5 Print Size'
	armr5-none-eabi-size vmr.elf  |tee "vmr.elf.size"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(EXECUTABLES)$(OBJS)$(S_UPPER_DEPS)$(C_DEPS)$(ELFSIZE) vmr.elf
	-@echo ' '

secondary-outputs: $(ELFSIZE)

.PHONY: all clean dependents

-include ../makefile.targets
