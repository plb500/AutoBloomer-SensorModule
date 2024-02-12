#ifndef _MULTICORE_MAILBOX_H_
#define _MULTICORE_MAILBOX_H_

#include "sensor_pod_messages.h"
#include "pico/util/queue.h"
#include <optional>

using std::optional;

class MulticoreMailbox {
    public:
        MulticoreMailbox();

        // core1 -> core0 functions
        // TODO: DO THIS DON'T FORGET
        // void sendSensorDataToCore0(SensorPodData& sensorPodData);
        optional<SensorPodMessages::SensorDataUpdateMessage> getLatestSensorDataMessage();

        // core0 -> core1 functions
        void sendSensorControlMessageToCore1(SensorPodMessages::MQTTMessage& mqttMessage);
        optional<SensorPodMessages::SensorControlMessage> getWaitingSensorControlMessage();

    private:
        constexpr static int NUM_SENSOR_UPDATE_MESSAGES     = 4;
        constexpr static int NUM_SENSOR_CONTROL_MESSAGES    = 4;

        queue_t mSensorUpdateQueue;         // Queue used for sending sensor updates from core1 to core0
        queue_t mSensorControlQueue;        // Queue used for sending sensor control commands from core0 to core1
};

#endif      // _MULTICORE_MAILBOX_H_
