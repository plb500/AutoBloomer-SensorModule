#include "serial_controller.h"

#include "util/debug_io.h"
#include <cstring>

SerialController::SerialController() : 
    mBufferLength(0)
{
    memset(mBuffer, 0, SerialController::COMMAND_BUFFER_SIZE);
}

bool SerialController::updateUserData(UserData& userData) {
    bool userDataUpdated = false;

    bool readLoop = true;
    while(readLoop) {
        int readResponse = getchar_timeout_us(2000);        // Timeout = 2ms
        if(readResponse != PICO_ERROR_TIMEOUT) {
            if(readResponse == '\n') {
                // Completed 
                if(mBufferLength) {
                    userDataUpdated = processSerialCommand(userData);
                }
                mBufferLength = 0;
                memset(mBuffer, 0, SerialController::COMMAND_BUFFER_SIZE);
            } else {
                mBuffer[mBufferLength++] = (char) readResponse;
            }
        } else {
            readLoop = false;
        }
    }

    return userDataUpdated;
}

bool SerialController::processSerialCommand(UserData& userData) {
    static const int COMMAND_PREFIX_LEN = 4;

    // Ensure there is at least something usable
    if(mBufferLength < COMMAND_PREFIX_LEN) {
        return false;
    }

    SerialCommand command = (SerialCommand)(
        (mBuffer[0] << 24) | 
        (mBuffer[1] << 16) | 
        (mBuffer[2] << 8) | 
        mBuffer[3]
    );

    const char *commandParams = 0;
    int8_t groupIndex = -1;
    bool userDataUpdated = false;

    switch(command) {
        case CMD_SSID:
            commandParams = mBuffer + 4;
            DEBUG_PRINT("Setting SSID (%s)", commandParams);
            userData.setSSID(commandParams);
            userDataUpdated = true;
            break;
        case CMD_PASS:
            commandParams = mBuffer + 4;
            DEBUG_PRINT("Setting private key (%s)", commandParams);
            userData.setPSK(commandParams);
            userDataUpdated = true;
            break;
        case CMD_NAME:
            commandParams = mBuffer + 4;
            DEBUG_PRINT("Setting host name (%s)", commandParams);
            userData.setHostName(commandParams);
            userDataUpdated = true;
            break;
        case CMD_BRKR:
            commandParams = mBuffer + 4;
            DEBUG_PRINT("Setting MQTT broker (%s)", commandParams);
            userData.setBrokerAddress(commandParams);
            userDataUpdated = true;
            break;
        case CMD_WIPE:
            DEBUG_PRINT("Wiping data");
            userData.wipe();
            userDataUpdated = true;
            break;
        case CMD_GRPN:
            groupIndex = *(mBuffer + 4) - '0';
            commandParams = mBuffer + 4 + 1;
            DEBUG_PRINT("Setting group %d name (%s)", groupIndex, commandParams);
            userData.setSensorGroupName(groupIndex, commandParams);
            userDataUpdated = true;
            break;
        case CMD_GRPL:
            groupIndex = *(mBuffer + 4) - '0';
            commandParams = mBuffer + 4 + 1;
            DEBUG_PRINT("Setting group %d location (%s)", groupIndex, commandParams);
            userData.setSensorGroupLocation(groupIndex, commandParams);
            userDataUpdated = true;
            break;
        default:
            break;
    }

    return userDataUpdated;
}
