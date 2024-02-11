#ifndef _SENSOR_POD_MESSAGES_H_
#define _SENSOR_POD_MESSAGES_H_

#include "pico/types.h"
#include <stdio.h>
#include <string.h>


#define MQTT_MAX_TOPIC_LENGTH               (128)
#define MQTT_MAX_PAYLOAD_LENGTH             (256)
#define AUTOBLOOMER_TOPIC_NAME              ("AutoBloomer")


typedef enum {
    SCD30_SET_TEMP_OFFSET       = 0x504D4554,       // "TEMP"
    SCD30_SET_FRC               = 0x00435246        // "FRC"
} SensorControlCommandType;


// Incoming Sensor Control message
typedef struct SensorControlMessage_t {
    SensorControlCommandType mCommand;
    char mCommandParams[8];
} SensorControlMessage;

// Outgoing MQTT message
typedef struct MQTTMessage_t {
    char mTopic[MQTT_MAX_TOPIC_LENGTH];
    char mPayload[MQTT_MAX_PAYLOAD_LENGTH];
} MQTTMessage;

// 
typedef struct {
    int mSensorStatus;
    bool mHasSoilReding;
    bool mHasSCD30Reading;
    float mCO2Level;
    float mTemperature;
    float mHumidity;
    uint16_t mSoilSensorData;
} SensorDataUpdateMessage;


                // Message conversion utilities

// Converts incoming MQTT message object to SensorControlMessage                
bool mqtt_to_control_message(MQTTMessage *mqttMessage, SensorControlMessage *controlMessage);

// Converts SensorUpdateMessage to outgoing MQTT message
void data_update_to_mqtt_message(const char *sensorName, const char *sensorLocation, SensorDataUpdateMessage *dataUpdate, MQTTMessage *mqttMsg);

// Creates a basic test message with the supplied message as payload contents
void create_test_mqtt_message(const char *sensorName, const char *sensorLocation, const char *message, MQTTMessage *mqttMsg);

#endif      // _SENSOR_POD_MESSAGES_H_
