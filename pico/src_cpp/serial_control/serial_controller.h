#ifndef _SERIAL_CONTROLLER_H_
#define _SERIAL_CONTROLLER_H_

#include "pico/types.h"
#include "userdata/user_data.h"

// COMMAND PREFIXES: 4 characters
//
// SSID - Sets the WiFi SSID
// PASS - Sets WiFi key
// LOCN - Sets sensor location
// NAME - Sets sensor name
// BRKR - Sets the broker address
enum SerialCommand {
    CMD_SSID = 0x53534944,
    CMD_PASS = 0x50415353,
    CMD_LOCN = 0x4C4F434E,
    CMD_NAME = 0x4E414D45,
    CMD_BRKR = 0x42524B52
};


class SerialController {
    public:
        SerialController();
    
        bool updateUserData(UserData& userData);

    private:
        bool processSerialCommand(UserData& userData);

        static constexpr int COMMAND_BUFFER_SIZE = 128;

        char mBuffer[COMMAND_BUFFER_SIZE];
        uint8_t mBufferLength;
};

#endif      // _SERIAL_CONTROLLER_H_
