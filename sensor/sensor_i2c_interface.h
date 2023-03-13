#ifndef _SENSOR_I2C_INTERFACE_H_
#define _SENSOR_I2C_INTERFACE_H_

#include "hardware/i2c.h"


typedef struct {
    i2c_inst_t *mI2C;               // The underlying I2C access struct
    int mBaud;                      // I2C baud rate
    int mSDA;                       // I2C SDA pin
    int mSCL;                       // I2C SCL pin
    int8_t mMultiplexerAddress;     // Address of I2C multiplexer (-1 if not connected via multiplexer)
} I2CInterface;

// I2C multiplexer channel definitions
typedef enum {
    I2C_CHANNEL_0 = (1 << 0),
    I2C_CHANNEL_1 = (1 << 1),
    I2C_CHANNEL_2 = (1 << 2),
    I2C_CHANNEL_3 = (1 << 3),
    I2C_CHANNEL_4 = (1 << 4),
    I2C_CHANNEL_5 = (1 << 5),
    I2C_CHANNEL_6 = (1 << 6),
    I2C_CHANNEL_7 = (1 << 7)
} I2CChannel;


void init_sensor_bus(I2CInterface *i2cInterface);
void select_i2c_channel(I2CInterface *i2cInterface, I2CChannel channel);
bool check_i2c_address(I2CInterface *i2cInterface, const uint8_t address);
bool write_i2c_data(
    I2CInterface *i2cInterface, 
    const uint8_t address, 
    const uint8_t *buffer, 
    size_t bufferLen 
);
bool write_prefixed_i2c_data(
    I2CInterface *i2cInterface, 
    const uint8_t address, 
    const uint8_t *prefixBuffer, 
    size_t prefixLen,
    const uint8_t *buffer, 
    size_t bufferLen 
);
bool write_to_i2c_register(
    I2CInterface *i2cInterface, 
    const uint8_t address, 
    const uint8_t regHigh, 
    const uint8_t regLow, 
    const uint8_t *buffer, 
    const uint8_t bufferLen
);
bool read_from_i2c(
    I2CInterface *i2cInterface,
    const uint8_t address,
    uint8_t *buffer, 
    const uint8_t amountToRead
);
bool read_from_i2c_register(
    I2CInterface *i2cInterface,
    const uint8_t address,
    const uint8_t regHigh, 
    const uint8_t regLow,
    uint8_t *buffer, 
    const uint8_t amountToRead, 
    const uint16_t readDelay
);


#endif
