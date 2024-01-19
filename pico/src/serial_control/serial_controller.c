#include "serial_controller.h"

#include "pico/stdio.h"
#include <stdio.h>
#include <string.h>


bool process_serial_command(UserData *userData, SerialCommandBuffer *buffer);

void init_serial_command_buffer(SerialCommandBuffer *buffer) {
    if(!buffer) {
        return;
    }

    buffer->m_bufferLen = 0;
}

bool update_user_data(UserData *userData, SerialCommandBuffer *buffer) {
    bool userDataUpdated = false;

    if(!userData || !buffer) {
        return userDataUpdated;
    }

    bool readLoop = true;
    while(readLoop) {
        int readResponse = getchar_timeout_us(2000);        // Timeout = 2ms
        if(readResponse != PICO_ERROR_TIMEOUT) {
            if(readResponse == '\n') {
                // Completed 
                if(buffer->m_bufferLen) {
                    userDataUpdated = process_serial_command(userData, buffer);
                }
                buffer->m_bufferLen = 0;
            } else {
                buffer->m_buffer[buffer->m_bufferLen++] = (char) readResponse;
            }
        } else {
            readLoop = false;
        }
    }

    return userDataUpdated;
}

// COMMAND PREFIXES: 4 characters
// SSID - Sets the WiFi SSID
// PKEY - Sets WiFi key
// LOCN - Sets sensor location
// NAME - Sets sensor name

#define COMMAND_PREFIX_LEN              (4)
bool process_serial_command(UserData *userData, SerialCommandBuffer *buffer) {
    // Ensure there is at least something usable
    if(buffer->m_bufferLen <= COMMAND_PREFIX_LEN) {
        return false;
    }

    char *destToSet = 0;

    if(strncmp(buffer->m_buffer, "SSID", COMMAND_PREFIX_LEN) == 0) {
        printf("Setting SSID\n");
        memset(userData->m_ssid, 0, MAX_SSID_LENGTH + 1);
        destToSet = userData->m_ssid;
    } else if(strncmp(buffer->m_buffer, "PKEY", COMMAND_PREFIX_LEN) == 0) {
        printf("Setting private key\n");
        memset(userData->m_psk, 0, MAX_PSK_LENGTH + 1);
        destToSet = userData->m_psk;
    } else if(strncmp(buffer->m_buffer, "LOCN", COMMAND_PREFIX_LEN) == 0) {
        printf("Setting location\n");
        memset(userData->m_locationName, 0, MAX_LOCATION_LENGTH + 1);
        destToSet = userData->m_locationName;
    } else if(strncmp(buffer->m_buffer, "NAME", COMMAND_PREFIX_LEN) == 0) {
        printf("Setting name\n");
        memset(userData->m_sensorName, 0, MAX_SENSOR_NAME_LENGTH + 1);
        destToSet = userData->m_sensorName;
    } else if(strncmp(buffer->m_buffer, "MQTT", COMMAND_PREFIX_LEN) == 0) {
        printf("Setting MQTT broker\n");
        memset(userData->m_brokerAddress, 0, MAX_SENSOR_NAME_LENGTH + 1);
        destToSet = userData->m_brokerAddress;
    }

    if(destToSet) {
        memcpy(destToSet, (buffer->m_buffer + COMMAND_PREFIX_LEN), buffer->m_bufferLen - COMMAND_PREFIX_LEN);
    }

    return (destToSet != 0);
}

