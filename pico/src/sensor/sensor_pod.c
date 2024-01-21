#include "sensor_pod.h"

#include "stemma_soil_sensor.h"
#include "sensirion/scd30_i2c.h"
#include "sensirion/sensirion_i2c_hal.h"

#include <stdio.h>

#define SENSOR_POD_TIMEOUT_MS                   (5000)

typedef struct {
    bool mValidReading;
    float mCO2Reading;
    float mTemperatureReading;
    float mHumidityReading;
} SCD30SensorData;

extern i2c_inst_t *sensirion_i2c_inst;
extern int sensirion_i2c_baud;
extern int sensirion_sda_pin;
extern int sensirion_scl_pin;

void initialize_soil_sensor_connection(SensorPod *sensorPod) {
    if(!sensorPod || !sensorPod->mSoilSensor) {
        return;
    }

    sensorPod->mSoilSensorActive = (init_soil_sensor(sensorPod->mSoilSensor) == I2C_RESPONSE_OK);
}

void initialize_scd30_connection(SensorPod *sensorPod) {
    uint8_t major, minor;

    sensirion_i2c_inst = sensorPod->mSCD30Interface->mI2C;
    sensirion_i2c_baud = sensorPod->mSCD30Interface->mBaud;
    sensirion_sda_pin = sensorPod->mSCD30Interface->mSDA;
    sensirion_scl_pin = sensorPod->mSCD30Interface->mSCL;

    init_driver(SCD30_I2C_ADDR_61);
    sensirion_i2c_hal_init();

    sleep_ms(2);

    // It's possible the SCD30 might still be in continuous measurement mode, do a stop-reset anyway
    if(scd30_stop_periodic_measurement() != 0) {
        return;
    }

    sleep_ms(2);

    if(scd30_soft_reset() != 0) {
        return;
    }

    sleep_ms(2);

    sensorPod->mSCD30SensorActive = !scd30_read_firmware_version(&major, &minor);
    printf("Firmware: 0x%2X - 0x%2X\n");
}

void reset_soil_sensor_connection(SensorPod *sensorPod) {
    if(!sensorPod || !sensorPod->mSoilSensor) {
        return;
    }

    reset_soil_sensor(sensorPod->mSoilSensor);
}

void reset_scd30_connection(SensorPod *sensorPod) {
    if(!sensorPod || !sensorPod->mSCD30Interface) {
        return;
    }

    if(scd30_stop_periodic_measurement() != 0) {
        return;
    }
    if(scd30_soft_reset() != 0) {
        return;
    }

    sleep_ms(1);
}

bool initialize_sensor_pod(SensorPod *sensorPod) {
    if(!sensorPod) {
        return false;
    }

    bool soilSensorGood = true;
    bool scd30Good = true;

    // Initialize sensirion I2C
    if(sensorPod->mSCD30Interface) {
        initialize_scd30_connection(sensorPod);
        scd30Good = sensorPod->mSCD30SensorActive;
    }

    // Initialize soil sensor
    if(sensorPod->mSoilSensor) {
        initialize_soil_sensor_connection(sensorPod);
        soilSensorGood = sensorPod->mSoilSensorActive;
    }

    sensorPod->mCurrentData.mSCD30SensorDataValid = false;
    sensorPod->mCurrentData.mSoilSensorDataValid = false;
    sensorPod->mPodResetTimeout = make_timeout_time_ms(SENSOR_POD_TIMEOUT_MS);

    return (soilSensorGood && scd30Good);
}

bool reset_sensor_pod(SensorPod *sensorPod) {
    static const uint16_t BOOT_DELAY_MS = 50;
    if(!sensorPod) {
        return false;
    }

    reset_scd30_connection(sensorPod);
    reset_soil_sensor_connection(sensorPod);

    sleep_ms(BOOT_DELAY_MS);

    start_sensor_pod(sensorPod);

    return true;
}

void start_sensor_pod(SensorPod *sensorPod) {
    if(!sensorPod) {
        return;
    }

    if(sensorPod->mSCD30Interface && sensorPod->mSCD30SensorActive) {
        scd30_set_measurement_interval(2);
        scd30_start_periodic_measurement(0);
    }
}


void update_sensor_pod(SensorPod *sensorPod) {
    if(!sensorPod) {
        return;
    }

    bool soilSensorGood = true;
    bool scd30Good = true;

    if(absolute_time_diff_us(sensorPod->mPodResetTimeout, get_absolute_time()) > 0) {
        reset_sensor_pod(sensorPod);
        sensorPod->mPodResetTimeout = make_timeout_time_ms(SENSOR_POD_TIMEOUT_MS);
        sensorPod->mCurrentData.mStatus = SENSOR_CONNECTED_MALFUNCTIONING;
        return;
    }

    // Read soil sensor
    if(sensorPod->mSoilSensor && sensorPod->mSCD30SensorActive) {
        uint16_t capValue = get_soil_sensor_capacitive_value(sensorPod->mSoilSensor);
        if(capValue != STEMMA_SOIL_SENSOR_INVALID_READING) {
            sensorPod->mCurrentData.mSoilSensorData = capValue;
            sensorPod->mCurrentData.mSoilSensorDataValid = true;

        } else {
            // Got an invalid reading, might be something up with the port
            soilSensorGood = false;
            reset_soil_sensor_connection(sensorPod);
        }
    }

    // Read SCD30 (if we have a valid reading)
    sensorPod->mCurrentData.mSCD30SensorDataValid = false;
    if(sensorPod->mSCD30Interface && sensorPod->mSCD30SensorActive) {
        uint16_t dataReady;
        if(!scd30_get_data_ready(&dataReady)) {
            if(dataReady) {
                SCD30SensorData tmpData;
                int16_t readResponse = scd30_read_measurement_data(
                    &tmpData.mCO2Reading,
                    &tmpData.mTemperatureReading,
                    &tmpData.mHumidityReading
                );
                tmpData.mValidReading = !readResponse;

                if(tmpData.mValidReading) {
                    sensorPod->mCurrentData.mCO2Level = tmpData.mCO2Reading;
                    sensorPod->mCurrentData.mTemperature = tmpData.mTemperatureReading;
                    sensorPod->mCurrentData.mHumidity = tmpData.mHumidityReading;
                    sensorPod->mCurrentData.mSCD30SensorDataValid = true;
                }
            }
        } else {
            // The actual data ready command failed, might be something up with the port
            scd30Good = false;
            reset_scd30_connection(sensorPod);
        }
    }

    // It's common for there to not be both readings available, so as long as we have at least one, we are
    // good to reset the watchdog timer
    if(scd30Good || soilSensorGood) {
        // Reset pod and interface timeouts
        sensorPod->mPodResetTimeout = make_timeout_time_ms(SENSOR_POD_TIMEOUT_MS);
        sensorPod->mCurrentData.mStatus = SENSOR_CONNECTED_VALID_DATA;
    } else {
        sensorPod->mCurrentData.mStatus = SENSOR_CONNECTED_NO_DATA;
    }
}

bool sensor_pod_has_valid_data(SensorPod *sensorPod) {
    return (sensorPod->mCurrentData.mSoilSensorDataValid || sensorPod->mCurrentData.mSCD30SensorDataValid);
}
