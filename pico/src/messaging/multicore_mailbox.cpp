#include "multicore_mailbox.h"


using std::nullopt;


MulticoreMailbox::MulticoreMailbox() :
    mSensorUpdateQueue2{NUM_SENSOR_UPDATE_MESSAGES},
    mSensorControlQueue2{NUM_SENSOR_CONTROL_MESSAGES}
{}

void MulticoreMailbox::sendSensorDataToCore0(const vector<SensorGroup>& sensorGroups) {
    // Fill our temporary message and push it to the queue
    mSensorUpdateWriteScratch.fillFromSensors(sensorGroups);
    mSensorUpdateQueue2.addToQueue(mSensorUpdateWriteScratch);
}

bool MulticoreMailbox::latestSensorDataToJSON(const vector<SensorGroup>& sensorGroups, vector<MQTTMessage>& outgoingMessages) {
    bool gotMessage = mSensorUpdateQueue2.readFromQueue(mSensorUpdateReadScratch);
    if(gotMessage) {
        mSensorUpdateReadScratch.toMQTT(sensorGroups, outgoingMessages);
    }
    return gotMessage;
}

void MulticoreMailbox::sendSensorControlMessageToCore1(MQTTMessage& mqttMessage) {
    SensorControlMessage msg;
    if(msg.fillFromMQTT(mqttMessage)) {
        mSensorControlQueue2.addToQueue(msg);
    }
}

optional<SensorControlMessage> MulticoreMailbox::getWaitingSensorControlMessage() {
    SensorControlMessage controlMessage;

    if(mSensorControlQueue2.readFromQueue(controlMessage)) {
        return controlMessage;
    } else {
        return nullopt;
    }
}
