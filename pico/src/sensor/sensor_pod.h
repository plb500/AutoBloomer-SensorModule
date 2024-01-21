#ifndef _SENSOR_POD_H_
#define _SENSOR_POD_H_

#include "sensor/stemma_soil_sensor.h"

#define SCD30_I2C_ADDRESS                       (0x61)

typedef enum {
    SENSOR_DISCONNECTED                 = 0,
    SENSOR_CONNECTED_MALFUNCTIONING     = 1,
    SENSOR_CONNECTED_NO_DATA            = 2,
    SENSOR_CONNECTED_VALID_DATA         = 3

} SensorPodStatus;

typedef struct {
    bool mSCD30SensorDataValid;
    bool mSoilSensorDataValid;
    SensorPodStatus mStatus;
    float mCO2Level;
    float mTemperature;
    float mHumidity;
    uint16_t mSoilSensorData;
} SensorPodData;

typedef struct {
    StemmaSoilSensor *mSoilSensor;
    I2CInterface *mSCD30Interface;

    bool mSoilSensorActive;
    bool mSCD30SensorActive;

    SensorPodData mCurrentData;
    absolute_time_t mPodResetTimeout;
} SensorPod;


bool initialize_sensor_pod(SensorPod *sensorPod);
bool reset_sensor_pod(SensorPod *sensorPod);
void start_sensor_pod(SensorPod *sensorPod);
void update_sensor_pod(SensorPod *sensorPod);
bool sensor_pod_has_valid_data(SensorPod *sensorPod);


#endif
