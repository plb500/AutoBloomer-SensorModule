#ifndef _SENSOR_I2C_INTERFACE_H_
#define _SENSOR_I2C_INTERFACE_H_

#include "hardware/i2c.h"

typedef struct {
    i2c_inst_t *mI2C;                           // The underlying I2C access struct
    int mBaud;                                  // I2C baud rate
    int mSDA;                                   // I2C SDA pin
    int mSCL;                                   // I2C SCL pin
    absolute_time_t mInterfaceResetTimeout;     // Watchdog timer for multiplexer/interface
} I2CInterface;

typedef enum {
    I2C_RESPONSE_OK                 = 0,
    I2C_RESPONSE_ERROR              = 1,
    I2C_RESPONSE_TIMEOUT            = 2,
    I2C_RESPONSE_INVALID_REQUEST    = 3,
    I2C_RESPONSE_MALFORMED          = 4,
    I2C_RESPONSE_INCOMPLETE         = 5,
    I2C_RESPONSE_COMMAND_FAILED     = 6,
    I2C_RESPONSE_DEVICE_NOT_FOUND   = 7
}  I2CResponse;


// Main interface functions
void init_sensor_bus(I2CInterface *i2cInterface);
void shutdown_sensor_bus(I2CInterface *i2cInterface);
void reset_sensor_bus(I2CInterface *i2cInterface, bool fullReset);
I2CResponse check_i2c_address(I2CInterface *i2cInterface, const uint8_t address);
void reset_interface_watchdog(I2CInterface *i2cInterface);
void check_interface_watchdog(I2CInterface *i2cInterface);
I2CResponse write_i2c_data(
    I2CInterface *i2cInterface, 
    const uint8_t address, 
    const uint8_t *buffer, 
    size_t bufferLen 
);
I2CResponse write_prefixed_i2c_data(
    I2CInterface *i2cInterface, 
    const uint8_t address, 
    const uint8_t *prefixBuffer, 
    size_t prefixLen,
    const uint8_t *buffer, 
    size_t bufferLen 
);
I2CResponse write_to_i2c_register(
    I2CInterface *i2cInterface, 
    const uint8_t address, 
    const uint8_t regHigh, 
    const uint8_t regLow, 
    const uint8_t *buffer, 
    const uint8_t bufferLen
);
I2CResponse read_from_i2c(
    I2CInterface *i2cInterface,
    const uint8_t address,
    uint8_t *buffer, 
    const uint8_t amountToRead
);
I2CResponse read_from_i2c_register(
    I2CInterface *i2cInterface,
    const uint8_t address,
    const uint8_t regHigh, 
    const uint8_t regLow,
    uint8_t *buffer, 
    const uint8_t amountToRead, 
    const uint16_t readDelay
);


#endif
