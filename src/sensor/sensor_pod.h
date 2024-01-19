#ifndef _SENSOR_POD_H_
#define _SENSOR_POD_H_

#include "sensor_i2c_interface.h"


#define SCD30_I2C_ADDRESS                       (0x61)
#define STEMMA_SOIL_SENSOR_INVALID_READING      (65535)

extern const uint16_t SCD30_SERIAL_BYTE_SIZE;


typedef struct {
    I2CInterface *mInterface;
    I2CChannel mI2CChannel;
    uint8_t mSCD30Address;
    uint8_t mSoilSensorAddress;
} SensorPod;

typedef enum {
    SOIL_SENSOR_1_ADDRESS = 0x36,
    SOIL_SENSOR_2_ADDRESS = 0x37,
    SOIL_SENSOR_3_ADDRESS = 0x38,
    SOIL_SENSOR_4_ADDRESS = 0x39
} SoilSensorAddresses;

typedef struct {
    bool mValidReading;
    float mCO2Reading;
    float mTemperatureReading;
    float mHumidityReading;
} SCD30SensorData;


bool select_sensor_pod(SensorPod *sensorPod);

// Soil sensor functions
bool init_soil_sensor(SensorPod *sensorPod);
bool reset_soil_sensor(SensorPod *sensorPod);
uint32_t get_soil_sensor_version(SensorPod *sensorPod);
uint16_t get_soil_sensor_capacitive_value(SensorPod *sensorPod);

// SCD30 sensor functions
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
