#include "sensor_data_message.h"

extern const int NUM_SENSOR_GROUPS;

SensorDataMessage::SensorDataMessage() {
    mData = new uint8_t[NUM_SENSOR_GROUPS * SensorGroup::NUM_SENSOR_GROUP_DATA_BYTES];
}

void SensorDataMessage::fillFromSensors(const vector<SensorGroup>& sensorGroups) {
    uint8_t *writePtr = mData;
    uint16_t bufferSize = (NUM_SENSOR_GROUPS * SensorGroup::NUM_SENSOR_GROUP_DATA_BYTES);

    for(auto sensorGroup : sensorGroups) {
        sensorGroup.packSensorData(writePtr, bufferSize);
        writePtr += SensorGroup::NUM_SENSOR_GROUP_DATA_BYTES;
        bufferSize -= SensorGroup::NUM_SENSOR_GROUP_DATA_BYTES;
    }
}

void SensorDataMessage::toJSON(const vector<SensorGroup>& sensorGroups, char* jsonBuffer, int jsonBufferSize) {
    char* writePtr = jsonBuffer;
    int currentOutbutBufferSize = jsonBufferSize;
    uint8_t* readPtr = mData; 

    for(auto sensorGroup : sensorGroups) {
        int charsWritten = sensorGroup.unpackSensorDataToJSON(
            readPtr, 
            SensorGroup::NUM_SENSOR_GROUP_DATA_BYTES, 
            writePtr, 
            currentOutbutBufferSize
        );

        currentOutbutBufferSize -= charsWritten;
        writePtr += charsWritten;
        readPtr += SensorGroup::NUM_SENSOR_GROUP_DATA_BYTES;
    }
}
