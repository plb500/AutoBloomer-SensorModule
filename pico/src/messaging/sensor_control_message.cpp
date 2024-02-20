#include "sensor_control_message.h"

#include <cstring>
#include <cstdio>

bool SensorControlMessage::fillFromMQTT(MQTTMessage& mqttMessage) {
    const int MAX_COMPONENT_LENGTH = 8;

    if(!strlen(mqttMessage.mTopic)) {
        return false;
    }

    strncpy(mControlTopic, mqttMessage.mTopic, MQTTMessage::MQTT_MAX_TOPIC_LENGTH);

    char command[MAX_COMPONENT_LENGTH + 1];
    char value[MAX_COMPONENT_LENGTH + 1];
    char formatString[16];

    memset(command, 0, MAX_COMPONENT_LENGTH + 1);
    memset(value, 0, MAX_COMPONENT_LENGTH + 1);
    sprintf(formatString, "%%%ds %%%ds", MAX_COMPONENT_LENGTH, MAX_COMPONENT_LENGTH);

    // Parse command and value strings, bail if we can't even get that far
    sscanf(mqttMessage.mPayload, formatString, command, value);
    if(!strlen(command) || !strlen(value)) {
        // Malformed message
        return false;
    }

    // Everything looks good. Set our internal parameters
    uint32_t commandInt = *((uint32_t*) command);
    memcpy(mCommandParams, value, 8);
    mCommand = commandInt;

    return true;
}
