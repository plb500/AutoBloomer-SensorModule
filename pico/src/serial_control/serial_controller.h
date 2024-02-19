#ifndef _SERIAL_CONTROLLER_H_
#define _SERIAL_CONTROLLER_H_

#include "pico/types.h"
#include "userdata/user_data.h"

// COMMAND PREFIXES: 4 characters
//
// SSID - Sets the WiFi SSID
// PASS - Sets WiFi key
// NAME - Sets hardware host name
// BRKR - Sets the broker address
// WIPE - Wipes all user data
// GRPN - Sets the name of a specific group
// GRPL - Sets the location of a specific group
enum SerialCommand {
    CMD_SSID = 0x53534944,
    CMD_PASS = 0x50415353,
    CMD_NAME = 0x4E414D45,
    CMD_BRKR = 0x42524B52,
    CMD_WIPE = 0x57495045,
    CMD_GRPN = 0x4752504E,
    CMD_GRPL = 0x4752504C
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
