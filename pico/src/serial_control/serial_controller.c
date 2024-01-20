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

#define COMMAND_PREFIX_LEN              (4)
bool process_serial_command(UserData *userData, SerialCommandBuffer *buffer) {
    // Ensure there is at least something usable
    if(buffer->m_bufferLen <= COMMAND_PREFIX_LEN) {
        return false;
    }

    char *destToSet = 0;
    SerialCommand command = (SerialCommand)(
        (buffer->m_buffer[0] << 24) | 
        (buffer->m_buffer[1] << 16) | 
        (buffer->m_buffer[2] << 8) | 
        buffer->m_buffer[3]
    );

    switch(command) {
        case CMD_SSID:
            printf("Setting SSID\n");
            memset(userData->m_ssid, 0, MAX_SSID_LENGTH + 1);
            destToSet = userData->m_ssid;
            break;
        case CMD_PASS:
            printf("Setting private key\n");
            memset(userData->m_psk, 0, MAX_PSK_LENGTH + 1);
            destToSet = userData->m_psk;
            break;
        case CMD_LOCN:
            printf("Setting location\n");
            memset(userData->m_locationName, 0, MAX_LOCATION_LENGTH + 1);
            destToSet = userData->m_locationName;
            break;
        case CMD_NAME:
            printf("Setting name\n");
            memset(userData->m_sensorName, 0, MAX_SENSOR_NAME_LENGTH + 1);
            destToSet = userData->m_sensorName;
            break;
        case CMD_BRKR:
            printf("Setting MQTT broker\n");
            memset(userData->m_brokerAddress, 0, MAX_SENSOR_NAME_LENGTH + 1);
            destToSet = userData->m_brokerAddress;
            break;
        default:
            break;
    }

    if(destToSet) {
        memcpy(destToSet, (buffer->m_buffer + COMMAND_PREFIX_LEN), buffer->m_bufferLen - COMMAND_PREFIX_LEN);
    }

    return (destToSet != 0);
}
