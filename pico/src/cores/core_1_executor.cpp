#include "core_1_executor.h"
#include "util/debug_io.h"

#include "pico/multicore.h"
#include <cstdlib>


Core1Executor* Core1Executor::sExecutor = nullptr;

Core1Executor::Core1Executor(
    MulticoreMailbox& mailbox,
    vector<SensorGroup>& sensors
) :
    mMailbox(mailbox),
    mSensorGroups(sensors)
{}

void Core1Executor::initialize() {
    for(auto i = mSensorGroups.begin(); i != mSensorGroups.end(); ++i) {
        i->initializeSensors();
    }
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
        absolute_time_t currentTime = get_absolute_time();

        // Check for sensor control messages
        DEBUG_PRINT("core1 checkpoint - processing control commands");
        processSensorControlCommands();

        // Perform sensor hardware updates
        DEBUG_PRINT("core1 checkpoint - updating sensors");
        for(auto i = mSensorGroups.begin(); i != mSensorGroups.end(); ++i) {
            i->update(currentTime);
        }

        // Package sensor data to core0
        DEBUG_PRINT("core1 checkpoint - updating core0");
        mMailbox.sendSensorDataToCore0(mSensorGroups);

        sleep_ms(500);
    }
}

void Core1Executor::processSensorControlCommands() {
    optional<SensorControlMessage> msgOpt;

    do {
        if(msgOpt = mMailbox.getWaitingSensorControlMessage()) {
            bool messageHandled = false;
            for(auto group : mSensorGroups) {
                if(group.handleSensorControlCommand(*msgOpt)) {
                    messageHandled = true;
                    break;
                }
            }

            if(messageHandled) {
                DEBUG_PRINT("Sensor control command (%d) handled", msgOpt->mCommand);
            } else {
                DEBUG_PRINT("Sensor control command (%d) went unhandled", msgOpt->mCommand);
            }
        }
    } while(msgOpt);
}
