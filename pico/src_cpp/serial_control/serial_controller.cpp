#include "serial_controller.h"

#include "util/debug_io.h"

SerialController::SerialController() : 
    mBufferLength(0)
{}

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
    if(mBufferLength <= COMMAND_PREFIX_LEN) {
        return false;
    }

    SerialCommand command = (SerialCommand)(
        (mBuffer[0] << 24) | 
        (mBuffer[1] << 16) | 
        (mBuffer[2] << 8) | 
        mBuffer[3]
    );

    const char *commandParams = mBuffer + 4;
    bool userDataUpdated = false;

    switch(command) {
        case CMD_SSID:
            DEBUG_PRINT("Setting SSID (%s)", commandParams);
            userData.setSSID(commandParams);
            userDataUpdated = true;
            break;
        case CMD_PASS:
            DEBUG_PRINT("Setting private key (%s)", commandParams);
            userData.setPSK(commandParams);
            userDataUpdated = true;
            break;
        case CMD_LOCN:
            DEBUG_PRINT("Setting location (%s)", commandParams);
            userData.setLocation(commandParams);
            userDataUpdated = true;
            break;
        case CMD_NAME:
            DEBUG_PRINT("Setting sensor name (%s)", commandParams);
            userData.setSensorName(commandParams);
            userDataUpdated = true;
            break;
        case CMD_BRKR:
            DEBUG_PRINT("Setting MQTT broker (%s)", commandParams);
            userData.setBrokerAddress(commandParams);
            userDataUpdated = true;
            break;
        default:
            break;
    }

    return userDataUpdated;
}
