#include "core_1_executor.h"
#include "util/debug_io.h"

#include "pico/multicore.h"


Core1Executor* Core1Executor::sExecutor = nullptr;

Core1Executor::Core1Executor(
    MulticoreMailbox& mailbox,
    optional<StemmaSoilSensor>& soilSensor,
    optional<I2CInterface>& scd30Interface
) :
    mMailbox(mailbox),
    mSensorPod(soilSensor, scd30Interface)
{}

void Core1Executor::initialize() {
    if(mSensorPod.initialize()) {
        DEBUG_PRINT("Sensor pod initialized");
        mSensorPod.startReadings();
    } else {
        DEBUG_PRINT("Sensor pod initialization failed");
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
        // Update sensors and push any data to core0 if necessary
        mSensorPod.update();

        if(mSensorPod.hasValidData()) {
            const SensorPod::Data& currentData = mSensorPod.getCurrentData();

            DEBUG_PRINT("+--------------------------------+");
            DEBUG_PRINT("|         SCD30 CO2: %7.2f PPM |", currentData.mCO2Level);
            DEBUG_PRINT("| SCD30 Temperature: %5.2f °C    |", currentData.mTemperature);
            DEBUG_PRINT("|    SCD30 Humidity: %5.2f%%      |", currentData.mHumidity);
            DEBUG_PRINT("+--------------------------------+\n");

            mMailbox.sendSensorDataToCore0(currentData);
        } else {
            DEBUG_PRINT("+---------------------------+");
            DEBUG_PRINT("|           NO DATA         |");
            DEBUG_PRINT("| Soil sensor active: %d [%c] |", mSensorPod.isSoilSensorActive(), mSensorPod.hasSoilSensor() ? '*' : ' ');
            DEBUG_PRINT("|       SCD30 active: %d [%c] |", mSensorPod.isSCD30Active(), mSensorPod.hasSCD30Sensor() ? '*' : ' ');
            DEBUG_PRINT("+---------------------------+\n");
        }

        sleep_ms(500);
    }
}