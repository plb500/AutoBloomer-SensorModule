#include "forced_recalibration_menu.h"

#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/time.h"

menu_return_behavior handle_forced_recalibration_reading_input(int input, ForcedRecalibrationMenuObject *menuObject);

const int MIN_REFERENCE_VALUE = 400;
const int MAX_REFERENCE_VALUE = 2000;

const char* FORCED_RECALIBRATION_MENU_TEXT = 
    "\n"
    "Recalibrating CO2 reference.\n";

menu_return_behavior handle_forced_recalibration_reading_input(int input, ForcedRecalibrationMenuObject *menuObject) {
    UserInputInt userResponse = read_int_input(input, &menuObject->mInputStateHolder.mArgumentBuffer);

    switch(userResponse.mInputState) {
        case INPUT_COMPLETE:
            if((userResponse.mIntValue >= MIN_REFERENCE_VALUE) && (userResponse.mIntValue <= MAX_REFERENCE_VALUE)) {
                printf("\nReference CO2 value: %d PPM\n", userResponse.mIntValue);

                // Set reference value                
                uint16_t referenceValue = (uint16_t) userResponse.mIntValue;
                if(set_scd30_forced_recalibration_value(menuObject->mSCD30Sensor, referenceValue)) {
                    printf("Value set successfully\n");
                } else {
                    printf("Failed to set value\n");
                }

                menuObject->mInputStateHolder.mState = INPUT_COMPLETED;
            } else {
                printf("\nInvalid reference value (%d). Value must be within the range %d-%d\n", 
                    userResponse.mIntValue,
                    MIN_REFERENCE_VALUE, 
                    MAX_REFERENCE_VALUE
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

menu_return_behavior forced_recalibration_menu_handler(int input, void *menuObject) {
    ForcedRecalibrationMenuObject* forcedRecalibrationMenuObject = (ForcedRecalibrationMenuObject*) menuObject;
    if(!forcedRecalibrationMenuObject) {
        return EXIT_MENU;
    }

    switch(forcedRecalibrationMenuObject->mInputStateHolder.mState) {
        case PROMPT_USER:
            printf("Enter the reference CO2 concentration (in PPM), hit Enter for none or 'q' to quit: ");
            reset_input_buffer(&forcedRecalibrationMenuObject->mInputStateHolder.mArgumentBuffer);
            forcedRecalibrationMenuObject->mInputStateHolder.mState = READING_INPUT;
            break;
        case READING_INPUT:
            return handle_forced_recalibration_reading_input(input, forcedRecalibrationMenuObject);
        case INPUT_COMPLETED:
            return EXIT_MENU;
        default:
            break;
    }

    return DO_NOTHING;
}

void reset_forced_recalibration_menu_object(ForcedRecalibrationMenuObject* menuObject) {
    menuObject->mInputStateHolder.mState = PROMPT_USER;
    reset_input_buffer(&(menuObject->mInputStateHolder.mArgumentBuffer));
}
