#include "util/debug_io.h"
#include "messaging/multicore_mailbox.h"
#include "cores/core_0_executor.h"
#include "cores/core_1_executor.h"

#include "pico/multicore.h"


MulticoreMailbox multicoreMailbox;
extern vector<SensorGroup> _SENSOR_GROUPS;

// Core data wrappers
Core0Executor dataCore0(
    multicoreMailbox,
    _SENSOR_GROUPS
);

Core1Executor dataCore1(
    multicoreMailbox,
    _SENSOR_GROUPS
);


int main() {
    // Setup stdio
    DEBUG_PRINT_INIT()

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