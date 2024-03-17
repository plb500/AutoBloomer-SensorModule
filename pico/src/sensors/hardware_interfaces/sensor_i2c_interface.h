#ifndef _SENSOR_I2C_INTERFACE_H_
#define _SENSOR_I2C_INTERFACE_H_

#include "hardware/i2c.h"

typedef enum {
    I2C_RESPONSE_OK                 = 0,
    I2C_RESPONSE_ERROR              = 1,
    I2C_RESPONSE_TIMEOUT            = 2,
    I2C_RESPONSE_INVALID_REQUEST    = 3,
    I2C_RESPONSE_MALFORMED          = 4,
    I2C_RESPONSE_INCOMPLETE         = 5,
    I2C_RESPONSE_COMMAND_FAILED     = 6,
    I2C_RESPONSE_DEVICE_NOT_FOUND   = 7
} I2CResponse;

#define DEFAULT_I2C_TIMEOUT_MS      (100)
#define I2C_WATCHDOG_TIMEOUT_MS     (5000)




#ifdef __cplusplus // only actually define the class if this is C++

class I2CInterface {
    public:

        I2CInterface(
            i2c_inst_t* i2c,
            int baud,
            int sdaPin,
            int sclPin,
            bool sendStopAfterTransactions
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


        i2c_inst_t *mI2C;                           // The underlying I2C access struct
        const int mBaud;                            // I2C baud rate
        const int mSDA;                             // I2C SDA pin
        const int mSCL;                             // I2C SCL pin
        const bool mSendStopAfterTransactions;      // If we relinquish the bus after transactions (for multi-master)
        absolute_time_t mInterfaceResetTimeout;     // Watchdog timer for multiplexer/interface
};

#else

// C struct equivalent
typedef struct {
    i2c_inst_t *mI2C;                           // The underlying I2C access struct
    const int mBaud;                            // I2C baud rate
    const int mSDA;                             // I2C SDA pin
    const int mSCL;                             // I2C SCL pin
    const bool mSendStopAfterTransactions;      // If we relinquish the bus after transactions (for multi-master)
    absolute_time_t mInterfaceResetTimeout;     // Watchdog timer for multiplexer/interface
} I2CInterface;

#endif      // __cplusplus


// I2CInterface C access functions
#ifdef __cplusplus
    #define EXPORT_C extern "C"
#else
    #define EXPORT_C
#endif

EXPORT_C void init_sensor_bus(I2CInterface* i2c);
EXPORT_C void shutdown_sensor_bus(I2CInterface* i2c);
EXPORT_C void reset_sensor_bus(I2CInterface* i2c);
EXPORT_C I2CResponse check_i2c_address(I2CInterface* i2c, const uint8_t address);
EXPORT_C void reset_interface_watchdog(I2CInterface* i2c);
EXPORT_C void check_interface_watchdog(I2CInterface* i2c);
EXPORT_C I2CResponse write_i2c_data(
    I2CInterface* i2c,
    const uint8_t address, 
    const uint8_t *buffer, 
    size_t bufferLen 
);
EXPORT_C I2CResponse write_prefixed_i2c_data(
    I2CInterface* i2c,
    const uint8_t address, 
    const uint8_t *prefixBuffer, 
    size_t prefixLen,
    const uint8_t *buffer, 
    size_t bufferLen 
);
EXPORT_C I2CResponse write_to_i2c_register(
    I2CInterface* i2c,
    const uint8_t address, 
    const uint8_t regHigh, 
    const uint8_t regLow, 
    const uint8_t *buffer, 
    const uint8_t bufferLen
);
EXPORT_C I2CResponse read_from_i2c(
    I2CInterface* i2c,
    const uint8_t address,
    uint8_t *buffer, 
    const uint8_t amountToRead
);
EXPORT_C I2CResponse read_from_i2c_register(
    I2CInterface* i2c,
    const uint8_t address,
    const uint8_t regHigh, 
    const uint8_t regLow,
    uint8_t *buffer, 
    const uint8_t amountToRead, 
    const uint16_t readDelay
);


#endif      // _SENSOR_I2C_INTERFACE_H_
