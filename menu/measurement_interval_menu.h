#ifndef _MEASUREMENT_INTERVAL_MENU_H_
#define _MEASUREMENT_INTERVAL_MENU_H_

#include "menu_interface.h"
#include "pico/stdlib.h"
#include "scd30_sensor.h"


typedef struct {
    UserInputStateHolder mInputStateHolder;
    SCD30Sensor *mSCD30Sensor;
} MeasurementIntervalMenuObject;

extern const char* MEASUREMENT_INTERVAL_MENU_TEXT;

menu_return_behavior measurement_interval_menu_handler(int input, void* menuObject);
void reset_measurement_interval_menu_object(MeasurementIntervalMenuObject* menuObject);

#endif