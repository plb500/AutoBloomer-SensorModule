#include "cores/sensor_multicore_utils.h"


        // PUBLIC FUNCTIONS //

void intitialize_sensor_data_queue(queue_t *sensorDataQueue, int numMessages) {
    queue_init(sensorDataQueue, sizeof(SensorDataUpdateMessage), numMessages);
}

void intitialize_sensor_control_queue(queue_t *sensorControlQueue, int numMessages) {
    queue_init(sensorControlQueue, sizeof(SensorControlMessage), numMessages);
}

void push_sensor_data_to_queue(queue_t *sensorDataQueue, SensorPodData *sensorPodData) {
    if(!sensorPodData || !sensorDataQueue) {
        return;
    }

    SensorDataUpdateMessage newData;
    newData.mHasSCD30Reading = sensorPodData->mSCD30SensorDataValid;
    newData.mHasSoilReding =  sensorPodData->mSoilSensorDataValid;
    newData.mSensorStatus = sensorPodData->mStatus;
    newData.mCO2Level = sensorPodData->mCO2Level;
    newData.mTemperature = sensorPodData->mTemperature;
    newData.mHumidity = sensorPodData->mHumidity;
    newData.mSoilSensorData = sensorPodData->mSoilSensorData;

    bool added = false;
    do {
        // We prefer new data to old data, so if the queue is full, pop off the oldest data to create space
        if(queue_is_full(sensorDataQueue)) {
            SensorDataUpdateMessage tmp;
            queue_remove_blocking(sensorDataQueue, &tmp);
        }
        added = queue_try_add(sensorDataQueue, &newData);

    } while(!added);
}

void pull_mqtt_data_from_queue(MQTTState *mqttState) {
    SensorDataUpdateMessage updateMsg;
    bool msgRead = false;
    bool haveMessage = false;
    MQTTMessage mqttMsg;

    // Since updates are sent sequentially from core 0, we are really only interested in the latest update data and
    // can discard any old messages since they contain stale sensor data. So we will just spin through the queue until
    // there are no new messages
    do {
        msgRead = queue_try_remove(mqttState->mSensorUpdateQueue, &updateMsg);
        if(msgRead) {
            haveMessage = true;
        }
    } while(msgRead);

    if(haveMessage) {
        data_update_to_mqtt_message(mqttState->mSensorName, mqttState->mSensorLocation, &updateMsg, &mqttMsg);
        publish_mqtt_message(mqttState, &mqttMsg);
    }
}

bool push_mqtt_control_data_to_queue(queue_t *sensorControlQueue, SensorControlMessage *controlMessage) {
    return queue_try_add(sensorControlQueue, controlMessage);
}
