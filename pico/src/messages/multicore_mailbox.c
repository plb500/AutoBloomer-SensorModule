#include "multicore_mailbox.h"


#define NUM_SENSOR_UPDATE_MESSAGES              (4)
#define NUM_SENSOR_CONTROL_MESSAGES             (4)


void init_multicore_mailbox(MulticoreMailbox *mailbox) {
    if(!mailbox) {
        return;
    }

    queue_init(&mailbox->mSensorUpdateQueue, sizeof(SensorDataUpdateMessage), NUM_SENSOR_UPDATE_MESSAGES);
    queue_init(&mailbox->mSensorControlQueue, sizeof(SensorControlMessage), NUM_SENSOR_CONTROL_MESSAGES);
}

void send_sensor_data_to_core0(MulticoreMailbox *mailbox, SensorPodData *sensorPodData) {
    if(!mailbox || !sensorPodData) {
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
        if(queue_is_full(&mailbox->mSensorUpdateQueue)) {
            SensorDataUpdateMessage tmp;
            queue_remove_blocking(&mailbox->mSensorUpdateQueue, &tmp);
        }
        added = queue_try_add(&mailbox->mSensorUpdateQueue, &newData);

    } while(!added);
}

bool get_latest_sensor_data_message(MulticoreMailbox *mailbox, SensorDataUpdateMessage *dataUpdateMsg) {
    if(!mailbox || !dataUpdateMsg) {
        return false;
    }

    bool msgRead = false;
    bool haveMessage = false;

    // Since updates are sent sequentially from core 0, we are really only interested in the latest update data and
    // can discard any old messages since they contain stale sensor data. So we will just spin through the queue until
    // there are no new messages
    do {
        msgRead = queue_try_remove(&mailbox->mSensorUpdateQueue, dataUpdateMsg);
        if(msgRead) {
            haveMessage = true;
        }
    } while(msgRead);

    return haveMessage;
}

void send_sensor_control_message_to_core1(MulticoreMailbox *mailbox, MQTTMessage *mqttMessage) {
    if(!mailbox || !mqttMessage) {
        return;
    }

    SensorControlMessage msg;
    if(mqtt_to_control_message(mqttMessage, &msg)) {
        queue_try_add(&mailbox->mSensorControlQueue, &msg);
    }
}

bool get_waiting_sensor_control_message(MulticoreMailbox *mailbox, SensorControlMessage *controlMessage) {
    if(!mailbox || !controlMessage) {
        return false;
    }

    return queue_try_remove(&mailbox->mSensorControlQueue, controlMessage);
}
