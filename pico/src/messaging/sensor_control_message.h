#ifndef _SENSOR_CONTROL_MESSAGE_H_
#define _SENSOR_CONTROL_MESSAGE_H_

#include "mqtt_message.h"
#include "pico/types.h"

// Incoming Sensor Control message
struct SensorControlMessage {
    uint32_t mCommand;
    char mCommandParams[8];

    bool fillFromMQTT(MQTTMessage& mqttMessage);
};

#endif      // _SENSOR_CONTROL_MESSAGE_H_