#include "sensor_control_message.h"

#include <cstring>
#include <cstdio>

bool SensorControlMessage::fillFromMQTT(MQTTMessage& mqttMessage) {
    char command[sizeof(CommandType)];
    char value[8];
    SensorControlMessage controlMessage;

    memset(command, 0, sizeof(CommandType));
    memset(value, 0, 8);

    // Parse command and value strings, bail if we can't even get that far
    sscanf(mqttMessage.mPayload, "%s %s", command, value);
    if(!strlen(command) || !strlen(value)) {
        // Malformed message
        return false;
    }

    // Make sure the command translates into something sensible
    uint32_t commandInt = *((uint32_t*) command);
    switch(commandInt) {
        case SCD30_SET_TEMP_OFFSET:
        case SCD30_SET_FRC:
            controlMessage.mCommand = static_cast<CommandType>(commandInt);
            memcpy(controlMessage.mCommandParams, value, 8);
            break;

        default:
            return false;
    }

    // Everything looks good. Set our internal parameters
    memcpy(&value, mCommandParams, 8);
    memcpy(&command, &mCommand, sizeof(CommandType));

    return true;
}
