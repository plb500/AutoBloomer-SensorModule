#include "dummy_sensor.h"

#include "pico/time.h"
#include "pico/rand.h"
#include "util/debug_io.h"

#include <cstring>

using std::make_tuple;


constexpr const char* DUMMY_INT_JSON_KEY               = "dummyInt";
constexpr const char* DUMMY_FLOAT_JSON_KEY             = "dummyFloat";

DummySensor::DummySensor() :
    Sensor(DUMMY_SENSOR, &DummySensor::serializeDataToJSON),
    mDummyInt(0),
    mDummyFloat(0.f),
    mNextUpdateTime(nil_time)
{}

int DummySensor::serializeDataToJSON(uint8_t* data, uint8_t dataSize, char* jsonBuffer, int jsonBufferSize) {
    int intValue;
    float floatValue;

    memcpy(&intValue, data, sizeof(int));
    data += sizeof(int);
    memcpy(&floatValue, data, sizeof(float));

    return sprintf(jsonBuffer,
        "\"%s\": %d, \"%s\": %.2f",
        DUMMY_INT_JSON_KEY, intValue,
        DUMMY_FLOAT_JSON_KEY, floatValue
    );
}

bool DummySensor::handleSensorControlCommand(SensorControlMessage& message) {
    if(message.mCommand == 0x44434241) {
        DEBUG_PRINT("HANDLING COMMAND");
        return true;
    }

    return false;
}

Sensor::SensorUpdateResponse DummySensor::doUpdate(absolute_time_t currentTime, uint8_t *dataStorageBuffer, size_t bufferSize) {
    if(is_nil_time(mNextUpdateTime) || absolute_time_diff_us(currentTime, mNextUpdateTime) <= 0) {

        float randValue = static_cast<float>(get_rand_32()) / 0xFFFFFFFF; 
        mDummyInt++;
        mDummyFloat += randValue;

        memcpy(dataStorageBuffer, &mDummyInt, sizeof(int));
        dataStorageBuffer += sizeof(int);
        memcpy(dataStorageBuffer, &mDummyFloat, sizeof(float));

        int dataSize = (sizeof(int) + sizeof(float));

        DEBUG_PRINT("+---------------------+");
        DEBUG_PRINT("|     DUMMY VALUES    |");
        DEBUG_PRINT("|   INT: 0x%08X   |", mDummyInt);
        DEBUG_PRINT("| FLOAT: %12.2f |", mDummyFloat);
        DEBUG_PRINT("+---------------------+");
        mNextUpdateTime = make_timeout_time_ms(UPDATE_TIME_MS);

        return make_tuple(SENSOR_OK, dataSize);
    }
    return make_tuple(SENSOR_OK_NO_DATA, 0);
}
