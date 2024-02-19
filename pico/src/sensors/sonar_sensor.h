#ifndef _SONAR_SENSOR_H_
#define _SONAR_SENSOR_H_

#include "sensor.h"
#include "hardware/pio.h"


struct PIOWrapper {
    PIO mPIO;
    uint mOffset;
    bool mInitialized;
};

class SonarSensor : public Sensor {
    public:
        SonarSensor(PIOWrapper &pioWrapper, int stateMachineID, int txPin, int rxPin, int baud);

        virtual void initialize();
        virtual void reset();
        virtual void shutdown();                

        static int serializeDataToJSON(uint8_t* data, uint8_t dataSize, char* jsonBuffer, int jsonBufferSize);

    protected:
        virtual SensorUpdateResponse doUpdate(absolute_time_t currentTime, uint8_t *dataStorageBuffer, size_t bufferSize);

    private:
        static void initializeSonarPIO(PIOWrapper& pioWrapper);

        static constexpr int SONAR_SENSOR_PACKET_SIZE   = 4;

        PIOWrapper& mPIOWrapper;
        const uint mStateMachineID;
        const int mTXPin;
        const int mRXPin;
        const int mBaudrate;

        char mPacketBuffer[SONAR_SENSOR_PACKET_SIZE];
        int mCurrentBufferPos;
};

#endif      // _SONAR_SENSOR_H
