#include "main_menu.h"

#include <stdio.h>
#include "pico/stdlib.h"

#include "continuous_measurement_menu.h"
#include "measurement_interval_menu.h"
#include "forced_recalibration_menu.h"
#include "temperature_offset_menu.h"
#include "altitude_compensation_menu.h"

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
void menu_do_start_scd30_continuous_readings(SCD30Sensor *sensor);
void menu_do_set_scd30_measurement_interval(SCD30Sensor *sensor);
void menu_activate_scd30_asc(SCD30Sensor *sensor);
void menu_deactivate_scd30_asc(SCD30Sensor *sensor);
void menu_do_set_forced_recalibration_value(SCD30Sensor *sensor);
void menu_do_set_temperature_offset(SCD30Sensor *sensor);
void menu_do_set_altitude_compensation(SCD30Sensor *sensor);
void menu_do_get_scd30_firmware_version(SCD30Sensor *sensor);
void menu_do_get_scd30_serial_number(SCD30Sensor *sensor);
void menu_do_soft_reset_scd30(SCD30Sensor *sensor);

void menu_do_get_soil_sensor_moisture_reading(StemmaSoilSensor *sensor);


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
            menu_do_start_scd30_continuous_readings(mainMenuObject->mSCDSensor);
            break;
        case 'C':
        case 'c':
            menu_do_set_scd30_measurement_interval(mainMenuObject->mSCDSensor);
            break;
        case 'D':
        case 'd':
            menu_activate_scd30_asc(mainMenuObject->mSCDSensor);
            break;
        case 'E':
        case 'e':
            menu_deactivate_scd30_asc(mainMenuObject->mSCDSensor);
            break;
        case 'F':
        case 'f':
            menu_do_set_forced_recalibration_value(mainMenuObject->mSCDSensor);
            break;
        case 'G':
        case 'g':
            menu_do_set_temperature_offset(mainMenuObject->mSCDSensor);
            break;
        case 'H':
        case 'h':
            menu_do_set_altitude_compensation(mainMenuObject->mSCDSensor);
            break;
        case 'I':
        case 'i':
            menu_do_get_scd30_firmware_version(mainMenuObject->mSCDSensor);
            break;
        case 'J':
        case 'j':
            menu_do_get_scd30_serial_number(mainMenuObject->mSCDSensor);
            break;
        case 'K':
        case 'k':
            printf("\n\nPerforming SCD30 soft reset...\n");
            menu_do_soft_reset_scd30(mainMenuObject->mSCDSensor);
            break;
        case 'L':
        case 'l':
            menu_do_get_soil_sensor_moisture_reading(mainMenuObject->mStemmaSoilSensor);
            break;
        default:
            printf("\nInvalid Input!\n");
            break;
    }

    return REPRINT_MENU_TEXT;
}



        // INTERNAL MENU FUNCTIONS

void menu_do_start_scd30_continuous_readings(SCD30Sensor *sensor) {
    // Switch to continuous measurement sub-menu
    ContinuousMeasurementMenuObject continuousMenuObject = {
        .mSCD30Sensor = sensor
    };
    reset_continuous_measurement_menu_object(&continuousMenuObject);
    do_menu(CONTINUOUS_MEASUREMENT_TEXT, continuous_measurement_handler, &continuousMenuObject);
}

void menu_do_set_scd30_measurement_interval(SCD30Sensor *sensor) {
    // Switch to measurement interval sub-menu
    MeasurementIntervalMenuObject measurementIntervalMenuObject = {
        .mSCD30Sensor = sensor
    };
    reset_measurement_interval_menu_object(&measurementIntervalMenuObject);
    do_menu(MEASUREMENT_INTERVAL_MENU_TEXT, measurement_interval_menu_handler, &measurementIntervalMenuObject);
}

void menu_activate_scd30_asc(SCD30Sensor *sensor) {
    printf("\n\nActivating Automatic Self-Calibration (ASC)...");
    if(set_scd30_automatic_self_calibration(sensor, true)) {
        printf("Success\n");
    } else {
        printf("Failed\n");
    }
}

void menu_deactivate_scd30_asc(SCD30Sensor *sensor) {
    printf("\n\nDeactivating Automatic Self-Calibration (ASC)...");
    if(set_scd30_automatic_self_calibration(sensor, false)) {
        printf("Success\n");
    } else {
        printf("Failed\n");
    }
}

void menu_do_set_forced_recalibration_value(SCD30Sensor *sensor) {
    // Switch to forced recalibration sub-menu
    ForcedRecalibrationMenuObject forcedRecalibrationMenuObject = {
        .mSCD30Sensor = sensor
    };
    reset_forced_recalibration_menu_object(&forcedRecalibrationMenuObject);
    do_menu(FORCED_RECALIBRATION_MENU_TEXT, forced_recalibration_menu_handler, &forcedRecalibrationMenuObject);
}

void menu_do_set_temperature_offset(SCD30Sensor *sensor) {
    // Switch to temperature offset sub-menu
    TemperatureOffsetMenuObject temperatureOffsetMenuObject = {
        .mSCD30Sensor = sensor
    };
    reset_temperature_offset_menu_object(&temperatureOffsetMenuObject);
    do_menu(TEMPERATURE_OFFSET_MENU_TEXT, temperature_offset_menu_handler, &temperatureOffsetMenuObject);
}

void menu_do_set_altitude_compensation(SCD30Sensor *sensor) {
    // Switch to altitude compensation sub-menu
    AltitudeCompensationMenuObject altitudeCompensationtMenuObject = {
        .mSCD30Sensor = sensor
    };
    reset_altitude_compensation_menu_object(&altitudeCompensationtMenuObject);
    do_menu(ALTITUDE_COMPENSATION_MENU_TEXT, altitude_compensation_menu_handler, &altitudeCompensationtMenuObject);
}

void menu_do_get_scd30_firmware_version(SCD30Sensor *sensor) {
    uint8_t scd30Firmware[2];

    printf("\n\nGetting SCD30 firmware version...\n");
    if(read_scd30_firmware_version(sensor, scd30Firmware)) {
        printf("  Firmware: 0x%02X - 0x%02X\n", scd30Firmware[0], scd30Firmware[1]);
    } else {
        printf("Could not obtain SCD30 firmware version\n");
    }
}

void menu_do_get_scd30_serial_number(SCD30Sensor *sensor) {
    char scd30Serial[SCD30_SERIAL_BYTE_SIZE];

    printf("\n\nGetting SCD30 serial number...\n");
    if(read_scd30_serial(sensor, scd30Serial)) {
        printf("  Serial: %s\n", scd30Serial);
    } else {
        printf("Could not obtain SCD30 serial number\n");
    }
}

void menu_do_soft_reset_scd30(SCD30Sensor *sensor) {
    do_scd30_soft_reset(sensor);
}

void menu_do_get_soil_sensor_moisture_reading(StemmaSoilSensor *sensor) {
    printf("\n\nGetting moisture sensor value...\n");
    uint16_t capValue = get_soil_sensor_capacitive_value(sensor);
    printf("  Capacitive value: %u\n", capValue);
}

