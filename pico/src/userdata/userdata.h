#ifndef _USERDATA_H_
#define _USERDATA_H_

#include "pico/types.h"


#define MAX_LOCATION_LENGTH             (32)
#define MAX_SENSOR_NAME_LENGTH          (32)
#define MAX_SSID_LENGTH                 (32)
#define MAX_PSK_LENGTH                  (64)
#define MAX_BROKER_LENGTH               (256)
#define VALID_DATA_KEY                  ("xXx This is valid data xXx")
#define VALID_DATA_KEY_LENGTH           (26)


// User-defined data
typedef struct UserData_t {
    char m_locationName[MAX_LOCATION_LENGTH + 1];
    char m_sensorName[MAX_SENSOR_NAME_LENGTH + 1];
    char m_ssid[MAX_SSID_LENGTH + 1];
    char m_psk[MAX_PSK_LENGTH + 1]; 
    char m_brokerAddress[MAX_BROKER_LENGTH + 1];
    char m_dataKey[VALID_DATA_KEY_LENGTH + 1];
} UserData;

#define USER_DATA_BLOCK_SIZE            (512)       // A nice page-sized block to hold a UserData struct


void init_userdata(UserData *userData);
bool is_userdata_valid(UserData *userData);
bool has_network_userdata(UserData *userData);
bool has_broker_details(UserData *userData);
void write_userdata_to_flash(UserData *userData);
bool read_userdata_from_flash(UserData *destUserData);

#endif      // _USERDATA_H_
