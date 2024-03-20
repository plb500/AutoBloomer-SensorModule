#ifndef _STEMMA_SOIL_SENSOR_H_
#define _STEMMA_SOIL_SENSOR_H_


#include "sensors/sensor.h"
#include "sensors/hardware_interfaces/sensor_i2c_interface.h"
#include "pico/types.h"


class StemmaSoilSensor : public Sensor {
    public:
        enum Addresses {
            SOIL_SENSOR_1_ADDRESS = 0x36,
            SOIL_SENSOR_2_ADDRESS = 0x37,
            SOIL_SENSOR_3_ADDRESS = 0x38,
            SOIL_SENSOR_4_ADDRESS = 0x39
        };

        StemmaSoilSensor(I2CInterface& i2cInterface, uint8_t address);

        virtual void reset();
        virtual void shutdown();

        static int serializeDataToJSON(uint8_t* data, uint8_t dataSize, char* jsonBuffer, int jsonBufferSize);
        virtual constexpr uint16_t getRawDataSize() const { return RAW_DATA_SIZE; }

        static const uint32_t RAW_DATA_SIZE = sizeof(uint16_t);
    protected:
        virtual void doInitialization();
        virtual SensorUpdateResponse doUpdate(absolute_time_t currentTime, uint8_t *dataStorageBuffer, size_t bufferSize);

        constexpr static uint16_t STEMMA_SOIL_SENSOR_INVALID_READING = -1;

    private:
        uint32_t getVersion();
        uint16_t getCapacitiveValue();

        I2CInterface& mI2CInterface;
        uint8_t mAddress;
        bool mActive;
};

#endif      // _STEMMA_SOIL_SENSOR_H_
