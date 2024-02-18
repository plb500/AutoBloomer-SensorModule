#ifndef _DUMMY_SENSOR_H_
#define _DUMMY_SENSOR_H_

#include "sensor.h"

class DummySensor : public Sensor {
    public:
        DummySensor();

        virtual void initialize() {}
        virtual void reset() {}
        virtual void shutdown() {}

        static int serializeDataToJSON(uint8_t* data, uint8_t dataSize, char* jsonBuffer, int jsonBufferSize);

    protected:
        Sensor::SensorUpdateResponse doUpdate(absolute_time_t currentTime, uint8_t *dataStorageBuffer, size_t bufferSize);

    private:
        static constexpr int UPDATE_TIME_MS     = 2000;

        int mDummyInt;
        float mDummyFloat;
        absolute_time_t mNextUpdateTime;
};

#endif      // _DUMMY_SENSOR_H_
