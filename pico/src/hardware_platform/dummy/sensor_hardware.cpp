#include "sensors/sensor_group.h"
#include "sensors/sensor_types/dummy_sensor.h"
#include "pico/stdlib.h"
#include <vector>

DummySensor _dummySensorA;
DummySensor _dummySensorB;
extern const int NUM_SENSOR_GROUPS      = 2;

vector<SensorGroup> _SENSOR_GROUPS = {
    SensorGroup(
        {
            &_dummySensorA
        }
    ),
    SensorGroup(
        {
            &_dummySensorB
        }
    )
};
