#include "sensor_data_message.h"
#include "util/debug_io.h"
#include <cstring>


extern const int NUM_SENSOR_GROUPS;

SensorDataMessage::SensorDataMessage() {}

void SensorDataMessage::fillFromSensors(const vector<SensorGroup>& sensorGroups) {
    uint8_t *writePtr = mData;
    uint16_t bufferSize = TOTAL_RAW_DATA_SIZE;

    for(auto& sensorGroup : sensorGroups) {
        sensorGroup.packSensorData(writePtr, bufferSize);
        writePtr += sensorGroup.getRawDataSize();
        bufferSize -= sensorGroup.getRawDataSize();
    }
}

void SensorDataMessage::toMQTT(const vector<SensorGroup>& sensorGroups,vector<MQTTMessage>& outboundMessages) {
    assert(sensorGroups.size() == outboundMessages.size());

    uint8_t* readPtr = mData; 

    for(int i = 0; i < sensorGroups.size(); ++i) {
        auto& group = sensorGroups[i];
        auto& mqttMsg = outboundMessages[i];

        if(group.hasTopics()) {
            strncpy(mqttMsg.mTopic, group.getTopic(), MQTTMessage::MQTT_MAX_TOPIC_LENGTH);
            group.unpackSensorDataToJSON(
                readPtr,
                group.getRawDataSize(),
                mqttMsg.mPayload,
                MQTTMessage::MQTT_MAX_PAYLOAD_LENGTH
            );
            mqttMsg.mReadyToSend = true;
        } else {
            mqttMsg.mReadyToSend = false;
        }

        readPtr += group.getRawDataSize();
    }
}
