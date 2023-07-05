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

set( INA3221_TEST_FILES ./sensors/utt_ina3221.c
                     ../../mocks/common/utm_cl_i2c.c )

add_executable( test_ina3221 ${INA3221_TEST_FILES} ../../../sensors/src/ina3221.c )

target_link_libraries( test_ina3221
                       cmocka
                       -Wl,--wrap=i2c_send_rs_recv )

add_test( NAME test_ina3221
          COMMAND test_ina3221 )

set( ISL68221_TEST_FILES ./sensors/utt_isl68221.c
          ../../mocks/common/utm_cl_i2c.c )

add_executable( test_isl68221 ${ISL68221_TEST_FILES} ../../../sensors/src/isl68221.c )

target_link_libraries( test_isl68221
            cmocka
            -Wl,--wrap=i2c_send_rs_recv
            -Wl,--wrap=i2c_send)

add_test( NAME test_isl68221
            COMMAND test_isl68221 )


set( CAT34TS02_TEST_FILES ./sensors/utt_cat34ts02.c
            ../../mocks/common/utm_cl_i2c.c
            ../../mocks/common/utm_cl_logs.c)

add_executable( test_cat34ts02 ${CAT34TS02_TEST_FILES} ../../../sensors/src/cat34ts02.c )

target_link_libraries( test_cat34ts02
              cmocka
              -Wl,--wrap=i2c_send_rs_recv
              -Wl,--wrap=i2c_send
              -Wl,--wrap=cl_printf)

add_test( NAME test_cat34ts02
              COMMAND test_cat34ts02 )

set( LM75_TEST_FILES ./sensors/utt_lm75.c
        ../../mocks/common/utm_cl_i2c.c
        ../../mocks/common/utm_cl_logs.c)

add_executable( test_lm75 ${LM75_TEST_FILES} ../../../sensors/src/lm75.c )

target_link_libraries( test_lm75
                cmocka
                -Wl,--wrap=i2c_send_rs_recv
                -Wl,--wrap=i2c_send
                -Wl,--wrap=cl_printf)

add_test( NAME test_lm75
                COMMAND test_lm75 )

set( M24C128_TEST_FILES ./sensors/utt_m24c128.c
                ../../mocks/common/utm_cl_i2c.c
                ../../mocks/common/utm_cl_logs.c)

add_executable( test_m24c128 ${M24C128_TEST_FILES} ../../../sensors/src/m24c128.c )

target_link_libraries( test_m24c128
                        cmocka
                        -Wl,--wrap=i2c_send_rs_recv
                        -Wl,--wrap=i2c_send
                        -Wl,--wrap=cl_printf)

add_test( NAME test_m24c128
                        COMMAND test_m24c128 )

set( MAX6639_TEST_FILES ./sensors/utt_max6639.c
                        ../../mocks/common/utm_cl_i2c.c
                        ../../mocks/common/utm_cl_logs.c)

add_executable( test_max6639 ${MAX6639_TEST_FILES} ../../../sensors/src/max6639.c )

target_link_libraries( test_max6639
                                cmocka
                                -Wl,--wrap=i2c_send_rs_recv
                                -Wl,--wrap=i2c_send
                                -Wl,--wrap=cl_printf)

add_test( NAME test_max6639
            COMMAND test_max6639 )

set( TCA6416A_TEST_FILES ./sensors/utt_tca6416a.c
                        ../../mocks/common/utm_cl_i2c.c
                        ../../mocks/common/utm_cl_logs.c)

add_executable( test_tca6416a ${TCA6416A_TEST_FILES} ../../../sensors/src/tca6416a.c )

target_link_libraries( test_tca6416a
                                cmocka
                                -Wl,--wrap=i2c_send_rs_recv
                                -Wl,--wrap=i2c_send
                                -Wl,--wrap=cl_printf)

add_test( NAME test_tca6416a
            COMMAND test_tca6416a )

set( QSFP_TEST_FILES ./sensors/utt_qsfp.c
                     ../../mocks/common/utm_cl_i2c.c
                     ../../mocks/common/utm_cl_logs.c
                     ../../mocks/freertos/utm_queue.c)

add_executable( test_qsfp ${QSFP_TEST_FILES} ../../../sensors/src/qsfp.c )

target_link_libraries( test_qsfp
                       cmocka
                       -Wl,--wrap=i2c_send_rs_recv
                       -Wl,--wrap=i2c_send
                       -Wl,--wrap=cl_printf
                       -Wl,--wrap=vTaskDelay
                       )

add_test( NAME test_qsfp
          COMMAND test_qsfp )


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
                     ../../mocks/vmc/utm_vmc_asdm.c
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
                       -Wl,--wrap=ucISL68221ReadVoltage0
                       -Wl,--wrap=ucISL68221ReadVoltage1
                       -Wl,--wrap=ucISL68221ReadVoltage2
                       -Wl,--wrap=ucISL68221ReadCurrent0
                       -Wl,--wrap=ucISL68221ReadCurrent1
                       -Wl,--wrap=ucISL68221ReadCurrent2
                       -Wl,--wrap=ucISL68221ReadTemperature0
                       -Wl,--wrap=ucISL68221ReadTemperature1
                       -Wl,--wrap=ucISL68221ReadTemperature2
                       -Wl,--wrap=INA3221_ReadVoltage
                       -Wl,--wrap=INA3221_ReadCurrent
                       -Wl,--wrap=INA3221_ReadPower
                       -Wl,--wrap=ucs_clock_shutdown
                       -Wl,--wrap=VMC_Get_BoardInfo
                       -Wl,--wrap=ClockThrottling_Initialize)
		  
add_test( NAME test_asdm
          COMMAND test_asdm )

set( ASDM_V80_TEST_FILES ./utt_vmc_asdm_v80.c
                     ./utt_assert_SDRs.c
                     ../../mocks/common/utm_cl_mem.c
                     ../../mocks/common/utm_cl_logs.c
                     ../../mocks/freertos/utm_heap_4.c
                     ../../mocks/vmc/utm_vmc_sensor.c
                     ../../mocks/freertos/utm_queue.c
                     ../../mocks/vmc/sensors/src/utm_cat34ts02.c
                     ../../mocks/vmc/sensors/src/utm_lm75.c
                     ../../mocks/vmc/sensors/src/utm_qsfp.c
                     ../../mocks/vmc/sensors/src/utm_isl68221.c
                     ../../mocks/vmc/sensors/src/utm_ina3221.c
                     ../../mocks/vmc/sensors/src/utm_tca6416a.c
                     ../../mocks/vmc/utm_vmc_sc_comms.c
                     ../../mocks/vmc/utm_vmc_api.c
                     ../../mocks/vmc/utm_vmc_main_v80.c
                     ../../mocks/vmc/utm_vmc_update_sc.c
                     ../../mocks/vmc/utm_clock_throttling.c
                     ../../../platforms/v80.c )

add_executable( test_asdm_v80 ${ASDM_V80_TEST_FILES} ../../../vmc_asdm.c )

target_link_libraries( test_asdm_v80
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
                       -Wl,--wrap=ucQSFPI2CMuxReadTemperature
                       -Wl,--wrap=M24C128_ReadMultiBytes
                       -Wl,--wrap=M24C128_ReadByte
                       -Wl,--wrap=ucCAT34TS02ReadTemperature
                       -Wl,--wrap=ucISL68221ReadVoltage0
                       -Wl,--wrap=ucISL68221ReadVoltage1
                       -Wl,--wrap=ucISL68221ReadVoltage2
                       -Wl,--wrap=ucISL68221ReadCurrent0
                       -Wl,--wrap=ucISL68221ReadCurrent1
                       -Wl,--wrap=ucISL68221ReadCurrent2
                       -Wl,--wrap=ucISL68221ReadTemperature0
                       -Wl,--wrap=ucISL68221ReadTemperature1
                       -Wl,--wrap=ucISL68221ReadTemperature2
                       -Wl,--wrap=INA3221_ReadVoltage
                       -Wl,--wrap=INA3221_ReadCurrent
                       -Wl,--wrap=INA3221_ReadPower
                       -Wl,--wrap=ucTca6416aRegisterRead
                       -Wl,--wrap=ucTca6416aRegisterWrite
                       -Wl,--wrap=ucEnableDDRDIMM
                       -Wl,--wrap=ucs_clock_shutdown
                       -Wl,--wrap=VMC_Get_BoardInfo
                       -Wl,--wrap=ClockThrottling_Initialize)

add_test( NAME test_asdm_v80
          COMMAND test_asdm_v80 )

set( V80_TEST_FILES ./utt_v80.c
          ../../mocks/common/utm_cl_i2c.c
          ../../mocks/common/utm_cl_logs.c
          ../../mocks/freertos/utm_queue.c
          ../../mocks/common/utm_cl_mem.c
          ../../mocks/vmc/sensors/src/utm_cat34ts02.c
          ../../mocks/vmc/sensors/src/utm_lm75.c
          ../../mocks/vmc/sensors/src/utm_qsfp.c
          ../../mocks/vmc/sensors/src/utm_isl68221.c
          ../../mocks/vmc/sensors/src/utm_ina3221.c
          ../../mocks/vmc/sensors/src/utm_tca6416a.c
          ../../mocks/vmc/utm_vmc_sensor.c
          ../../mocks/vmc/utm_vmc_main_v80.c
          ../../mocks/vmc/utm_vmc_asdm.c
          ../../mocks/vmc/utm_vmc_api.c
          ../../mocks/vmc/utm_vmc_sc_comms.c
          ../../mocks/vmc/utm_clock_throttling.c
          )

add_executable( test_v80 ${V80_TEST_FILES} ../../../platforms/v80.c )

target_link_libraries( test_v80
            cmocka
            -Wl,--wrap=i2c_send_rs_recv
            -Wl,--wrap=i2c_send
            -Wl,--wrap=cl_printf
            -Wl,--wrap=vTaskDelay
            -Wl,--wrap=Cl_SecureMemcpy
            -Wl,--wrap=LM75_ReadTemperature
            -Wl,--wrap=ucISL68221ReadVoltage0
            -Wl,--wrap=ucISL68221ReadVoltage1
            -Wl,--wrap=ucISL68221ReadVoltage2
            -Wl,--wrap=ucISL68221ReadCurrent0
            -Wl,--wrap=ucISL68221ReadCurrent1
            -Wl,--wrap=ucISL68221ReadCurrent2
            -Wl,--wrap=ucISL68221ReadTemperature0
            -Wl,--wrap=ucISL68221ReadTemperature1
            -Wl,--wrap=ucISL68221ReadTemperature2
            -Wl,--wrap=INA3221_ReadVoltage
            -Wl,--wrap=INA3221_ReadCurrent
            -Wl,--wrap=INA3221_ReadPower
            -Wl,--wrap=ucCAT34TS02ReadTemperature
            -Wl,--wrap=ucQSFPI2CMuxReadTemperature
            -Wl,--wrap=ucs_clock_shutdown
            -Wl,--wrap=VMC_Get_BoardInfo
            -Wl,--wrap=ClockThrottling_Initialize
            -Wl,--wrap=ucEnableDDRDIMM
            )

add_test( NAME test_v80
COMMAND test_v80 )

#Code coverage for the src files
include(CodeCoverage.cmake)
append_coverage_compiler_flags()

set( COVERAGE_EXCLUDES ${SE98A_TEST_FILES}
                       ${INA3221_TEST_FILES}
                       ${CAT34TS02_TEST_FILES}
                       ${LM75_TEST_FILES}
                       ${M24C128_TEST_FILES}
                       ${MAX6639_TEST_FILES}
                       ${TCA6416A_TEST_FILES}
                       ${ISL68221_TEST_FILES}
                       ${QSFP_TEST_FILES}
                       ${V80_TEST_FILES}
                       ${ASDM_TEST_FILES}
                       ../../mocks/*
                       ../../../../../../build/build_dir/*
                       )

SETUP_TARGET_FOR_COVERAGE_LCOV(
        NAME test_coverage
        EXECUTABLE ctest
        DEPENDENCIES    test_se98a
                        test_ina3221
                        test_isl68221
                        test_cat34ts02
                        test_m24c128
                        test_max6639
                        test_tca6416a
                        test_lm75
                        test_qsfp
                        test_v80
                        test_asdm
                        test_asdm_v80 )
