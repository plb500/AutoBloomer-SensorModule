#ifndef _SENSOR_POD_MESSAGES_H_
#define _SENSOR_POD_MESSAGES_H_

#include "sensors/sensor_group.h"
#include "pico/types.h"
#include <optional>

using std::optional;

const int NUM_SENSOR_GROUPS =4;

class SensorPodMessages {
    public:
        static constexpr int MQTT_MAX_TOPIC_LENGTH              = 128;
        static constexpr int MQTT_MAX_PAYLOAD_LENGTH            = 256;
        static constexpr const char* AUTOBLOOMER_TOPIC_NAME     = "AutoBloomer";


        // Type of incoming control command
        enum SensorControlCommandType {
            SCD30_SET_TEMP_OFFSET       = 0x504D4554,       // "TEMP"
            SCD30_SET_FRC               = 0x00435246        // "FRC"
        };

        // Incoming Sensor Control message
        struct SensorControlMessage {
            SensorControlCommandType mCommand;
            char mCommandParams[8];
        };

        // Outgoing MQTT message
        struct MQTTMessage {
            char mTopic[SensorPodMessages::MQTT_MAX_TOPIC_LENGTH];
            char mPayload[MQTT_MAX_PAYLOAD_LENGTH];
        };

        struct SensorDataUpdateMessage {
            int mSensorStatus;
            bool mHasSoilReding;
            bool mHasSCD30Reading;
            float mCO2Level;
            float mTemperature;
            float mHumidity;
            uint16_t mSoilSensorData;
        };

        struct CurrentSensorDataMessage {
            CurrentSensorDataMessage();

            void fromSensors(const vector<SensorGroup>& sensorGroups);
            void toJSON(const vector<SensorGroup>& sensorGroups, char* jsonBuffer, int jsonBufferSize);
            
            uint8_t* mData;
        };

        SensorPodMessages() = delete;    // Utility class. Do not instantiate

        // Converts incoming MQTT message object to SensorControlMessage                
        static optional<SensorControlMessage> mqttToControlMessage(MQTTMessage& mqttMessage);

        // Converts SensorUpdateMessage to outgoing MQTT message
        static optional<MQTTMessage> dataUpdateToMQTTMessage(const char *sensorName, const char *sensorLocation, SensorDataUpdateMessage& dataUpdate);

        // Creates a basic test message with the supplied message as payload contents
        static optional<MQTTMessage> createTestMQTTMessage(const char *sensorName, const char *sensorLocation, const char *message);
};

#endif      // _SENSOR_POD_MESSAGES_H_
