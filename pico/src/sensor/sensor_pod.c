#include "sensor_pod.h"

#include "stemma_soil_sensor.h"
#include "scd30_sensor.h"

#define SENSOR_POD_TIMEOUT_MS                   (5000)


void initialize_soil_sensor_connection(SensorPod *sensorPod) {
    sensorPod->mSoilSensorActive = (init_soil_sensor(sensorPod->mInterface, sensorPod->mSoilSensorAddress) == I2C_RESPONSE_OK);
}

void initialize_scd30_connection(SensorPod *sensorPod) {
    char tmpSerial[SCD30_SERIAL_BYTE_SIZE];

    sensorPod->mSCD30SensorActive = (read_scd30_serial(sensorPod->mInterface, sensorPod->mSCD30Address, tmpSerial) == I2C_RESPONSE_OK);

    if(sensorPod->mSCD30SensorActive) {
        I2CResponse readResponse = trigger_scd30_continuous_measurement(sensorPod->mInterface, sensorPod->mSCD30Address, 0);
    }
}

bool initialize_sensor_pod(SensorPod *sensorPod) {
    if(!sensorPod) {
        return false;
    }

    initialize_soil_sensor_connection(sensorPod);
    initialize_scd30_connection(sensorPod);

    if(!sensorPod->mSoilSensorActive || !sensorPod->mSCD30SensorActive) {
        return false;
    }

    sensorPod->mCurrentData.mSCD30SensorDataValid = false;
    sensorPod->mCurrentData.mSoilSensorDataValid = false;

    return true;
}

bool reset_sensor_pod(SensorPod *sensorPod) {
    static const uint16_t BOOT_DELAY_MS = 50;
    if(!sensorPod) {
        return false;
    }

    I2CResponse resetSoilSensorResponse = reset_soil_sensor(sensorPod->mInterface, sensorPod->mSoilSensorAddress);
    I2CResponse resetSCDResponse = do_scd30_soft_reset(sensorPod->mInterface, sensorPod->mSCD30Address);

    sleep_ms(BOOT_DELAY_MS);

    bool initResponse = initialize_sensor_pod(sensorPod);

    return (
        (resetSoilSensorResponse == I2C_RESPONSE_OK) &&
        (resetSCDResponse == I2C_RESPONSE_OK) &&
        initResponse
    );
}


void update_sensor_pod(SensorPod *sensorPod) {
    if(!sensorPod) {
        return;
    }

    bool gotSoilReading = false;
    bool gotSCDReading = false;

    if(absolute_time_diff_us(sensorPod->mPodResetTimeout, get_absolute_time()) > 0) {
        reset_sensor_pod(sensorPod);
        sensorPod->mPodResetTimeout = make_timeout_time_ms(SENSOR_POD_TIMEOUT_MS);
        sensorPod->mCurrentData.mStatus = SENSOR_CONNECTED_MALFUNCTIONING;
        return;
    }

    // Read soil sensor
    sensorPod->mCurrentData.mSoilSensorDataValid = false;
    if(sensorPod->mSoilSensorActive) {
        uint16_t capValue = get_soil_sensor_capacitive_value(sensorPod->mInterface, sensorPod->mSoilSensorAddress);
        if(capValue != STEMMA_SOIL_SENSOR_INVALID_READING) {
            sensorPod->mCurrentData.mSoilSensorData = capValue;
            sensorPod->mCurrentData.mSoilSensorDataValid = true;

            gotSoilReading = true;
        }
    } else {
        initialize_soil_sensor_connection(sensorPod);
    }

    // Read SCD30 (if we have a valid reading)
    if(sensorPod->mSCD30SensorActive) {
        if(get_scd30_data_ready_status(sensorPod->mInterface, sensorPod->mSCD30Address)) {
            SCD30SensorData tmpData = get_scd30_reading(sensorPod->mInterface, sensorPod->mSCD30Address);

            if(tmpData.mValidReading) {
                sensorPod->mCurrentData.mCO2Level = tmpData.mCO2Reading;
                sensorPod->mCurrentData.mTemperature = tmpData.mTemperatureReading;
                sensorPod->mCurrentData.mHumidity = tmpData.mHumidityReading;
                sensorPod->mCurrentData.mSCD30SensorDataValid = true;

                gotSCDReading = true;
            } else {
                sensorPod->mCurrentData.mSCD30SensorDataValid = false;
            }
        }
    } else {
        initialize_scd30_connection(sensorPod);
    }

    // It's common for there to not be both readings available, so as long as we have at least one, we are
    // good to reset the watchdog timer
    if(gotSoilReading || gotSCDReading) {
        // Reset pod and interface timeouts
        sensorPod->mPodResetTimeout = make_timeout_time_ms(SENSOR_POD_TIMEOUT_MS);
        reset_interface_watchdog(sensorPod->mInterface);
        sensorPod->mCurrentData.mStatus = SENSOR_CONNECTED_VALID_DATA;
    }

    sensorPod->mCurrentData.mStatus = SENSOR_CONNECTED_VALID_DATA;
}

bool sensor_pod_has_valid_data(SensorPod *sensorPod) {
    return (sensorPod->mCurrentData.mSoilSensorDataValid || sensorPod->mCurrentData.mSCD30SensorDataValid);
}
