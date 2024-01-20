#ifndef SENSOR_MULTICORE_UTILS_H
#define SENSOR_MULTICORE_UTILS_H

#include "network/mqtt_utils.h"
#include "pico/util/queue.h"
#include "sensor/sensor_pod.h"


// Queue management
void intitialize_sensor_data_queue(queue_t *sensorDataQueue, int numMessages);
void push_sensor_data_to_queue(queue_t *sensorDataQueue, SensorPodData *sensorPodData);
void pull_mqtt_data_from_queue(MQTTState *mqttState);

#endif
