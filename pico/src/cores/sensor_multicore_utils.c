#include "cores/sensor_multicore_utils.h"

const char * const AUTOBLOOMER_TOPIC_NAME           = "AutoBloomer";
const char * const SENSOR_STATUS_JSON_KEY           = "Status";
const char * const FEED_LEVEL_JSON_KEY              = "Feed Level";
const char * const CO2_LEVEL_JSON_KEY               = "CO2 Level";
const char * const TEMPERATURE_JSON_KEY             = "Temperature";
const char * const HUMIDITY_JSON_KEY                = "Humidity";
const char * const SOIL_MOISTURE_JSON_KEY           = "Soil Moisture";

typedef struct {
    int mSensorStatus;
    float mCO2Level;
    float mTemperature;
    float mHumidity;
    uint16_t mSoilSensorData;
} SensorDataUpdateMessage;


void data_update_to_mqtt_message(MQTTState *mqttState, SensorDataUpdateMessage *dataUpdate, MQTTMessage *mqttMsg) {
    // Sanity check
    if(!dataUpdate || !mqttMsg || !mqttState) {
        return;
    }

    const char *sensorName = mqttState->mSensorName;
    const char *sensorLocation = mqttState->mSensorLocation;

    snprintf(mqttMsg->mTopic, MQTT_MAX_TOPIC_LENGTH, "%s/%s/%s",
        AUTOBLOOMER_TOPIC_NAME,
        sensorLocation,
        sensorName
    );

    snprintf(mqttMsg->mPayload, MQTT_MAX_PAYLOAD_LENGTH, 
        "{\"%s\":%d, \"%s\":%.2f, \"%s\":%.2f, \"%s\":%.2f, \"%s\":%d}",
        SENSOR_STATUS_JSON_KEY, dataUpdate->mSensorStatus,
        CO2_LEVEL_JSON_KEY, dataUpdate->mCO2Level,
        TEMPERATURE_JSON_KEY, dataUpdate->mTemperature,
        HUMIDITY_JSON_KEY, dataUpdate->mHumidity,
        SOIL_MOISTURE_JSON_KEY, dataUpdate->mSoilSensorData
    );
}

        // PUBLIC FUNCTIONS //

void intitialize_sensor_data_queue(queue_t *sensorDataQueue, int numMessages) {
    queue_init(sensorDataQueue, sizeof(SensorDataUpdateMessage), numMessages);
}

void push_sensor_data_to_queue(queue_t *sensorDataQueue, SensorPodData *sensorPodData) {
    if(!sensorPodData || !sensorDataQueue) {
        return;
    }

    SensorDataUpdateMessage newData;
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

    data_update_to_mqtt_message(mqttState, &updateMsg, &mqttMsg);
    publish_mqtt_message(mqttState, &mqttMsg);
}
