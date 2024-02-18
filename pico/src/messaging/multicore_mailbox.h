#ifndef _MULTICORE_MAILBOX_H_
#define _MULTICORE_MAILBOX_H_

#include "messaging/sensor_control_message.h"
#include "messaging/mqtt_message.h"
#include "messaging/sensor_data_message.h"
#include "sensors/sensor_group.h"
#include "pico/util/queue.h"
#include <optional>

using std::optional;

class MulticoreMailbox {
    public:
        MulticoreMailbox();

        // core1 -> core0 functions
        void sendSensorDataToCore0(const vector<SensorGroup>& sensorGroups);
        bool latestSensorDataToJSON(const vector<SensorGroup>& sensorGroups, char* jsonBuffer, int jsonBufferSize);

        // core0 -> core1 functions
        void sendSensorControlMessageToCore1(MQTTMessage& mqttMessage);
        optional<SensorControlMessage> getWaitingSensorControlMessage();

    private:
        constexpr static int NUM_SENSOR_UPDATE_MESSAGES     = 2;    // We only really need double-buffering
        constexpr static int NUM_SENSOR_CONTROL_MESSAGES    = 4;

        queue_t mSensorUpdateQueue;         // Queue used for sending sensor updates from core1 to core0
        queue_t mSensorControlQueue;        // Queue used for sending sensor control commands from core0 to core1

        SensorDataMessage mSensorUpdateReadScratch;
        SensorDataMessage mSensorUpdateWriteScratch;
};

#endif      // _MULTICORE_MAILBOX_H_
