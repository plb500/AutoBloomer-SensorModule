#ifndef _CORE_1_EXECUTOR_H_
#define _CORE_1_EXECUTOR_H_

#include "messaging/multicore_mailbox.h"


class Core1Executor {
    public:
        Core1Executor(MulticoreMailbox& mailbox);

        void initialize();
        
        static void loop();
        static void setExecutor(Core1Executor& executor);

    private:
        void doLoop(); 

        static Core1Executor* sExecutor;

        MulticoreMailbox& mMailbox;
};

#endif      // _CORE_0_EXECUTOR_H_
