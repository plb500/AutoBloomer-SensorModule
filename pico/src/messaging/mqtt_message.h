#ifndef _MQTT_MESSAGE_H_
#define _MQTT_MESSAGE_H_

#include <optional>

using std::optional;

// Outgoing MQTT message
struct MQTTMessage {
    static constexpr int MQTT_MAX_TOPIC_LENGTH              = 128;
    static constexpr int MQTT_MAX_PAYLOAD_LENGTH            = 256;
    static constexpr const char* AUTOBLOOMER_TOPIC_NAME     = "AutoBloomer";

    static void setSensorDataTopic(const char* name, const char* location, MQTTMessage& message);

    // Creates a basic test message with the supplied message as payload contents
    static optional<MQTTMessage> createTestMQTTMessage(const char *sensorName, const char *sensorLocation, const char *message);

    char mTopic[MQTT_MAX_TOPIC_LENGTH];
    char mPayload[MQTT_MAX_PAYLOAD_LENGTH];
};

#endif      //  _MQTT_MESSAGE_H_
