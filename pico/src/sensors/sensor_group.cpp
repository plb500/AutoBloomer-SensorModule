#include "sensor_group.h"
#include <cstring>
#include <cstdio>


SensorGroup::SensorGroup(initializer_list<Sensor*> sensors) :
    mSensors(sensors)
{
    memset(mName, 0, UserData::MAX_HOST_NAME_LENGTH + 1);
    memset(mLocation, 0, UserData::MAX_GROUP_LOCATION_LENGTH + 1);
    memset(mTopic, 0,MQTTMessage::MQTT_MAX_TOPIC_LENGTH);
    memset(mControlTopic, 0,MQTTMessage::MQTT_MAX_TOPIC_LENGTH);
}

void SensorGroup::initializeSensors() {
    for(auto& s : mSensors) {
        s->initialize();
    }
}

void SensorGroup::shutdown() {
    for(auto& s : mSensors) {
        s->shutdown();
    }
}

void SensorGroup::update(absolute_time_t currentTime) {
    for(auto& s : mSensors) {
        s->update(currentTime);
    }
}

uint32_t SensorGroup::getRawDataSize() const {
    uint32_t dataSize = 0;
    for(auto& s : mSensors) {
        // We need the actual data size, plus one byte for status and another byte for sensor type
        dataSize += s->getRawDataSize() + 2;
    }

    return dataSize;
}

void SensorGroup::packSensorData(uint8_t* sensorDataBuffer, uint16_t bufferSize) const {
    assert(sensorDataBuffer);
    assert(bufferSize >= getRawDataSize());

    uint8_t *writePtr = sensorDataBuffer;
    for(auto& s : mSensors) {
        const Sensor::SensorDataBuffer& cachedData = s->getCachedData();
        *writePtr++ = (uint8_t) cachedData.mStatus;
        *writePtr++ = cachedData.mDataLen;

        memcpy(writePtr, cachedData.mDataBytes, cachedData.mDataLen);
        writePtr += cachedData.mDataLen;
    }
}

int SensorGroup::unpackSensorDataToJSON(uint8_t* sensorDataBuffer, int bufferSize, char* jsonBuffer, int jsonBufferSize) const {
    char* writePtr = jsonBuffer;
    uint8_t* readPtr = sensorDataBuffer;
    *writePtr++ = '{';
    --jsonBufferSize;

    for(auto& s : mSensors) {
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

        readPtr += s->getRawDataSize();
    }

    *writePtr++ = '}';
    --jsonBufferSize;

    return (writePtr - jsonBuffer);
}

bool SensorGroup::handleSensorControlCommand(SensorControlMessage& message) {
    for(auto& s : mSensors) {
        if(s->handleSensorControlCommand(message)) {
            return true;
        }
    }

    return false;
}

void SensorGroup::setName(const char* name) {
    strncpy(mName, name, UserData::MAX_HOST_NAME_LENGTH);
    createTopics();
}

void SensorGroup::setLocation(const char* location) {
    strncpy(mLocation, location, UserData::MAX_GROUP_LOCATION_LENGTH);
    createTopics();
}

bool SensorGroup::hasTopics() const {
    return (strlen(mTopic) && strlen(mControlTopic));
}

const char* SensorGroup::getTopic() const {
    return mTopic;
}

const char* SensorGroup::getControlTopic() const {
    return mControlTopic;
}

void SensorGroup::createTopics() {
    memset(mTopic, 0, MQTTMessage::MQTT_MAX_TOPIC_LENGTH);

    if(!strlen(mName) || !strlen(mLocation)) {
        return;
    }

    snprintf(mTopic, MQTTMessage::MQTT_MAX_TOPIC_LENGTH, "%s/%s/%s",
        MQTTMessage::AUTOBLOOMER_TOPIC_NAME,
        mLocation,
        mName
    );

    snprintf(mControlTopic, MQTTMessage::MQTT_MAX_TOPIC_LENGTH, "%s/control",
        mTopic
    );
}
