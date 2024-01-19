#include "sensor_i2c_interface.h"
#include "hardware/gpio.h"


const bool I2C_NOSTOP = false;

void init_sensor_bus(I2CInterface *i2cInterface) {
    if(!i2cInterface) {
        return;
    }

    i2c_init(i2cInterface->mI2C, i2cInterface->mBaud);
    gpio_set_function(i2cInterface->mSDA, GPIO_FUNC_I2C);
    gpio_set_function(i2cInterface->mSCL, GPIO_FUNC_I2C);

    gpio_pull_up(i2cInterface->mSDA);
    gpio_pull_up(i2cInterface->mSCL);
}

bool select_i2c_channel(I2CInterface *i2cInterface, I2CChannel channel) {
    if(i2cInterface->mMultiplexerAddress < 0) {
        return false;
    }

    if(channel == NO_I2C_CHANNEL) {
        return false;
    }

    uint8_t data = (uint8_t) channel;

    int bytesWritten = i2c_write_blocking(i2cInterface->mI2C, i2cInterface->mMultiplexerAddress, &data, 1, I2C_NOSTOP);
    return (bytesWritten == 1);
}

bool check_i2c_address(I2CInterface *i2cInterface, const uint8_t address) {
    if (i2c_write_blocking(i2cInterface->mI2C, address, 0, 0, I2C_NOSTOP) == PICO_ERROR_GENERIC) {
        return false;
    }
    return true;
}

bool write_i2c_data(
    I2CInterface *i2cInterface, 
    const uint8_t address, 
    const uint8_t *buffer, 
    size_t bufferLen 
) {
    // Write the data itself, if we have any
    if(buffer && bufferLen) {
        return (i2c_write_blocking(i2cInterface->mI2C, address, buffer, bufferLen, I2C_NOSTOP) == bufferLen);
    }

    return true;
}

bool write_prefixed_i2c_data(
    I2CInterface *i2cInterface, 
    const uint8_t address, 
    const uint8_t *prefixBuffer, 
    size_t prefixLen,
    const uint8_t *buffer, 
    size_t bufferLen 
) {
    // Write the prefix data (usually an address)
    if ((prefixLen != 0) && (prefixBuffer != NULL)) {
        // Again, since we don't want to relinquish the I2C bus we won't bother with the STOP
        int written = i2c_write_blocking(i2cInterface->mI2C, address, prefixBuffer, prefixLen, I2C_NOSTOP); 
        if (written != prefixLen) {
            return false;
        }
    }

    if(buffer) {
        return write_i2c_data(i2cInterface, address, buffer, bufferLen);
    }

    return true;
}

bool write_to_i2c_register(
    I2CInterface *i2cInterface, 
    const uint8_t address, 
    const uint8_t regHigh, 
    const uint8_t regLow, 
    const uint8_t *buffer, 
    const uint8_t bufferLen
) {
    uint8_t prefix[] = {
        regHigh,
        regLow
    };

    return write_prefixed_i2c_data(i2cInterface, address, prefix, 2, buffer, bufferLen);
}

bool read_from_i2c(
    I2CInterface *i2cInterface,
    const uint8_t address,
    uint8_t *buffer, 
    const uint8_t amountToRead
) {
    uint16_t amountRead = i2c_read_blocking(i2cInterface->mI2C, address, buffer, amountToRead, I2C_NOSTOP);

    return (amountRead > 0);
}

bool read_from_i2c_register(
    I2CInterface *i2cInterface,
    const uint8_t address,
    const uint8_t regHigh, 
    const uint8_t regLow,
    uint8_t *buffer, 
    const uint8_t amountToRead, 
    const uint16_t readDelay
) {
    uint8_t pos = 0;

    // Write register/command data
    if(!write_to_i2c_register(i2cInterface, address, regHigh, regLow, 0, 0)) {
        return false;
    }

    // Wait for response
    sleep_ms(readDelay);

    // Read response
    return read_from_i2c(i2cInterface, address, buffer, amountToRead);
}
