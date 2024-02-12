#ifndef _STEMMA_SOIL_SENSOR_H_
#define _STEMMA_SOIL_SENSOR_H_


#include "sensor_i2c_interface.h"
#include "pico/types.h"


class StemmaSoilSensor {
    public:
        enum Addresses {
            SOIL_SENSOR_1_ADDRESS = 0x36,
            SOIL_SENSOR_2_ADDRESS = 0x37,
            SOIL_SENSOR_3_ADDRESS = 0x38,
            SOIL_SENSOR_4_ADDRESS = 0x39
        };

        StemmaSoilSensor(I2CInterface& i2cInterface, uint8_t address);

        I2CInterface::I2CResponse init();
        I2CInterface::I2CResponse reset();
        uint32_t getVersion();
        uint16_t getCapacitiveValue();

        constexpr static int STEMMA_SOIL_SENSOR_INVALID_READING = -1;

    private:
        I2CInterface& mI2CInterface;
        uint8_t mAddress;
};

#endif      // _STEMMA_SOIL_SENSOR_H_
