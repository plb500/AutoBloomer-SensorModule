#include "sensors/sensor_group.h"
#include "sensors/sensor_types/dummy_sensor.h"
#include "board_hardware/pico_w_onboard_led_indicator.h"
#include "pico/stdlib.h"
#include <vector>

DummySensor _dummySensorA;
DummySensor _dummySensorB;
PicoWOnboardLEDIndicator _ledIndicator;

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

extern const int NUM_SENSOR_GROUPS      = 2;
extern WiFiIndicator* _wifiIndicator = &_ledIndicator;
