#ifndef _STEMMA_SOIL_SENSOR_H_
#define _STEMMA_SOIL_SENSOR_H_

#include "sensor_pod.h"

typedef enum {
    SOIL_SENSOR_1_ADDRESS = 0x36,
    SOIL_SENSOR_2_ADDRESS = 0x37,
    SOIL_SENSOR_3_ADDRESS = 0x38,
    SOIL_SENSOR_4_ADDRESS = 0x39
} SoilSensorAddresses;

// typedef struct {
//     I2CInterface *mInterface;
//     uint8_t mAddress;
// } StemmaSoilSensor;

#define STEMMA_SOIL_SENSOR_INVALID_READING      (65535)

bool init_soil_sensor(SensorPod *sensorPod);
bool reset_soil_sensor(SensorPod *sensorPod);
uint32_t get_soil_sensor_version(SensorPod *sensorPod);
uint16_t get_soil_sensor_capacitive_value(SensorPod *sensorPod);

#endif  // _STEMMA_SOIL_SENSOR_H_
