#ifndef _MQTT_CONTROLLER_H_
#define _MQTT_CONTROLLER_H_

#include "messaging/mqtt_message.h"
#include "messaging/multicore_mailbox.h"

#include "pico/types.h"
#include "lwip/ip_addr.h"
#include "lwip/apps/mqtt.h"


class MQTTController {
    public:
        struct MQTTMessageBuffer {
            MQTTMessage mMessage;
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
        void setClientParameters(const char* clientName);

        MQTTMessageBuffer& getBuffer();
        void handleIncomingControlMessage(MQTTMessage& mMessage);

        void initializeMessage(MQTTMessage& message);
        err_t publishMessage(MQTTMessage& message);

    private:
        mqtt_client_t* mMQTTClient;
        ip_addr_t mBrokerAddress;
        uint16_t mBrokerPort;
        const char *mClientName;
        MulticoreMailbox& mCoreMailbox;
        MQTTMessageBuffer mIncomingMessageBuffer;
};


#endif      // _MQTT_CONTROLLER_H_
