#ifndef _SENSOR_DATA_MESSAGE_H_
#define _SENSOR_DATA_MESSAGE_H_

#include "sensor_hardware.h"
#include "sensors/sensor_group.h"
#include <vector>

using std::vector;

// Raw sensor data holder for passing between cores
struct SensorDataMessage {
    SensorDataMessage();

    void fillFromSensors(const vector<SensorGroup>& sensorGroups);
    void toMQTT(const vector<SensorGroup>& sensorGroups, vector<MQTTMessage>& outboundMessages);
    
    uint8_t mData[TOTAL_RAW_DATA_SIZE];
};

#endif      // _SENSOR_DATA_MESSAGE_H_
