#include "scd30_sensor.h"
#include "sensors/hardware_interfaces/sensirion/scd30.h"
#include "sensors/hardware_interfaces/sensirion/sensirion_i2c.h"
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

#define SCD30_WAIT_SLEEP()    (busy_wait_us_32(10))

SCD30Sensor::SCD30Sensor(uint8_t sdaPin, uint8_t sclPin, uint8_t powerPin) :
    Sensor{SCD30_SENSOR, &SCD30Sensor::serializeDataToJSON},
    mActive{false},
    mSDAPin{sdaPin},
    mSCLPin{sclPin},
    mPowerControlPin{powerPin}
{}

void SCD30Sensor::doInitialization() {
    char serial[33];

    // Setup our power control pin
    gpio_init(mPowerControlPin);
    gpio_set_dir(mPowerControlPin, GPIO_OUT);
    gpio_pull_up(mPowerControlPin);

    sleep_ms(2);

    gpio_put(mPowerControlPin, 0);

    // Initialize I2C lib
    sensirion_i2c_init();

    // See if we can get access
    while (scd30_probe() != NO_ERROR) {
        sensirion_sleep_usec(1000000u);
    }

    // Validate we can communicate with the SCD30
    mActive = !scd30_read_serial(serial);
    DEBUG_PRINT("SCD30 serial: %s", serial);

    startReadings();
}

void SCD30Sensor::reset() {
    shutdown();

    // Bounce the power to the sensor
    sleep_ms(2);
    gpio_put(mPowerControlPin, 1);
    sleep_ms(2);
    gpio_put(mPowerControlPin, 0);
    sleep_ms(2);

    initialize();
}

void SCD30Sensor::shutdown() {
    scd30_stop_periodic_measurement();
    SCD30_WAIT_SLEEP();
    sensirion_i2c_release();
}           

bool SCD30Sensor::handleSensorControlCommand(SensorControlMessage& message) {
    bool consumed = false;
    
    switch(message.mCommand) {
        case SCD30_SET_TEMP_OFFSET:
            handleSetTemperatureOffsetCommand(message.mCommandParams);
            consumed = true;
            break;

        case SCD30_SET_FRC:
            handleSetFRCCommand(message.mCommandParams);
            consumed = true;
            break;

        default:
            break;
    }    

    return consumed;
}

void SCD30Sensor::setTemperatureOffset(double offset) {
    // We can only do this if we are active
    if(!mActive) {
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

void SCD30Sensor::setForcedRecalibrationValue(uint16_t frc) {
    // We can only do this if we are active
    if(!mActive) {
        return;
    }

    // Pause readings while we set the offset (not sure if we need to do this but it seems like a good idea)
    scd30_stop_periodic_measurement();

    DEBUG_PRINT(" -- Setting FRC to: %d", frc);
    scd30_set_forced_recalibration(frc);

    // Restart readings
    scd30_set_measurement_interval(SCD30_MEASUREMENT_INTERVAL_SECONDS);
    scd30_start_periodic_measurement(0);
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

            if(!scd30_read_measurement(
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

                DEBUG_PRINT("+--------------------------------+");
                DEBUG_PRINT("|             SCD30              |");
                DEBUG_PRINT("|         SCD30 CO2: %7.2f PPM |", co2Reading);
                DEBUG_PRINT("| SCD30 Temperature: %5.2f °C    |", temperatureReading);
                DEBUG_PRINT("|    SCD30 Humidity: %5.2f%%      |", humidityReading);
                DEBUG_PRINT("+--------------------------------+\n");
            } else {
                // No data when we were told there was data available. This may indiciate a sensor issue
                get<0>(response) = SENSOR_MALFUNCTIONING;
                DEBUG_PRINT("+--------------------------------+");
                DEBUG_PRINT("|             SCD30              |");
                DEBUG_PRINT("|       * MALFUNCTION *          |");
                DEBUG_PRINT("+--------------------------------+");
            }
        } else {
            // There was no data available. If this goes on too long it might indicate
            // an issue with the sensor
            DEBUG_PRINT("+--------------------------------+");
            DEBUG_PRINT("|             SCD30              |");
            DEBUG_PRINT("|          * nothing *           |");
            DEBUG_PRINT("+--------------------------------+");
            get<0>(response) = SENSOR_OK_NO_DATA;
        }
    } else {
        // The actual data ready command failed, might be something up with the port
        DEBUG_PRINT("+--------------------------------+");
        DEBUG_PRINT("|             SCD30              |");
        DEBUG_PRINT("|       * MALFUNCTION *          |");
        DEBUG_PRINT("+--------------------------------+");
        get<0>(response) = SENSOR_MALFUNCTIONING;
    }

    return response;
}

void SCD30Sensor::startReadings() {
    if(mActive) {
        scd30_set_measurement_interval(SCD30_MEASUREMENT_INTERVAL_SECONDS);
        sensirion_sleep_usec(20000u);
        scd30_start_periodic_measurement(0);
    }
}

void SCD30Sensor::handleSetTemperatureOffsetCommand(const char *commandParam) {
    double val;
    char *end;

    val = strtod(commandParam, &end);
    if(end == commandParam) {
        // Could not convert supplied value
        DEBUG_PRINT("Conversion error while setting temperature offset.");
        return;
    }

    DEBUG_PRINT("SETTING TEMPERATURE OFFSET (%f)", val);
    // setTemperatureOffset(val);
}

void SCD30Sensor::handleSetFRCCommand(const char *commandParam) {
    long val;
    char *end;

    val = strtol(commandParam, &end, 10);
    if(end == commandParam) {
        // Could not convert supplied value
        DEBUG_PRINT("Conversion error while setting FRC.");
        return;
    }

    DEBUG_PRINT("SETTING FRC: %d", val);
    // setForcedRecalibrationValue(val);
}
