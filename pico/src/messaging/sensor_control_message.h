#ifndef _SENSOR_CONTROL_MESSAGE_H_
#define _SENSOR_CONTROL_MESSAGE_H_

#include "mqtt_message.h"

// Incoming Sensor Control message
struct SensorControlMessage {
    // Type of incoming control command
    enum CommandType {
        SCD30_SET_TEMP_OFFSET       = 0x504D4554,       // "TEMP"
        SCD30_SET_FRC               = 0x00435246        // "FRC"
    };

    CommandType mCommand;
    char mCommandParams[8];

    bool fillFromMQTT(MQTTMessage& mqttMessage);
};

#endif      // _SENSOR_CONTROL_MESSAGE_H_