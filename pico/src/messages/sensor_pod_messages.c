#include "sensor_pod_messages.h"

const char * const SENSOR_STATUS_JSON_KEY           = "Status";
const char * const FEED_LEVEL_JSON_KEY              = "Feed Level";
const char * const CO2_LEVEL_JSON_KEY               = "CO2 Level";
const char * const TEMPERATURE_JSON_KEY             = "Temperature";
const char * const HUMIDITY_JSON_KEY                = "Humidity";
const char * const SOIL_MOISTURE_JSON_KEY           = "Soil Moisture";


bool mqtt_to_control_message(MQTTMessage *mqttMessage, SensorControlMessage *controlMessage) {
    char command[8];
    char value[8];

    memset(command, 0, 8);
    memset(value, 0, 8);

    if(!mqttMessage || !controlMessage) {
        return false;
    }

    // Parse command and value strings, bail if we can't even get that far
    sscanf(mqttMessage->mPayload, "%s %s", command, value);
    if(!strlen(command) || !strlen(value)) {
        // Malformed message
        return false;
    }

    // Make sure the command translates into something sensible
    uint32_t commandInt = *((uint32_t*) command);
    switch(commandInt) {
        case SCD30_SET_TEMP_OFFSET:
        case SCD30_SET_FRC:
            controlMessage->mCommand = commandInt;
            memcpy(controlMessage->mCommandParams, value, 8);
            return true;

        default:
            return false;
    }

    return false;
}


void data_update_to_mqtt_message(const char *sensorName, const char *sensorLocation, SensorDataUpdateMessage *dataUpdate, MQTTMessage *mqttMsg) {
    char soilReadingString[32];
    char scd30ReadingString[64];

    // Sanity check
    if(!sensorName || !sensorLocation || !dataUpdate || !mqttMsg) {
        return;
    }

    snprintf(mqttMsg->mTopic, MQTT_MAX_TOPIC_LENGTH, "%s/%s/%s",
        AUTOBLOOMER_TOPIC_NAME,
        sensorLocation,
        sensorName
    );

    if(dataUpdate->mHasSoilReding) {
        snprintf(soilReadingString, 32,
            ", \"%s\":%d",
            SOIL_MOISTURE_JSON_KEY, dataUpdate->mSoilSensorData
        );
    } else {
        soilReadingString[0] = 0;
    }

    if(dataUpdate->mHasSCD30Reading) {
        snprintf(scd30ReadingString, 64,
            ", \"%s\":%.2f, \"%s\":%.2f, \"%s\":%.2f",
            CO2_LEVEL_JSON_KEY, dataUpdate->mCO2Level,
            TEMPERATURE_JSON_KEY, dataUpdate->mTemperature,
            HUMIDITY_JSON_KEY, dataUpdate->mHumidity
        );
    } else {
        scd30ReadingString[0] = 0;
    }


    snprintf(mqttMsg->mPayload, MQTT_MAX_PAYLOAD_LENGTH, 
        "{\"%s\":%d%s%s}",
        SENSOR_STATUS_JSON_KEY, dataUpdate->mSensorStatus,
        scd30ReadingString,
        soilReadingString
    );
}

void create_test_mqtt_message(const char *sensorName, const char *sensorLocation, const char *message, MQTTMessage *mqttMsg) {
    snprintf(mqttMsg->mTopic, MQTT_MAX_TOPIC_LENGTH, "%s/%s/%s",
        AUTOBLOOMER_TOPIC_NAME,
        sensorLocation,
        sensorName
    );

    int messageLength = strlen(message);
    memcpy(mqttMsg->mPayload, message, messageLength);
    mqttMsg->mPayload[messageLength] = 0;
}
