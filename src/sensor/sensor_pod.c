#include "sensor_pod.h"

bool select_sensor_pod(SensorPod *sensorPod) {
    if(!sensorPod) {
        return false;
    }

    return select_i2c_channel(sensorPod->mInterface, sensorPod->mI2CChannel);
}
