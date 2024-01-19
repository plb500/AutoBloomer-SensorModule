#ifndef _MQTT_UTILS_H
#define _MQTT_UTILS_H

#include "pico/types.h"
#include "pico/util/queue.h"
#include "lwip/ip_addr.h"
#include "lwip/apps/mqtt.h"


#define MQTT_MAX_TOPIC_LENGTH               (128)
#define MQTT_MAX_PAYLOAD_LENGTH             (256)

typedef struct MQTTState_t {
    mqtt_client_t *mqttClient;
    ip_addr_t mBrokerAddress;
    uint16_t mBrokerPort;
    queue_t *mSensorUpdateQueue;
} MQTTState;

typedef struct MQTTMessage_t {
    char mTopic[MQTT_MAX_TOPIC_LENGTH];
    char mPayload[MQTT_MAX_PAYLOAD_LENGTH];
} MQTTMessage;


void init_mqtt_state(MQTTState *mqttState);
bool connect_to_broker_blocking(MQTTState *mqttState);
void disconnect_from_broker(MQTTState *mqttState);

void init_mqtt_message(MQTTMessage *message);
err_t publish_mqtt_message(MQTTState *mqttState, MQTTMessage *message);

#endif      //_MQTT_UTILS_H

