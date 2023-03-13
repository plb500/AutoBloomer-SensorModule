#ifndef _FORCED_RECALIBRATION_MENU_H_
#define _FORCED_RECALIBRATION_MENU_H_

#include "menu_interface.h"
#include "pico/stdlib.h"
#include "scd30_sensor.h"


typedef struct {
    UserInputStateHolder mInputStateHolder;
    SCD30Sensor *mSCD30Sensor;
} ForcedRecalibrationMenuObject;

extern const char* FORCED_RECALIBRATION_MENU_TEXT;

menu_return_behavior forced_recalibration_menu_handler(int input, void* menuObject);
void reset_forced_recalibration_menu_object(ForcedRecalibrationMenuObject* menuObject);

#endif