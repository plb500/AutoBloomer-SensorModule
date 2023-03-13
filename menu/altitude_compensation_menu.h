#ifndef _ALTITUDE_COMPENSATION_MENU_H_
#define _ALTITUDE_COMPENSATION_MENU_H_

#include "menu_interface.h"
#include "pico/stdlib.h"
#include "scd30_sensor.h"


typedef struct {
    UserInputStateHolder mInputStateHolder;
    SCD30Sensor *mSCD30Sensor;
} AltitudeCompensationMenuObject;

extern const char* ALTITUDE_COMPENSATION_MENU_TEXT;

menu_return_behavior altitude_compensation_menu_handler(int input, void* menuObject);
void reset_altitude_compensation_menu_object(AltitudeCompensationMenuObject* menuObject);

#endif