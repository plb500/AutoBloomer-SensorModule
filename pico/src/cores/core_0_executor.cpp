#include "core_0_executor.h"
#include "util/debug_io.h"

#include "hardware/watchdog.h"
#include "pico/multicore.h"
#include <malloc.h>
#include <cstring>

Core0Executor* Core0Executor::sExecutor = nullptr;

Core0Executor::Core0Executor(MulticoreMailbox& mailbox, vector<SensorGroup>& sensorGroups, WiFiIndicator* wifiIndicator) :
    mMailbox{mailbox},
    mMQTTController{mailbox},
    mSensorGroups{sensorGroups},
    mWifiIndicator{wifiIndicator}
{}

void Core0Executor::initialize() {
    // Grab user data
    if(mUserData.readFromFlash()) {
        DEBUG_PRINT(0, "Flash contents:");
        DEBUG_PRINT(0, "  +- SSID: %s", mUserData.getSSID().c_str());
        DEBUG_PRINT(0, "  +- PASS: %s", mUserData.getPSK().c_str());
        DEBUG_PRINT(0, "  +- NAME: %s", mUserData.getHostName().c_str());
        DEBUG_PRINT(0, "  +- BRKR: %s", mUserData.getBrokerAddress().c_str());
        DEBUG_PRINT(0, "  +- Sensor Groups");
        for(int i = 0; i < mSensorGroups.size(); ++i) {
            DEBUG_PRINT(0, "    [%d] GRPN: %s, GRPL: %s",
                i,
                mUserData.getSensorGroupName(i).c_str(),
                mUserData.getSensorGroupLocation(i).c_str()
            );

            mSensorGroups[i].setName(mUserData.getSensorGroupName(i).c_str());
            mSensorGroups[i].setLocation(mUserData.getSensorGroupLocation(i).c_str());
        }
    } else {
        DEBUG_PRINT(0, "Could not read user data from flash memory")
    }

    mOutgoingMQTTMessageBuffer.resize(mSensorGroups.size());
}

void Core0Executor::loop() {
    if(!sExecutor) {
        return;
    }

    sExecutor->doLoop();
}

void Core0Executor::setExecutor(Core0Executor& executor) {
    sExecutor = &executor;
}

void Core0Executor::doLoop() {
    NetworkController::DNSRequest brokerRequest;
    absolute_time_t pingTimeout = nil_time;
    absolute_time_t mqttUpdateTimeout = nil_time;
    char controlTopic[MQTTMessage::MQTT_MAX_TOPIC_LENGTH];

    while(1) {
        absolute_time_t now = get_absolute_time();

        // Periodically send an update through the serial port just to show core0 is still functioning
        if(is_nil_time(pingTimeout) || absolute_time_diff_us(now, pingTimeout) <= 0) {
            DEBUG_PRINT(0, "Memory status: %d bytes free", getFreeMemory());
            pingTimeout = make_timeout_time_ms(STDIO_PING_TIMEOUT);
        }

        // Process any incoming serial data
        if(mSerialController.updateUserData(mUserData)) {
            // If user data has changed it's probably best to just reboot the board
            stopCore1AndWriteUserData();
            softwareReset();
        }

        // Check to see if we need to (re)connect to the network
        checkNetworkConnection();

        if(mNetworkController.isConnected()) {
            if(mWifiIndicator) mWifiIndicator->ledOn();

            // We may need to trigger a new MQTT connection, so check for that now
            if(createMQTTConnection()) {
                mqttUpdateTimeout = make_timeout_time_ms(MQTT_UPDATE_CHECK_PERIOD_MS);
            }

            // If we are connected and it's time to publish sensor data, do so.
            if(
                mMQTTController.isConnected() &&
                (is_nil_time(mqttUpdateTimeout) || absolute_time_diff_us(now, mqttUpdateTimeout) <= 0)
            ) {
                transmitData();
                mqttUpdateTimeout = make_timeout_time_ms(MQTT_UPDATE_CHECK_PERIOD_MS);
            }
        } else {
            if(mWifiIndicator) mWifiIndicator->ledOff();
        }

        sleep_ms(1);
    }
}

void Core0Executor::softwareReset() {
    watchdog_enable(1, 1);
    while(1);
}

void Core0Executor::stopCore1AndWriteUserData() {
    multicore_lockout_start_blocking();
    mUserData.writeToFlash();
    multicore_lockout_end_blocking();
}

void Core0Executor::checkNetworkConnection() {
    if(mUserData.hasNetworkUserData() && !mNetworkController.isConnected()) {
        if(mWifiIndicator) mWifiIndicator->ledOff();
        
        DEBUG_PRINT(0, "Network is not connected, connecting....");
        int connectResponse = mNetworkController.connectToWiFi(
            mUserData.getSSID().c_str(),
            mUserData.getPSK().c_str(),
            mUserData.getHostName().c_str()
        );
        DEBUG_PRINT(0, "...connect %s (%d)", 
            connectResponse ? "failed" : "succeeded",
            connectResponse
        );

        if(!connectResponse) {
            if(mWifiIndicator) mWifiIndicator->ledOn();
            mMQTTController.initMQTTClient();
        }
    }
}

bool Core0Executor::createMQTTConnection() {
    // If we aren't connected to the broker yet, connect and subscribe
    if(!mMQTTController.isConnected() && mUserData.hasMQTTUserData()) {
        DEBUG_PRINT(0, "Not connected to MQTT broker, opening connection...");

        // Lookup the broker's IP
        NetworkController::DNSRequest brokerRequest;
        brokerRequest.mResolvedAddress.addr = 0;
        brokerRequest.mHost = mUserData.getBrokerAddress().c_str();
        mNetworkController.resolveHost(brokerRequest);

        // If we have the broker's address, proceed with the connection
        if(brokerRequest.mResolvedAddress.addr) {
            char ipString[16];
            NetworkController::ipAddressToString(ipString, &brokerRequest.mResolvedAddress);
            DEBUG_PRINT(0, "Broker resolved (IP: %s). Connecting to broker...", ipString);

            mMQTTController.setBrokerParameters(
                brokerRequest.mResolvedAddress,
                MQTT_PORT
            );
            mMQTTController.setClientParameters(
                mUserData.getHostName().c_str()
            );

            if(!mMQTTController.connectToBrokerBlocking(2000)) {
                DEBUG_PRINT(0, "Broker connection failed");
            } else {
                DEBUG_PRINT(0, "Broker connection succeeded. Subscribing to control topics")
                for(auto& s : mSensorGroups) {
                    if(s.hasTopics()) {
                        mMQTTController.subscribeToTopic(s.getControlTopic());
                    }
                }
                DEBUG_PRINT(0, "MQTT broker subscription complete");
                return true;
            }
        } else {
            DEBUG_PRINT(0, "Broker IP resolution failed");
        }
    }
    return false;
}

void Core0Executor::transmitData() {
    transmitSensorData();
}

void Core0Executor::transmitSensorData() {
    if(!mUserData.hasMQTTUserData()) {
        return;
    }

    if(mMailbox.latestSensorDataToJSON(mSensorGroups, mOutgoingMQTTMessageBuffer)) {
        for(auto& msg : mOutgoingMQTTMessageBuffer) {
            if(msg.mReadyToSend) {
                DEBUG_PRINT(0, "Publishing MQTT message *");
                mMQTTController.publishMessage(msg);
            }
        }
    }
}

uint32_t Core0Executor::getFreeMemory() {
   struct mallinfo m = mallinfo();

   return getHeapSize() - m.uordblks;
}

uint32_t Core0Executor::getHeapSize() {
   extern char __StackLimit, __bss_end__;
   
   return &__StackLimit  - &__bss_end__;
}
