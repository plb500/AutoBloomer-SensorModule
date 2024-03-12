#ifndef _CORE_0_EXECUTOR_H_
#define _CORE_0_EXECUTOR_H_

#include "messaging/multicore_mailbox.h"
#include "messaging/mqtt_message.h"
#include "userdata/user_data.h"
#include "serial_control/serial_controller.h"
#include "network/network_controller.h"
#include "network/mqtt_controller.h"
#include "board_hardware/wifi_indicator.h"



class Core0Executor {
    public:
        Core0Executor(MulticoreMailbox& mailbox, vector<SensorGroup>& sensorGroups, WiFiIndicator* wifiIndicator);

        void initialize();
        
        static void loop();
        static void setExecutor(Core0Executor& executor);

    private:
        void doLoop();

        void softwareReset();
        void stopCore1AndWriteUserData();
        
        void transmitData();
        void transmitTestMQTTMessage();
        void transmitSensorData();

        uint32_t getFreeMemory();
        uint32_t getHeapSize();

        constexpr static int STDIO_PING_TIMEOUT                 = 2000;
        constexpr static uint16_t MQTT_UPDATE_CHECK_PERIOD_MS   = 750;
        constexpr static int JSON_BUFFER_SIZE                   = 256;
        constexpr static int WATCHDOG_TIMEOUT_MS                = 15000;

        static Core0Executor* sExecutor;

        MulticoreMailbox& mMailbox;
        UserData mUserData;
        SerialController mSerialController;
        NetworkController mNetworkController;
        MQTTController mMQTTController;
        vector<SensorGroup>& mSensorGroups;
        vector<MQTTMessage> mOutgoingMQTTMessageBuffer;
        WiFiIndicator* mWifiIndicator;
};

#endif      // _CORE_0_EXECUTOR_H_
