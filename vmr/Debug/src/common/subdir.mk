################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/common/cl_i2c.c \
../src/common/cl_log.c \
../src/common/cl_main.c \
../src/common/cl_ospi_polled.c \
../src/common/cl_uart_rtos.c \
../src/common/cl_xgq_client.c \
../src/common/cl_xgq_server.c \
../src/common/cl_mem.c 

OBJS += \
./src/common/cl_i2c.o \
./src/common/cl_log.o \
./src/common/cl_main.o \
./src/common/cl_ospi_polled.o \
./src/common/cl_uart_rtos.o \
./src/common/cl_xgq_client.o \
./src/common/cl_xgq_server.o \
./src/common/cl_mem.o

C_DEPS += \
./src/common/cl_i2c.d \
./src/common/cl_log.d \
./src/common/cl_main.d \
./src/common/cl_ospi_polled.d \
./src/common/cl_uart_rtos.d \
./src/common/cl_xgq_client.d \
./src/common/cl_xgq_server.d \
./src/common/cl_mem.d


# Each subdirectory must supply rules for building sources it contributes
src/common/%.o: ../src/common/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM R5 gcc compiler'
	armr5-none-eabi-gcc -DARMR5 -Wall -O0 -g3 -I../src/include -c -fmessage-length=0 -MT"$@" -mcpu=cortex-r5  -mfloat-abi=hard  -mfpu=vfpv3-d16 -Werror -I../../bsp/psv_cortexr5_0/include -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


