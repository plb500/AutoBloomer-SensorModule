#include "temperature_offset_menu.h"

#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/time.h"

menu_return_behavior handle_temperature_offset_reading_input(int input, TemperatureOffsetMenuObject *menuObject);

const int MIN_TEMPERATURE_VALUE = 0;
const int MAX_TEMPERATURE_VALUE = 65535;

const char* TEMPERATURE_OFFSET_MENU_TEXT = 
    "\n"
    "Set temperature offset\n";

menu_return_behavior handle_temperature_offset_reading_input(int input, TemperatureOffsetMenuObject *menuObject) {
    UserInputInt userResponse = read_int_input(input, &menuObject->mInputStateHolder.mArgumentBuffer);

    switch(userResponse.mInputState) {
        case INPUT_COMPLETE:
            if((userResponse.mIntValue >= MIN_TEMPERATURE_VALUE) && (userResponse.mIntValue <= MAX_TEMPERATURE_VALUE)) {
                printf("\nTemmperature offset value (°C x 100): %d\n", userResponse.mIntValue);

                // Set reference value                
                uint16_t temperatureValue = (uint16_t) userResponse.mIntValue;
                if(set_scd30_temperature_offset(menuObject->mSCD30Sensor, temperatureValue)) {
                    printf("Value set successfully\n");
                } else {
                    printf("Failed to set value\n");
                }

                menuObject->mInputStateHolder.mState = INPUT_COMPLETED;
            } else {
                printf("\nInvalid temperature value (%d). Value must be within the range %d-%d\n", 
                    userResponse.mIntValue,
                    MIN_TEMPERATURE_VALUE, 
                    MAX_TEMPERATURE_VALUE
                );
                menuObject->mInputStateHolder.mState = PROMPT_USER;
            }
            break;
    
        case INPUT_ABORTED:
            return EXIT_MENU;

        case INPUT_INCOMPLETE:
        default:
            break;
    }

    return DO_NOTHING;
}

menu_return_behavior temperature_offset_menu_handler(int input, void *menuObject) {
    TemperatureOffsetMenuObject* temperatureOffsetMenuObject = (TemperatureOffsetMenuObject*) menuObject;
    if(!temperatureOffsetMenuObject) {
        return EXIT_MENU;
    }

    switch(temperatureOffsetMenuObject->mInputStateHolder.mState) {
        case PROMPT_USER:
            printf("Enter the temperature offset (°C x 100), hit Enter for none or 'q' to quit: ");
            reset_input_buffer(&temperatureOffsetMenuObject->mInputStateHolder.mArgumentBuffer);
            temperatureOffsetMenuObject->mInputStateHolder.mState = READING_INPUT;
            break;
        case READING_INPUT:
            return handle_temperature_offset_reading_input(input, temperatureOffsetMenuObject);
        case INPUT_COMPLETED:
            return EXIT_MENU;
        default:
            break;
    }

    return DO_NOTHING;
}

void reset_temperature_offset_menu_object(TemperatureOffsetMenuObject* menuObject) {
    menuObject->mInputStateHolder.mState = PROMPT_USER;
    reset_input_buffer(&(menuObject->mInputStateHolder.mArgumentBuffer));
}
