#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/watchdog.h"

#include "util/debug_io.h"
#include "userdata/userdata.h"
#include "cores/sensor_pod_core_1.h"
#include "messages/multicore_mailbox.h"
#include "serial_control/serial_controller.h"
#include "network/network_utils.h"
#include "network/mqtt_utils.h"

const uint32_t SCHEDULED_REBOOT_TIMEOUT = (6 * 60 * 60 * 1000);       // Let's go with every 6 hours
const uint16_t MQTT_UPDATE_CHECK_PERIOD_MS  = 750;

MulticoreMailbox coreMailbox;

// extern queue_t sensorUpdateQueue;
MQTTState mqttState = {
    .mqttClient = 0,
    .mSensorName = 0,
    .mSensorLocation = 0,
    .mCoreMailbox = &coreMailbox
};

DNSResolutionRequest brokerRequest = {
    .mResolvedAddress.addr = 0,
    .mHost = 0
};

uint8_t userDataMem[USER_DATA_BLOCK_SIZE];
UserData *userData = (UserData *) userDataMem;

SerialCommandBuffer serialCommand;

bool mqttTestMode = true;

void test_pin_init(int pin) {
    gpio_init(pin);
    gpio_set_dir(pin, GPIO_OUT);
}

void flip_test_pin(int pin) {
    bool newVal = !gpio_get(pin);
    gpio_put(pin, newVal);
}

void software_reset() {
    watchdog_enable(1, 1);
    while(1);
}

void stop_core_1_and_write_user_data(UserData *userData) {
    if(!mqttTestMode) {
        multicore_lockout_start_blocking();
    }

    write_userdata_to_flash(userData);

    if(!mqttTestMode) {
        multicore_lockout_end_blocking();
    }
}

void get_sensor_control_topic(MQTTState *state, char *topicBuffer) {
    if(!state || !topicBuffer) {
        return;
    }

    snprintf(
        topicBuffer,
        MQTT_MAX_TOPIC_LENGTH,
        "%s/%s/%s/control",
        AUTOBLOOMER_TOPIC_NAME, state->mSensorLocation, state->mSensorName
    );
}

void transmit_sensor_data(MQTTState *state) {
    SensorDataUpdateMessage dataUpdateMsg;
    MQTTMessage mqttMsg;

    if(!state) {
        return;
    }

    // Process any sensor data waiting for us in the mailbox
    if(get_latest_sensor_data_message(state->mCoreMailbox, &dataUpdateMsg)) {
        data_update_to_mqtt_message(state->mSensorName, state->mSensorLocation, &dataUpdateMsg, &mqttMsg);
        publish_mqtt_message(state, &mqttMsg);
    }
}

void transmit_test_mqtt_message(MQTTState *state) {
    MQTTMessage mqttMsg;
    char message[64];
    static int messageValue = 0;
    sprintf(message, "TEST MESSAGE: %d", messageValue++);

    if(!state) {
        return;
    }

    DEBUG_PRINT("Publishing MQTT test message (# %d)", messageValue);

    create_test_mqtt_message(
        state->mSensorName, 
        state->mSensorLocation, 
        message, 
        &mqttMsg
    );

    publish_mqtt_message(state, &mqttMsg);
}

void transmit_data(MQTTState *state) {
    if(mqttTestMode) {
        transmit_test_mqtt_message(state);
    } else {
        transmit_sensor_data(state);
    }
}

int main() {
    absolute_time_t mqttUpdateTimeout = nil_time;
    absolute_time_t pingTimeout = nil_time;

    char controlTopic[MQTT_MAX_TOPIC_LENGTH];

    // Setup UART
    DEBUG_PRINT_INIT()

    // Initialise I/O
    stdio_init_all(); 

    test_pin_init(6);

    // Initialize the mailbox
    init_multicore_mailbox(&coreMailbox);

    // Grab user data
    if(!read_userdata_from_flash(userData)) {
        init_userdata(userData);
    } else {
        DEBUG_PRINT("Flash contents:");
        DEBUG_PRINT("  +- NAME: %s", userData->m_sensorName);
        DEBUG_PRINT("  +- LOCN: %s", userData->m_locationName);
        DEBUG_PRINT("  +- SSID: %s", userData->m_ssid);
        DEBUG_PRINT("  +- PKEY: %s", userData->m_psk);
        DEBUG_PRINT("  +- MQTT: %s\n", userData->m_brokerAddress);
    }

    init_serial_command_buffer(&serialCommand);

    // Launch secondary core (core 1) to process sensor data
    if(!mqttTestMode) {
        multicore_launch_core1(sensor_pod_core_1_main);
    }

    while(1) {
        absolute_time_t now = get_absolute_time();

        // Periodically send an update through the serial port just to show core0 is still functioning
        if(is_nil_time(pingTimeout) || absolute_time_diff_us(now, pingTimeout) <= 0) {
            DEBUG_PRINT("- core0 ping -")
            pingTimeout = make_timeout_time_ms(2000);
            flip_test_pin(6);
        }

        // Process any incoming serial data
        if(update_user_data(userData, &serialCommand)) {
            // If the flash data has changed we probably want to reboot the device
            stop_core_1_and_write_user_data(userData);
            software_reset();
        }

        // Check to see if we need to (re)connect to the network
        if(has_network_userdata(userData) && !is_network_connected()) {
            wifi_led_off();
            DEBUG_PRINT("Network is not connected, connecting....");
            int connectResponse = connect_to_wifi(userData->m_ssid, userData->m_psk, userData->m_sensorName);
            DEBUG_PRINT("...connect %s (%d)", 
                connectResponse ? "failed" : "succeeded",
                connectResponse
            );

            if(!connectResponse) {
                wifi_led_on();
                init_mqtt_state(&mqttState);
            }
        }

        if(is_network_connected()) {
            wifi_led_on();

            // If we aren't connected to the broker yet, connect and subscribe
            if(!mqtt_client_is_connected(mqttState.mqttClient) && has_mqtt_userdata(userData)) {
                DEBUG_PRINT("Not connected to MQTT broker, opening connection...");

                // Lookup the broker's IP
                brokerRequest.mResolvedAddress.addr = 0;
                brokerRequest.mHost = userData->m_brokerAddress;
                resolve_host_blocking(&brokerRequest);

                // If we have the broker's address, proceed with the connection
                if(brokerRequest.mResolvedAddress.addr) {
                    char ip_string[16];
                    ip_address_to_string(ip_string, &brokerRequest.mResolvedAddress);
                    DEBUG_PRINT("Broker resolved (IP: %s). Connecting to broker...", ip_string);

                    mqttState.mBrokerAddress = brokerRequest.mResolvedAddress;
                    mqttState.mBrokerPort = MQTT_PORT;

                    mqttState.mSensorName = userData->m_sensorName;
                    mqttState.mSensorLocation = userData->m_locationName;

                    if(!connect_to_broker_blocking(&mqttState)) {
                        DEBUG_PRINT("Broker connection failed");
                    } else {
                        DEBUG_PRINT("Broker connection succeeded. Subscribing to control topic")
                        get_sensor_control_topic(&mqttState, controlTopic);
                        subscribe_to_topic(&mqttState, controlTopic);
                        DEBUG_PRINT("MQTT broker subscription complete");
                        mqttUpdateTimeout = make_timeout_time_ms(MQTT_UPDATE_CHECK_PERIOD_MS);
                    }
                } else {
                    DEBUG_PRINT("Broker IP resolution failed");
                }
            }

            // If we are connected and it's time to publish sensor data, do so.
            if(
                (is_nil_time(mqttUpdateTimeout) || absolute_time_diff_us(now, mqttUpdateTimeout) <= 0) && 
                mqtt_client_is_connected(mqttState.mqttClient)
            ) {
                transmit_data(&mqttState);
                mqttUpdateTimeout = make_timeout_time_ms(MQTT_UPDATE_CHECK_PERIOD_MS);
            }
        } else {
            wifi_led_off();
        }

        // Chill
        sleep_ms(1);
    }
}
