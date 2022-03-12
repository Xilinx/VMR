################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/vmc/vmc_api.c \
../src/vmc/vmc_asdm.c \
../src/vmc/vmc_demo.c \
../src/vmc/vmc_main.c \
../src/vmc/vmc_sc_comms.c \
../src/vmc/vmc_sensors.c \
../src/vmc/vmc_update_sc.c 

OBJS += \
./src/vmc/vmc_api.o \
./src/vmc/vmc_asdm.o \
./src/vmc/vmc_demo.o \
./src/vmc/vmc_main.o \
./src/vmc/vmc_sc_comms.o \
./src/vmc/vmc_sensors.o \
./src/vmc/vmc_update_sc.o 

C_DEPS += \
./src/vmc/vmc_api.d \
./src/vmc/vmc_asdm.d \
./src/vmc/vmc_demo.d \
./src/vmc/vmc_main.d \
./src/vmc/vmc_sc_comms.d \
./src/vmc/vmc_sensors.d \
./src/vmc/vmc_update_sc.d 


# Each subdirectory must supply rules for building sources it contributes
src/vmc/%.o: ../src/vmc/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM R5 gcc compiler'
	armr5-none-eabi-gcc -DARMR5 -Wall -O0 -g3 -I../src/include -c -fmessage-length=0 -MT"$@" -mcpu=cortex-r5  -mfloat-abi=hard  -mfpu=vfpv3-d16 -Werror -I../../bsp/psv_cortexr5_0/include -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


