#ifndef _STEMMA_SOIL_SENSOR_H
#define _STEMMA_SOIL_SENSOR_H

#include "sensor_i2c_interface.h"


#define STEMMA_SOIL_SENSOR_INVALID_READING      (-1)

typedef enum {
    SOIL_SENSOR_1_ADDRESS = 0x36,
    SOIL_SENSOR_2_ADDRESS = 0x37,
    SOIL_SENSOR_3_ADDRESS = 0x38,
    SOIL_SENSOR_4_ADDRESS = 0x39
} SoilSensorAddresses;

typedef struct StemmaSoilSensor_t {
    I2CInterface *mI2CInterface;
    uint8_t mAddress;
} StemmaSoilSensor;


I2CResponse init_soil_sensor(StemmaSoilSensor *sensor);
I2CResponse reset_soil_sensor(StemmaSoilSensor *sensor);
uint32_t get_soil_sensor_version(StemmaSoilSensor *sensor);
uint16_t get_soil_sensor_capacitive_value(StemmaSoilSensor *sensor);


#endif
