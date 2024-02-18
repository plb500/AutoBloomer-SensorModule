cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)

set(PICO_BOARD pico_w CACHE STRING "Board type")

include(pico_sdk_import.cmake)

project(SensorPodController C CXX ASM)

pico_sdk_init()

string(APPEND CMAKE_EXE_LINKER_FLAGS "-Wl,--print-memory-usage")

add_executable(SensorPodController
    src/cores/core_0_executor.cpp
    src/cores/core_1_executor.cpp

    # src/messaging/sensor_pod_messages.cpp
    src/messaging/sensor_data_message.cpp
    src/messaging/sensor_control_message.cpp
    src/messaging/mqtt_message.cpp
    src/messaging/multicore_mailbox.cpp

    src/network/network_controller.cpp
    src/network/mqtt_controller.cpp

    src/sensors/sensirion/scd30_i2c.c
    src/sensors/sensirion/sensirion_common.c
    src/sensors/sensirion/sensirion_i2c_hal.c
    src/sensors/sensirion/sensirion_i2c.c

    src/sensors/sensor_i2c_interface.cpp
    src/sensors/stemma_soil_sensor.cpp
    # src/sensors/sensor_pod.cpp
    src/sensors/sensor.cpp
    src/sensors/sensor_group.cpp
    src/sensors/dummy_sensor.cpp
    src/sensors/scd30_sensor.cpp
    src/sensors/stemma_soil_sensor.cpp

    src/serial_control/serial_controller.cpp

    src/userdata/user_data.cpp

    src/main.cpp
    src/sensor_hardware.cpp
)

pico_set_linker_script(SensorPodController ${CMAKE_SOURCE_DIR}/memmap_custom.ld)

pico_enable_stdio_usb(SensorPodController 0)
pico_enable_stdio_uart(SensorPodController 1)
pico_add_extra_outputs(SensorPodController)

target_include_directories(SensorPodController PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}

    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/src/network
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
    pico_rand
)

target_compile_definitions(SensorPodController PUBLIC
    DUMMY=1
    SENSOR_POD=2
    HIB=3

    SENSOR_HARDWARE=SENSOR_POD
)
