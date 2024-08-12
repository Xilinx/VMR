collect(PROJECT_LIB_HEADERS vmc_update_sc.h)
collect(PROJECT_LIB_HEADERS vmc_asdm.h)
collect(PROJECT_LIB_HEADERS clock_throttling.h)
collect(PROJECT_LIB_HEADERS vmc_api.h)
collect(PROJECT_LIB_HEADERS vmc_sensors.h)
collect(PROJECT_LIB_HEADERS vmc_sc_comms.h)
collect(PROJECT_LIB_HEADERS vmc_main.h)
collect(PROJECT_LIB_HEADERS rmi.h)


collect(PROJECT_LIB_SOURCES vmc_sc_comms.c)
collect(PROJECT_LIB_SOURCES rmi.c)
collect(PROJECT_LIB_SOURCES vmc_demo.c)
collect(PROJECT_LIB_SOURCES vmc_api.c)
collect(PROJECT_LIB_SOURCES vmc_asdm.c)
collect(PROJECT_LIB_SOURCES vmc_update_sc.c)
collect(PROJECT_LIB_SOURCES vmc_sensors.c)
collect(PROJECT_LIB_SOURCES vmc_main.c)
collect(PROJECT_LIB_SOURCES clock_throttling.c)


add_subdirectory(sensors)
add_subdirectory(platforms)
