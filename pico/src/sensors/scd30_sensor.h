#ifndef _SCD30_SENSOR_H_
#define _SCD30_SENSOR_H_

#include "sensor.h"
#include "sensor_i2c_interface.h"

class SCD30Sensor : public Sensor {
    public:
        SCD30Sensor(I2CInterface& i2c, uint8_t powerPin);

        virtual void initialize();
        virtual void reset();
        virtual void shutdown();                

        static int serializeDataToJSON(uint8_t* data, uint8_t dataSize, char* jsonBuffer, int jsonBufferSize);

    protected:
        virtual Sensor::SensorUpdateResponse doUpdate(absolute_time_t currentTime, uint8_t *dataStorageBuffer, size_t bufferSize);

    private:
        static constexpr int SCD30_MEASUREMENT_INTERVAL_SECONDS      = 2;

        void startReadings();

        I2CInterface& mI2C;
        bool mActive;
        uint8_t mPowerControlPin;
};

#endif      // _SCD30_SENSOR_H_
