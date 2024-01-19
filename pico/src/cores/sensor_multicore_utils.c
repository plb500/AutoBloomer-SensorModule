#include "sensor_multicore_utils.h"


const char * const AUTOBLOOMER_TOPIC_NAME           = "AutoBloomer";
const char * const SENSOR_STATUS_JSON_KEY           = "Status";
const char * const FEED_LEVEL_JSON_KEY              = "Feed Level";
const char * const CO2_LEVEL_JSON_KEY               = "CO2 Level";
const char * const TEMPERATURE_JSON_KEY             = "Temperature";
const char * const HUMIDITY_JSON_KEY                = "Humidity";
const char * const SOIL_MOISTURE_JSON_KEY           = "Soil Moisture";
const char * const BATTERY_LEVEL_JSON_KEY           = "Battery Level";

typedef struct {
    uint8_t                 mSensorID;
    SensorType              mSensorType;
    const char *            mSensorName;
    const char *            mSensorLocation;
    SensorData              mSensorData;
} SensorDataUpdate;

typedef struct {
    SensorDataUpdate        mSensorUpdates[NUM_SENSORS];
} SensorDataUpdateMessage;


void sensor_to_data_update(Sensor *sensor, SensorDataUpdate *dataUpdate) {
    if(!sensor || !dataUpdate) {
        return;
    }

    dataUpdate->mSensorID = sensor->mSensorDefinition.mSensorID;
    dataUpdate->mSensorType = sensor->mSensorDefinition.mSensorType;
    dataUpdate->mSensorName = sensor->mSensorDefinition.mSensorName;
    dataUpdate->mSensorLocation = sensor->mSensorDefinition.mSensorLocation;
    dataUpdate->mSensorData = sensor->mCurrentSensorData;
}

void data_update_to_mqtt_messages(SensorDataUpdate *dataUpdate, MQTTMessage *mqttMsg) {
    char statusString[16];

    // Sanity check
    if(!dataUpdate || !mqttMsg) {
        return;
    }

    const char *sensorName = dataUpdate->mSensorName;
    const char *sensorLocation = dataUpdate->mSensorLocation;

    snprintf(mqttMsg->mTopic, MQTT_MAX_TOPIC_LENGTH, "%s/%s/%s",
        AUTOBLOOMER_TOPIC_NAME,
        sensorLocation,
        sensorName
    );

    snprintf(statusString, 16, "\"%s\":%d", SENSOR_STATUS_JSON_KEY, dataUpdate->mSensorData.mSensorStatus);
    
    switch(dataUpdate->mSensorType) {
        case SONAR_SENSOR:
            snprintf(mqttMsg->mPayload, MQTT_MAX_PAYLOAD_LENGTH, "{%s, \"%s\":%d}",
                statusString,
                FEED_LEVEL_JSON_KEY, dataUpdate->mSensorData.mSensorReading.mSonarSensorDistance   
            );
            break;
        
        case SENSOR_POD:
            snprintf(mqttMsg->mPayload, MQTT_MAX_PAYLOAD_LENGTH, 
                "{%s, \"%s\":%.2f, \"%s\":%.2f, \"%s\":%.2f, \"%s\":%d}",
                statusString,
                CO2_LEVEL_JSON_KEY, dataUpdate->mSensorData.mSensorReading.mSensorPodData.mCO2Level,
                TEMPERATURE_JSON_KEY, dataUpdate->mSensorData.mSensorReading.mSensorPodData.mTemperature,
                HUMIDITY_JSON_KEY, dataUpdate->mSensorData.mSensorReading.mSensorPodData.mHumidity,
                SOIL_MOISTURE_JSON_KEY, dataUpdate->mSensorData.mSensorReading.mSensorPodData.mSoilSensorData
            );
            break;

        case BATTERY_SENSOR:
            snprintf(mqttMsg->mPayload, MQTT_MAX_PAYLOAD_LENGTH, "{%s, \"%s\":%.2f}",
                statusString,
                BATTERY_LEVEL_JSON_KEY, dataUpdate->mSensorData.mSensorReading.mBatteryVoltage
            );
            break;
    }
}

void sensor_data_to_update_message(Sensor *sensors, SensorDataUpdateMessage *updateMessage) {
    if(!sensors || !updateMessage) {
        return;
    }

    for(int i = 0; i < NUM_SENSORS; ++i) {
        sensor_to_data_update(&sensors[i], &updateMessage->mSensorUpdates[i]); 
    }
}

        // PUBLIC FUNCTIONS //

void intitialize_sensor_data_queue(queue_t *sensorDataQueue, int numMessages) {
    queue_init(sensorDataQueue, sizeof(SensorDataUpdateMessage), numMessages);
}

void push_sensor_data_to_queue(queue_t *sensorDataQueue, Sensor *sensors) {
    if(!sensors || !sensorDataQueue) {
        return;
    }

    SensorDataUpdateMessage newData;
    sensor_data_to_update_message(sensors, &newData);

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

    for(int i = 0; i < NUM_SENSORS; ++i) {
        data_update_to_mqtt_messages(
            &updateMsg.mSensorUpdates[i],
            &mqttMsg
        );

        publish_mqtt_message(mqttState, &mqttMsg);
    }
}
