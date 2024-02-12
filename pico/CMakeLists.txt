cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

set(PICO_BOARD pico_w CACHE STRING "Board type")

include(pico_sdk_import.cmake)

project(SensorPodController C CXX ASM)

pico_sdk_init()

string(APPEND CMAKE_EXE_LINKER_FLAGS "-Wl,--print-memory-usage")

add_executable(SensorPodController
    src_cpp/messaging/sensor_pod_messages.cpp
    src_cpp/messaging/multicore_mailbox.cpp

    src_cpp/network/network_controller.cpp

    src_cpp/sensors/sensirion/scd30_i2c.c
    src_cpp/sensors/sensirion/sensirion_common.c
    src_cpp/sensors/sensirion/sensirion_i2c_hal.c
    src_cpp/sensors/sensirion/sensirion_i2c.c

    src_cpp/sensors/sensor_i2c_interface.cpp
    src_cpp/sensors/stemma_soil_sensor.cpp
    src_cpp/sensors/sensor_pod.cpp

    src_cpp/serial_control/serial_controller.cpp

    src_cpp/userdata/user_data.cpp

    src_cpp/main.cpp
)

pico_set_linker_script(SensorPodController ${CMAKE_SOURCE_DIR}/memmap_custom.ld)

pico_enable_stdio_usb(SensorPodController 0)
pico_enable_stdio_uart(SensorPodController 1)
pico_add_extra_outputs(SensorPodController)

target_include_directories(SensorPodController PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}

    ${PROJECT_SOURCE_DIR}/src_cpp
    ${PROJECT_SOURCE_DIR}/src_cpp/network
    )

target_link_libraries(SensorPodController
    pico_stdlib
    hardware_adc
    hardware_i2c
    hardware_uart
    hardware_pio
    pico_util
    pico_multicore 
    pico_cyw43_arch_lwip_threadsafe_background
    pico_lwip_mqtt
)
