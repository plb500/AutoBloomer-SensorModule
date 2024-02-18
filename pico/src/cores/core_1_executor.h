#ifndef _CORE_1_EXECUTOR_H_
#define _CORE_1_EXECUTOR_H_


#include <optional>
#include <vector>
#include "messaging/multicore_mailbox.h"
#include "sensors/sensor_group.h"


using std::optional;
using std::vector;


class Core1Executor {
    public:
        Core1Executor(
            MulticoreMailbox& mailbox,
            vector<SensorGroup>& sensors
        );

        void initialize();
        
        static void loop();
        static void setExecutor(Core1Executor& executor);

    private:
        void doLoop(); 
        void processSensorControlCommands();


        static Core1Executor* sExecutor;

        MulticoreMailbox& mMailbox;
        vector<SensorGroup>& mSensorGroups;

        // SensorPod mSensorPod;
};

#endif      // _CORE_0_EXECUTOR_H_
