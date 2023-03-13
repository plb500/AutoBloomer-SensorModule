#include "altitude_compensation_menu.h"

#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/time.h"

menu_return_behavior handle_altitude_compensation_reading_input(int input, AltitudeCompensationMenuObject *menuObject);

const int MIN_ALTITUDE_VALUE = 0;
const int MAX_ALTITUDE_VALUE = 65535;

const char* ALTITUDE_COMPENSATION_MENU_TEXT = 
    "\n"
    "Set altitude compensation\n";

menu_return_behavior handle_altitude_compensation_reading_input(int input, AltitudeCompensationMenuObject *menuObject) {
    UserInputInt userResponse = read_int_input(input, &menuObject->mInputStateHolder.mArgumentBuffer);

    switch(userResponse.mInputState) {
        case INPUT_COMPLETE:
            if((userResponse.mIntValue >= MIN_ALTITUDE_VALUE) && (userResponse.mIntValue <= MAX_ALTITUDE_VALUE)) {
                printf("\nAltitude value (m above sea level): %d\n", userResponse.mIntValue);

                // Set reference value                
                uint16_t altitudeValue = (uint16_t) userResponse.mIntValue;
                if(set_scd30_altitude_compensation(menuObject->mSCD30Sensor, altitudeValue)) {
                    printf("Value set successfully\n");
                } else {
                    printf("Failed to set value\n");
                }

                menuObject->mInputStateHolder.mState = INPUT_COMPLETED;
            } else {
                printf("\nInvalid altitude value (%d). Value must be within the range %d-%d\n", 
                    userResponse.mIntValue,
                    MIN_ALTITUDE_VALUE, 
                    MAX_ALTITUDE_VALUE
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

menu_return_behavior altitude_compensation_menu_handler(int input, void *menuObject) {
    AltitudeCompensationMenuObject* altitudeCompensationMenuObject = (AltitudeCompensationMenuObject*) menuObject;
    if(!altitudeCompensationMenuObject) {
        return EXIT_MENU;
    }

    switch(altitudeCompensationMenuObject->mInputStateHolder.mState) {
        case PROMPT_USER:
            printf("Enter the current altitude (m above sea level), hit Enter for none or 'q' to quit: ");
            reset_input_buffer(&altitudeCompensationMenuObject->mInputStateHolder.mArgumentBuffer);
            altitudeCompensationMenuObject->mInputStateHolder.mState = READING_INPUT;
            break;
        case READING_INPUT:
            return handle_altitude_compensation_reading_input(input, altitudeCompensationMenuObject);
        case INPUT_COMPLETED:
            return EXIT_MENU;
        default:
            break;
    }

    return DO_NOTHING;
}

void reset_altitude_compensation_menu_object(AltitudeCompensationMenuObject* menuObject) {
    menuObject->mInputStateHolder.mState = PROMPT_USER;
    reset_input_buffer(&(menuObject->mInputStateHolder.mArgumentBuffer));
}
