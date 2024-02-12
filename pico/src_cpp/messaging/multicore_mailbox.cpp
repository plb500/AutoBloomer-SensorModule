#include "multicore_mailbox.h"


using std::nullopt;

MulticoreMailbox::MulticoreMailbox() {
    queue_init(&mSensorUpdateQueue, sizeof(SensorPodMessages::SensorDataUpdateMessage), NUM_SENSOR_UPDATE_MESSAGES);
    queue_init(&mSensorControlQueue, sizeof(SensorPodMessages::SensorControlMessage), NUM_SENSOR_CONTROL_MESSAGES);
}

void MulticoreMailbox::sendSensorDataToCore0(SensorPod::Data& sensorPodData) {
    SensorPodMessages::SensorDataUpdateMessage newData;
    newData.mHasSCD30Reading = sensorPodData.mSCD30SensorDataValid;
    newData.mHasSoilReding =  sensorPodData.mSoilSensorDataValid;
    newData.mSensorStatus = sensorPodData.mStatus;
    newData.mCO2Level = sensorPodData.mCO2Level;
    newData.mTemperature = sensorPodData.mTemperature;
    newData.mHumidity = sensorPodData.mHumidity;
    newData.mSoilSensorData = sensorPodData.mSoilSensorData;

    bool added = false;
    do {
        // We prefer new data to old data, so if the queue is full, pop off the oldest data to create space
        if(queue_is_full(&mSensorUpdateQueue)) {
            SensorPodMessages::SensorDataUpdateMessage tmp;
            queue_remove_blocking(&mSensorUpdateQueue, &tmp);
        }
        added = queue_try_add(&mSensorUpdateQueue, &newData);

    } while(!added);
}

optional<SensorPodMessages::SensorDataUpdateMessage> MulticoreMailbox::getLatestSensorDataMessage() {
    SensorPodMessages::SensorDataUpdateMessage dataUpdateMsg;

    bool msgRead = false;
    bool haveMessage = false;

    // Since updates are sent sequentially from core 0, we are really only interested in the latest update data and
    // can discard any old messages since they contain stale sensor data. So we will just spin through the queue until
    // there are no new messages
    do {
        msgRead = queue_try_remove(&mSensorUpdateQueue, &dataUpdateMsg);
        if(msgRead) {
            haveMessage = true;
        }
    } while(msgRead);

    return haveMessage ? std::optional(dataUpdateMsg) : nullopt;
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
