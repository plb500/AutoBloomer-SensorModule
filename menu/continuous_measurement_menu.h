#ifndef _CONTINUOUS_MEASUREMENT_MENU_H_
#define _CONTINUOUS_MEASUREMENT_MENU_H_

#include "menu_interface.h"
#include "pico/stdlib.h"
#include "scd30_sensor.h"


typedef struct {
    UserInputStateHolder mInputStateHolder;
    SCD30Sensor *mSCD30Sensor;
} ContinuousMeasurementMenuObject;

extern const char* CONTINUOUS_MEASUREMENT_TEXT;

menu_return_behavior continuous_measurement_handler(int input, void* menuObject);
void reset_continuous_measurement_menu_object(ContinuousMeasurementMenuObject* menuObject);

#endif