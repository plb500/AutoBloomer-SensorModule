#ifndef _CORE_0_EXECUTOR_H_
#define _CORE_0_EXECUTOR_H_

#include "messaging/multicore_mailbox.h"
#include "userdata/user_data.h"
#include "serial_control/serial_controller.h"
#include "network/network_controller.h"
#include "network/mqtt_controller.h"


class Core0Executor {
    public:
        Core0Executor(MulticoreMailbox& mailbox, const vector<SensorGroup>& sensorGroups);

        void initialize();
        
        static void loop();
        static void setExecutor(Core0Executor& executor);

    private:
        void doLoop();

        void softwareReset();
        void stopCore1AndWriteUserData();
        
        void transmitData();
        void transmitSensorData();
        void transmitTestMQTTMessage();

        uint32_t getFreeMemory();
        uint32_t getHeapSize();

        constexpr static int STDIO_PING_TIMEOUT                 = 2000;
        constexpr static uint16_t MQTT_UPDATE_CHECK_PERIOD_MS   = 750;

        static Core0Executor* sExecutor;

        MulticoreMailbox& mMailbox;
        UserData mUserData;
        SerialController mSerialController;
        NetworkController mNetworkController;
        MQTTController mMQTTController;
        const vector<SensorGroup>& mSensorGroups;

};

#endif      // _CORE_0_EXECUTOR_H_
