#ifndef _SCD30_SENSOR_H_
#define _SCD30_SENSOR_H_

#include "sensor_i2c_interface.h"


extern const uint16_t SCD30_SERIAL_BYTE_SIZE;


typedef struct {
    I2CInterface *mInterface;
    uint8_t mAddress;
} SCD30Sensor;

typedef struct {
    bool mValidReading;
    float mCO2Reading;
    float mTemperatureReading;
    float mHumidityReading;
} SCD30SensorData;


bool trigger_scd30_continuous_measurement(SCD30Sensor *sensor, uint16_t pressureCompensation);
bool stop_scd30_continuous_measurement(SCD30Sensor *sensor);
bool set_scd30_measurement_interval(SCD30Sensor *sensor, uint16_t measurementInterval);
bool get_scd30_data_ready_status(SCD30Sensor *sensor);
SCD30SensorData get_scd30_reading(SCD30Sensor *sensor);
bool set_scd30_automatic_self_calibration(SCD30Sensor *sensor, bool selfCalibrationOn);
bool set_scd30_forced_recalibration_value(SCD30Sensor *sensor, uint16_t referenceValue);
bool set_scd30_temperature_offset(SCD30Sensor *sensor, uint16_t temperatureOffset);
bool set_scd30_altitude_compensation(SCD30Sensor *sensor, uint16_t altitude);
bool read_scd30_firmware_version(SCD30Sensor *sensor, uint8_t *dst);
bool do_scd30_soft_reset(SCD30Sensor *sensor);
bool read_scd30_serial(SCD30Sensor *sensor, char *dst);

#endif
