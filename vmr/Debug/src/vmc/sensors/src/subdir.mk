################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/vmc/sensors/src/ina3221.c \
../src/vmc/sensors/src/lm75.c \
../src/vmc/sensors/src/m24c128.c \
../src/vmc/sensors/src/max6639.c \
../src/vmc/sensors/src/qsfp.c \
../src/vmc/sensors/src/se98a.c 

OBJS += \
./src/vmc/sensors/src/ina3221.o \
./src/vmc/sensors/src/lm75.o \
./src/vmc/sensors/src/m24c128.o \
./src/vmc/sensors/src/max6639.o \
./src/vmc/sensors/src/qsfp.o \
./src/vmc/sensors/src/se98a.o 

C_DEPS += \
./src/vmc/sensors/src/ina3221.d \
./src/vmc/sensors/src/lm75.d \
./src/vmc/sensors/src/m24c128.d \
./src/vmc/sensors/src/max6639.d \
./src/vmc/sensors/src/qsfp.d \
./src/vmc/sensors/src/se98a.d 


# Each subdirectory must supply rules for building sources it contributes
src/vmc/sensors/src/%.o: ../src/vmc/sensors/src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM R5 gcc compiler'
	armr5-none-eabi-gcc -DARMR5 -Wall -O0 -g3 -I../src/include -c -fmessage-length=0 -MT"$@" -mcpu=cortex-r5  -mfloat-abi=hard  -mfpu=vfpv3-d16 -Werror -I../../bsp/psv_cortexr5_0/include -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


