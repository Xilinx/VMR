cmake_minimum_required( VERSION 3.5.0 )

project(VMR)

include( CTest )
enable_testing()

set( SE98A_TEST_FILES ./sensors/utt_se98a.c
                     ../../mocks/common/utm_cl_i2c.c )

add_executable( test_se98a ${SE98A_TEST_FILES} ../../../sensors/src/se98a.c )

target_link_libraries( test_se98a 
                       cmocka 
                       -Wl,--wrap=i2c_send_rs_recv )

add_test( NAME test_se98a 
          COMMAND test_se98a )
		  
set( ASDM_TEST_FILES ./utt_vmc_asdm.c
                     ./utt_assert_SDRs.c
                     ../../mocks/common/utm_cl_mem.c
                     ../../mocks/common/utm_cl_logs.c
                     ../../mocks/freertos/utm_heap_4.c
                     ../../mocks/vmc/utm_vmc_sensor.c
                     ../../mocks/freertos/utm_queue.c
                     ../../mocks/vmc/sensors/src/utm_lm75.c
                     ../../mocks/vmc/sensors/src/utm_isl68221.c
                     ../../mocks/vmc/sensors/src/utm_ina3221.c
                     ../../mocks/vmc/utm_vmc_sc_comms.c
                     ../../mocks/vmc/utm_vmc_api.c
                     ../../mocks/vmc/utm_vmc_main.c
                     ../../mocks/vmc/utm_vmc_update_sc.c
                     ../../mocks/vmc/utm_clock_throttling.c
                     ../../../platforms/v70.c )
					
add_executable( test_asdm ${ASDM_TEST_FILES} ../../../vmc_asdm.c )

target_link_libraries( test_asdm
                       cmocka 
                       -Wl,--wrap=Cl_SecureMemcpy
                       -Wl,--wrap=Cl_SecureMemset
                       -Wl,--wrap=Cl_SecureStrncmp
                       -Wl,--wrap=Cl_SecureMemcmp
                       -Wl,--wrap=pvPortMalloc
                       -Wl,--wrap=vPortFree
                       -Wl,--wrap=cl_printf
                       -Wl,--wrap=Temperature_Read_ACAP_Device_Sysmon
                       -Wl,--wrap=VCCINT_Read_ACAP_Device_Sysmon
                       -Wl,--wrap=PMBUS_SC_Sensor_Read
                       -Wl,--wrap=PMBUS_SC_Vccint_Read
                       -Wl,--wrap=xQueueSemaphoreTake
                       -Wl,--wrap=xQueueGenericSend
                       -Wl,--wrap=LM75_ReadTemperature
                       -Wl,--wrap=ISL68221_ReadVCCINT_Voltage
                       -Wl,--wrap=ISL68221_ReadVCCINT_Current
                       -Wl,--wrap=ISL68221_ReadVCCINT_Temperature
                       -Wl,--wrap=INA3221_ReadVoltage
                       -Wl,--wrap=INA3221_ReadCurrent
                       -Wl,--wrap=INA3221_ReadPower
                       -Wl,--wrap=ucs_clock_shutdown
                       -Wl,--wrap=VMC_Get_BoardInfo
                       -Wl,--wrap=ClockThrottling_Initialize)
		  
add_test( NAME test_asdm
          COMMAND test_asdm )

#Code coverage for the src files
if( COVERAGE_ENABLE )
    include(CodeCoverage.cmake)
    append_coverage_compiler_flags()

    set(COVERAGE_EXCLUDES 
		${SE98A_TEST_FILES}
		${ASDM_TEST_FILES} )

    SETUP_TARGET_FOR_COVERAGE_LCOV(
        NAME test_coverage
        EXECUTABLE ctest
        DEPENDENCIES 
            test_se98a 
            test_asdm
    )
endif()
