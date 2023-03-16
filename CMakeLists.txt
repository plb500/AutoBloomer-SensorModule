# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)
set(PICO_SDK_PATH "/Users/philip/Development/Pico/pico-sdk")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(SensorPodController C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

add_executable(SensorPodController 
        SensorPodController.c
        menu/user_int_input_menu.c
        menu/menu_interface.c
        menu/main_menu.c
        menu/continuous_measurement_menu.c
        sensor/sensor_pod.c
        sensor/sensor_i2c_interface.c
        sensor/stemma_soil_sensor.c
        sensor/scd30_sensor.c
        hardware/shift_register.c
)

pico_set_program_name(SensorPodController "SensorPodController")
pico_set_program_version(SensorPodController "0.1")

pico_enable_stdio_uart(SensorPodController 1)
pico_enable_stdio_usb(SensorPodController 0)

target_include_directories(SensorPodController
        PRIVATE menu
        PRIVATE sensor
        PRIVATE hardware
) 

# Add the standard library to the build
target_link_libraries(SensorPodController pico_stdlib)

# Add any user requested libraries
target_link_libraries(SensorPodController
        hardware_i2c
)

pico_add_extra_outputs(SensorPodController)

