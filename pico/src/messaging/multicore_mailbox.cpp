#include "multicore_mailbox.h"


using std::nullopt;


MulticoreMailbox::MulticoreMailbox() {
    queue_init(&mSensorUpdateQueue, sizeof(SensorPodMessages::CurrentSensorDataMessage), NUM_SENSOR_UPDATE_MESSAGES);
    queue_init(&mSensorControlQueue, sizeof(SensorPodMessages::SensorControlMessage), NUM_SENSOR_CONTROL_MESSAGES);
}

void MulticoreMailbox::sendSensorDataToCore0(const vector<SensorGroup>& sensorGroups) {

    bool added = false;
    do {
        // We prefer new data to old data, so if the queue is full, pop off the oldest data to create space
        if(queue_is_full(&mSensorUpdateQueue)) {
            queue_remove_blocking(&mSensorUpdateQueue, &mSensorUpdateWriteScratch);
        }
        mSensorUpdateWriteScratch.fromSensors(sensorGroups);
        added = queue_try_add(&mSensorUpdateQueue, &mSensorUpdateWriteScratch);

    } while(!added);
}

bool MulticoreMailbox::latestSensorDataToJSON(const vector<SensorGroup>& sensorGroups, char* jsonBuffer, int jsonBufferSize) {
    bool msgRead = false;
    bool haveMessage = false;

    // Since updates are sent sequentially from core 0, we are really only interested in the latest update data and
    // can discard any old messages since they contain stale sensor data. So we will just spin through the queue until
    // there are no new messages
    do {
        msgRead = queue_try_remove(&mSensorUpdateQueue, &mSensorUpdateReadScratch);
        if(msgRead) {
            haveMessage = true;
        }
    } while(msgRead);

    if(!haveMessage) {
        return false;
    }

    mSensorUpdateReadScratch.toJSON(sensorGroups, jsonBuffer, jsonBufferSize);
    return true;
}

void MulticoreMailbox::sendSensorControlMessageToCore1(SensorPodMessages::MQTTMessage& mqttMessage) {
    if(auto controlMessage = SensorPodMessages::mqttToControlMessage(mqttMessage)) {
        queue_try_add(&mSensorControlQueue, &controlMessage);
    }
}

optional<SensorPodMessages::SensorControlMessage> MulticoreMailbox::getWaitingSensorControlMessage() {
    SensorPodMessages::SensorControlMessage controlMessage;

    if(queue_try_remove(&mSensorControlQueue, &controlMessage)) {
        return controlMessage;
    } else {
        return nullopt;
    }
}
