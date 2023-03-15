#ifndef _SENSOR_POD_H_
#define _SENSOR_POD_H_

#include "sensor_i2c_interface.h"

typedef struct {
    I2CInterface *mInterface;
    I2CChannel mI2CChannel;
    uint8_t mSCD30Address;
    uint8_t mSoilSensorAddress;
} SensorPod;

bool select_sensor_pod(SensorPod *sensorPod);

#endif
