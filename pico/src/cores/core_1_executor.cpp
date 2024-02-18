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
        processSensorControlCommands();

        // Perform sensor hardware updates
        for(auto i = mSensorGroups.begin(); i != mSensorGroups.end(); ++i) {
            i->update(currentTime);
        }

        // Package sensor data to core0
        mMailbox.sendSensorDataToCore0(mSensorGroups);

        sleep_ms(500);
    }
}

// void Core1Executor::doLoop() {
//     multicore_lockout_victim_init();

//     while(1) {
//         // Check for sensor control messages
//         processSensorControlCommands();

//         // Update sensors and push any data to core0 if necessary
//         mSens
//         mSensorPod.update();

//         if(mSensorPod.hasValidData()) {
//             const SensorPod::Data& currentData = mSensorPod.getCurrentData();

//             DEBUG_PRINT("+--------------------------------+");
//             DEBUG_PRINT("|         SCD30 CO2: %7.2f PPM |", currentData.mCO2Level);
//             DEBUG_PRINT("| SCD30 Temperature: %5.2f °C    |", currentData.mTemperature);
//             DEBUG_PRINT("|    SCD30 Humidity: %5.2f%%      |", currentData.mHumidity);
//             DEBUG_PRINT("+--------------------------------+\n");

//             mMailbox.sendSensorDataToCore0(currentData);
//         } else {
//             DEBUG_PRINT("+---------------------------+");
//             DEBUG_PRINT("|           NO DATA         |");
//             DEBUG_PRINT("| Soil sensor active: %d [%c] |", mSensorPod.isSoilSensorActive(), mSensorPod.hasSoilSensor() ? '*' : ' ');
//             DEBUG_PRINT("|       SCD30 active: %d [%c] |", mSensorPod.isSCD30Active(), mSensorPod.hasSCD30Sensor() ? '*' : ' ');
//             DEBUG_PRINT("+---------------------------+\n");
//         }

//         sleep_ms(500);
//     }
// }

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
