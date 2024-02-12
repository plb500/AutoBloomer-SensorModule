#include "sensor_i2c_interface.h"

#include "util/debug_io.h"
#include "hardware/gpio.h"


const bool I2C_NOSTOP = false;


I2CInterface::I2CInterface(
    i2c_inst_t *i2c,
    int baud,
    int sdaPin,
    int sclPin
) : 
    mI2C(i2c),
    mBaud(baud),
    mSDA(sdaPin),
    mSCL(sclPin),
    mInterfaceResetTimeout(nil_time)
{}

void I2CInterface::initSensorBus() {
    i2c_init(mI2C, mBaud);
    gpio_set_function(mSDA, GPIO_FUNC_I2C);
    gpio_set_function(mSCL, GPIO_FUNC_I2C);

    gpio_pull_up(mSDA);
    gpio_pull_up(mSCL);

    mInterfaceResetTimeout = make_timeout_time_ms(I2C_WATCHDOG_TIMEOUT_MS);
}

void I2CInterface::shutdownSensorBus() {
    i2c_deinit(mI2C);
}

void I2CInterface::resetSensorBus() {
    shutdownSensorBus();
    initSensorBus();
}

I2CInterface::I2CResponse I2CInterface::checkI2CAddress(const uint8_t address) {
    absolute_time_t timeout = make_timeout_time_ms(DEFAULT_I2C_TIMEOUT_MS);

    int response = i2c_write_blocking_until(
        mI2C, 
        address,
        0,
        0,
        I2C_NOSTOP,
        timeout
    );

    switch(response) {
        case PICO_ERROR_GENERIC:
            return I2C_RESPONSE_ERROR;

        case PICO_ERROR_TIMEOUT:
            return I2C_RESPONSE_TIMEOUT;

        default:
            return I2C_RESPONSE_OK;
    }
}

void I2CInterface::resetInterfaceWatchdog() {
    mInterfaceResetTimeout = make_timeout_time_ms(I2C_WATCHDOG_TIMEOUT_MS);
}

void I2CInterface::checkInterfaceWatchdog() {
    if(absolute_time_diff_us(mInterfaceResetTimeout, get_absolute_time()) > 0) {
        DEBUG_PRINT("**** I2C interface timed out, resetting ****");
        resetSensorBus();
    }
}


I2CInterface::I2CResponse I2CInterface::writeI2CData(
    const uint8_t address, 
    const uint8_t *buffer, 
    size_t bufferLen 
) {
    absolute_time_t timeout = make_timeout_time_ms(DEFAULT_I2C_TIMEOUT_MS);

    // Write the data itself, if we have any
    if(buffer && bufferLen) {
        int response = i2c_write_blocking_until(mI2C, address, buffer, bufferLen, I2C_NOSTOP, timeout);

        switch(response) {
            case PICO_ERROR_GENERIC:
                return I2C_RESPONSE_ERROR;

            case PICO_ERROR_TIMEOUT:
                return I2C_RESPONSE_TIMEOUT;

            default:
                return (response == bufferLen) ? I2C_RESPONSE_OK : I2C_RESPONSE_INCOMPLETE;
        }
    }

    return I2C_RESPONSE_OK;
}

I2CInterface::I2CResponse I2CInterface::writePrefixedI2CData(
    const uint8_t address, 
    const uint8_t *prefixBuffer, 
    size_t prefixLen,
    const uint8_t *buffer, 
    size_t bufferLen 
) {
    // Write the prefix data (usually an address)
    if ((prefixLen != 0) && (prefixBuffer != NULL)) {
        // Again, since we don't want to relinquish the I2C bus we won't bother with the STOP
        absolute_time_t timeout = make_timeout_time_ms(DEFAULT_I2C_TIMEOUT_MS);
        int response = i2c_write_blocking_until(mI2C, address, prefixBuffer, prefixLen, I2C_NOSTOP, timeout);

        switch(response) {
            case PICO_ERROR_GENERIC:
                return I2C_RESPONSE_ERROR;

            case PICO_ERROR_TIMEOUT:
                return I2C_RESPONSE_TIMEOUT;

            default:
                if(response != prefixLen) return I2C_RESPONSE_INCOMPLETE;
        }
    }

    if(buffer) {
        return writeI2CData(address, buffer, bufferLen);
    }

    return I2C_RESPONSE_OK;
}

I2CInterface::I2CResponse I2CInterface::writeToI2CRegister(
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

    return writePrefixedI2CData(address, prefix, 2, buffer, bufferLen);
}

I2CInterface::I2CResponse I2CInterface::readFromI2C(
    const uint8_t address,
    uint8_t *buffer, 
    const uint8_t amountToRead
) {
    absolute_time_t timeout = make_timeout_time_ms(DEFAULT_I2C_TIMEOUT_MS);
    int response = i2c_read_blocking_until(mI2C, address, buffer, amountToRead, I2C_NOSTOP, timeout);

    switch(response) {
        case PICO_ERROR_GENERIC:
            return I2C_RESPONSE_ERROR;

        case PICO_ERROR_TIMEOUT:
            return I2C_RESPONSE_TIMEOUT;

        default:
            return I2C_RESPONSE_OK;
    }
}

I2CInterface::I2CResponse I2CInterface::readFromI2CRegister(
    const uint8_t address,
    const uint8_t regHigh, 
    const uint8_t regLow,
    uint8_t *buffer, 
    const uint8_t amountToRead, 
    const uint16_t readDelay
) {
    uint8_t pos = 0;

    // Write register/command data
    I2CResponse registerResponse = writeToI2CRegister(address, regHigh, regLow, 0, 0);
    if(registerResponse != I2C_RESPONSE_OK) {
        return registerResponse;
    }

    // Wait for response
    sleep_ms(readDelay);

    // Read response
    return readFromI2C(address, buffer, amountToRead);
}
