#ifndef _MQTT_UTILS_H
#define _MQTT_UTILS_H

#include "messages/multicore_mailbox.h"

#include "pico/types.h"
#include "pico/util/queue.h"
#include "lwip/ip_addr.h"
#include "lwip/apps/mqtt.h"


typedef struct MQTTMessageBuffer_t {
    MQTTMessage mMessage;
    uint32_t mPayloadSize;
    uint32_t mBufferIndex;
} MQTTMessageBuffer;

typedef struct MQTTState_t {
    mqtt_client_t *mqttClient;
    ip_addr_t mBrokerAddress;
    uint16_t mBrokerPort;
    const char *mSensorName;
    const char *mSensorLocation;
    MulticoreMailbox *mCoreMailbox;
    MQTTMessageBuffer mIncomingMessageBuffer;
} MQTTState;


void init_mqtt_state(MQTTState *mqttState);
bool connect_to_broker_blocking(MQTTState *mqttState);
void disconnect_from_broker(MQTTState *mqttState);
bool subscribe_to_topic(MQTTState *mqttState, const char *topic);

void init_mqtt_message(MQTTMessage *message);
err_t publish_mqtt_message(MQTTState *mqttState, MQTTMessage *message);

#endif      //_MQTT_UTILS_H

