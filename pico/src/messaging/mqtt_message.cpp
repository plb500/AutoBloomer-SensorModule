#include "mqtt_message.h"
#include <cstdio>
#include <cstring>
#include <cassert>

using std:: nullopt;

optional<MQTTMessage> MQTTMessage::createTestMQTTMessage(
    const char *sensorName, 
    const char *sensorLocation,
    const char *message
) {
    MQTTMessage mqttMsg;

    // // Sanity check
    if(!sensorName || !sensorLocation || !message) {
        return nullopt;
    }

    snprintf(mqttMsg.mTopic, MQTT_MAX_TOPIC_LENGTH, "%s/%s/%s",
        AUTOBLOOMER_TOPIC_NAME,
        sensorLocation,
        sensorName
    );

    int messageLength = strlen(message);
    memcpy(mqttMsg.mPayload, message, messageLength);
    mqttMsg.mPayload[messageLength] = 0;

    return mqttMsg;
}
