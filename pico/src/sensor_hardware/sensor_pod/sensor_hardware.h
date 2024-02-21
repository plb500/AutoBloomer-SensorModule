#ifndef _SENSOR_HARDWARE_H_
#define _SENSOR_HARDWARE_H_

#include "pico/types.h"
#include "sensors/scd30_sensor.h"
#include "sensors/stemma_soil_sensor.h"

constexpr const uint32_t TOTAL_RAW_DATA_SIZE = (
    (SCD30Sensor::RAW_DATA_SIZE + 2) +
    (StemmaSoilSensor::RAW_DATA_SIZE + 2)
);

#endif      // _SENSOR_HARDWARE_H_
