#ifndef _SERIAL_CONTROLLER_H_
#define _SERIAL_CONTROLLER_H_

#include "pico/types.h"

#include "userdata/userdata.h"


#define COMMAND_BUFFER_SIZE             (128)


// COMMAND PREFIXES: 4 characters
//
// SSID - Sets the WiFi SSID
// PASS - Sets WiFi key
// LOCN - Sets sensor location
// NAME - Sets sensor name
// BRKR - Sets the broker address
typedef enum {
    CMD_SSID = 0x53534944,
    CMD_PASS = 0x50415353,
    CMD_LOCN = 0x4C4F434E,
    CMD_NAME = 0x4E414D45,
    CMD_BRKR = 0x42524B52
} SerialCommand;

typedef struct SerialCommandBuffer_t {
    char m_buffer[COMMAND_BUFFER_SIZE];
    uint8_t m_bufferLen;
} SerialCommandBuffer;

void init_serial_command_buffer(SerialCommandBuffer *buffer);
bool update_user_data(UserData *userData, SerialCommandBuffer *buffer);

#endif      // _SERIAL_CONTROLLER_H_
