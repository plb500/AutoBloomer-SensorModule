#include "measurement_interval_menu.h"

#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/time.h"

menu_return_behavior handle_measurement_interval_reading_input(int input, MeasurementIntervalMenuObject *menuObject);

const int MIN_INTERVAL_VALUE = 2;
const int MAX_INTERVAL_VALUE = 1800;

const char* MEASUREMENT_INTERVAL_MENU_TEXT = 
    "\n"
    "Setting measurement interval.\n";

menu_return_behavior handle_measurement_interval_reading_input(int input, MeasurementIntervalMenuObject *menuObject) {
    UserInputInt userResponse = read_int_input(input, &menuObject->mInputStateHolder.mArgumentBuffer);

    switch(userResponse.mInputState) {
        case INPUT_COMPLETE:
            if((userResponse.mIntValue >= MIN_INTERVAL_VALUE) && (userResponse.mIntValue <= MAX_INTERVAL_VALUE)) {
                printf("\nMeasurement interval value: %d seconds\n", userResponse.mIntValue);

                // Set interval value                
                uint16_t measurementInterval = (uint16_t) userResponse.mIntValue;
                if(set_scd30_measurement_interval(menuObject->mSCD30Sensor, measurementInterval)) {
                    printf("Value set successfully\n");
                } else {
                    printf("Failed to set value\n");
                }

                menuObject->mInputStateHolder.mState = INPUT_COMPLETED;
            } else {
                printf("\nInvalid interval value (%d). Value must be within the range %d-%d\n", 
                    userResponse.mIntValue,
                    MIN_INTERVAL_VALUE, 
                    MAX_INTERVAL_VALUE
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

menu_return_behavior measurement_interval_menu_handler(int input, void *menuObject) {
    MeasurementIntervalMenuObject* measurementIntervalMenuObject = (MeasurementIntervalMenuObject*) menuObject;
    if(!measurementIntervalMenuObject) {
        return EXIT_MENU;
    }

    switch(measurementIntervalMenuObject->mInputStateHolder.mState) {
        case PROMPT_USER:
            printf("Enter the measurement interval (in seconds), hit Enter for none or 'q' to quit: ");
            reset_input_buffer(&measurementIntervalMenuObject->mInputStateHolder.mArgumentBuffer);
            measurementIntervalMenuObject->mInputStateHolder.mState = READING_INPUT;
            break;
        case READING_INPUT:
            return handle_measurement_interval_reading_input(input, measurementIntervalMenuObject);
        case INPUT_COMPLETED:
            return EXIT_MENU;
        default:
            break;
    }

    return DO_NOTHING;
}

void reset_measurement_interval_menu_object(MeasurementIntervalMenuObject* menuObject) {
    menuObject->mInputStateHolder.mState = PROMPT_USER;
    reset_input_buffer(&(menuObject->mInputStateHolder.mArgumentBuffer));
}
