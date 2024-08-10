#include "pico/sync.h"
#include "debug_io.h"

mutex_t DEBUG_PRINT_MUTEX;
char DEBUG_PRINT_BUF[DEBUG_PRINT_BUF_SIZE];
const char* CORE_PREFIXES[2] = {
    "[0] ",
    "    [1] "
};
