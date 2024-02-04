#ifndef _MULTICORE_MAILBOX_H
#define _MULTICORE_MAILBOX_H

#include "pico/util/queue.h"

#include "sensor_pod_messages.h"
#include "sensor/sensor_pod.h"


typedef struct MulticoreMailbox_t {
    queue_t mSensorUpdateQueue;         // Queue used for sending sensor updates from core1 to core0
    queue_t mSensorControlQueue;        // Queue used for sending sensor control commands from core0 to core1
} MulticoreMailbox;


void init_multicore_mailbox(MulticoreMailbox *mailbox);

// core1 -> core0 functions
void send_sensor_data_to_core0(MulticoreMailbox *mailbox, SensorPodData *sensorPodData);
bool get_latest_sensor_data_message(MulticoreMailbox *mailbox, SensorDataUpdateMessage *dataUpdateMsg);

// core0 -> core1 functions
void send_sensor_control_message_to_core1(MulticoreMailbox *mailbox, MQTTMessage *mqttMessage);
bool get_waiting_sensor_control_message(MulticoreMailbox *mailbox, SensorControlMessage *controlMessage);


#endif      // _MULTICORE_MAILBOX_H
