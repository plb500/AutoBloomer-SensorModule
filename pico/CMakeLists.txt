cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

set(PICO_BOARD pico_w CACHE STRING "Board type")

include(pico_sdk_import.cmake)

project(SensorPodController C CXX ASM)

pico_sdk_init()

string(APPEND CMAKE_EXE_LINKER_FLAGS "-Wl,--print-memory-usage")

add_executable(SensorPodController
    src/userdata/userdata.c

    src/serial_control/serial_controller.c

    src/cores/sensor_pod_core_0.c
    src/cores/sensor_pod_core_1.c
    src/cores/sensor_multicore_utils.c

    src/network/network_utils.c
    src/network/mqtt_utils.c

    src/sensor/sensor_i2c_interface.c
    src/sensor/sensor_pod.c
    src/sensor/scd30_sensor.c
    src/sensor/stemma_soil_sensor.c
)

pico_set_linker_script(SensorPodController ${CMAKE_SOURCE_DIR}/memmap_custom.ld)

pico_enable_stdio_usb(SensorPodController 1)
pico_enable_stdio_uart(SensorPodController 0)
pico_add_extra_outputs(SensorPodController)

target_include_directories(SensorPodController PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/.. # for our common lwipopts
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
)












# # Generated Cmake Pico project file

# cmake_minimum_required(VERSION 3.13)

# set(CMAKE_C_STANDARD 11)
# set(CMAKE_CXX_STANDARD 17)

# # Initialise pico_sdk from installed location
# # (note this can come from environment, CMake cache etc)
# set(PICO_SDK_PATH "/Users/philip/Development/Pico/pico-sdk")

# # Pull in Raspberry Pi Pico SDK (must be before project)
# include(pico_sdk_import.cmake)

# project(SensorPodController C CXX ASM)

# # Initialise the Raspberry Pi Pico SDK
# pico_sdk_init()


# pico_set_program_name(SensorPodController "SensorPodController")
# pico_set_program_version(SensorPodController "0.1")

# pico_enable_stdio_uart(SensorPodController 0)
# pico_enable_stdio_usb(SensorPodController 1)

# target_include_directories(SensorPodController
#         PRIVATE src
# ) 

# # Add any user requested libraries
# target_link_libraries(SensorPodController
#         pico_stdlib
#         hardware_i2c
#         pico_multicore
#         hardware_flash
#         hardware_sync
#         pico_util
#         pico_multicore 
#         pico_cyw43_arch_lwip_threadsafe_background
#         pico_lwip_mqtt
#     )

# pico_add_extra_outputs(SensorPodController)

