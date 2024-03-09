#ifndef _CORE_MESSAGE_QUEUE_H_
#define _CORE_MESSAGE_QUEUE_H_

#include "pico/sync.h"
#include <cstring>

template
<typename T>
class CoreMessageQueue {
    public:
        CoreMessageQueue(int numMessages) :
            mQueueSize{numMessages},
            mFront{-1},
            mRear{-1}
        {
            mQueueEntries = new T[numMessages];
            mutex_init(&mQueueMutex);
        }

        ~CoreMessageQueue() {
            delete[] mQueueEntries;
        }

        bool addToQueue(const T& message) {
            mutex_enter_blocking(&mQueueMutex);

            if(++mRear == mQueueSize) {
                mRear = 0;
            }
            if(mFront == -1) {
                mFront = 0;
            }

            memcpy(&mQueueEntries[mRear], &message, sizeof(T));

            mutex_exit(&mQueueMutex);

            return true;
        }

        bool readFromQueue(T& destination) {
            bool readValue = false;

            mutex_enter_blocking(&mQueueMutex);

            if(mFront != -1) {
                // Queue has values, copy next available value
                memcpy(&destination, &mQueueEntries[mFront], sizeof(T));

                if(mFront == mRear) {
                    // Queue is drained
                    mFront = -1;
                    mRear = -1;
                } else {
                    if(++mFront == mQueueSize) {
                        mFront = 0;
                    }
                }

                readValue = true;
            }

            mutex_exit(&mQueueMutex);

            return readValue;
        }

        bool isFull() {
            bool full = false;
            mutex_enter_blocking(&mQueueMutex);

            full = (
                (mRear == (mQueueSize - 1) && mFront == 0) || 
                (mRear == (mFront - 1))
            );

            mutex_exit(&mQueueMutex);

            return full;
        }

    private:
        T*  mQueueEntries;
        const int mQueueSize;
        int mFront;
        int mRear;
        mutex_t mQueueMutex;
};

#endif      //  _CORE_MESSAGE_QUEUE_H_
