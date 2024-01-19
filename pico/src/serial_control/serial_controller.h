#ifndef _SERIAL_CONTROLLER_H_
#define _SERIAL_CONTROLLER_H_

#include "pico/types.h"

#include "userdata/userdata.h"


#define COMMAND_BUFFER_SIZE             (128)

typedef struct SerialCommandBuffer_t {
    char m_buffer[COMMAND_BUFFER_SIZE];
    uint8_t m_bufferLen;
} SerialCommandBuffer;

void init_serial_command_buffer(SerialCommandBuffer *buffer);
bool update_user_data(UserData *userData, SerialCommandBuffer *buffer);

#endif      // _SERIAL_CONTROLLER_H_
