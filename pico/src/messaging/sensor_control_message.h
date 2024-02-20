#ifndef _SENSOR_CONTROL_MESSAGE_H_
#define _SENSOR_CONTROL_MESSAGE_H_

#include "mqtt_message.h"
#include "pico/types.h"

// Incoming sensor control/calibration message
struct SensorControlMessage {
    uint32_t mCommand;
    char mCommandParams[8];
    char mControlTopic[MQTTMessage::MQTT_MAX_TOPIC_LENGTH];
    bool fillFromMQTT(MQTTMessage& mqttMessage);
};

#endif      // _SENSOR_CONTROL_MESSAGE_H_