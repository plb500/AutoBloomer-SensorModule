#ifndef _MAIN_MENU_H_
#define _MAIN_MENU_H_

#include "menu_interface.h"
#include "scd30_sensor.h"
#include "stemma_soil_sensor.h"


typedef struct {
    int mLEDPin;
    SCD30Sensor *mSCDSensor;
    StemmaSoilSensor *mStemmaSoilSensor;
} MainMenuObject;

extern const char* MAIN_MENU_TEXT;
menu_return_behavior main_menu_handler(int input, void* menuObject);

#endif
