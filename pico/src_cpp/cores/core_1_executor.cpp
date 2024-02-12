#include "core_1_executor.h"
#include "util/debug_io.h"

#include "pico/multicore.h"


Core1Executor* Core1Executor::sExecutor = nullptr;

Core1Executor::Core1Executor(MulticoreMailbox& mailbox) :
    mMailbox(mailbox)
{}

void Core1Executor::initialize() {
}

void Core1Executor::loop() {
    if(!sExecutor) {
        return;
    }

    sExecutor->doLoop();
}

void Core1Executor::setExecutor(Core1Executor& executor) {
    sExecutor = &executor;
}

void Core1Executor::doLoop() {
    multicore_lockout_victim_init();

    while(1) {
        DEBUG_PRINT("Core 1 running....");
        sleep_ms(3000);
    }
}