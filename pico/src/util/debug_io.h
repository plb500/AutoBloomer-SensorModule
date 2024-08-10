#ifndef DEBUG_IO_H
#define DEBUG_IO_H

#ifdef __cplusplus
extern "C"
{
#endif

#if DEBUG_PRINT_ON
#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/sync.h"
#   define DEBUG_PRINT_BUF_SIZE                         (128)

extern mutex_t DEBUG_PRINT_MUTEX;
extern char DEBUG_PRINT_BUF[DEBUG_PRINT_BUF_SIZE];
extern const char* CORE_PREFIXES[2];

// Initialize the debug logging system
#   if LIB_PICO_STDIO_UART                                      
#       define DEBUG_PRINT_INIT()                              {            \
            mutex_init(&DEBUG_PRINT_MUTEX);                                 \
            gpio_set_function(PICO_DEFAULT_UART_TX_PIN, GPIO_FUNC_UART);    \
            gpio_set_function(PICO_DEFAULT_UART_RX_PIN, GPIO_FUNC_UART);    \
            uart_init(STDIO_UART, STDIO_UART_BAUDRATE);                     \
            uart_set_format (STDIO_UART, 8, 1, UART_PARITY_NONE);           \
            uart_set_hw_flow(STDIO_UART, false, false);                     \
            stdio_init_all();                                               \
        }
#   else                                                           
#       define DEBUG_PRINT_INIT() {                                 \
            stdio_init_all();                                       \
}
#   endif

// Print the supplied sformatted string to debug out UART
#   if LIB_PICO_STDIO_UART                                      
#       define DEBUG_PRINT(core_num, format, ...) {                                             \
            mutex_enter_blocking(&DEBUG_PRINT_MUTEX);                                           \
                                                                                                \
            const char* pfx = CORE_PREFIXES[core_num];                                          \
            snprintf(DEBUG_PRINT_BUF, DEBUG_PRINT_BUF_SIZE, format __VA_OPT__(,) __VA_ARGS__);  \
            uart_puts(STDIO_UART, pfx);                                                         \
            uart_puts(STDIO_UART, DEBUG_PRINT_BUF);                                             \
            uart_puts(STDIO_UART, "\n");                                                        \
                                                                                                \
            mutex_exit(&DEBUG_PRINT_MUTEX);                                                     \
        }
#   else
#       define DEBUG_PRINT(format, ...) {                                                       \
            snprintf(DEBUG_PRINT_BUF, DEBUG_PRINT_BUF_SIZE, format __VA_OPT__(,) __VA_ARGS__);  \
            puts(DEBUG_PRINT_BUF);                                                              \
        }                                                   
#   endif   

// Shut down the debug logging UART
#   define DEBUG_PRINT_DEINIT()                            {    \
        uart_deinit(STDIO_UART);                                \
    }
#else
// No-op all debug logging functions
#   define DEBUG_PRINT_INIT()                              {}
#   define DEBUG_PRINT(format, ...)                        {}
#   define DEBUG_PRINT_DEINIT()                            {}
#endif

#ifdef __cplusplus
}
#endif

#endif      // DEBUG_IO_H
