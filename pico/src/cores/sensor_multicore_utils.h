#ifndef SENSOR_MULTICORE_UTILS_H
#define SENSOR_MULTICORE_UTILS_H

#include "network/mqtt_utils.h"
#include "pico/util/queue.h"
#include "sensor/sensor_pod.h"


// Queue management
void intitialize_sensor_data_queue(queue_t *sensorDataQueue, int numMessages);
void intitialize_sensor_control_queue(queue_t *sensorControlQueue, int numMessages);

    // Outbound MQTT message management
// Push current sensor data into the queue for pending outbound MQTT messages (core1 -> core0)
void push_sensor_data_to_queue(queue_t *sensorDataQueue, SensorPodData *sensorPodData);
// Read current sensor data messages, convert into MQTT and publish (core0)
void pull_mqtt_data_from_queue(MQTTState *mqttState);

    // Inbound MQTT message management
// Takes an MQTT message received from an incoming publish (core0) and puts it on the sensor core (core1) queue
bool push_mqtt_control_data_to_queue(queue_t *sensorControlQueue, SensorControlMessage *controlMessage);



#endif
