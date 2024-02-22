target_sources(SensorPodController PRIVATE
    sensor_hardware.cpp
)

target_include_directories(SensorPodController PRIVATE
    .
)

target_compile_definitions(SensorPodController PUBLIC
    PICO_DEFAULT_UART_TX_PIN=0
    PICO_DEFAULT_UART_RX_PIN=1
    STDIO_UART_BAUDRATE=57600
    STDIO_UART=uart0
)
