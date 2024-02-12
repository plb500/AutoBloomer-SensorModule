#ifndef _SENSOR_I2C_INTERFACE_H_
#define _SENSOR_I2C_INTERFACE_H_

#include "hardware/i2c.h"


class I2CInterface {
    public:
        enum I2CResponse {
            I2C_RESPONSE_OK                 = 0,
            I2C_RESPONSE_ERROR              = 1,
            I2C_RESPONSE_TIMEOUT            = 2,
            I2C_RESPONSE_INVALID_REQUEST    = 3,
            I2C_RESPONSE_MALFORMED          = 4,
            I2C_RESPONSE_INCOMPLETE         = 5,
            I2C_RESPONSE_COMMAND_FAILED     = 6,
            I2C_RESPONSE_DEVICE_NOT_FOUND   = 7
        };

        I2CInterface(
            i2c_inst_t &i2c,
            int baud,
            int sdaPin,
            int sclPin
        );

        void initSensorBus();
        void shutdownSensorBus();
        void resetSensorBus();
        I2CResponse checkI2CAddress(const uint8_t address);
        void resetInterfaceWatchdog();
        void checkInterfaceWatchdog();

        I2CResponse writeI2CData(
            const uint8_t address, 
            const uint8_t *buffer, 
            size_t bufferLen 
        );
        I2CResponse writePrefixedI2CData(
            const uint8_t address, 
            const uint8_t *prefixBuffer, 
            size_t prefixLen,
            const uint8_t *buffer, 
            size_t bufferLen 
        );
        I2CResponse writeToI2CRegister(
            const uint8_t address, 
            const uint8_t regHigh, 
            const uint8_t regLow, 
            const uint8_t *buffer, 
            const uint8_t bufferLen
        );
        I2CResponse readFromI2C(
            const uint8_t address,
            uint8_t *buffer, 
            const uint8_t amountToRead
        );
        I2CResponse readFromI2CRegister(
            const uint8_t address,
            const uint8_t regHigh, 
            const uint8_t regLow,
            uint8_t *buffer, 
            const uint8_t amountToRead, 
            const uint16_t readDelay
        );

        constexpr static int DEFAULT_I2C_TIMEOUT_MS        = 100;
        constexpr static int I2C_WATCHDOG_TIMEOUT_MS       = 5000;


        i2c_inst_t &mI2C;                           // The underlying I2C access struct
        int mBaud;                                  // I2C baud rate
        int mSDA;                                   // I2C SDA pin
        int mSCL;                                   // I2C SCL pin
        absolute_time_t mInterfaceResetTimeout;     // Watchdog timer for multiplexer/interface
};


#endif      // _SENSOR_I2C_INTERFACE_H_
