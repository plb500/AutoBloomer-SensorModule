#ifndef _SCD30_SENSOR_H_
#define _SCD30_SENSOR_H_

#include "sensor_pod.h"


extern const uint16_t SCD30_SERIAL_BYTE_SIZE;
#define SCD30_I2C_ADDRESS       (0x61)


typedef struct {
    bool mValidReading;
    float mCO2Reading;
    float mTemperatureReading;
    float mHumidityReading;
} SCD30SensorData;


bool trigger_scd30_continuous_measurement(SensorPod *sensorPod, uint16_t pressureCompensation);
bool stop_scd30_continuous_measurement(SensorPod *sensorPod);
bool set_scd30_measurement_interval(SensorPod *sensorPod, uint16_t measurementInterval);
bool get_scd30_data_ready_status(SensorPod *sensorPod);
SCD30SensorData get_scd30_reading(SensorPod *sensorPod);
bool set_scd30_automatic_self_calibration(SensorPod *sensorPod, bool selfCalibrationOn);
bool set_scd30_forced_recalibration_value(SensorPod *sensorPod, uint16_t referenceValue);
bool set_scd30_temperature_offset(SensorPod *sensorPod, uint16_t temperatureOffset);
bool set_scd30_altitude_compensation(SensorPod *sensorPod, uint16_t altitude);
bool read_scd30_firmware_version(SensorPod *sensorPod, uint8_t *dst);
bool do_scd30_soft_reset(SensorPod *sensorPod);
bool read_scd30_serial(SensorPod *sensorPod, char *dst);

#endif
