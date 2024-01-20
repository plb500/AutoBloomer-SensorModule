#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/watchdog.h"

#include <stdio.h>

#include "userdata/userdata.h"
#include "cores/sensor_pod_core_1.h"
#include "cores/sensor_multicore_utils.h"
#include "serial_control/serial_controller.h"
#include "network/network_utils.h"
#include "network/mqtt_utils.h"


static const uint8_t UART_TX_PIN        = 0;
static const uint8_t UART_RX_PIN        = 1;

const uint16_t MQTT_UPDATE_PERIOD_MS    = 750;

// Queue used for sending sensor updates from core0 to core1
queue_t sensorUpdateQueue;

// extern queue_t sensorUpdateQueue;
MQTTState mqttState = {
    .mqttClient = 0,
    .mSensorName = 0,
    .mSensorLocation = 0,
    .mSensorUpdateQueue = &sensorUpdateQueue
};

DNSResolutionRequest brokerRequest = {
    .mResolvedAddress.addr = 0,
    .mHost = 0
};

uint8_t userDataMem[USER_DATA_BLOCK_SIZE];
UserData *userData = (UserData *) userDataMem;

SerialCommandBuffer serialCommand;

void software_reset() {
    watchdog_enable(1, 1);
    while(1);
}

void stop_core_1_and_write_user_data(UserData *userData) {
    multicore_lockout_start_blocking();
    write_userdata_to_flash(userData);
    multicore_lockout_end_blocking();
}

int main() {
    bool forceReconnect = false;
    absolute_time_t timeout = nil_time;

    // Setup UART
#if LIB_PICO_STDIO_UART
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
#endif

    // Initialise I/O
    stdio_init_all(); 

    // Initialize the cross-core queue
    intitialize_sensor_data_queue(&sensorUpdateQueue, 4);

    // Grab user data
    if(!read_userdata_from_flash(userData)) {
        init_userdata(userData);
    } else {
        printf("Flash contents:\n");
        printf("  +- NAME: %s\n", userData->m_sensorName);
        printf("  +- LOCN: %s\n", userData->m_locationName);
        printf("  +- SSID: %s\n", userData->m_ssid);
        printf("  +- PKEY: %s\n", userData->m_psk);
        printf("  +- MQTT: %s\n", userData->m_brokerAddress);
    }

    init_serial_command_buffer(&serialCommand);

    // Launch secondary core (core 1) to process sensor data
    multicore_launch_core1(sensor_pod_core_1_main);

    while(1) {
        // Process any incoming serial data
        if(update_user_data(userData, &serialCommand)) {
            // If the flash data has changed we probably want to reboot the device
            stop_core_1_and_write_user_data(userData);
            software_reset();
        }

        // Check to see if we need to (re)connect to the network
        if(has_network_userdata(userData) && (!is_network_connected() || forceReconnect)) {
            printf("Network is not connected, connecting....\n");
            connect_to_wifi(userData->m_ssid, userData->m_psk, userData->m_sensorName);
            forceReconnect = false;
            init_mqtt_state(&mqttState);
        }

        // Update network LED indicator
        if(is_network_connected()) {
            wifi_led_on();
        } else {
            wifi_led_off();
        }

        absolute_time_t now = get_absolute_time();
        if(
            (is_nil_time(timeout) || absolute_time_diff_us(now, timeout) <= 0) && 
            has_mqtt_userdata(userData)
        ) {
            brokerRequest.mResolvedAddress.addr = 0;
            brokerRequest.mHost = userData->m_brokerAddress;

            if(is_network_connected()) {
                resolve_host_blocking(&brokerRequest);
            }

            if(brokerRequest.mResolvedAddress.addr) {
                mqttState.mBrokerAddress = brokerRequest.mResolvedAddress;
                mqttState.mBrokerPort = MQTT_PORT;

                mqttState.mSensorName = userData->m_sensorName;
                mqttState.mSensorLocation = userData->m_locationName;

                if(!mqtt_client_is_connected(mqttState.mqttClient)) {
                    connect_to_broker_blocking(&mqttState);
                }

                if(mqtt_client_is_connected(mqttState.mqttClient)) {
                    printf("CONNECTED TO BROKER\n");
                    pull_mqtt_data_from_queue(&mqttState);
                    disconnect_from_broker(&mqttState);
                    timeout = make_timeout_time_ms(MQTT_UPDATE_PERIOD_MS);
                } else {
                    forceReconnect = true;
                }
            } else {
                forceReconnect = true;
            }
        }

        // Chill
        sleep_ms(1);
    }
}
