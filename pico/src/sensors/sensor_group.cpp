#include "sensor_group.h"
#include <cstring>
#include <cstdio>


SensorGroup::SensorGroup(initializer_list<Sensor*> sensors) :
    mSensors(sensors)
{}

void SensorGroup::initializeSensors() {
    for(auto s : mSensors) {
        s->initialize();
    }
}

void SensorGroup::shutdown() {
    for(auto s : mSensors) {
        s->shutdown();
    }
}

void SensorGroup::update(absolute_time_t currentTime) {
    for(auto s : mSensors) {
        s->update(currentTime);
    }
}

void SensorGroup::packSensorData(uint8_t* sensorDataBuffer, uint16_t bufferSize) const {
    assert(sensorDataBuffer);
    assert(bufferSize >= mSensors.size() * DATA_BUFFER_PER_SENSOR);

    uint8_t *writePtr = sensorDataBuffer;
    for(auto s : mSensors) {
        const Sensor::SensorDataBuffer& cachedData = s->getCachedData();
        *writePtr++ = (uint8_t) cachedData.mStatus;
        *writePtr++ = cachedData.mDataLen;

        memcpy(writePtr, cachedData.mDataBytes, Sensor::SensorDataBuffer::SENSOR_DATA_BUFFER_SIZE);
        writePtr += Sensor::SensorDataBuffer::SENSOR_DATA_BUFFER_SIZE;
    }
}

int SensorGroup::unpackSensorDataToJSON(uint8_t* sensorDataBuffer, int bufferSize, char* jsonBuffer, int jsonBufferSize) const {
    char* writePtr = jsonBuffer;
    uint8_t* readPtr = sensorDataBuffer;
    *writePtr++ = '{';
    --jsonBufferSize;

    for(auto s : mSensors) {
        // First byte is status
        Sensor::SensorStatus status = (Sensor::SensorStatus) *readPtr++;

        // Second byte is data length
        uint8_t dataLength = *readPtr++;

        uint8_t sensorType = s->getSensorTypeID();

        // Regardless of whether there is data or not, we write the sensor's type and status
        uint8_t pfxBytesWritten = sprintf(writePtr, "{\"type\": %d, \"status\": %d", sensorType, status);
        writePtr += pfxBytesWritten;
        jsonBufferSize -= pfxBytesWritten;

        // If there is data, add that to the JSON block too        
        if(dataLength) {
            // JSON separator
            uint8_t separatorBytesWritten = sprintf(writePtr, ", ");
            writePtr += separatorBytesWritten;
            jsonBufferSize -= separatorBytesWritten;

            // Data
            uint8_t dataBytesWritten = Sensor::getDataAsJSON(sensorType, readPtr, dataLength, writePtr, jsonBufferSize);
            writePtr += dataBytesWritten;
            jsonBufferSize -= dataBytesWritten;
        }

        // Close out JSON block
        *writePtr++ = '}';
        --jsonBufferSize;

        // If there are more sensors, add a separator for the next JSON block
        if(s != mSensors.back()) {
            *writePtr++ = ',';
            --jsonBufferSize;
        }

        readPtr += Sensor::SensorDataBuffer::SENSOR_DATA_BUFFER_SIZE;
    }

    *writePtr++ = '}';
    --jsonBufferSize;

    return (writePtr - jsonBuffer);
}
