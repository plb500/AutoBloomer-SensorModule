#ifndef _SENSOR_HARDWARE_H_
#define _SENSOR_HARDWARE_H_

#include "pico/types.h"

#include "sensors/sensor_types/sonar_sensor.h"
#include "sensors/sensor_types/battery_sensor.h"

constexpr const uint32_t TOTAL_RAW_DATA_SIZE = (
    ((SonarSensor::RAW_DATA_SIZE + 2) * 2) +
    (BatteryVoltageSensor::RAW_DATA_SIZE + 2)
);

#endif      // _SENSOR_HARDWARE_H_
