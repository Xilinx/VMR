################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/sysmon/sysmon.c \
../src/sysmon/sysmon_dcg.c \
../src/sysmon/sysmon_services.c 

OBJS += \
./src/sysmon/sysmon.o \
./src/sysmon/sysmon_dcg.o \
./src/sysmon/sysmon_services.o 

C_DEPS += \
./src/sysmon/sysmon.d \
./src/sysmon/sysmon_dcg.d \
./src/sysmon/sysmon_services.d 


# Each subdirectory must supply rules for building sources it contributes
src/sysmon/%.o: ../src/sysmon/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM R5 gcc compiler'
	armr5-none-eabi-gcc -DARMR5 -Wall -O0 -g3 -I../src/include -c -fmessage-length=0 -MT"$@" -mcpu=cortex-r5  -mfloat-abi=hard  -mfpu=vfpv3-d16 -Werror -I../../bsp/psv_cortexr5_0/include -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


