#include "mqtt_controller.h"

#include <cstring>
#include "pico/cyw43_arch.h"

#include "util/debug_io.h"

// MQTT callback functions /////////////////////////////////////////////////////////////
struct ConnectionMonitor {
    mqtt_connection_status_t mStatus;
    bool mConnectionCompleted;
};

void mqtt_connection_callback(mqtt_client_t *client, void *arg, mqtt_connection_status_t status);
void mqtt_publish_start_callback(void *arg, const char *topic, u32_t tot_len);
void mqtt_publish_data_callback(void *arg, const u8_t *data, u16_t len, u8_t flags);
void mqtt_subscribe_request_callback(void *arg, err_t err);
void mqtt_publish_request_callback(void *arg, err_t err);
////////////////////////////////////////////////////////////////////////////////////////


void MQTTController::MQTTMessageBuffer::initialize(uint32_t payloadSize) {
    memset(mMessage.mTopic, 0, 128);
    memset(mMessage.mPayload, 0, 128);
    mPayloadSize = payloadSize;
    mBufferIndex = 0;
}

void MQTTController::MQTTMessageBuffer::setMessageTopic(const char *topic) {
    memcpy(mMessage.mTopic, topic, strlen(topic));
}


MQTTController::MQTTController(MulticoreMailbox& mailbox) :
    mMQTTClient(nullptr),
    mBrokerPort(0),
    mSensorName(nullptr),
    mSensorLocation(nullptr),
    mCoreMailbox(mailbox)
{}

void MQTTController::initMQTTClient() {
    mMQTTClient = mqtt_client_new();
    mIncomingMessageBuffer.initialize(0);
}

bool MQTTController::connectToBrokerBlocking(uint16_t timeoutMS) {
    struct mqtt_connect_client_info_t ci;
    err_t err;
    char controlTopic[MQTTMessage::MQTT_MAX_TOPIC_LENGTH];

    memset(&ci, 0, sizeof(ci));
    ci.client_id    = mSensorName;
    ci.client_user  = NULL;
    ci.client_pass  = NULL;
    ci.keep_alive   = 10;
    ci.will_topic   = NULL;
    ci.will_msg     = NULL;
    ci.will_retain  = 0;
    ci.will_qos     = 0;

    ConnectionMonitor connectionMonitor;
    connectionMonitor.mConnectionCompleted = false;
    absolute_time_t connectTimeout = make_timeout_time_ms(timeoutMS);

    cyw43_arch_lwip_begin();
    mqtt_client_connect(mMQTTClient, &mBrokerAddress, mBrokerPort, mqtt_connection_callback, &connectionMonitor, &ci);
    cyw43_arch_lwip_end();

    absolute_time_t now = get_absolute_time();
    while(!connectionMonitor.mConnectionCompleted) {
        sleep_ms(1);
        now = get_absolute_time();
    }

    return connectionMonitor.mStatus == MQTT_CONNECT_ACCEPTED;
}

bool MQTTController::isConnected() {
    if(!mMQTTClient) {
        return false;
    }

    return mqtt_client_is_connected(mMQTTClient);
}

void MQTTController::disconnectFromBroker() {
    if(mqtt_client_is_connected(mMQTTClient)) {
        cyw43_arch_lwip_begin();
        mqtt_disconnect(mMQTTClient);
        cyw43_arch_lwip_end();
    }
}

void MQTTController::subscribeToTopic(const char* topic) {
    // Set callbacks for incoming published data
    mqtt_set_inpub_callback(
        mMQTTClient,
        mqtt_publish_start_callback,
        mqtt_publish_data_callback,
        this
    );

    // Subscribe to supplied topic
    mqtt_sub_unsub(
        mMQTTClient,
        topic,
        0,
        mqtt_subscribe_request_callback,
        0,
        1
    );
}

void MQTTController::setBrokerParameters(ip_addr_t& address, uint16_t port) {
    mBrokerAddress = address;
    mBrokerPort = port;
}

void MQTTController::setClientParameters(const char* sensorName, const char *sensorLocation) {
    mSensorName = sensorName;
    mSensorLocation = sensorLocation;
}

MQTTController::MQTTMessageBuffer& MQTTController::getBuffer() {
    return mIncomingMessageBuffer;
}

void MQTTController::handleIncomingControlMessage(MQTTMessage& message) {
    mCoreMailbox.sendSensorControlMessageToCore1(message);
}

void MQTTController::getSensorControlTopic(char *dest) {
    if(!dest) {
        return;
    }

    snprintf(
        dest,
        MQTTMessage::MQTT_MAX_TOPIC_LENGTH,
        "%s/%s/%s/control",
        MQTTMessage::AUTOBLOOMER_TOPIC_NAME,
        mSensorLocation,
        mSensorName
    );
}


void MQTTController::initializeMessage(MQTTMessage& message) {
    memset(message.mTopic, 0, MQTTMessage::MQTT_MAX_PAYLOAD_LENGTH);
    memset(message.mPayload, 0, MQTTMessage::MQTT_MAX_PAYLOAD_LENGTH);
}

err_t MQTTController::publishMessage(MQTTMessage& message) {
    err_t err;
    u8_t qos = 0;
    u8_t retain = 0;
  
    cyw43_arch_lwip_begin();
    err = mqtt_publish(
        mMQTTClient,
        message.mTopic, 
        message.mPayload, 
        strlen(message.mPayload), 
        qos, 
        retain, 
        mqtt_publish_request_callback, 
        this
    );
    cyw43_arch_lwip_end();

    return err; 
}


// CALLBACK FUNCTIONS //////
void mqtt_connection_callback(mqtt_client_t *client, void *arg, mqtt_connection_status_t status) {
    ConnectionMonitor *monitor = (ConnectionMonitor*) arg;
    monitor->mStatus = status;
    monitor->mConnectionCompleted = true;
}

// Callback when a new publish has started on a subscribed topic
void mqtt_publish_start_callback(void *arg, const char *topic, u32_t tot_len) {
    char expectedTopic[MQTTMessage::MQTT_MAX_TOPIC_LENGTH];

    if(!arg) {
        return;
    }

    MQTTController* controller = (MQTTController *) arg;
    MQTTController::MQTTMessageBuffer& buffer = controller->getBuffer();

    if(tot_len > MQTTMessage::MQTT_MAX_PAYLOAD_LENGTH) {
        // This is unfortunate - our payload is larger than our buffer
        DEBUG_PRINT("Incoming publish is too large (%d bytes)", tot_len);
        buffer.initialize(0);
    } else {
        DEBUG_PRINT("Receiving control message (%s)", topic);
        buffer.initialize(tot_len);
        buffer.setMessageTopic(topic);
    }
}

// Callback when a payload fragment has been received
void mqtt_publish_data_callback(void *arg, const u8_t *data, u16_t len, u8_t flags) {
    char command[8];
    char value[8];
    if(!arg) {
        return;
    }

    MQTTController* controller = (MQTTController *) arg;
    MQTTController::MQTTMessageBuffer& buffer = controller->getBuffer();

    if((buffer.mBufferIndex + len) <= buffer.mPayloadSize) {
        memcpy(
            buffer.mMessage.mPayload + buffer.mBufferIndex,
            data, 
            len
        );
        buffer.mBufferIndex += len;

        if(buffer.mBufferIndex >= buffer.mPayloadSize) {
            controller->handleIncomingControlMessage(buffer.mMessage);
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
