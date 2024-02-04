#include "sensor_i2c_interface.h"
#include "hardware/gpio.h"

#include "util/debug_io.h"

#define DEFAULT_I2C_TIMEOUT_MS      (100)
#define I2C_WATCHDOG_TIMEOUT_MS     (5000)
const bool I2C_NOSTOP = false;


// Main interface functions
void init_sensor_bus(I2CInterface *i2cInterface) {
    if(!i2cInterface) {
        return;
    }

    i2c_init(i2cInterface->mI2C, i2cInterface->mBaud);
    gpio_set_function(i2cInterface->mSDA, GPIO_FUNC_I2C);
    gpio_set_function(i2cInterface->mSCL, GPIO_FUNC_I2C);

    gpio_pull_up(i2cInterface->mSDA);
    gpio_pull_up(i2cInterface->mSCL);

    i2cInterface->mInterfaceResetTimeout = make_timeout_time_ms(I2C_WATCHDOG_TIMEOUT_MS);
}

void shutdown_sensor_bus(I2CInterface *i2cInterface) {
    if(!i2cInterface) {
        return;
    }

    i2c_deinit(i2cInterface->mI2C);
}

void reset_sensor_bus(I2CInterface *i2cInterface, bool fullReset) {
    if(!i2cInterface) {
        return;
    }

    if(fullReset) {
        shutdown_sensor_bus(i2cInterface);
    }

    if(fullReset) {
        init_sensor_bus(i2cInterface);
    }
}

void reset_interface_watchdog(I2CInterface *i2cInterface) {
    if(i2cInterface) {
        i2cInterface->mInterfaceResetTimeout = make_timeout_time_ms(I2C_WATCHDOG_TIMEOUT_MS);
    }
}

void check_interface_watchdog(I2CInterface *i2cInterface) {
    if(i2cInterface && (absolute_time_diff_us(i2cInterface->mInterfaceResetTimeout, get_absolute_time()) > 0)) {
        DEBUG_PRINT("**** I2C interface timed out, resetting ****");
        reset_sensor_bus(i2cInterface, true);
    }
}


I2CResponse check_i2c_address(I2CInterface *i2cInterface, const uint8_t address) {
    absolute_time_t timeout = make_timeout_time_ms(DEFAULT_I2C_TIMEOUT_MS);

    int response = i2c_write_blocking_until(
        i2cInterface->mI2C, 
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

I2CResponse write_i2c_data(
    I2CInterface *i2cInterface, 
    const uint8_t address, 
    const uint8_t *buffer, 
    size_t bufferLen 
) {
    absolute_time_t timeout = make_timeout_time_ms(DEFAULT_I2C_TIMEOUT_MS);

    // Write the data itself, if we have any
    if(buffer && bufferLen) {
        int response = i2c_write_blocking_until(i2cInterface->mI2C, address, buffer, bufferLen, I2C_NOSTOP, timeout);

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

I2CResponse write_prefixed_i2c_data(
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
        absolute_time_t timeout = make_timeout_time_ms(DEFAULT_I2C_TIMEOUT_MS);
        int response = i2c_write_blocking_until(i2cInterface->mI2C, address, prefixBuffer, prefixLen, I2C_NOSTOP, timeout);

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
        return write_i2c_data(i2cInterface, address, buffer, bufferLen);
    }

    return I2C_RESPONSE_OK;
}

I2CResponse write_to_i2c_register(
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

I2CResponse read_from_i2c(
    I2CInterface *i2cInterface,
    const uint8_t address,
    uint8_t *buffer, 
    const uint8_t amountToRead
) {
    absolute_time_t timeout = make_timeout_time_ms(DEFAULT_I2C_TIMEOUT_MS);
    int response = i2c_read_blocking_until(i2cInterface->mI2C, address, buffer, amountToRead, I2C_NOSTOP, timeout);

    switch(response) {
        case PICO_ERROR_GENERIC:
            return I2C_RESPONSE_ERROR;

        case PICO_ERROR_TIMEOUT:
            return I2C_RESPONSE_TIMEOUT;

        default:
            return I2C_RESPONSE_OK;
    }
}

I2CResponse read_from_i2c_register(
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
    I2CResponse registerResponse = write_to_i2c_register(i2cInterface, address, regHigh, regLow, 0, 0);
    if(registerResponse != I2C_RESPONSE_OK) {
        return registerResponse;
    }

    // Wait for response
    sleep_ms(readDelay);

    // Read response
    return read_from_i2c(i2cInterface, address, buffer, amountToRead);
}
