#ifndef _SENSOR_GROUP_H_
#define _SENSOR_GROUP_H_

#include "sensor.h"
#include "messaging/sensor_control_message.h"
#include "userdata/user_data.h"

#include <initializer_list>
#include <vector>
#include <string>
#include "pico/time.h"

using std::string;
using std::vector;
using std::initializer_list;


class SensorGroup {
    public:
        SensorGroup(initializer_list<Sensor*> sensors);

        void initializeSensors();
        void shutdown();
        void update(absolute_time_t currentTime);

        void packSensorData(uint8_t* sensorDataBuffer, uint16_t bufferSize) const;
        int unpackSensorDataToJSON(uint8_t* sensorDataBuffer, int bufferSize, char* jsonBuffer, int jsonBufferSize) const;
        bool handleSensorControlCommand(SensorControlMessage& message);

        void setName(const char* name);
        void setLocation(const char* location);
        bool hasTopics() const;
        const char* getTopic() const;
        const char* getControlTopic() const;

        static constexpr int DATA_BUFFER_PER_SENSOR = (Sensor::SensorDataBuffer::SENSOR_DATA_BUFFER_SIZE + 1 + 1);
        static constexpr int MAX_NUM_SENSORS = 5;
        static constexpr int NUM_SENSOR_GROUP_DATA_BYTES = (DATA_BUFFER_PER_SENSOR * MAX_NUM_SENSORS);

    private:
        void createTopics();

        char mName[UserData::MAX_HOST_NAME_LENGTH + 1];
        char mLocation[UserData::MAX_GROUP_LOCATION_LENGTH + 1];
        char mTopic[MQTTMessage::MQTT_MAX_TOPIC_LENGTH];
        char mControlTopic[MQTTMessage::MQTT_MAX_TOPIC_LENGTH];
        const vector<Sensor*> mSensors;
};

#endif
