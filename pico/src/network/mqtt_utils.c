#include "mqtt_utils.h"

#include "util/debug_io.h"

#include "cores/sensor_multicore_utils.h"

#include <string.h>
#include "lwip/pbuf.h"
#include "lwip/apps/mqtt_priv.h"
#include "pico/cyw43_arch.h"


typedef struct ConnectMonitor_t {
    mqtt_connection_status_t mStatus;
    bool mConnectionCompleted;
} ConnectionMonitor;


void mqtt_publish_start_callback(void *arg, const char *topic, u32_t tot_len);
void mqtt_publish_data_callback(void *arg, const u8_t *data, u16_t len, u8_t flags);
void get_sensor_control_topic(MQTTState *state, char *topicBuffer);
void mqtt_subscribe_request_callback(void *arg, err_t err);
void mqtt_publish_request_callback(void *arg, err_t err);


void init_mqtt_message_buffer(MQTTMessageBuffer *buffer, uint32_t payloadSize) {
    if(!buffer) {
        return;
    }

    memset(buffer->mMessage.mTopic, 0, 128);
    memset(buffer->mMessage.mPayload, 0, 128);
    buffer->mPayloadSize = payloadSize;
    buffer->mBufferIndex = 0;
}

void init_mqtt_state(MQTTState *mqttState) {
    if(!mqttState) {
        return;
    }

    if(mqttState->mqttClient) {
        return;
    }

    mqttState->mqttClient = mqtt_client_new();
    init_mqtt_message_buffer(&mqttState->mIncomingMessageBuffer, 0);
}

static void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status) {
    ConnectionMonitor *monitor = (ConnectionMonitor*) arg;
    monitor->mStatus = status;
    monitor->mConnectionCompleted = true;
}

bool connect_to_broker_blocking(MQTTState *mqttState) {
    struct mqtt_connect_client_info_t ci;
    err_t err;
    char controlTopic[MQTT_MAX_TOPIC_LENGTH];

    if(!mqttState) {
        return false;
    }

    memset(&ci, 0, sizeof(ci));
    ci.client_id    = mqttState->mSensorName;
    ci.client_user  = NULL;
    ci.client_pass  = NULL;
    ci.keep_alive   = 10;
    ci.will_topic   = NULL;
    ci.will_msg     = NULL;
    ci.will_retain  = 0;
    ci.will_qos     = 0;

    ConnectionMonitor connectionMonitor;
    connectionMonitor.mConnectionCompleted = false;

    cyw43_arch_lwip_begin();
    mqtt_client_connect(mqttState->mqttClient, &mqttState->mBrokerAddress, mqttState->mBrokerPort, mqtt_connection_cb, &connectionMonitor, &ci);
    cyw43_arch_lwip_end();

    while(!connectionMonitor.mConnectionCompleted) {
        sleep_ms(1);
    }

    return connectionMonitor.mStatus == MQTT_CONNECT_ACCEPTED;
}

void disconnect_from_broker(MQTTState *mqttState) {
    if(!mqttState) {
        return;
    }

    if(mqtt_client_is_connected(mqttState->mqttClient)) {
        cyw43_arch_lwip_begin();
        mqtt_disconnect(mqttState->mqttClient);
        cyw43_arch_lwip_end();
    }
}

bool subscribe_to_topic(MQTTState *mqttState, const char *topic) {
    // Set callbacks for incoming published data
    mqtt_set_inpub_callback(
        mqttState->mqttClient,
        mqtt_publish_start_callback,
        mqtt_publish_data_callback,
        mqttState
    );

    // Subscribe to supplied topic
    mqtt_sub_unsub(
        mqttState->mqttClient,
        topic,
        0,
        mqtt_subscribe_request_callback,
        0,
        1
    );
}

err_t publish_mqtt_data(MQTTState *mqttState, const char *topic, const char *payload) {
    err_t err;
    u8_t qos = 0;
    u8_t retain = 0;
  
    cyw43_arch_lwip_begin();
    err = mqtt_publish(mqttState->mqttClient, topic, payload, strlen(payload), qos, retain, mqtt_publish_request_callback, mqttState);
    cyw43_arch_lwip_end();

    return err; 
}

void init_mqtt_message(MQTTMessage *message) {
    if(message) {
        memset(message->mTopic, 0, MQTT_MAX_PAYLOAD_LENGTH);
        memset(message->mPayload, 0, MQTT_MAX_PAYLOAD_LENGTH);
    }
}

err_t publish_mqtt_message(MQTTState *mqttState, MQTTMessage *message) {
    if(message && mqttState) {
        return publish_mqtt_data(mqttState, message->mTopic, message->mPayload);
    }

    return ERR_ARG;
}


                ////////////////////
                // MQTT CALLBACKS //
                ////////////////////

// Callback when a new publish has started on a subscribed topic
void mqtt_publish_start_callback(void *arg, const char *topic, u32_t tot_len) {
    char expectedTopic[MQTT_MAX_TOPIC_LENGTH];

    if(!arg) {
        return;
    }

    MQTTState *state = (MQTTState *) arg;

    // Sanity check topic
    get_sensor_control_topic(state, expectedTopic);
    if(strcmp(topic, expectedTopic)) {
        DEBUG_PRINT("Unexpected topic message: %s", topic);
        return;
    }

    if(tot_len > MQTT_MAX_PAYLOAD_LENGTH) {
        // This is unfortunate - our payload is larger than our buffer
        DEBUG_PRINT("Incoming publish is too large (%d bytes)", tot_len);
        init_mqtt_message_buffer(&state->mIncomingMessageBuffer, 0);
    } else {
        DEBUG_PRINT("Receiving control message (%s)", topic);
        init_mqtt_message_buffer(&state->mIncomingMessageBuffer, tot_len);
        memcpy(state->mIncomingMessageBuffer.mMessage.mTopic, topic, strlen(topic));
    }
}

// Callback when a payload fragment has been received
void mqtt_publish_data_callback(void *arg, const u8_t *data, u16_t len, u8_t flags) {
    char command[8];
    char value[8];
    if(!arg) {
        return;
    }

    MQTTState *state = (MQTTState *) arg;

    if((state->mIncomingMessageBuffer.mBufferIndex + len) <= state->mIncomingMessageBuffer.mPayloadSize) {
        memcpy(
            state->mIncomingMessageBuffer.mMessage.mPayload + state->mIncomingMessageBuffer.mBufferIndex,
            data, 
            len
        );
        state->mIncomingMessageBuffer.mBufferIndex += len;

        if(state->mIncomingMessageBuffer.mBufferIndex >= state->mIncomingMessageBuffer.mPayloadSize) {
            SensorControlMessage controlMessage;
            if(mqtt_to_control_message(&(state->mIncomingMessageBuffer.mMessage), &controlMessage)) {
                push_mqtt_control_data_to_queue(state->mSensorControlQueue, &controlMessage);
            }
        }
    } else {
        // Payload would overflow buffer
        DEBUG_PRINT("MQTT payload overflows buffer");
    }
}

// Called when a subscribe/unsubscribe request has been completed
void mqtt_subscribe_request_callback(void *arg, err_t err) {
    if(!arg) {
        return;
    }

    DEBUG_PRINT("mqtt_sub_request_cb: err %d\n", err);
}

// Called when a local publish to a topic has been completed
// Currently unused
void mqtt_publish_request_callback(void *arg, err_t err) {}
