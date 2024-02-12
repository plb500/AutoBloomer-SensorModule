#ifndef DEBUG_IO_H
#define DEBUG_IO_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include "pico/stdlib.h"

#define DEBUG_UART_TX_PIN       (0)
#define DEBUG_UART_RX_PIN       (1)
#define STDIO_UART_BAUDRATE     (57600)
#define STDIO_UART              (uart0)



// Debug/Serial output macro block
#define DEBUG_PRINT_ON                                  1      // Set to 0 to disable all stdio UART logging

#if DEBUG_PRINT_ON
#include <stdio.h>
#   define DEBUG_PRINT_BUF_SIZE                         (128)
static char DEBUG_PRINT_BUF[DEBUG_PRINT_BUF_SIZE];

// Initialize the debug logging system
#   if LIB_PICO_STDIO_UART                                      
#       define DEBUG_PRINT_INIT()                              {    \
            gpio_set_function(DEBUG_UART_TX_PIN, GPIO_FUNC_UART);   \
            gpio_set_function(DEBUG_UART_RX_PIN, GPIO_FUNC_UART);   \
            uart_init(STDIO_UART, STDIO_UART_BAUDRATE);             \
            uart_set_format (STDIO_UART, 8, 1, UART_PARITY_NONE);   \
            uart_set_hw_flow(STDIO_UART, false, false);             \
            stdio_init_all();                                       \
        }
#   else                                                           
#       define DEBUG_PRINT_INIT() {                                 \
            stdio_init_all();                                       \
}
#   endif

// Print the supplied sformatted string to debug out UART
#   if LIB_PICO_STDIO_UART                                      
#       define DEBUG_PRINT(format, ...) {                                                       \
            snprintf(DEBUG_PRINT_BUF, DEBUG_PRINT_BUF_SIZE, format __VA_OPT__(,) __VA_ARGS__);  \
            uart_puts(STDIO_UART, DEBUG_PRINT_BUF);                                             \
            uart_puts(STDIO_UART, "\n");                                                        \
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
