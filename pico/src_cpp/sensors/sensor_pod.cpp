#include "sensor_pod.h"
#include "sensirion/scd30_i2c.h"
#include "sensirion/sensirion_i2c_hal.h"
#include "util/debug_io.h"


// Sensirion SCD30 variables
extern i2c_inst_t *sensirion_i2c_inst;
extern int sensirion_i2c_baud;
extern int sensirion_sda_pin;
extern int sensirion_scl_pin;

constexpr int SENSOR_POD_TIMEOUT_MS                   = 10000;
constexpr int SCD30_MEASUREMENT_INTERVAL_SECONDS      = 2;

struct SCD30SensorData {
    bool mValidReading;
    float mCO2Reading;
    float mTemperatureReading;
    float mHumidityReading;
};


SensorPod::SensorPod(
    optional<StemmaSoilSensor>& soilSensor,
    optional<I2CInterface>& scd30Interface
) :
    mSoilSensor(soilSensor),
    mSCD30Interface(scd30Interface)
{}

bool SensorPod::initialize() {
    bool soilSensorGood = true;
    bool scd30Good = true;

    // Initialize sensirion I2C
    if(mSCD30Interface) {
        initializeSCD30Connection();
        scd30Good = mSCD30SensorActive;
    }

    // Initialize soil sensor
    if(mSoilSensor) {
        initializeSoilSensorConnection();
        soilSensorGood = mSoilSensorActive;
    }

    mCurrentData.mSCD30SensorDataValid = false;
    mCurrentData.mSoilSensorDataValid = false;

    return (soilSensorGood && scd30Good);
}

bool SensorPod::reset() {
    static const uint16_t BOOT_DELAY_MS = 50;

    resetSCD30Connection();
    resetSoilSensorConnection();

    sleep_ms(BOOT_DELAY_MS);

    startReadings();

    return true;
}

void SensorPod::startReadings() {
    startSoilSensorReadings();
    startSCD30Readings();
}

void SensorPod::update() {
    updateSoilSensor();
    updateSCD30();
}

bool SensorPod::hasValidData() {
    return (mCurrentData.mSoilSensorDataValid || mCurrentData.mSCD30SensorDataValid);
}

const SensorPod::Data& SensorPod::getCurrentData() {
    return mCurrentData;
}

bool SensorPod::hasSoilSensor() {
    return mSoilSensor.has_value();
}

bool SensorPod::hasSCD30Sensor() {
    return mSCD30Interface.has_value();
}

bool SensorPod::isSoilSensorActive() {
    return mSoilSensorActive;
}

bool SensorPod::isSCD30Active() {
    return mSCD30SensorActive;
}

void SensorPod::setTemperatureOffset(double offset) {
    // We can only do this on a valid sensor pod with a valid, active SCD30 interface
    if(!mSCD30Interface || !mSCD30SensorActive) {
        return;
    }

    uint16_t offsetInt = (uint16_t) (offset * 100);

    // Pause readings while we set the offset (not sure if we need to do this but it seems like a good idea)
    scd30_stop_periodic_measurement();

    DEBUG_PRINT(" -- Setting temperature offset to: %d", offsetInt);
    scd30_set_temperature_offset(offsetInt);

    // Restart readings
    scd30_set_measurement_interval(SCD30_MEASUREMENT_INTERVAL_SECONDS);
    scd30_start_periodic_measurement(0);
}

void SensorPod::setForcedRecalibrationValue(uint16_t frc) {
    // We can only do this on a valid sensor pod with a valid, active SCD30 interface
    if(!mSCD30Interface || !mSCD30SensorActive) {
        return;
    }

    // Pause readings while we set the offset (not sure if we need to do this but it seems like a good idea)
    scd30_stop_periodic_measurement();

    DEBUG_PRINT(" -- Setting FRC to: %d", frc);
    scd30_force_recalibration(frc);

    // Restart readings
    scd30_set_measurement_interval(SCD30_MEASUREMENT_INTERVAL_SECONDS);
    scd30_start_periodic_measurement(0);

}


void SensorPod::initializeSoilSensorConnection() {
    if(mSoilSensor) {
        mSoilSensorActive = mSoilSensor->init();
    }
}

void SensorPod::initializeSCD30Connection() {
    uint8_t major, minor;

    if(!mSCD30Interface) {
        return;
    }

    // I2C hardware init
    sensirion_i2c_inst = mSCD30Interface->mI2C;
    sensirion_i2c_baud = mSCD30Interface->mBaud;
    sensirion_sda_pin = mSCD30Interface->mSDA;
    sensirion_scl_pin = mSCD30Interface->mSCL;
    init_driver(SCD30_I2C_ADDR_61);
    sensirion_i2c_hal_init();

    // It's possible the SCD30 might still be in continuous measurement mode, do a stop-reset anyway
    if(scd30_stop_periodic_measurement() != 0) {
        return;
    }

    sleep_ms(2);

    if(scd30_soft_reset() != 0) {
        return;
    }

    sleep_ms(2);

    mSCD30SensorActive = !scd30_read_firmware_version(&major, &minor);
    mSCD30ResetTimeout = make_timeout_time_ms(SENSOR_POD_TIMEOUT_MS);
    DEBUG_PRINT("Firmware: 0x%2X - 0x%2X");
}

void SensorPod::shutdownSoilSensorConnection() {
    // Nothing to do yet
}

void SensorPod::shutdownSCD30Connection() {
    scd30_stop_periodic_measurement();

    sleep_ms(1);

    sensirion_i2c_hal_free();
}

void SensorPod::resetSoilSensorConnection() {
    if(mSoilSensor) {
        mSoilSensor.reset();
    }
}

void SensorPod::resetSCD30Connection() {
    if(mSCD30Interface) {
        shutdownSCD30Connection();
        sleep_ms(1);
        initializeSCD30Connection();
    }
}

void SensorPod::startSoilSensorReadings() {
    // Nothing to do here yet (or ever, maybe)
}

void SensorPod::startSCD30Readings() {
    if(mSCD30Interface && mSCD30SensorActive) {
        scd30_set_measurement_interval(SCD30_MEASUREMENT_INTERVAL_SECONDS);
        scd30_start_periodic_measurement(0);
    }
}

bool SensorPod::updateSoilSensor() {
    bool soilSensorGood = true;

    if(mSoilSensor && mSoilSensorActive) {
        uint16_t capValue = mSoilSensor->getCapacitiveValue();
        if(capValue != StemmaSoilSensor::STEMMA_SOIL_SENSOR_INVALID_READING) {
            mCurrentData.mSoilSensorData = capValue;
            mCurrentData.mSoilSensorDataValid = true;

        } else {
            // Got an invalid reading, might be something up with the port
            soilSensorGood = false;
        }
    } else if(mSoilSensor) {
        // We have the interface defined but it's not active, this is not a good sign
        soilSensorGood = false;   
    }

    return soilSensorGood;
}

bool SensorPod::updateSCD30() {
    bool scd30Good = true;

    // Check if we maybe need to bounce the port
    if(absolute_time_diff_us(mSCD30ResetTimeout, get_absolute_time()) > 0) {
        DEBUG_PRINT("SCD30 has not responded within the timeout period. Resetting.");
        resetSCD30Connection();
        startSCD30Readings();

        mSCD30ResetTimeout = make_timeout_time_ms(SENSOR_POD_TIMEOUT_MS);
        mCurrentData.mStatus = SENSOR_CONNECTED_MALFUNCTIONING;
        return false;
    }

    // Read SCD30 (if we have a valid reading ready)
    mCurrentData.mSCD30SensorDataValid = false;
    if(mSCD30Interface && mSCD30SensorActive) {
        uint16_t dataReady;
        if(!scd30_get_data_ready(&dataReady)) {
            if(dataReady) {
                SCD30SensorData tmpData;
                int16_t readResponse = scd30_read_measurement_data(
                    &tmpData.mCO2Reading,
                    &tmpData.mTemperatureReading,
                    &tmpData.mHumidityReading
                );
                tmpData.mValidReading = !readResponse;

                if(tmpData.mValidReading) {
                    mCurrentData.mCO2Level = tmpData.mCO2Reading;
                    mCurrentData.mTemperature = tmpData.mTemperatureReading;
                    mCurrentData.mHumidity = tmpData.mHumidityReading;
                    mCurrentData.mSCD30SensorDataValid = true;
                }
            } else {
                // There was no data available. If this goes on too long it might indicate
                // an issue with the sensor
                scd30Good = false;
            }
        } else {
            // The actual data ready command failed, might be something up with the port
            scd30Good = false;
        }
    } else if(mSCD30Interface) {
        // We have the interface defined but it's not active, this is not a good sign
        scd30Good = false;
    }

    if(scd30Good) {
        mSCD30ResetTimeout = make_timeout_time_ms(SENSOR_POD_TIMEOUT_MS);
    }

    return scd30Good;
}
