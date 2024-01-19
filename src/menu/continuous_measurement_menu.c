#include "continuous_measurement_menu.h"

#include "sensor_pod.h"

#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/time.h"

menu_return_behavior handle_reading_input(int input, ContinuousMeasurementMenuObject *menuObject);
menu_return_behavior handle_doing_reading(int input, ContinuousMeasurementMenuObject *menuObject);

const int MIN_MBAR_VALUE = 700;
const int MAX_MBAR_VALUE = 1400;

const char* CONTINUOUS_MEASUREMENT_TEXT = 
    "\n"
    "Performing continuous sensor measurement.\n";

uint32_t get_current_time_ms() {
    return to_ms_since_boot(get_absolute_time());
}

menu_return_behavior handle_reading_input(int input, ContinuousMeasurementMenuObject *menuObject) {
    UserInputInt userResponse = read_int_input(input, 0, &menuObject->mInputStateHolder.mArgumentBuffer);

    switch(userResponse.mInputState) {
        case INPUT_COMPLETE:
        if((userResponse.mIntValue == 0) || ((userResponse.mIntValue >= MIN_MBAR_VALUE) && (userResponse.mIntValue <= MAX_MBAR_VALUE))) {
            printf("\nAmbient pressure compensation value: %d\n", userResponse.mIntValue);
            
            // Start the sensor
            uint16_t pressureCompensation = (uint16_t) userResponse.mIntValue;
            trigger_scd30_continuous_measurement(menuObject->mSensorPod, pressureCompensation);

            menuObject->mInputStateHolder.mState = INPUT_COMPLETED;
        } else {
            printf("\nInvalid pressure value (%d). Value must be either 0, or within the range %d-%d\n", 
                userResponse.mIntValue,
                MIN_MBAR_VALUE, 
                MAX_MBAR_VALUE
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

menu_return_behavior handle_doing_reading(int input, ContinuousMeasurementMenuObject *menuObject) {
    if(input != PICO_ERROR_TIMEOUT) {
        // Stop the sensor and exit
        stop_scd30_continuous_measurement(menuObject->mSensorPod);
        return EXIT_MENU;
    }

    if(get_scd30_data_ready_status(menuObject->mSensorPod)) {
        SCD30SensorData data = get_scd30_reading(menuObject->mSensorPod);
        if(data.mValidReading) {
            printf("Sensor reading:\n");
            printf("   CO2: %f\n", data.mCO2Reading);
            printf("   Temperature: %f\n", data.mTemperatureReading);
            printf("   RH: %f\n", data.mHumidityReading);
        } else {
            printf("Sensor reading ready but invalid\n");
        }
    }

    return DO_NOTHING;
}

menu_return_behavior continuous_measurement_handler(int input, void *menuObject) {
    ContinuousMeasurementMenuObject* continuousMenuObject = (ContinuousMeasurementMenuObject*) menuObject;
    if(!continuousMenuObject) {
        return EXIT_MENU;
    }

    switch(continuousMenuObject->mInputStateHolder.mState) {
        case PROMPT_USER:
            printf("Enter the optional ambient pressure compensation value (in mBar), hit Enter for none or 'q' to quit: ");
            reset_input_buffer(&continuousMenuObject->mInputStateHolder.mArgumentBuffer);
            continuousMenuObject->mInputStateHolder.mState = READING_INPUT;
            break;
        case READING_INPUT:
            return handle_reading_input(input, continuousMenuObject);
        case INPUT_COMPLETED:
            return handle_doing_reading(input, continuousMenuObject);
        default:
            break;
    }

    return DO_NOTHING;
}

void reset_continuous_measurement_menu_object(ContinuousMeasurementMenuObject* menuObject) {
    menuObject->mInputStateHolder.mState = PROMPT_USER;
    reset_input_buffer(&(menuObject->mInputStateHolder.mArgumentBuffer));
}
