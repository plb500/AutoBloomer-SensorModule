#include "sensor_pod.h"

#include "stemma_soil_sensor.h"
#include "sensirion/scd30_i2c.h"
#include "sensirion/sensirion_i2c_hal.h"

#include "util/debug_io.h"

#define SENSOR_POD_TIMEOUT_MS                   (10000)
#define SCD30_MEASUREMENT_INTERVAL_SECONDS      (2)

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


            // Init functions //

void initialize_soil_sensor_connection(SensorPod *sensorPod) {
    if(!sensorPod || !sensorPod->mSoilSensor) {
        return;
    }

    sensorPod->mSoilSensorActive = (init_soil_sensor(sensorPod->mSoilSensor) == I2C_RESPONSE_OK);
}

void initialize_scd30_connection(SensorPod *sensorPod) {
    uint8_t major, minor;

    if(!sensorPod || !sensorPod->mSCD30Interface) {
        return;
    }

    // I2C hardware init
    sensirion_i2c_inst = sensorPod->mSCD30Interface->mI2C;
    sensirion_i2c_baud = sensorPod->mSCD30Interface->mBaud;
    sensirion_sda_pin = sensorPod->mSCD30Interface->mSDA;
    sensirion_scl_pin = sensorPod->mSCD30Interface->mSCL;
    init_driver(SCD30_I2C_ADDR_61);
    sensirion_i2c_hal_init();

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
    sensorPod->mSCD30ResetTimeout = make_timeout_time_ms(SENSOR_POD_TIMEOUT_MS);
    DEBUG_PRINT("Firmware: 0x%2X - 0x%2X");
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

    return (soilSensorGood && scd30Good);
}

            // Shutdown functions //

void shutdown_soil_sensor_connection(SensorPod *sensorPod) {
    if(!sensorPod || !sensorPod->mSoilSensor) {
        return;
    }

    // Nothing to do yet
}

void shutdown_scd30_connection(SensorPod *sensorPod) {
    if(!sensorPod || !sensorPod->mSCD30Interface) {
        return;
    }


    scd30_stop_periodic_measurement();

    sleep_ms(1);

    sensirion_i2c_hal_free();
}
            // Reset functions //

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

    shutdown_scd30_connection(sensorPod);

    sleep_ms(1);

    initialize_scd30_connection(sensorPod);
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


            // Start functions //

void start_soil_sensor_readings(SensorPod *sensorPod) {
    if(!sensorPod) {
        return;
    }

    // Nothing to do here yet (or ever, maybe)
}

void start_scd30_readings(SensorPod *sensorPod) {
    if(!sensorPod) {
        return;
    }

    if(sensorPod->mSCD30Interface && sensorPod->mSCD30SensorActive) {
        scd30_set_measurement_interval(SCD30_MEASUREMENT_INTERVAL_SECONDS);
        scd30_start_periodic_measurement(0);
    }
}

void start_sensor_pod(SensorPod *sensorPod) {
    start_soil_sensor_readings(sensorPod);
    start_scd30_readings(sensorPod);
}


            // Update functions //

bool update_soil_sensor(SensorPod *sensorPod) {
    bool soilSensorGood = true;

    // Read soil sensor
    if(sensorPod->mSoilSensor && sensorPod->mSoilSensorActive) {
        uint16_t capValue = get_soil_sensor_capacitive_value(sensorPod->mSoilSensor);
        if(capValue != STEMMA_SOIL_SENSOR_INVALID_READING) {
            sensorPod->mCurrentData.mSoilSensorData = capValue;
            sensorPod->mCurrentData.mSoilSensorDataValid = true;

        } else {
            // Got an invalid reading, might be something up with the port
            soilSensorGood = false;
        }
    } else if(sensorPod->mSoilSensor) {
        // We have the interface defined but it's not active, this is not a good sign
        soilSensorGood = false;   
    }

    return soilSensorGood;
}

bool update_scd30_sensor(SensorPod *sensorPod) {
    bool scd30Good = true;

    // Check if we maybe need to bounce the port
    if(absolute_time_diff_us(sensorPod->mSCD30ResetTimeout, get_absolute_time()) > 0) {
        DEBUG_PRINT("SCD30 has not responded within the timeout period. Resetting.");
        reset_scd30_connection(sensorPod);
        start_scd30_readings(sensorPod);

        sensorPod->mSCD30ResetTimeout = make_timeout_time_ms(SENSOR_POD_TIMEOUT_MS);
        sensorPod->mCurrentData.mStatus = SENSOR_CONNECTED_MALFUNCTIONING;
        return false;
    }

    // Read SCD30 (if we have a valid reading ready)
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
            } else {
                // There was no data available. If this goes on too long it might indicate
                // an issue with the sensor
                scd30Good = false;
            }
        } else {
            // The actual data ready command failed, might be something up with the port
            scd30Good = false;
        }
    } else if(sensorPod->mSCD30Interface) {
        // We have the interface defined but it's not active, this is not a good sign
        scd30Good = false;
    }

    if(scd30Good) {
        sensorPod->mSCD30ResetTimeout = make_timeout_time_ms(SENSOR_POD_TIMEOUT_MS);
    }

    return scd30Good;
}

void update_sensor_pod(SensorPod *sensorPod) {
    if(!sensorPod) {
        return;
    }

    update_soil_sensor(sensorPod);
    update_scd30_sensor(sensorPod);
}


            // Management functions //

bool sensor_pod_has_valid_data(SensorPod *sensorPod) {
    return (sensorPod->mCurrentData.mSoilSensorDataValid || sensorPod->mCurrentData.mSCD30SensorDataValid);
}

void set_sensor_pod_temperature_offset(SensorPod *sensorPod, double offset) {
    // We can only do this on a valid sensor pod with a valid, active SCD30 interface
    if(!sensorPod || !sensorPod->mSCD30Interface || !sensorPod->mSCD30SensorActive) {
        return;
    }

    uint16_t offsetInt = (uint16_t) (offset * 100);

    // Pause readings while we set the offset (not sure if we need to do this but it seems like a good idea)
    scd30_stop_periodic_measurement();

    DEBUG_PRINT(" -- Setting temperature offset to: %d", offsetInt);
    scd30_set_temperature_offset(offsetInt);

    // Restart readings
    scd30_set_measurement_interval(SCD30_MEASUREMENT_INTERVAL_SECONDS);
    scd30_start_periodic_measurement(0);
}

void set_sensor_pod_forced_recalibration_value(SensorPod *sensorPod, uint16_t frc) {
    // We can only do this on a valid sensor pod with a valid, active SCD30 interface
    if(!sensorPod || !sensorPod->mSCD30Interface || !sensorPod->mSCD30SensorActive) {
        return;
    }

    // Pause readings while we set the offset (not sure if we need to do this but it seems like a good idea)
    scd30_stop_periodic_measurement();

    DEBUG_PRINT(" -- Setting FRC to: %d", frc);
    scd30_force_recalibration(frc);

    // Restart readings
    scd30_set_measurement_interval(SCD30_MEASUREMENT_INTERVAL_SECONDS);
    scd30_start_periodic_measurement(0);
}
