SRC += $(VMR_MAIN)/vmr/src/vmc/vmc_asdm.c \
       $(VMR_MAIN)/vmr/src/vmc/platforms/v70.c \
       $(ROOT_DIR)/test/vmc/asdm/utt_vmc_asdm.c \
       $(ROOT_DIR)/test/vmc/asdm/utt_assert_SDRs.c \
       $(ROOT_DIR)/mocks/common/utm_cl_mem.c \
       $(ROOT_DIR)/mocks/common/utm_cl_logs.c \
       $(ROOT_DIR)/mocks/freertos/utm_heap_4.c \
       $(ROOT_DIR)/mocks/vmc/utm_vmc_sensor.c \
       $(ROOT_DIR)/mocks/freertos/utm_queue.c \
       $(ROOT_DIR)/mocks/vmc/sensors/src/utm_lm75.c \
       $(ROOT_DIR)/mocks/vmc/sensors/src/utm_isl68221.c \
       $(ROOT_DIR)/mocks/vmc/sensors/src/utm_ina3221.c \
       $(ROOT_DIR)/mocks/vmc/utm_vmc_sc_comms.c \
       $(ROOT_DIR)/mocks/vmc/utm_vmc_api.c \
       $(ROOT_DIR)/mocks/vmc/utm_vmc_main.c \
       $(ROOT_DIR)/mocks/vmc/utm_vmc_update_sc.c \
       $(ROOT_DIR)/mocks/vmc/utm_clock_throttling.c
       
#Source code for which code coverage report will be generated
COV_SRC = $(VMR_MAIN)/vmr/src/vmc/vmc_asdm.c \
          $(VMR_MAIN)/vmr/src/vmc/platforms/v70.c

MOCKS += Cl_SecureMemcpy \
	 Cl_SecureMemset \
	 Cl_SecureStrncmp \
	 Cl_SecureMemcmp \
	 pvPortMalloc \
	 vPortFree \
	 cl_printf \
	 Temperature_Read_ACAP_Device_Sysmon \
	 VCCINT_Read_ACAP_Device_Sysmon \
	 PMBUS_SC_Sensor_Read \
	 PMBUS_SC_Vccint_Read \
	 xQueueSemaphoreTake \
	 xQueueGenericSend \
	 LM75_ReadTemperature \
	 ISL68221_ReadVCCINT_Voltage \
	 ISL68221_ReadVCCINT_Current \
	 ISL68221_ReadVCCINT_Temperature \
	 INA3221_ReadVoltage \
	 INA3221_ReadCurrent \
	 INA3221_ReadPower \
	 ucs_clock_shutdown \
	 VMC_Get_BoardInfo \
	 ClockThrottling_Initialize

