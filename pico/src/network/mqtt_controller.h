#ifndef _MQTT_CONTROLLER_H_
#define _MQTT_CONTROLLER_H_

#include "messaging/sensor_pod_messages.h"
#include "messaging/multicore_mailbox.h"

#include "pico/types.h"
#include "lwip/ip_addr.h"
#include "lwip/apps/mqtt.h"


class MQTTController {
    public:
        struct MQTTMessageBuffer {
            SensorPodMessages::MQTTMessage mMessage;
            uint32_t mPayloadSize;
            uint32_t mBufferIndex;

            void initialize(uint32_t payloadSize);
            void setMessageTopic(const char *topic);
        };

        MQTTController(MulticoreMailbox& mailbox);

        void initMQTTClient();
        bool connectToBrokerBlocking(uint16_t timeoutMS);
        bool isConnected();
        void disconnectFromBroker();
        void subscribeToTopic(const char* topic);

        void setBrokerParameters(ip_addr_t& address, uint16_t port);
        void setClientParameters(const char* sensorName, const char *sensorLocation);

        MQTTMessageBuffer& getBuffer();
        void handleIncomingControlMessage(SensorPodMessages::MQTTMessage& mMessage);

        void getSensorControlTopic(char *dest);
        void initializeMessage(SensorPodMessages::MQTTMessage& message);
        err_t publishMessage(SensorPodMessages::MQTTMessage& message);

    private:
        mqtt_client_t* mMQTTClient;
        ip_addr_t mBrokerAddress;
        uint16_t mBrokerPort;
        const char *mSensorName;
        const char *mSensorLocation;
        MulticoreMailbox& mCoreMailbox;
        MQTTMessageBuffer mIncomingMessageBuffer;
};


#endif      // _MQTT_CONTROLLER_H_
