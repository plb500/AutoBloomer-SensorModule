#ifndef _USER_INT_INPUT_MENU_H
#define _USER_INT_INPUT_MENU_H

#include "menu_interface.h"
#include "pico/stdlib.h"
#include "scd30_sensor.h"


extern const uint32_t NO_USER_INPUT_VALUE;

typedef struct {
    const char *mMenuText;
    const char *mUserPrompt;
    const uint16_t mMinValue;
    const uint16_t mMaxValue;
    const uint16_t mDefaultValue;
    uint32_t mEnteredValue;
    UserInputStateHolder mInputStateHolder;
} UserIntInputMenuObject;


menu_return_behavior user_int_input_menu_handler(int input, void* menuObject);
void reset_user_int_input_menu_object(UserIntInputMenuObject* menuObject);


#endif
