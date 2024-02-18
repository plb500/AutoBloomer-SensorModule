#include "scd30_sensor.h"
#include "sensirion/scd30_i2c.h"
#include "sensirion/sensirion_i2c_hal.h"
#include "util/debug_io.h"

#include <cstring>

using std::make_tuple;

constexpr const char* CO2_LEVEL_JSON_KEY               = "CO2";
constexpr const char* TEMPERATURE_JSON_KEY             = "Temperature";
constexpr const char* HUMIDITY_JSON_KEY                = "Humidity";

// Sensirion SCD30 variables
extern i2c_inst_t *sensirion_i2c_inst;
extern int sensirion_i2c_baud;
extern int sensirion_sda_pin;
extern int sensirion_scl_pin;

SCD30Sensor::SCD30Sensor(I2CInterface& i2c, uint8_t powerPin) :
    Sensor(0, &SCD30Sensor::serializeDataToJSON),
    mI2C(i2c),
    mActive(false),
    mPowerControlPin(powerPin)
{}

void SCD30Sensor::initialize() {
    uint8_t major, minor;

    // I2C hardware init
    sensirion_i2c_inst = mI2C.mI2C;
    sensirion_i2c_baud = mI2C.mBaud;
    sensirion_sda_pin = mI2C.mSDA;
    sensirion_scl_pin = mI2C.mSCL;
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

    mActive = !scd30_read_firmware_version(&major, &minor);
    DEBUG_PRINT("SCD30 Firmware: 0x%2X - 0x%2X");

    startReadings();
}

void SCD30Sensor::reset() {
    shutdown();
    initialize();
}

void SCD30Sensor::shutdown() {
    scd30_stop_periodic_measurement();
    sleep_ms(1);
    sensirion_i2c_hal_free();
}           

int SCD30Sensor::serializeDataToJSON(uint8_t* data, uint8_t dataSize, char* jsonBuffer, int jsonBufferSize) {
    float co2, temp, humidity;
    memcpy(&co2, data, sizeof(float));
    data += sizeof(float);
    memcpy(&temp, data, sizeof(float));
    data += sizeof(float);
    memcpy(&humidity, data, sizeof(float));

    return snprintf(jsonBuffer, jsonBufferSize,
        "\"%s\":%.2f, \"%s\":%.2f, \"%s\":%.2f",
        CO2_LEVEL_JSON_KEY, co2,
        TEMPERATURE_JSON_KEY, temp,
        HUMIDITY_JSON_KEY, humidity
    );
}

Sensor::SensorUpdateResponse SCD30Sensor::doUpdate(absolute_time_t currentTime, uint8_t *dataStorageBuffer, size_t bufferSize) {
    Sensor::SensorUpdateResponse response = make_tuple(SENSOR_INACTIVE, 0);

    if(!mActive) {
        return response;
    }

    get<0>(response) = SENSOR_INACTIVE;

    uint16_t dataReady;
    if(!scd30_get_data_ready(&dataReady)) {
        if(dataReady) {
            float co2Reading;
            float temperatureReading;
            float humidityReading;

            if(!scd30_read_measurement_data(
                &co2Reading,
                &temperatureReading,
                &humidityReading
            )) {
                // Good data, copy it into the output buffer
                float* writePtr = reinterpret_cast<float*>(dataStorageBuffer);
                *writePtr = co2Reading;
                writePtr += 1;
                *writePtr = temperatureReading;
                writePtr += 1;
                *writePtr = humidityReading;

                get<0>(response) = SENSOR_OK;
                get<1>(response) = sizeof(float) * 3;

            } else {
                // No data when we were told there was data available. This may indiciate a sensor issue
                get<0>(response) = SENSOR_MALFUNCTIONING;
            }
        } else {
            // There was no data available. If this goes on too long it might indicate
            // an issue with the sensor
            get<0>(response) = SENSOR_OK_NO_DATA;
        }
    } else {
        // The actual data ready command failed, might be something up with the port
        get<0>(response) = SENSOR_MALFUNCTIONING;
    }

    return response;
}

void SCD30Sensor::startReadings() {
    if(mActive) {
        scd30_set_measurement_interval(SCD30_MEASUREMENT_INTERVAL_SECONDS);
        scd30_start_periodic_measurement(0);
    }
}
