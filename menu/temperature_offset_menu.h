#ifndef _TEMPERATURE_OFFSET_MENU_H_
#define _TEMPERATURE_OFFSET_MENU_H_

#include "menu_interface.h"
#include "pico/stdlib.h"
#include "scd30_sensor.h"


typedef struct {
    UserInputStateHolder mInputStateHolder;
    SCD30Sensor *mSCD30Sensor;
} TemperatureOffsetMenuObject;

extern const char* TEMPERATURE_OFFSET_MENU_TEXT;

menu_return_behavior temperature_offset_menu_handler(int input, void* menuObject);
void reset_temperature_offset_menu_object(TemperatureOffsetMenuObject* menuObject);

#endif