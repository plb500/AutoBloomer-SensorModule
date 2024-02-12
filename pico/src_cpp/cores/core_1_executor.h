#ifndef _CORE_1_EXECUTOR_H_
#define _CORE_1_EXECUTOR_H_


#include <optional>
#include "messaging/multicore_mailbox.h"
#include "sensors/sensor_pod.h"


using std::optional;


class Core1Executor {
    public:
        Core1Executor(
            MulticoreMailbox& mailbox,
            optional<StemmaSoilSensor>& soilSensor,
            optional<I2CInterface>& scd30Interface
        );

        void initialize();
        
        static void loop();
        static void setExecutor(Core1Executor& executor);

    private:
        void doLoop(); 

        void processSensorControlCommands();
        void handleSetTemperatureOffsetCommand(const char *commandParam);
        void handleSetFRCCommand(const char *commandParam);


        static Core1Executor* sExecutor;

        MulticoreMailbox& mMailbox;
        SensorPod mSensorPod;
};

#endif      // _CORE_0_EXECUTOR_H_
