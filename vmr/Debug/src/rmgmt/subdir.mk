################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/rmgmt/rmgmt_clock.c \
../src/rmgmt/rmgmt_fpt.c \
../src/rmgmt/rmgmt_main.c \
../src/rmgmt/rmgmt_xclbin.c \
../src/rmgmt/rmgmt_xfer.c 

OBJS += \
./src/rmgmt/rmgmt_clock.o \
./src/rmgmt/rmgmt_fpt.o \
./src/rmgmt/rmgmt_main.o \
./src/rmgmt/rmgmt_xclbin.o \
./src/rmgmt/rmgmt_xfer.o 

C_DEPS += \
./src/rmgmt/rmgmt_clock.d \
./src/rmgmt/rmgmt_fpt.d \
./src/rmgmt/rmgmt_main.d \
./src/rmgmt/rmgmt_xclbin.d \
./src/rmgmt/rmgmt_xfer.d 


# Each subdirectory must supply rules for building sources it contributes
src/rmgmt/%.o: ../src/rmgmt/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM R5 gcc compiler'
	armr5-none-eabi-gcc -DARMR5 -Wall -O0 -g3 -I../src/include -c -fmessage-length=0 -MT"$@" -mcpu=cortex-r5  -mfloat-abi=hard  -mfpu=vfpv3-d16 -Werror -I../../bsp/psv_cortexr5_0/include -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


