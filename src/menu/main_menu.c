#include "main_menu.h"

#include <stdio.h>
#include "pico/stdlib.h"

#include "continuous_measurement_menu.h"
#include "user_int_input_menu.h"

const char* MAIN_MENU_TEXT = 
    "\n"
    "          * Sensor Pod Tester Menu *\n"
    "\n\n"
    " (A) Toggle Pico LED on/off\n"
    " (B) SCD30 continuous measurement\n"
    " (C) Set SCD30 measurement interval\n"
    " (D) Activate SCD30 Automatic Self-Calibration (ASC)\n"
    " (E) Deactivate SCD30 Automatic Self-Calibration (ASC)\n"
    " (F) Set SCD30 Forced Recalibration value (FRC)\n"
    " (G) Set SCD30 Temperature Offset\n"
    " (H) SCD30 Altitude Compensation\n"
    " (I) Show SCD30 firmware version\n"
    " (J) Show SCD30 serial number\n"
    " (K) Perform SCD30 Soft Reset\n"
    " (L) Check moisture sensor\n"
    "\n\n"
    " Please select an option from above: ";


// Menu functions
void menu_do_start_scd30_continuous_readings(SensorPod *sensorPod);
void menu_do_set_scd30_measurement_interval(SensorPod *sensorPod);
void menu_activate_scd30_asc(SensorPod *sensorPod);
void menu_deactivate_scd30_asc(SensorPod *sensorPod);
void menu_do_set_forced_recalibration_value(SensorPod *sensorPod);
void menu_do_set_temperature_offset(SensorPod *sensorPod);
void menu_do_set_altitude_compensation(SensorPod *sensorPod);
void menu_do_get_scd30_firmware_version(SensorPod *sensorPod);
void menu_do_get_scd30_serial_number(SensorPod *sensorPod);
void menu_do_soft_reset_scd30(SensorPod *sensorPod);
void menu_do_get_soil_sensor_moisture_reading(SensorPod *sensorPod);


menu_return_behavior main_menu_handler(int input, void* menuObject) {
    if(input == PICO_ERROR_TIMEOUT) {
        return DO_NOTHING;
    }

    if(!menuObject) {
        return EXIT_MENU;
    }

    MainMenuObject* mainMenuObject = (MainMenuObject*) menuObject;

    char c = (char)(input & 0xFF);
    printf("%c\n", c);

    switch(c) {
        case 'A':
        case 'a':
            // Toggle LED on/off
            gpio_put(mainMenuObject->mLEDPin, !gpio_get(mainMenuObject->mLEDPin));
            printf("\n\n ** LED switched %s! **\n", gpio_get(mainMenuObject->mLEDPin) ? "ON" : "OFF");
            break;
        case 'B':
        case 'b':
            menu_do_start_scd30_continuous_readings(mainMenuObject->mSensorPod);
            break;
        case 'C':
        case 'c':
            menu_do_set_scd30_measurement_interval(mainMenuObject->mSensorPod);
            break;
        case 'D':
        case 'd':
            menu_activate_scd30_asc(mainMenuObject->mSensorPod);
            break;
        case 'E':
        case 'e':
            menu_deactivate_scd30_asc(mainMenuObject->mSensorPod);
            break;
        case 'F':
        case 'f':
            menu_do_set_forced_recalibration_value(mainMenuObject->mSensorPod);
            break;
        case 'G':
        case 'g':
            menu_do_set_temperature_offset(mainMenuObject->mSensorPod);
            break;
        case 'H':
        case 'h':
            menu_do_set_altitude_compensation(mainMenuObject->mSensorPod);
            break;
        case 'I':
        case 'i':
            menu_do_get_scd30_firmware_version(mainMenuObject->mSensorPod);
            break;
        case 'J':
        case 'j':
            menu_do_get_scd30_serial_number(mainMenuObject->mSensorPod);
            break;
        case 'K':
        case 'k':
            printf("\n\nPerforming SCD30 soft reset...\n");
            menu_do_soft_reset_scd30(mainMenuObject->mSensorPod);
            break;
        case 'L':
        case 'l':
            menu_do_get_soil_sensor_moisture_reading(mainMenuObject->mSensorPod);
            break;
        default:
            printf("\nInvalid Input!\n");
            break;
    }

    return REPRINT_MENU_TEXT;
}



        // INTERNAL MENU FUNCTIONS

void menu_do_start_scd30_continuous_readings(SensorPod *sensorPod) {
    // Switch to continuous measurement sub-menu
    ContinuousMeasurementMenuObject continuousMenuObject = {
        .mSensorPod = sensorPod
    };
    reset_continuous_measurement_menu_object(&continuousMenuObject);
    do_menu(CONTINUOUS_MEASUREMENT_TEXT, continuous_measurement_handler, &continuousMenuObject);
}

void menu_do_set_scd30_measurement_interval(SensorPod *sensorPod) {
    UserIntInputMenuObject menuObject = {
        .mMenuText      = "\nSetting measurement interval.\n",
        .mUserPrompt    = "Enter the measurement interval (in seconds), hit Enter for default (2) or 'q' to quit: ",
        .mMinValue      = 2,
        .mMaxValue      = 1800,
        .mDefaultValue  = 2
    };

    reset_user_int_input_menu_object(&menuObject);
    do_menu(menuObject.mMenuText, user_int_input_menu_handler, &menuObject);

    // Handle response
    if(menuObject.mEnteredValue == NO_USER_INPUT_VALUE) {
        return;
    }

    // Set interval value                
    uint16_t measurementInterval = (uint16_t) menuObject.mEnteredValue;
    printf("Setting interval to %d....", measurementInterval);
    if(set_scd30_measurement_interval(sensorPod, measurementInterval)) {
        printf("Success\n");
    } else {
        printf("Failed\n");
    }
}

void menu_activate_scd30_asc(SensorPod *sensorPod) {
    printf("\n\nActivating Automatic Self-Calibration (ASC)...");
    if(set_scd30_automatic_self_calibration(sensorPod, true)) {
        printf("Success\n");
    } else {
        printf("Failed\n");
    }
}

void menu_deactivate_scd30_asc(SensorPod *sensorPod) {
    printf("\n\nDeactivating Automatic Self-Calibration (ASC)...");
    if(set_scd30_automatic_self_calibration(sensorPod, false)) {
        printf("Success\n");
    } else {
        printf("Failed\n");
    }
}

void menu_do_set_forced_recalibration_value(SensorPod *sensorPod) {
    UserIntInputMenuObject menuObject = {
        .mMenuText      = "\nRecalibrating CO2 reference.\n",
        .mUserPrompt    = "Enter the reference CO2 concentration (in PPM), hit Enter for default (400) or 'q' to quit: ",
        .mMinValue      = 400,
        .mMaxValue      = 2000,
        .mDefaultValue  = 400,
    };

    reset_user_int_input_menu_object(&menuObject);
    do_menu(menuObject.mMenuText, user_int_input_menu_handler, &menuObject);

    // Handle response
    if(menuObject.mEnteredValue == NO_USER_INPUT_VALUE) {
        return;
    }

    // Set reference value                
    uint16_t referenceValue = (uint16_t) menuObject.mEnteredValue;
    printf("Setting reference value to %d....", referenceValue);
    if(set_scd30_forced_recalibration_value(sensorPod, referenceValue)) {
        printf("Success\n");
    } else {
        printf("Failed\n");
    }
}

void menu_do_set_temperature_offset(SensorPod *sensorPod) {
    UserIntInputMenuObject menuObject = {
        .mMenuText      = "\nSet temperature offset\n",
        .mUserPrompt    = "Enter the temperature offset (°C x 100), hit Enter for default (2400/24°C) or 'q' to quit: ",
        .mMinValue      = 0,
        .mMaxValue      = 65535,
        .mDefaultValue  = 2400,
    };

    reset_user_int_input_menu_object(&menuObject);
    do_menu(menuObject.mMenuText, user_int_input_menu_handler, &menuObject);

    // Handle response
    if(menuObject.mEnteredValue == NO_USER_INPUT_VALUE) {
        return;
    }

    // Set temperature offset                
    uint16_t temperatureValue = (uint16_t) menuObject.mEnteredValue;
    printf("Setting temperature offset to %d....", temperatureValue);
    if(set_scd30_temperature_offset(sensorPod, temperatureValue)) {
        printf("Success\n");
    } else {
        printf("Failed\n");
    }
}

void menu_do_set_altitude_compensation(SensorPod *sensorPod) {
    UserIntInputMenuObject menuObject = {
        .mMenuText      = "\nSet altitude compensation\n",
        .mUserPrompt    = "Enter the current altitude (m above sea level), hit Enter for none or 'q' to quit: ",
        .mMinValue      = 0,
        .mMaxValue      = 65535,
        .mDefaultValue  = 0,
    };

    reset_user_int_input_menu_object(&menuObject);
    do_menu(menuObject.mMenuText, user_int_input_menu_handler, &menuObject);

    // Handle response
    if(menuObject.mEnteredValue == NO_USER_INPUT_VALUE) {
        return;
    }

    // Set altitude
    uint16_t altitudeValue = (uint16_t) menuObject.mEnteredValue;
    printf("Setting altitude to %d....", altitudeValue);
    if(set_scd30_altitude_compensation(sensorPod, altitudeValue)) {
        printf("Success\n");
    } else {
        printf("Failed\n");
    }
}

void menu_do_get_scd30_firmware_version(SensorPod *sensorPod) {
    uint8_t scd30Firmware[2];

    printf("\n\nGetting SCD30 firmware version...\n");
    if(read_scd30_firmware_version(sensorPod, scd30Firmware)) {
        printf("  Firmware: 0x%02X - 0x%02X\n", scd30Firmware[0], scd30Firmware[1]);
    } else {
        printf("Could not obtain SCD30 firmware version\n");
    }
}

void menu_do_get_scd30_serial_number(SensorPod *sensorPod) {
    char scd30Serial[SCD30_SERIAL_BYTE_SIZE];

    printf("\n\nGetting SCD30 serial number...\n");
    if(read_scd30_serial(sensorPod, scd30Serial)) {
        printf("  Serial: %s\n", scd30Serial);
    } else {
        printf("Could not obtain SCD30 serial number\n");
    }
}

void menu_do_soft_reset_scd30(SensorPod *sensorPod) {
    do_scd30_soft_reset(sensorPod);
}

void menu_do_get_soil_sensor_moisture_reading(SensorPod *sensorPod) {
    printf("\n\nGetting moisture sensor value...\n");
    uint16_t capValue = get_soil_sensor_capacitive_value(sensorPod);
    printf("  Capacitive value: %u\n", capValue);
}

