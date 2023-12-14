cmake_minimum_required(VERSION 3.5)
project(modbusCService LANGUAGES C)

#set (CMAKE_C_FLAGS "-std=gnu99 ${CMAKE_C_FLAGS}")

# uninstall target
if(NOT TARGET uninstall)
  configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
endif()

find_library(MODBUS_LIB modbus)
if(NOT MODBUS_LIB)
  message(FATAL_ERROR "libmodbus library not found! Please install. #apt install libmodbus-dev")
endif()

find_library(MOSQUITTO_LIB mosquitto)
if(NOT MOSQUITTO_LIB)
  message(FATAL_ERROR "mosquitto library not found! Please install. #apt install libmosquitto-dev")
endif()


add_executable(modbusCService
        modbusCService.c
        helperf.h
        helperf.c
        mb.h
        mb.c
        mq.h
        mq.c
        asService.h
        asService.c
        )

install(TARGETS modbusCService DESTINATION bin)

install(FILES modbuscs.conf modbusCService.service_example DESTINATION /etc/modbusCService)
install(PROGRAMS modbus_scr.sh mqtt_scr.sh DESTINATION /etc/modbusCService)

target_link_libraries(modbusCService PUBLIC modbus)
target_link_libraries(modbusCService PUBLIC mosquitto)
