#include "util/debug_io.h"
#include "userdata/user_data.h"
#include "messaging/multicore_mailbox.h"
#include "cores/core_0_executor.h"
#include "cores/core_1_executor.h"

#include "pico/multicore.h"

MulticoreMailbox multicoreMailbox;

Core0Executor dataCore0(multicoreMailbox);
Core1Executor dataCore1(multicoreMailbox);

int main() {
    // Setup stdio
    DEBUG_PRINT_INIT()

    sleep_ms(3000);

    // Initialize cores
    dataCore0.initialize();
    Core0Executor::setExecutor(dataCore0);

    dataCore1.initialize();
    Core1Executor::setExecutor(dataCore1);

    // Run our separate cores
    multicore_launch_core1(Core1Executor::loop);
    Core0Executor::loop();

    return 1;
}