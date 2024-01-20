#ifndef _STEMMA_SOIL_SENSOR_H
#define _STEMMA_SOIL_SENSOR_H

#include "sensor_i2c_interface.h"


#define STEMMA_SOIL_SENSOR_INVALID_READING      (65535)


I2CResponse init_soil_sensor(I2CInterface *i2cInterface, uint8_t address);
I2CResponse reset_soil_sensor(I2CInterface *i2cInterface, uint8_t address);
uint32_t get_soil_sensor_version(I2CInterface *i2cInterface, uint8_t address);
uint16_t get_soil_sensor_capacitive_value(I2CInterface *i2cInterface, uint8_t address);


#endif
