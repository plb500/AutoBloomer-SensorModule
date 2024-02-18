#ifndef _SENSOR_DATA_MESSAGE_H_
#define _SENSOR_DATA_MESSAGE_H_

#include "sensors/sensor_group.h"
#include <vector>

using std::vector;

// Raw sensor data for passing between cores
struct SensorDataMessage {
    SensorDataMessage();

    void fillFromSensors(const vector<SensorGroup>& sensorGroups);
    void toJSON(const vector<SensorGroup>& sensorGroups, char* jsonBuffer, int jsonBufferSize);
    
    uint8_t* mData;
};

#endif      // _SENSOR_DATA_MESSAGE_H_
